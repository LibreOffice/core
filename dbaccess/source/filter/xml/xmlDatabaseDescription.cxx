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

#include "xmlDatabaseDescription.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include "xmlEnums.hxx"
#include "xmlFileBasedDatabase.hxx"
#include "xmlServerDatabase.hxx"

namespace dbaxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml::sax;

OXMLDatabaseDescription::OXMLDatabaseDescription( ODBFilter& rImport ) :
    SvXMLImportContext( rImport )
    ,m_bFoundOne(false)
{
}

OXMLDatabaseDescription::~OXMLDatabaseDescription()
{

}

css::uno::Reference< css::xml::sax::XFastContextHandler > OXMLDatabaseDescription::createFastChildContext(
            sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

    switch( nElement & TOKEN_MASK )
    {
        case XML_FILE_BASED_DATABASE:
            if ( !m_bFoundOne )
            {
                m_bFoundOne = true;
                GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                pContext = new OXMLFileBasedDatabase( GetOwnImport(), xAttrList );
            }
            break;
        case XML_SERVER_DATABASE:
            if ( !m_bFoundOne )
            {
                m_bFoundOne = true;
                GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                pContext = new OXMLServerDatabase( GetOwnImport(), xAttrList );
            }
            break;
    }

    return pContext;
}

ODBFilter& OXMLDatabaseDescription::GetOwnImport()
{
    return static_cast<ODBFilter&>(GetImport());
}

} // namespace dbaxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
