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
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmlEnums.hxx"
#include "xmlstrings.hrc"
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <vector>

namespace dbaxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::xml::sax;
DBG_NAME(OXMLLogin)

OXMLLogin::OXMLLogin( ODBFilter& rImport,
                sal_uInt16 nPrfx, const OUString& _sLocalName,
                const Reference< XAttributeList > & _xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, _sLocalName )
{
    DBG_CTOR(OXMLLogin,NULL);

    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");
    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetLoginElemTokenMap();

    Reference<XPropertySet> xDataSource(rImport.getDataSource());

    const sal_Int16 nLength = (xDataSource.is() && _xAttrList.is()) ? _xAttrList->getLength() : 0;
    static const OUString s_sTRUE = ::xmloff::token::GetXMLToken(XML_TRUE);
    bool bUserFound = false;
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
        OUString sLocalName;
        OUString sAttrName = _xAttrList->getNameByIndex( i );
        sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        OUString sValue = _xAttrList->getValueByIndex( i );

        try
        {
            switch( rTokenMap.Get( nPrefix, sLocalName ) )
            {
                case XML_TOK_USER_NAME:
                    if ( !bUserFound )
                    {
                        bUserFound = true;
                        try
                        {
                            xDataSource->setPropertyValue(PROPERTY_USER,makeAny(sValue));
                        }
                        catch(const Exception&)
                        {
                            DBG_UNHANDLED_EXCEPTION();
                        }
                    }
                    break;
                case XML_TOK_IS_PASSWORD_REQUIRED:
                    try
                    {
                        xDataSource->setPropertyValue(PROPERTY_ISPASSWORDREQUIRED,makeAny((sValue == s_sTRUE ? sal_True : sal_False)));
                    }
                    catch(const Exception&)
                    {
                        DBG_UNHANDLED_EXCEPTION();
                    }
                    break;
                case XML_TOK_USE_SYSTEM_USER:
                    if ( !bUserFound )
                    {
                        bUserFound = true;
                        PropertyValue aProperty;
                        aProperty.Name = OUString("UseSystemUser");
                        aProperty.Value <<= (sValue == s_sTRUE ? sal_True : sal_False);
                        rImport.addInfo(aProperty);
                    }
                    break;
                case XML_TOK_LOGIN_TIMEOUT:
                    try
                    {
                        Reference< XDataSource>(xDataSource,UNO_QUERY_THROW)->setLoginTimeout(sValue.toInt32());
                    }
                    catch(const Exception&)
                    {
                        DBG_UNHANDLED_EXCEPTION();
                    }
                    break;
            }
        }
        catch(const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}

OXMLLogin::~OXMLLogin()
{

    DBG_DTOR(OXMLLogin,NULL);
}

} // namespace dbaxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
