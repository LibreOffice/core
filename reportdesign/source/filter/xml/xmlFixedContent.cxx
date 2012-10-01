/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#include "xmlFixedContent.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmlEnums.hxx"
#include "xmlReportElement.hxx"
#include <tools/debug.hxx>
#include <com/sun/star/report/XShape.hpp>
#include "xmlCell.hxx"
#include "xmlstrings.hrc"
#include <com/sun/star/report/XShape.hpp>
#include <com/sun/star/report/XFormattedField.hpp>
#include <com/sun/star/report/XFixedText.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include "xmlTable.hxx"
#include <xmloff/XMLCharContext.hxx>

namespace rptxml
{
    using namespace ::com::sun::star;

class OXMLCharContent : public XMLCharContext
{
    OXMLFixedContent* m_pFixedContent;
    OXMLCharContent(const OXMLCharContent&);
    OXMLCharContent operator =(const OXMLCharContent&);
public:
    OXMLCharContent(
            SvXMLImport& rImport,
            OXMLFixedContent* _pFixedContent,
            sal_uInt16 nPrfx,
            const ::rtl::OUString& rLName,
            const uno::Reference< xml::sax::XAttributeList > & xAttrList,
            sal_Unicode c,
            sal_Bool bCount );
    OXMLCharContent(
            SvXMLImport& rImport,
            OXMLFixedContent* _pFixedContent,
            sal_uInt16 nPrfx,
            const ::rtl::OUString& rLName,
            const uno::Reference< xml::sax::XAttributeList > & xAttrList,
            sal_Int16 nControl );

    virtual void InsertControlCharacter(sal_Int16   _nControl);
    virtual void InsertString(const ::rtl::OUString& _sString);
};
OXMLCharContent::OXMLCharContent(
        SvXMLImport& rImport,
        OXMLFixedContent* _pFixedContent,
        sal_uInt16 nPrfx,
        const ::rtl::OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList,
        sal_Unicode c,
        sal_Bool bCount )
    : XMLCharContext(rImport,nPrfx,rLName,xAttrList,c,bCount)
    ,m_pFixedContent(_pFixedContent)
{
}
// -----------------------------------------------------------------------------
OXMLCharContent::OXMLCharContent(
        SvXMLImport& rImport,
        OXMLFixedContent* _pFixedContent,
        sal_uInt16 nPrfx,
        const ::rtl::OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList,
        sal_Int16 nControl )
    : XMLCharContext(rImport,nPrfx,rLName,xAttrList,nControl)
    ,m_pFixedContent(_pFixedContent)
{
}
// -----------------------------------------------------------------------------
void OXMLCharContent::InsertControlCharacter(sal_Int16   _nControl)
{
    switch( _nControl )
    {
        case ControlCharacter::LINE_BREAK:
            m_pFixedContent->Characters(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n")));
            break;
        default:
            OSL_FAIL("Not supported control character");
            break;
    }
}
// -----------------------------------------------------------------------------
void OXMLCharContent::InsertString(const ::rtl::OUString& _sString)
{
    m_pFixedContent->Characters(_sString);
}
// -----------------------------------------------------------------------------

DBG_NAME( rpt_OXMLFixedContent )

OXMLFixedContent::OXMLFixedContent( ORptFilter& rImport,
                sal_uInt16 nPrfx, const ::rtl::OUString& rLName
                ,OXMLCell& _rCell
                ,OXMLTable* _pContainer
                ,OXMLFixedContent* _pInP) :
    OXMLReportElementBase( rImport, nPrfx, rLName,NULL,_pContainer)
,m_rCell(_rCell)
,m_pInP(_pInP)
,m_bFormattedField(false)
{
    DBG_CTOR( rpt_OXMLFixedContent,NULL);
}
// -----------------------------------------------------------------------------

OXMLFixedContent::~OXMLFixedContent()
{

    DBG_DTOR( rpt_OXMLFixedContent,NULL);
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
SvXMLImportContext* OXMLFixedContent::_CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = OXMLReportElementBase::_CreateChildContext(nPrefix,rLocalName,xAttrList);
    if ( pContext )
        return pContext;

    static const ::rtl::OUString s_sStringConcat(RTL_CONSTASCII_USTRINGPARAM(" & "));
    const SvXMLTokenMap&    rTokenMap   = m_rImport.GetCellElemTokenMap();
    Reference<XMultiServiceFactory> xFactor = m_rImport.getServiceFactory();

    m_rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
    const sal_uInt16 nToken = rTokenMap.Get( nPrefix, rLocalName );
    switch( nToken )
    {
        case XML_TOK_P:
            pContext = new OXMLFixedContent(m_rImport,nPrefix, rLocalName,m_rCell,m_pContainer,this);
            break;
        case XML_TOK_TEXT_TAB_STOP:
            pContext = new OXMLCharContent( m_rImport, this,nPrefix,
                                                rLocalName, xAttrList,
                                                0x0009, sal_False );
            break;

        case XML_TOK_TEXT_LINE_BREAK:
            pContext = new OXMLCharContent( m_rImport, this,nPrefix,
                                                rLocalName, xAttrList,
                                                ControlCharacter::LINE_BREAK );
            break;

        case XML_TOK_TEXT_S:
            pContext = new OXMLCharContent( m_rImport, this,nPrefix,
                                                rLocalName, xAttrList,
                                                0x0020, sal_True );
            break;
        case XML_TOK_PAGE_NUMBER:
            m_sPageText += s_sStringConcat + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" PageNumber()"));
            m_bFormattedField = true;
            break;
        case XML_TOK_PAGE_COUNT:
            m_sPageText += s_sStringConcat + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" PageCount()"));
            m_bFormattedField = true;
            break;
        default:
            ;
    }
    return pContext;
}
// -----------------------------------------------------------------------------
void OXMLFixedContent::EndElement()
{
    if ( m_pInP )
    {
        const Reference<XMultiServiceFactory> xFactor(m_rImport.GetModel(),uno::UNO_QUERY);
        if ( m_bFormattedField )
        {
            uno::Reference< uno::XInterface> xInt = xFactor->createInstance(SERVICE_FORMATTEDFIELD);
            Reference< report::XFormattedField > xControl(xInt,uno::UNO_QUERY);
            xControl->setDataField(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("rpt:")) + m_sPageText);
             OSL_ENSURE(xControl.is(),"Could not create FormattedField!");
            m_pInP->m_xComponent = xControl.get();
            m_xComponent = xControl.get();
        }
        else
        {
            Reference< XFixedText > xControl(xFactor->createInstance(SERVICE_FIXEDTEXT),uno::UNO_QUERY);
             OSL_ENSURE(xControl.is(),"Could not create FixedContent!");
            m_pInP->m_xComponent = xControl.get();
            m_xComponent = xControl.get();
            xControl->setLabel(m_sLabel);
        }

        m_pContainer->addCell(m_xComponent);
        m_rCell.setComponent(m_xComponent);

        OXMLReportElementBase::EndElement();
    }
}
// -----------------------------------------------------------------------------
void OXMLFixedContent::Characters( const ::rtl::OUString& rChars )
{
    m_sLabel += rChars;
    if ( !rChars.isEmpty() )
    {
        static const ::rtl::OUString s_Quote(RTL_CONSTASCII_USTRINGPARAM("\""));
        if ( !m_sPageText.isEmpty() )
        {
            static const ::rtl::OUString s_sStringConcat(RTL_CONSTASCII_USTRINGPARAM(" & "));
            m_sPageText += s_sStringConcat;
        }

        m_sPageText += s_Quote + rChars + s_Quote;
    }
}

//----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
