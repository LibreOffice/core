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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#ifndef DBA_XMLLOGIN_HXX
#include "xmlLogin.hxx"
#endif
#ifndef DBA_XMLFILTER_HXX
#include "xmlfilter.hxx"
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef DBA_XMLENUMS_HXX
#include "xmlEnums.hxx"
#endif
#ifndef DBACCESS_SHARED_XMLSTRINGS_HRC
#include "xmlstrings.hrc"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <vector>

namespace dbaxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::xml::sax;
DBG_NAME(OXMLLogin)

OXMLLogin::OXMLLogin( ODBFilter& rImport,
                sal_uInt16 nPrfx, const ::rtl::OUString& _sLocalName,
                const Reference< XAttributeList > & _xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, _sLocalName )
{
    DBG_CTOR(OXMLLogin,NULL);

    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");
    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetLoginElemTokenMap();

    Reference<XPropertySet> xDataSource(rImport.getDataSource());

    const sal_Int16 nLength = (xDataSource.is() && _xAttrList.is()) ? _xAttrList->getLength() : 0;
    static const ::rtl::OUString s_sTRUE = ::xmloff::token::GetXMLToken(XML_TRUE);
    bool bUserFound = false;
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
        ::rtl::OUString sLocalName;
        rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
        sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        rtl::OUString sValue = _xAttrList->getValueByIndex( i );

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
                        catch(Exception)
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
                    catch(Exception)
                    {
                        DBG_UNHANDLED_EXCEPTION();
                    }
                    break;
                case XML_TOK_USE_SYSTEM_USER:
                    if ( !bUserFound )
                    {
                        bUserFound = true;
                        PropertyValue aProperty;
                        aProperty.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UseSystemUser"));
                        aProperty.Value <<= (sValue == s_sTRUE ? sal_True : sal_False);
                        rImport.addInfo(aProperty);
                    }
                    break;
                case XML_TOK_LOGIN_TIMEOUT:
                    try
                    {
                        Reference< XDataSource>(xDataSource,UNO_QUERY_THROW)->setLoginTimeout(sValue.toInt32());
                    }
                    catch(Exception)
                    {
                        DBG_UNHANDLED_EXCEPTION();
                    }
                    break;
            }
        }
        catch(Exception)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}
// -----------------------------------------------------------------------------

OXMLLogin::~OXMLLogin()
{

    DBG_DTOR(OXMLLogin,NULL);
}
// -----------------------------------------------------------------------------

//----------------------------------------------------------------------------
} // namespace dbaxml
// -----------------------------------------------------------------------------
