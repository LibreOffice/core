/*************************************************************************
 *
 *  $RCSfile: bootstrapcontext.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2002-06-12 16:37:43 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "bootstrapcontext.hxx"

#ifndef CONFIGMGR_BOOTSTRAP_HXX_
#include "bootstrap.hxx"
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

namespace configmgr
{
// ---------------------------------------------------------------------------
static const sal_Char k_BOOTSTRAP_ARGNAME_PREFIX[]      = "com.sun.star.configuration.bootstrap.";
static const sal_Char k_BOOTSTRAP_ITEM_PROFILE_NAME[]   = "CFG_INIFILE";
// ---------------------------------------------------------------------------

#define OUSTR( text )     OUString( RTL_CONSTASCII_USTRINGPARAM( text ) )
#define OU2ASCII( str ) ( rtl::OUStringToOString(str,RTL_TEXTENCODING_ASCII_US) .getStr() )
// ---------------------------------------------------------------------------

static void testComplete(BootstrapContext::Arguments const & _aArguments)
{
    uno::Reference< uno::XInterface > test = * new BootstrapContext(_aArguments);
}
// ---------------------------------------------------------------------------

BootstrapContext::BootstrapContext(Arguments const & _aArguments, ComponentContext const & _xContext)
: m_aArguments(_aArguments)
, m_xContext(_xContext)
, m_hBootstrapData(NULL)
{
    uno::Any aExplicitURL = getValueByName(OUSTR(k_BOOTSTRAP_ITEM_PROFILE_NAME));

    OUString sURL;
    if (!(aExplicitURL >>= sURL))
        sURL = BootstrapSettings::getURL();

    this->setBootstrapURL(sURL);
}
// ---------------------------------------------------------------------------

BootstrapContext::~BootstrapContext()
{
    rtl_bootstrap_args_close(m_hBootstrapData);
}
// ---------------------------------------------------------------------------

void BootstrapContext::setBootstrapURL( const OUString& _aURL )
{
    if (rtlBootstrapHandle hNew = rtl_bootstrap_args_open(_aURL.pData))
    {
        rtl_bootstrap_args_close(m_hBootstrapData);
        m_hBootstrapData = hNew;
    }
    else
    {
        OSL_TRACE( "configmgr: Cannot open bootstrap data URL: %s", OU2ASCII(_aURL) );
    }
}
// ---------------------------------------------------------------------------

OUString BootstrapContext::makeLongName(OUString const & _aName)
{
    // check if already is long
    if (_aName.matchIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM(k_BOOTSTRAP_ARGNAME_PREFIX) ) )
        return _aName;

    return OUSTR(k_BOOTSTRAP_ARGNAME_PREFIX).concat(_aName);
}
// ---------------------------------------------------------------------------

OUString BootstrapContext::makeShortName(OUString const & _aName)
{
    // check if already is short
    if (!_aName.matchIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM(k_BOOTSTRAP_ARGNAME_PREFIX) ) )
        return _aName;

    return _aName.copy(RTL_CONSTASCII_LENGTH(k_BOOTSTRAP_ARGNAME_PREFIX));
}
// ---------------------------------------------------------------------------

uno::Any SAL_CALL
    BootstrapContext::getValueByName( const OUString& Name )
        throw (uno::RuntimeException)
{
    OUString aShortName = makeShortName(Name);

    uno::Any aResult;

    bool bFound =
        lookupInArguments( aResult, aShortName ) ||
        lookupInContext  ( aResult, makeLongName(Name) ) ||
        lookupInBootstrap( aResult, aShortName );

    if (!bFound)
    {
        OSL_TRACE( "configmgr: Cannot find bootstrap data item: %s", OU2ASCII(Name) );
    }
    return aResult;
}
// ---------------------------------------------------------------------------

bool BootstrapContext::lookupInArguments( uno::Any & _rValue, const OUString& _aName )
{
    for (sal_Int32 nIx = 0; nIx < m_aArguments.getLength(); ++nIx)
    {
        if (m_aArguments[nIx].Name.equalsIgnoreAsciiCase(_aName))
        {
            _rValue = m_aArguments[nIx].Value;
            return true;
        }
    }
    return false;
}
// ---------------------------------------------------------------------------

bool BootstrapContext::lookupInContext( uno::Any & _rValue, const OUString& _aName )
{
    if (!m_xContext.is()) return false;

    uno::Any aCtxValue = m_xContext->getValueByName( _aName );

    if (aCtxValue.hasValue())
    {
        _rValue = aCtxValue;
        return true;
    }
    else
        return false;
}
// ---------------------------------------------------------------------------

bool BootstrapContext::lookupInBootstrap( uno::Any & _rValue, const OUString& _aName )
{
    OUString sResult;
    if ( rtl_bootstrap_get_from_handle( m_hBootstrapData, _aName.pData, &sResult.pData, 0) )
    {
        _rValue <<= sResult;
        return true;
    }
    else
        return false;
}
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
} // namespace config


