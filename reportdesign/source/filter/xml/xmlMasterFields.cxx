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
#include "xmlMasterFields.hxx"
#include "xmlReportElementBase.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include "xmlEnums.hxx"
#include <sal/log.hxx>


namespace rptxml
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml::sax;

OXMLMasterFields::OXMLMasterFields( ORptFilter& rImport,
                const Reference< XFastAttributeList > & _xAttrList
                ,IMasterDetailFieds* _pReport
                ) :
    SvXMLImportContext( rImport )
,m_pReport(_pReport)
{
    OUString sMasterField,sDetailField;
    for (auto &aIter : sax_fastparser::castToFastAttributeList( _xAttrList ))
    {
        OUString sValue = aIter.toString();

        switch( aIter.getToken() )
        {
            case XML_ELEMENT(REPORT, XML_MASTER):
                sMasterField = sValue;
                break;
            case XML_ELEMENT(REPORT, XML_DETAIL):
                sDetailField = sValue;
                break;
            default:
                SAL_WARN("reportdesign", "unknown attribute " << SvXMLImport::getPrefixAndNameFromToken(aIter.getToken()) << " = " << sValue);
                break;
        }
    }
    if ( sDetailField.isEmpty() )
        sDetailField = sMasterField;
    if ( !sMasterField.isEmpty() )
        m_pReport->addMasterDetailPair(::std::pair< OUString,OUString >(sMasterField,sDetailField));
}


OXMLMasterFields::~OXMLMasterFields()
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > OXMLMasterFields::createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    css::uno::Reference< css::xml::sax::XFastContextHandler > xContext;

    switch( nElement )
    {
        case XML_ELEMENT(REPORT, XML_MASTER_DETAIL_FIELD):
            {
                GetImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                xContext = new OXMLMasterFields(static_cast<ORptFilter&>(GetImport()),xAttrList ,m_pReport);
            }
            break;
        default:
            break;
    }

    return xContext;
}


} // namespace rptxml


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
