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

#include "xmlDataSourceSettings.hxx"
#include "xmlDataSourceSetting.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include "xmlEnums.hxx"
#include <stringconstants.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <vector>

namespace dbaxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml::sax;

OXMLDataSourceSettings::OXMLDataSourceSettings( ODBFilter& rImport
                ,sal_uInt16 nPrfx
                ,const OUString& _sLocalName) :
    SvXMLImportContext( rImport, nPrfx, _sLocalName )
{

}

OXMLDataSourceSettings::~OXMLDataSourceSettings()
{

}

SvXMLImportContextRef OXMLDataSourceSettings::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = nullptr;
    const SvXMLTokenMap&    rTokenMap   = GetOwnImport().GetDataSourceInfoElemTokenMap();

    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
        case XML_TOK_DATA_SOURCE_SETTING:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLDataSourceSetting( GetOwnImport(), nPrefix, rLocalName,xAttrList);
            break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

ODBFilter& OXMLDataSourceSettings::GetOwnImport()
{
    return static_cast<ODBFilter&>(GetImport());
}

} // namespace dbaxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
