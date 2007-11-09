/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlLogin.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2007-11-09 08:17:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
