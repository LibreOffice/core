/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_rptxml.hxx"

#include "xmlColumn.hxx"
#include <xmloff/xmluconv.hxx>
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmlEnums.hxx"
#include <tools/debug.hxx>
#include "xmlStyleImport.hxx"
#include <comphelper/namecontainer.hxx>
#include <comphelper/genericpropertyset.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include "xmlstrings.hrc"

namespace rptxml
{
    using namespace ::comphelper;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::xml::sax;

DBG_NAME( rpt_OXMLRow )

OXMLRow::OXMLRow( ORptFilter& rImport
                ,sal_uInt16 nPrfx
                ,const ::rtl::OUString& _sLocalName
                ,const Reference< XAttributeList > & _xAttrList
                ,OXMLTable* _pContainer) :
    SvXMLImportContext( rImport, nPrfx, _sLocalName )
    ,m_pContainer(_pContainer)
{
    DBG_CTOR( rpt_OXMLRow,NULL);

    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");

    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetColumnTokenMap();

    const sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
     ::rtl::OUString sLocalName;
        const rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
        const sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        const rtl::OUString sValue = _xAttrList->getValueByIndex( i );

        switch( rTokenMap.Get( nPrefix, sLocalName ) )
        {
            case XML_TOK_COLUMN_STYLE_NAME:
                m_sStyleName = sValue;
                break;
            case XML_TOK_NUMBER_ROWS_SPANNED:
                m_pContainer->setRowSpanned(sValue.toInt32());
                break;
            default:
                break;
        }
    }

}
// -----------------------------------------------------------------------------

OXMLRow::~OXMLRow()
{
    DBG_DTOR( rpt_OXMLRow,NULL);
}
// -----------------------------------------------------------------------------
SvXMLImportContext* OXMLRow::CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    ORptFilter& rImport = GetOwnImport();
    const SvXMLTokenMap&    rTokenMap   = rImport.GetControlPropertyElemTokenMap();

    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
        case XML_TOK_COLUMN:
            rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLRow( rImport, nPrefix, rLocalName,xAttrList,m_pContainer);
            break;
        case XML_TOK_ROW:
            rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLRow( rImport, nPrefix, rLocalName,xAttrList,m_pContainer);
            break;
        case XML_TOK_CELL:
            rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLRow( rImport, nPrefix, rLocalName,xAttrList,m_pContainer);
            break;
        case XML_TOK_COV_CELL:
            rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLRow( rImport, nPrefix, rLocalName,xAttrList,m_pContainer);
            break;
        default:
            break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}
// -----------------------------------------------------------------------------
void OXMLRow::EndElement()
{
    if ( m_sStyleName.getLength() )
    {
        const SvXMLStylesContext* pAutoStyles = GetImport().GetAutoStyles();
        if ( pAutoStyles )
        {
            PropertySetInfo* pInfo = new PropertySetInfo();
            static PropertyMapEntry pMap[] =
            {
                {PROPERTY_WIDTH,        static_cast<sal_uInt16>(PROPERTY_WIDTH.length),     0,          &::getCppuType(static_cast< sal_Int32* >( NULL ))       ,PropertyAttribute::BOUND,0},
                {PROPERTY_HEIGHT,       static_cast<sal_uInt16>(PROPERTY_HEIGHT.length),    0,          &::getCppuType(static_cast< sal_Int32* >( NULL ))       ,PropertyAttribute::BOUND,0},
                { NULL, 0, 0, NULL, 0, 0 }
            };
            pInfo->add(pMap);
            Reference<XPropertySet> xProp = GenericPropertySet_CreateInstance(pInfo);
            XMLPropStyleContext* pAutoStyle = PTR_CAST(XMLPropStyleContext,pAutoStyles->FindStyleChildContext(XML_STYLE_FAMILY_TABLE_COLUMN,m_sStyleName));
            if ( pAutoStyle )
            {
                pAutoStyle->FillPropertySet(xProp);
                sal_Int32 nWidth = 0;
                xProp->getPropertyValue(PROPERTY_WIDTH) >>= nWidth;
                m_pContainer->addWidth(nWidth);
            }
            else
            {
                pAutoStyle = PTR_CAST(XMLPropStyleContext,pAutoStyles->FindStyleChildContext(XML_STYLE_FAMILY_TABLE_ROW,m_sStyleName));
                if ( pAutoStyle )
                {
                    pAutoStyle->FillPropertySet(xProp);
                    sal_Int32 nHeight = 0;
                    xProp->getPropertyValue(PROPERTY_HEIGHT) >>= nHeight;
                    m_pContainer->addHeight(nHeight);
                }
            }
        }
    }
}
// -----------------------------------------------------------------------------
ORptFilter& OXMLRow::GetOwnImport()
{
    return static_cast<ORptFilter&>(GetImport());
}
//----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
