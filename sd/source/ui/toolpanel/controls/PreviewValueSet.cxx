/*************************************************************************
 *
 *  $RCSfile: PreviewValueSet.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:46:39 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "PreviewValueSet.hxx"
#include <vcl/image.hxx>
#include "../TaskPaneTreeNode.hxx"

namespace sd { namespace toolpanel { namespace controls {


PreviewValueSet::PreviewValueSet (TreeNode* pParent)
    : ValueSet (pParent->GetWindow(), WB_TABSTOP),
      mpParent(pParent),
      mnPreviewWidth(10),
      mnBorderWidth(3),
      mnBorderHeight(3)
{
    SetStyle (
        GetStyle()
        & ~(WB_ITEMBORDER)// | WB_MENUSTYLEVALUESET)
        //        | WB_FLATVALUESET);
        );

    SetColCount(2);
    SetLineCount(1);
    SetExtraSpacing (2);
}




PreviewValueSet::~PreviewValueSet (void)
{
}




void PreviewValueSet::SetPreviewWidth (int nWidth)
{
    mnPreviewWidth = nWidth;
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



void PreviewValueSet::Rearrange (void)
{
    USHORT nOldColumnCount (GetColCount());
    USHORT nOldRowCount (GetLineCount());

    USHORT nNewColumnCount (CalculateColumnCount (
        GetOutputSizePixel().Width()));
    USHORT nNewRowCount (CalculateRowCount (nNewColumnCount));

    SetColCount (nNewColumnCount);
    SetLineCount (nNewRowCount);

    if (nOldColumnCount != nNewColumnCount
        || nOldRowCount != nNewRowCount)
        mpParent->RequestResize();
}




USHORT PreviewValueSet::CalculateColumnCount (int nWidth) const
{
    int nColumnCount = 0;
    if (nWidth > 0)
    {
        nColumnCount = nWidth / (mnPreviewWidth + 2*mnBorderWidth);
        if (nColumnCount < 1)
            nColumnCount = 1;
        else if (nColumnCount > 4)
            nColumnCount = 4;
    }
    return nColumnCount;
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

    return nRowCount;
}



sal_Int32 PreviewValueSet::GetPreferredWidth (sal_Int32 nHeight)
{
    int nPreferredWidth = (mnPreviewWidth + 2*mnBorderWidth) * 1;

    // Get height of each row.
    int nItemHeight = mnPreviewWidth*100/141;
    if (GetItemCount() > 0)
    {
        Image aImage = GetItemImage(GetItemId(0));
        Size aItemSize = CalcItemSizePixel (aImage.GetSizePixel());
        if (aItemSize.Height() > 0)
            nItemHeight = aItemSize.Height();
    }
    nItemHeight += 2*mnBorderHeight;

    // Calculate the row- and column count and from the later the preferred
    // width.
    int nRowCount = nHeight / nItemHeight;
    if (nRowCount > 0)
    {
        int nColumnCount = (GetItemCount()+nRowCount-1) / nRowCount;
        if (nColumnCount > 0)
            nPreferredWidth = (mnPreviewWidth + 2*mnBorderWidth)
                * nColumnCount;
    }

    return nPreferredWidth;
}




sal_Int32 PreviewValueSet::GetPreferredHeight (sal_Int32 nWidth)
{
    int nRowCount (CalculateRowCount(CalculateColumnCount(nWidth)));
    int nItemHeight (mnPreviewWidth*100/141);
    if (GetItemCount() > 0)
    {
        Image aImage (GetItemImage(GetItemId(0)));
        Size aItemSize (CalcItemSizePixel (aImage.GetSizePixel()));
        if (aItemSize.Height() > 0)
            nItemHeight = aItemSize.Height();
    }

    return nRowCount * (nItemHeight + 2*mnBorderHeight);
}





} } } // end of namespace ::sd::toolpanel::controls
