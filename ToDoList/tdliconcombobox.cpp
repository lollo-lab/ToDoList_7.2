// tdliconcombobox.cpp : implementation file
//

#include "stdafx.h"
#include "tdliconcombobox.h"
#include "tdcimagelist.h"
#include "tdcstruct.h"

#include "..\Shared\GraphicsMisc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTDLIconComboBox

CTDLIconComboBox::CTDLIconComboBox(const CTDCImageList& ilImages, BOOL bMultiSel, BOOL bFilter)
	:
	CEnCheckComboBox(bMultiSel, (bFilter ? IDS_TDC_NONE : 0), (bFilter ? IDS_TDC_ANY : 0)),
	m_ilImages(ilImages)
{
}

CTDLIconComboBox::~CTDLIconComboBox()
{
}

IMPLEMENT_DYNAMIC(CTDLIconComboBox, CEnCheckComboBox)

BEGIN_MESSAGE_MAP(CTDLIconComboBox, CEnCheckComboBox)
	//{{AFX_MSG_MAP(CTDLIconComboBox)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTDLIconComboBox message handlers

void CTDLIconComboBox::DrawItemText(CDC& dc, const CRect& rect, int nItem, UINT nItemState,
									DWORD dwItemData, const CString& sItem, BOOL bList, COLORREF crText)
{
	CRect rImage(rect);
	rImage.bottom = (rImage.top + m_ilImages.GetImageSize());

	GraphicsMisc::CentreRect(rImage, rect, FALSE, TRUE);

	const int nImageSize = GetItemHeight(-1);

	CStringArray aImages;
	int nNumImage = 0;
	
	if (bList)
	{
		if (m_bMultiSel)
		{
			// draw checkbox with no text
			DrawCheckBox(dc, rect, nItem, nItemState, dwItemData, FALSE);

			// update image rect
			rImage.left += nImageSize;
		}

		aImages.Add(sItem);
		nNumImage = 1;
	}
	else // static text portion
	{
		if (m_bMultiSel)
		{
			nNumImage = Misc::Split(m_sText, aImages, _T(""), HasItemNone());
		}
		else if (!sItem.IsEmpty())
		{
			aImages.Add(sItem);
			nNumImage = 1;
		}
	}

	if (nNumImage > 0)
	{
		for (int nImg = 0; nImg < nNumImage; nImg++)
		{
			CString sImage, sName;

			if (TDCCUSTOMATTRIBUTEDEFINITION::DecodeImageTag(aImages[nImg], sImage, sName))
			{
				// draw image
				if (m_ilImages.GetSafeHandle() && !sImage.IsEmpty())
				{
					CPoint pt = rImage.TopLeft();

					m_ilImages.Draw(&dc, sImage, pt, ILD_TRANSPARENT);
					rImage.left += (nImageSize + 2);
				}

				// draw optional text
				if (bList && !sName.IsEmpty())
					CEnCheckComboBox::DrawItemText(dc, rImage, nItem, nItemState, dwItemData, sName, bList, crText);
			}
			else if (aImages[nImg].IsEmpty())
			{
				if (bList)
				{
					// Draw none/any
					CEnCheckComboBox::DrawItemText(dc, rect, nItem, nItemState, dwItemData, sItem, bList, crText);
				}
				else
				{
					ASSERT(nImg == 0);
					ASSERT(HasItemNone());

					CEnCheckComboBox::DrawItemText(dc, rImage, nItem, nItemState, dwItemData, m_sNone, bList, crText);
					rImage.left += (dc.GetTextExtent(m_sNone).cx + 2);

					if (nNumImage > 1)
						rImage.left += (dc.GetTextExtent(Misc::GetListSeparator()).cx);
				}
			}
		}
	}
	else if (!bList)
	{
		CEnCheckComboBox::DrawItemText(dc, rect, nItem, nItemState, dwItemData, sItem, bList, crText);
	}
}

int CTDLIconComboBox::SelectImage(const CString& sImage)
{
	CString sPartial = TDCCUSTOMATTRIBUTEDEFINITION::EncodeImageTag(sImage, _T(""));

	return CComboBox::SelectString(-1, sPartial);
}

CString CTDLIconComboBox::GetSelectedImage() const
{
	CString sItem = GetItemText(GetCurSel());
	
	if (sItem.IsEmpty())
		GetWindowText(sItem);

	if (!sItem.IsEmpty())
	{
		CString sImage, sName;
		TDCCUSTOMATTRIBUTEDEFINITION::DecodeImageTag(sItem, sImage, sName);
		
		return sImage;
	}

	// else
	return sItem; // empty
}

int CTDLIconComboBox::GetChecked(CStringArray& aItems, CCB_CHECKSTATE nCheck) const
{
	CStringArray aTemp;
	int nNumItems = CEnCheckComboBox::GetChecked(aTemp, nCheck);

	aItems.RemoveAll();

	for (int nImg = 0; nImg < nNumItems; nImg++)
	{
		CString sImage, sName;
		TDCCUSTOMATTRIBUTEDEFINITION::DecodeImageTag(aTemp[nImg], sImage, sName);

		aItems.Add(sImage);
	}

	if (IsNoneChecked())
		aItems.Add(_T(""));

	return aItems.GetSize();
}

BOOL CTDLIconComboBox::SetChecked(const CStringArray& aItems)
{
	// clear existing checks first but don't update window
	int nCount = GetCount();
	
	for (int i = 0; i < nCount; i++)
		CCheckComboBox::SetCheck(i, CCBC_UNCHECKED, FALSE);

	// assume that all the correct items have already 
	// been added to the list
	int nItem = aItems.GetSize();

	while (nItem--)
	{
		CString sPartial = TDCCUSTOMATTRIBUTEDEFINITION::EncodeImageTag(aItems[nItem], _T(""));
		int nIndex = FindString(-1, sPartial);

		if (nIndex != CB_ERR)
			SetCheck(nIndex, CCBC_CHECKED);
		else
			return FALSE;
	}

	return TRUE;
}
