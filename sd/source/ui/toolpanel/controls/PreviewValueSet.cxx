/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PreviewValueSet.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 18:50:32 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
    USHORT nOldColumnCount (GetColCount());
    USHORT nOldRowCount (GetLineCount());

    USHORT nNewColumnCount (CalculateColumnCount (
        GetOutputSizePixel().Width()));
    USHORT nNewRowCount (CalculateRowCount (nNewColumnCount));

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




USHORT PreviewValueSet::CalculateColumnCount (int nWidth) const
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
    return (USHORT)nColumnCount;
}




USHORT PreviewValueSet::CalculateRowCount (USHORT nColumnCount) const
{
    int nRowCount = 0;
    int nItemCount = GetItemCount();
    if (nColumnCount > 0)
    {
        nRowCount = (nItemCount+nColumnCount-1) / nColumnCount;
        if (nRowCount < 1)
            nRowCount = 1;
    }

    return (USHORT)nRowCount;
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
