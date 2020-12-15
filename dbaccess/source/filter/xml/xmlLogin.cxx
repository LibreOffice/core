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

#include "xmlLogin.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <strings.hxx>
#include <tools/diagnose_ex.h>
#include <com/sun/star/sdbc/XDataSource.hpp>

namespace dbaxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::xml::sax;

OXMLLogin::OXMLLogin( ODBFilter& rImport,
                const Reference< XFastAttributeList > & _xAttrList ) :
    SvXMLImportContext( rImport )
{
    Reference<XPropertySet> xDataSource(rImport.getDataSource());

    bool bUserFound = false;
    if (!xDataSource.is())
        return;

    for (auto &aIter : sax_fastparser::castToFastAttributeList( _xAttrList ))
    {
        try
        {
            switch( aIter.getToken() & TOKEN_MASK )
            {
                case XML_USER_NAME:
                    if ( !bUserFound )
                    {
                        bUserFound = true;
                        try
                        {
                            xDataSource->setPropertyValue(PROPERTY_USER,makeAny(aIter.toString()));
                        }
                        catch(const Exception&)
                        {
                            DBG_UNHANDLED_EXCEPTION("dbaccess");
                        }
                    }
                    break;
                case XML_IS_PASSWORD_REQUIRED:
                    try
                    {
                        xDataSource->setPropertyValue(PROPERTY_ISPASSWORDREQUIRED,makeAny(IsXMLToken(aIter, XML_TRUE)));
                    }
                    catch(const Exception&)
                    {
                        DBG_UNHANDLED_EXCEPTION("dbaccess");
                    }
                    break;
                case XML_USE_SYSTEM_USER:
                    if ( !bUserFound )
                    {
                        bUserFound = true;
                        PropertyValue aProperty;
                        aProperty.Name = "UseSystemUser";
                        aProperty.Value <<= IsXMLToken(aIter, XML_TRUE);
                        rImport.addInfo(aProperty);
                    }
                    break;
                case XML_LOGIN_TIMEOUT:
                    try
                    {
                        Reference< XDataSource>(xDataSource,UNO_QUERY_THROW)->setLoginTimeout(aIter.toInt32());
                    }
                    catch(const Exception&)
                    {
                        DBG_UNHANDLED_EXCEPTION("dbaccess");
                    }
                    break;
                default:
                    XMLOFF_WARN_UNKNOWN("dbaccess", aIter);
            }
        }
        catch(const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }
    }
}

OXMLLogin::~OXMLLogin()
{

}

} // namespace dbaxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
