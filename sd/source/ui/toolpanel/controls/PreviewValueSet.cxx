/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "PreviewValueSet.hxx"
#include <vcl/image.hxx>
#include "taskpane/TaskPaneTreeNode.hxx"

namespace sd { namespace toolpanel { namespace controls {


PreviewValueSet::PreviewValueSet (TreeNode* pParent)
    : ValueSet (pParent->GetWindow(), WB_TABSTOP),
      mpParent(pParent),
      maPreviewSize(10,10),
      mnBorderWidth(3),
      mnBorderHeight(3),
      mnMaxColumnCount(-1)
{
    SetStyle (
        GetStyle()
        & ~(WB_ITEMBORDER)// | WB_MENUSTYLEVALUESET)
        //        | WB_FLATVALUESET);
        );

    SetColCount(2);
    //  SetLineCount(1);
    SetExtraSpacing (2);
}




PreviewValueSet::~PreviewValueSet (void)
{
}




void PreviewValueSet::SetPreviewSize (const Size& rSize)
{
    maPreviewSize = rSize;
}




void PreviewValueSet::SetRightMouseClickHandler (const Link& rLink)
{
    maRightMouseClickHandler = rLink;
}




void PreviewValueSet::MouseButtonDown (const MouseEvent& rEvent)
{
    if (rEvent.IsRight())
        maRightMouseClickHandler.Call(reinterpret_cast<void*>(
            &const_cast<MouseEvent&>(rEvent)));
    else
        ValueSet::MouseButtonDown (rEvent);

}




void PreviewValueSet::Paint (const Rectangle& rRect)
{
    SetBackground (GetSettings().GetStyleSettings().GetWindowColor());

    ValueSet::Paint (rRect);

    SetBackground (Wallpaper());
}




void PreviewValueSet::Resize (void)
{
    ValueSet::Resize ();

    Size aWindowSize (GetOutputSizePixel());
    if (aWindowSize.Width()>0 && aWindowSize.Height()>0)
    {
        Rearrange();
    }
}




void PreviewValueSet::Command (const CommandEvent& rEvent)
{
    switch (rEvent.GetCommand())
    {
        case COMMAND_CONTEXTMENU:
        {
            CommandEvent aNonConstEventCopy (rEvent);
            maContextMenuCallback.Call(&aNonConstEventCopy);
        }
        break;

        default:
            ValueSet::Command(rEvent);
            break;
    }
}




void PreviewValueSet::Rearrange (bool bForceRequestResize)
{
    sal_uInt16 nOldColumnCount (GetColCount());
    sal_uInt16 nOldRowCount (GetLineCount());

    sal_uInt16 nNewColumnCount (CalculateColumnCount (
        GetOutputSizePixel().Width()));
    sal_uInt16 nNewRowCount (CalculateRowCount (nNewColumnCount));

    SetColCount(nNewColumnCount);
    SetLineCount(nNewRowCount);

    if (bForceRequestResize
        || nOldColumnCount != nNewColumnCount
        || nOldRowCount != nNewRowCount)
        mpParent->RequestResize();
}




void PreviewValueSet::SetContextMenuCallback (const Link& rLink)
{
    maContextMenuCallback = rLink;
}




sal_uInt16 PreviewValueSet::CalculateColumnCount (int nWidth) const
{
    int nColumnCount = 0;
    if (nWidth > 0)
    {
        nColumnCount = nWidth / (maPreviewSize.Width() + 2*mnBorderWidth);
        if (nColumnCount < 1)
            nColumnCount = 1;
        else if (mnMaxColumnCount>0 && nColumnCount>mnMaxColumnCount)
            nColumnCount = mnMaxColumnCount;
    }
    return (sal_uInt16)nColumnCount;
}




sal_uInt16 PreviewValueSet::CalculateRowCount (sal_uInt16 nColumnCount) const
{
    int nRowCount = 0;
    int nItemCount = GetItemCount();
    if (nColumnCount > 0)
    {
        nRowCount = (nItemCount+nColumnCount-1) / nColumnCount;
        if (nRowCount < 1)
            nRowCount = 1;
    }

    return (sal_uInt16)nRowCount;
}




sal_Int32 PreviewValueSet::GetPreferredWidth (sal_Int32 nHeight)
{
    int nPreferredWidth (maPreviewSize.Width() + 2*mnBorderWidth);

    // Get height of each row.
    int nItemHeight (maPreviewSize.Height() + 2*mnBorderHeight);

    // Calculate the row- and column count and from the later the preferred
    // width.
    int nRowCount = nHeight / nItemHeight;
    if (nRowCount > 0)
    {
        int nColumnCount = (GetItemCount()+nRowCount-1) / nRowCount;
        if (nColumnCount > 0)
            nPreferredWidth = (maPreviewSize.Width() + 2*mnBorderWidth)
                * nColumnCount;
    }

    return nPreferredWidth;
}




sal_Int32 PreviewValueSet::GetPreferredHeight (sal_Int32 nWidth)
{
    int nRowCount (CalculateRowCount(CalculateColumnCount(nWidth)));
    int nItemHeight (maPreviewSize.Height());

    return nRowCount * (nItemHeight + 2*mnBorderHeight);
}




} } } // end of namespace ::sd::toolpanel::controls
