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
#include "xmlReportElement.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmlEnums.hxx"
#include "xmlComponent.hxx"
#include "xmlCondPrtExpr.hxx"
#include <tools/debug.hxx>
#include "xmlFormatCondition.hxx"


namespace rptxml
{
    using namespace ::com::sun::star;
    using namespace report;
    using namespace uno;
    using namespace xml::sax;
DBG_NAME( rpt_OXMLReportElement )

OXMLReportElement::OXMLReportElement( ORptFilter& rImport,
                sal_uInt16 nPrfx, const ::rtl::OUString& rLName,
                const Reference< XAttributeList > & _xAttrList
                ,const Reference< XReportControlModel > & _xComponent) :
    SvXMLImportContext( rImport, nPrfx, rLName )
,m_xComponent(_xComponent)
{
    DBG_CTOR( rpt_OXMLReportElement,NULL);

    OSL_ENSURE(m_xComponent.is(),"Component is NULL!");
    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetReportElementElemTokenMap();

    static const ::rtl::OUString s_sTRUE = ::xmloff::token::GetXMLToken(XML_TRUE);
    const sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    try
    {
        for(sal_Int16 i = 0; i < nLength; ++i)
        {
         ::rtl::OUString sLocalName;
            const rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
            const sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
            const rtl::OUString sValue = _xAttrList->getValueByIndex( i );

            switch( rTokenMap.Get( nPrefix, sLocalName ) )
            {
                case XML_TOK_PRINT_ONLY_WHEN_GROUP_CHANGE:
                    m_xComponent->setPrintWhenGroupChange(s_sTRUE == sValue);
                    break;
                   case XML_TOK_PRINT_REPEATED_VALUES:
                    m_xComponent->setPrintRepeatedValues(sValue == s_sTRUE ? sal_True : sal_False);
                    break;
                default:
                    break;
            }
        }
    }
    catch(Exception&)
    {
        OSL_FAIL("Exception catched while filling the report definition props");
    }
}
// -----------------------------------------------------------------------------

OXMLReportElement::~OXMLReportElement()
{

    DBG_DTOR( rpt_OXMLReportElement,NULL);
}
// -----------------------------------------------------------------------------

SvXMLImportContext* OXMLReportElement::CreateChildContext(
        sal_uInt16 _nPrefix,
        const ::rtl::OUString& _rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    ORptFilter& rImport = GetOwnImport();
    const SvXMLTokenMap&    rTokenMap   = rImport.GetReportElementElemTokenMap();

    switch( rTokenMap.Get( _nPrefix, _rLocalName ) )
    {
        case XML_TOK_COMPONENT:
            rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLComponent( rImport, _nPrefix, _rLocalName,xAttrList,m_xComponent.get());
            break;
        case XML_TOK_REP_CONDITIONAL_PRINT_EXPRESSION:
            rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLCondPrtExpr( rImport, _nPrefix, _rLocalName,xAttrList,m_xComponent.get());
            break;
        case XML_TOK_FORMATCONDITION:
            {
                uno::Reference< report::XFormatCondition > xNewCond = m_xComponent->createFormatCondition();
                m_xComponent->insertByIndex(m_xComponent->getCount(),uno::makeAny(xNewCond));
                rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                pContext = new OXMLFormatCondition( rImport, _nPrefix, _rLocalName,xAttrList,xNewCond);
            }
            break;
        default:
            break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), _nPrefix, _rLocalName );


    return pContext;
}
// -----------------------------------------------------------------------------
ORptFilter& OXMLReportElement::GetOwnImport()
{
    return static_cast<ORptFilter&>(GetImport());
}
//----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
