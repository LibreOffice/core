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
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include "xmlEnums.hxx"
#include "xmlReport.hxx"


namespace rptxml
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml::sax;

OXMLMasterFields::OXMLMasterFields( ORptFilter& rImport,
                sal_uInt16 nPrfx, const OUString& rLName,
                const Reference< XAttributeList > & _xAttrList
                ,IMasterDetailFieds* _pReport
                ) :
    SvXMLImportContext( rImport, nPrfx, rLName)
,m_pReport(_pReport)
{

    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetSubDocumentElemTokenMap();

    OUString sMasterField,sDetailField;
    const sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
        OUString sLocalName;
        const OUString sAttrName = _xAttrList->getNameByIndex( i );
        const sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        const OUString sValue = _xAttrList->getValueByIndex( i );

        switch( rTokenMap.Get( nPrefix, sLocalName ) )
        {
            case XML_TOK_MASTER:
                sMasterField = sValue;
                break;
            case XML_TOK_SUB_DETAIL:
                sDetailField = sValue;
                break;
            default:
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

SvXMLImportContextRef OXMLMasterFields::CreateChildContext(
        sal_uInt16 _nPrefix,
        const OUString& _rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = nullptr;
    const SvXMLTokenMap&    rTokenMap   = static_cast<ORptFilter&>(GetImport()).GetSubDocumentElemTokenMap();

    switch( rTokenMap.Get( _nPrefix, _rLocalName ) )
    {
        case XML_TOK_MASTER_DETAIL_FIELD:
            {
                GetImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                pContext = new OXMLMasterFields(static_cast<ORptFilter&>(GetImport()), _nPrefix, _rLocalName,xAttrList ,m_pReport);
            }
            break;
        default:
            break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), _nPrefix, _rLocalName );

    return pContext;
}


} // namespace rptxml


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
