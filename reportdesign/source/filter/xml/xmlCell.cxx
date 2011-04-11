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
#include "precompiled_reportdesign.hxx"
#include "xmlCell.hxx"
#include "xmlHelper.hxx"
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
#include <com/sun/star/report/XShape.hpp>
#include <com/sun/star/report/XFixedLine.hpp>
#include <com/sun/star/table/BorderLine.hpp>
#include "xmlstrings.hrc"
#include "xmlTable.hxx"
#include "xmlFormattedField.hxx"
#include "xmlImage.hxx"
#include "xmlFixedContent.hxx"
#include "xmlSubDocument.hxx"

namespace rptxml
{
    using namespace ::comphelper;
    using namespace ::com::sun::star;
    using namespace uno;
    using namespace beans;
    using namespace xml::sax;

DBG_NAME( rpt_OXMLCell )

OXMLCell::OXMLCell( ORptFilter& rImport
                ,sal_uInt16 nPrfx
                ,const ::rtl::OUString& _sLocalName
                ,const Reference< XAttributeList > & _xAttrList
                ,OXMLTable* _pContainer
                ,OXMLCell* _pCell) :
    SvXMLImportContext( rImport, nPrfx, _sLocalName )
    ,m_pContainer(_pContainer)
    ,m_pCell(_pCell)
    ,m_nCurrentCount(0)
    ,m_bContainsShape(false)
{
    DBG_CTOR( rpt_OXMLCell,NULL);
    if ( !m_pCell )
        m_pCell = this;

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
            case XML_TOK_NUMBER_COLUMNS_SPANNED:
                m_pContainer->setColumnSpanned(sValue.toInt32());
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
OXMLCell::~OXMLCell()
{
    DBG_DTOR( rpt_OXMLCell,NULL);
}
// -----------------------------------------------------------------------------
SvXMLImportContext* OXMLCell::CreateChildContext(
        sal_uInt16 _nPrefix,
        const ::rtl::OUString& _rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    ORptFilter& rImport = GetOwnImport();
    const SvXMLTokenMap&    rTokenMap   = rImport.GetCellElemTokenMap();
    Reference<XMultiServiceFactory> xFactor(rImport.GetModel(),uno::UNO_QUERY);
    static const ::rtl::OUString s_sStringConcat(RTL_CONSTASCII_USTRINGPARAM(" & "));

    const sal_uInt16 nToken = rTokenMap.Get( _nPrefix, _rLocalName );
    switch( nToken )
    {
        case XML_TOK_FIXED_CONTENT:
            {
                rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                pContext = new OXMLFixedContent( rImport, _nPrefix, _rLocalName,*m_pCell,m_pContainer);
            }
            break;
        case XML_TOK_PAGE_NUMBER:
            m_sText += s_sStringConcat + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" PageNumber()"));
            break;
        case XML_TOK_PAGE_COUNT:
            m_sText += s_sStringConcat + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" PageCount()"));
            break;
        case XML_TOK_FORMATTED_TEXT:
            {
                rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                uno::Reference< uno::XInterface> xInt = xFactor->createInstance(SERVICE_FORMATTEDFIELD);
                Reference< report::XFormattedField > xControl(xInt,uno::UNO_QUERY);

                OSL_ENSURE(xControl.is(),"Could not create FormattedField!");
                setComponent(xControl.get());
                if ( xControl.is() )
                    pContext = new OXMLFormattedField( rImport, _nPrefix, _rLocalName,xAttrList,xControl.get(),m_pContainer,XML_TOK_PAGE_COUNT == nToken);
            }
            break;
        case XML_TOK_IMAGE:
            {
                rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                Reference< XImageControl > xControl(xFactor->createInstance(SERVICE_IMAGECONTROL),uno::UNO_QUERY);

                OSL_ENSURE(xControl.is(),"Could not create ImageControl!");
                setComponent(xControl.get());
                if ( xControl.is() )
                    pContext = new OXMLImage( rImport, _nPrefix, _rLocalName,xAttrList,xControl.get(),m_pContainer);
            }
            break;
        case XML_TOK_SUB_DOCUMENT:
            {
                rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                if ( !m_bContainsShape )
                    m_nCurrentCount = m_pContainer->getSection()->getCount();
                uno::Reference< uno::XInterface> xInt = xFactor->createInstance(SERVICE_FORMATTEDFIELD);
                Reference< report::XFormattedField > xControl(xInt,uno::UNO_QUERY);
                pContext = new OXMLSubDocument( rImport, _nPrefix, _rLocalName,xControl.get(),m_pContainer, this /* give the current cell as parent*/ );
            }
            break;

        case XML_TOK_P:
            pContext = new OXMLCell( rImport, _nPrefix, _rLocalName,xAttrList ,m_pContainer,this);
            break;
        case XML_TOK_CUSTOM_SHAPE:
        case XML_TOK_FRAME:
            {
                if ( !m_bContainsShape )
                    m_nCurrentCount = m_pContainer->getSection()->getCount();
                UniReference< XMLShapeImportHelper > xShapeImportHelper = rImport.GetShapeImport();
                uno::Reference< drawing::XShapes > xShapes = m_pContainer->getSection().get();
                pContext = xShapeImportHelper->CreateGroupChildContext(rImport,_nPrefix,_rLocalName,xAttrList,xShapes);
                m_bContainsShape = true;
            }
            break;
        default:
            break;
    }

    if ( m_xComponent.is() )
        m_pContainer->addCell(m_xComponent);

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), _nPrefix, _rLocalName );

    return pContext;
}
// -----------------------------------------------------------------------------
void OXMLCell::EndElement()
{
    if ( m_bContainsShape )
    {
        const sal_Int32 nCount = m_pContainer->getSection()->getCount();
        for (sal_Int32 i = m_nCurrentCount; i < nCount; ++i)
        {
            uno::Reference<report::XShape> xShape(m_pContainer->getSection()->getByIndex(i),uno::UNO_QUERY);
            if ( xShape.is() )
                m_pContainer->addCell(xShape.get());
        }
    }
    if ( m_pCell != this && m_sText.getLength() )
    {
        ORptFilter& rImport = GetOwnImport();
        Reference<XMultiServiceFactory> xFactor(rImport.GetModel(),uno::UNO_QUERY);
        uno::Reference< uno::XInterface> xInt = xFactor->createInstance(SERVICE_FORMATTEDFIELD);
        Reference< report::XFormattedField > xControl(xInt,uno::UNO_QUERY);
        xControl->setDataField(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("rpt:")) + m_sText);

        OSL_ENSURE(xControl.is(),"Could not create FormattedField!");
        setComponent(xControl.get());
        m_xComponent = xControl.get();
        m_pContainer->getSection()->add(m_xComponent.get());
        m_pContainer->addCell(m_xComponent);
    }
    // check if we have a FixedLine
    else if ( m_sStyleName.getLength() && !m_xComponent.is() && m_pCell == this )
    {
        ORptFilter& rImport = GetOwnImport();
        Reference<XMultiServiceFactory> xFactor(rImport.GetModel(),uno::UNO_QUERY);
        Reference<XFixedLine> xFixedLine(xFactor->createInstance(SERVICE_FIXEDLINE),uno::UNO_QUERY);
        m_xComponent = xFixedLine.get();
        m_pContainer->getSection()->add(m_xComponent.get());
        m_pContainer->addCell(m_xComponent);
        XMLPropStyleContext* pAutoStyle = PTR_CAST(XMLPropStyleContext,GetImport().GetAutoStyles()->FindStyleChildContext(XML_STYLE_FAMILY_TABLE_CELL,m_sStyleName));
        if ( pAutoStyle )
        {
            uno::Reference<beans::XPropertySet> xBorderProp = OXMLHelper::createBorderPropertySet();
            try
            {
                pAutoStyle->FillPropertySet(xBorderProp);
                table::BorderLine aRight,aLeft;
                xBorderProp->getPropertyValue(PROPERTY_BORDERRIGHT) >>= aRight;
                xBorderProp->getPropertyValue(PROPERTY_BORDERLEFT) >>= aLeft;
                xFixedLine->setOrientation( (aRight.OuterLineWidth != 0 || aLeft.OuterLineWidth != 0) ? 1 : 0);
               }
            catch(uno::Exception&)
            {
                OSL_FAIL("OXMLCell::EndElement -> exception catched");
            }
        }
    }
    else
        OXMLHelper::copyStyleElements(GetOwnImport().isOldFormat(),m_sStyleName,GetImport().GetAutoStyles(),m_xComponent.get());
}
// -----------------------------------------------------------------------------
ORptFilter& OXMLCell::GetOwnImport()
{
    return static_cast<ORptFilter&>(GetImport());
}
// -----------------------------------------------------------------------------
void OXMLCell::setComponent(const uno::Reference< report::XReportComponent >& _xComponent)
{
    m_pCell->m_xComponent = _xComponent;
    m_xComponent = _xComponent;
}
// -----------------------------------------------------------------------------
void OXMLCell::Characters( const ::rtl::OUString& rChars )
{
    if ( rChars.getLength() )
    {
        static const ::rtl::OUString s_Quote(RTL_CONSTASCII_USTRINGPARAM("\""));
        if ( m_sText.getLength() )
        {
            static const ::rtl::OUString s_sStringConcat(RTL_CONSTASCII_USTRINGPARAM(" & "));
            m_sText += s_sStringConcat;
        }

        m_sText += s_Quote + rChars + s_Quote;
    }
}

void OXMLCell::setContainsShape(bool _bContainsShape)
{
    m_bContainsShape = _bContainsShape;
}

//----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
