/*************************************************************************
 *
 *  $RCSfile: XMLTableShapeResizer.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: sab $ $Date: 2001-03-20 16:19:41 $
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

#ifndef _SC_XMLTABLESHAPERESIZER_HXX
#include "XMLTableShapeResizer.hxx"
#endif
#ifndef SC_UNONAMES_HXX
#include "unonames.hxx"
#endif
#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_XMLIMPRT_HXX
#include "xmlimprt.hxx"
#endif

#ifndef _COM_SUN_STAR_TABLE_XCOLUMNROWRANGE_HPP_
#include <com/sun/star/table/XColumnRowRange.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

using namespace ::com::sun::star;

ScMyShapeResizer::ScMyShapeResizer(ScXMLImport& rTempImport)
    : aShapes(),
    rImport(rTempImport)
{
}

ScMyShapeResizer::~ScMyShapeResizer()
{
}

void ScMyShapeResizer::AddShape(uno::Reference <drawing::XShape>& rShape,
    table::CellAddress& rStartAddress, table::CellAddress& rEndAddress,
    sal_Int32 nStartX, sal_Int32 nStartY, sal_Int32 nEndX, sal_Int32 nEndY)
{
    ScMyToResizeShape aShape;
    aShape.xShape = rShape;
    aShape.aEndCell = rEndAddress;
    aShape.aStartCell = rStartAddress;
    aShape.nStartY = nStartY;
    aShape.nStartX = nStartX;
    aShape.nEndY = nEndY;
    aShape.nEndX = nEndX;
    aShapes.push_back(aShape);
}

void ScMyShapeResizer::ResizeShapes(uno::Reference< sheet::XSpreadsheet > xSheet)
{
    if (aShapes.size())
    {
        rtl::OUString sRowHeight(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_CELLHGT));
        ScMyToResizeShapes::iterator aItr = aShapes.begin();
        uno::Reference<table::XColumnRowRange> xColumnRowRange (xSheet, uno::UNO_QUERY);
        if (xColumnRowRange.is())
        {
            uno::Reference<table::XTableRows> xTableRows = xColumnRowRange->getRows();
            if (xTableRows.is())
            {
                uno::Reference<table::XCellRange> xTableRow;
                sal_Int32 nOldRow(-1);
                Rectangle* pRect = NULL;
                while (aItr != aShapes.end())
                {
                    if (nOldRow != aItr->aEndCell.Row || !xTableRow.is())
                    {
                        nOldRow = aItr->aEndCell.Row;
                        uno::Any aRow = xTableRows->getByIndex(aItr->aEndCell.Row);
                        aRow >>= xTableRow;
                    }
                    if (xTableRow.is())
                    {
                        uno::Reference <beans::XPropertySet> xRowProperties(xTableRow, uno::UNO_QUERY);
                        if (xRowProperties.is())
                        {
                            uno::Any aAny = xRowProperties->getPropertyValue(sRowHeight);
                            sal_Int32 nHeight;
                            if (aAny >>= nHeight)
                            {
                                Rectangle aRec = rImport.GetDocument()->GetMMRect(static_cast<USHORT>(aItr->aStartCell.Column), static_cast<USHORT>(aItr->aStartCell.Row),
                                    static_cast<USHORT>(aItr->aStartCell.Column), static_cast<USHORT>(aItr->aStartCell.Row), aItr->aStartCell.Sheet);
                                awt::Point aRefPoint;
                                aRefPoint.X = aRec.Left();
                                aRefPoint.Y = aRec.Top();
                                pRect = new Rectangle(rImport.GetDocument()->GetMMRect(
                                    static_cast<USHORT>(aItr->aEndCell.Column), static_cast<USHORT>(aItr->aEndCell.Row),
                                    static_cast<USHORT>(aItr->aEndCell.Column), static_cast<USHORT>(aItr->aEndCell.Row), aItr->aEndCell.Sheet ));
                                sal_Int32 Y (nHeight - aItr->nEndY);
                                aItr->nEndX += pRect->Left();
                                Y = pRect->Bottom() - Y;
                                awt::Point aPoint(aItr->nStartX, aItr->nStartY);// = aItr->xShape->getPosition();
                                awt::Size aSize = aItr->xShape->getSize();
                                aPoint.X += aRefPoint.X;
                                aPoint.Y += aRefPoint.Y;
                                aSize.Width = aItr->nEndX - aPoint.X;
                                aSize.Height = Y - aPoint.Y;
                                aItr->xShape->setPosition(aPoint);
                                aItr->xShape->setSize(aSize);
                                delete pRect;
                            }
                        }
                    }
                    aItr = aShapes.erase(aItr);
                }
            }
        }
    }
}
