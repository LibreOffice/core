/*************************************************************************
 *
 *  $RCSfile: XMLTableShapeImportHelper.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: sab $ $Date: 2001-07-23 15:24:06 $
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

#ifndef _SC_XMLTABLESHAPEIMPORTHELPER_HXX
#include "XMLTableShapeImportHelper.hxx"
#endif

#ifndef SC_XMLIMPRT_HXX
#include "xmlimprt.hxx"
#endif
#ifndef _SC_XMLCONVERTER_HXX
#include "XMLConverter.hxx"
#endif
#ifndef SC_DRWLAYER_HXX
#include "drwlayer.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif

#ifndef _SVX_UNOSHAPE_HXX
#include <svx/unoshape.hxx>
#endif

#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif

#define SC_LAYERID "LayerID"

using namespace ::com::sun::star;
using namespace xmloff::token;

XMLTableShapeImportHelper::XMLTableShapeImportHelper(
        ScXMLImport& rImp, SvXMLImportPropertyMapper *pImpMapper ) :
    XMLShapeImportHelper(rImp, rImp.GetModel(), pImpMapper )
{
}

XMLTableShapeImportHelper::~XMLTableShapeImportHelper()
{
}

void XMLTableShapeImportHelper::finishShape(
    uno::Reference< drawing::XShape >& rShape,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes )
{
    XMLShapeImportHelper::finishShape( rShape, xAttrList, rShapes );
    if (rShapes == static_cast<ScXMLImport&>(mrImporter).GetTables().GetCurrentXShapes())
    {
        sal_Bool bBackground(sal_False);
        Rectangle* pRect = NULL;
        sal_Int32 nEndX(-1);
        sal_Int32 nEndY(-1);
        sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
        table::CellAddress aEndCell;
        rtl::OUString sRangeList;
        rtl::OUString sOleName;
        for( sal_Int16 i=0; i < nAttrCount; i++ )
        {
            const rtl::OUString& rAttrName = xAttrList->getNameByIndex( i );
            const rtl::OUString& rValue = xAttrList->getValueByIndex( i );

            rtl::OUString aLocalName;
            sal_uInt16 nPrefix =
                static_cast<ScXMLImport&>(mrImporter).GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                                &aLocalName );
            if(nPrefix == XML_NAMESPACE_TABLE)
            {
                if (IsXMLToken(aLocalName, XML_END_CELL_ADDRESS))
                {
                    sal_Int32 nOffset(0);
                    ScXMLConverter::GetAddressFromString(aEndCell, rValue, static_cast<ScXMLImport&>(mrImporter).GetDocument(), nOffset);
                }
                else if (IsXMLToken(aLocalName, XML_END_X))
                    static_cast<ScXMLImport&>(mrImporter).GetMM100UnitConverter().convertMeasure(nEndX, rValue);
                else if (IsXMLToken(aLocalName, XML_END_Y))
                    static_cast<ScXMLImport&>(mrImporter).GetMM100UnitConverter().convertMeasure(nEndY, rValue);
                else if (IsXMLToken(aLocalName, XML_TABLE_BACKGROUND))
                    if (IsXMLToken(rValue, XML_TRUE))
                        bBackground = sal_True;
            }
            else if(nPrefix == XML_NAMESPACE_DRAW)
            {
                if (IsXMLToken(aLocalName, XML_NOTIFY_ON_UPDATE_OF_RANGES))
                    sRangeList = rValue;
                else if (IsXMLToken(aLocalName, XML_NAME))
                    sOleName = rValue;
            }
        }
        if (bBackground)
        {
            uno::Reference< beans::XPropertySet > xShapeProp( rShape, uno::UNO_QUERY );
            if( xShapeProp.is() )
            {
                sal_Int16 nLayerID = SC_LAYER_BACK;
                uno::Any aPropAny;
                aPropAny <<= nLayerID;
                xShapeProp->setPropertyValue(OUString( RTL_CONSTASCII_USTRINGPARAM( SC_LAYERID ) ), aPropAny );
            }
        }

        if (!bOnTable)
        {
            static_cast<ScXMLImport&>(mrImporter).GetTables().AddShape(rShape,
                sOleName, sRangeList, aStartCell, aEndCell, nEndX, nEndY);
            SvxShape* pShapeImp = SvxShape::getImplementation(rShape);
            if (pShapeImp)
            {
                SdrObject *pSdrObj = pShapeImp->GetSdrObject();
                if (pSdrObj)
                    ScDrawLayer::SetAnchor(pSdrObj, SCA_CELL);
            }
        }
        else
        {
            SvxShape* pShapeImp = SvxShape::getImplementation(rShape);
            if (pShapeImp)
            {
                SdrObject *pSdrObj = pShapeImp->GetSdrObject();
                if (pSdrObj)
                    ScDrawLayer::SetAnchor(pSdrObj, SCA_PAGE);
            }
        }
    }
}
