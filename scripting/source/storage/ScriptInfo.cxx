/*************************************************************************
 *
 *  $RCSfile: ScriptInfo.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dsherwin $ $Date: 2002-09-23 16:30:52 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include <stdio.h>

#include <cppuhelper/implementationentry.hxx>

#include <util/util.hxx>
#include <ScriptInfo.hxx>
#include <osl/file.hxx>

#ifndef _DRAFTS_COM_SUN_STAR_SCRIPT_FRAMEWORK_STORAGE_XSCRIPTSTORAGEMANAGER_HPP_
#include <drafts/com/sun/star/script/framework/storage/XScriptStorageManager.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_SCRIPT_FRAMEWORK_STORAGE_XPARCELINVOCATIONPREP_HPP_
#include <drafts/com/sun/star/script/framework/storage/XParcelInvocationPrep.hpp>
#endif

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace ::drafts::com::sun::star::script::framework;
using namespace ::drafts::com::sun::star::script::framework::storage;

namespace scripting_impl
{

const sal_Char* const SI_SERVICE_NAME="drafts.com.sun.star.script.framework.storage.ScriptInfo";
const sal_Char* const SI_IMPL_NAME="drafts.com.sun.star.script.framework.storage.ScriptInfo";

static OUString si_implName = OUString::createFromAscii(SI_IMPL_NAME);
static OUString si_serviceName = OUString::createFromAscii(SI_SERVICE_NAME);
static Sequence< OUString > si_serviceNames = Sequence< OUString >( &si_serviceName, 1 );

extern ::rtl_StandardModuleCount s_moduleCount;

static sal_Char docUriPrefix [] = "vnd.sun.star.pkg";

//*************************************************************************
ScriptInfo::ScriptInfo( const Reference< XComponentContext > & xContext )
        : m_xContext( xContext )
{
    OSL_TRACE( "< ++++++ ScriptInfo ctor called >\n" );
    s_moduleCount.modCnt.acquire( &s_moduleCount.modCnt );
}

//*************************************************************************
ScriptInfo::~ScriptInfo()
{
    OSL_TRACE( "< ScriptInfo dtor called >\n" );
    s_moduleCount.modCnt.release( &s_moduleCount.modCnt );
}

//*************************************************************************
void ScriptInfo::initialize(Sequence <Any> const & args)
throw (RuntimeException, Exception)
{
    try
    {
        if (((args[0] >>= m_scriptImplInfo) == sal_False) ||
        ((args[1] >>= m_storageID) == sal_False)) {
        throw RuntimeException(OUSTR("ScriptInfo: initialize(): "), Reference< XInterface >());
        }
    }
    catch (Exception &e)
    {
    throw RuntimeException(OUSTR("ScriptInfo: initialize(): ") + e.Message, Reference< XInterface >());
    }
}

//*************************************************************************
OUString SAL_CALL ScriptInfo::getLogicalName(  ) throw (RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_mutex );
    return m_scriptImplInfo.logicalName;
}

//*************************************************************************
void SAL_CALL ScriptInfo::setLogicalName( const OUString& name ) throw (RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_mutex );
    m_scriptImplInfo.logicalName = name;
}

//*************************************************************************
OUString SAL_CALL ScriptInfo::getDescription(  ) throw (RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_mutex );
    return m_scriptImplInfo.scriptDescription;
}

//*************************************************************************
void SAL_CALL ScriptInfo::setDescription( const OUString& desc ) throw (RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_mutex );
    m_scriptImplInfo.scriptDescription = desc;
}

//*************************************************************************
OUString SAL_CALL ScriptInfo::getLanguage(  ) throw (RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_mutex );
    return m_scriptImplInfo.scriptLanguage;
}

//*************************************************************************
void SAL_CALL ScriptInfo::setLanguage( const OUString& language ) throw (RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_mutex );
    m_scriptImplInfo.scriptLanguage = language;
}

//*************************************************************************
OUString SAL_CALL ScriptInfo::getScriptLocation()
    throw ( RuntimeException )
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_mutex );
    return m_scriptImplInfo.scriptLocation;
}

//*************************************************************************
sal_Bool SAL_CALL ScriptInfo::hasSource(  ) throw (RuntimeException)
{
    return m_scriptImplInfo.scriptSource;
}

//*************************************************************************
OUString SAL_CALL ScriptInfo::getLanguageSpecificName(  ) throw (RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_mutex );
    return m_scriptImplInfo.functionName;
}

//*************************************************************************
void SAL_CALL ScriptInfo::setLanguageSpecificName( const OUString& langName ) throw (RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_mutex );
    m_scriptImplInfo.functionName = langName;
}

//*************************************************************************
OUString SAL_CALL ScriptInfo::getRoot(  ) throw (RuntimeException)
{
    return m_scriptImplInfo.scriptRoot;
}

//*************************************************************************
Sequence< OUString > SAL_CALL ScriptInfo::getDependencies(  ) throw (RuntimeException)
{
    storage::ScriptDepFile *pArray = m_scriptImplInfo.scriptDependencies.getArray();
    int len = m_scriptImplInfo.scriptDependencies.getLength();
    Sequence< OUString > r_deps(len);
    for(int i = 0; i < len; i++)
    {
        r_deps[i] = pArray[i].fileName;
    }

    return r_deps;
}

//*************************************************************************
OUString SAL_CALL ScriptInfo::getLocation(  ) throw (RuntimeException)
{
    OUString location = OUString::createFromAscii("need to be done");

    return location;
}

//*************************************************************************
Reference< XPropertySet > SAL_CALL ScriptInfo::extraProperties(  ) throw (RuntimeException)
{
    Reference <XPropertySet> x;

    return x;
}

//*************************************************************************
OUString SAL_CALL ScriptInfo::getImplementationName(  )
throw(RuntimeException)
{
    return si_implName;
}

//*************************************************************************
/**
 *  This function prepares the script for invocation and returns the full path
 *  to the prepared parcel folder
 *
 */
::rtl::OUString SAL_CALL ScriptInfo::prepareForInvocation() throw(RuntimeException)
{
    OSL_TRACE("******* In ScriptInfo::prepareForInvocation() *************\n");
    try
    {
        if (m_scriptImplInfo.parcelURI.compareToAscii(docUriPrefix, 16) != 0) {
        return m_scriptImplInfo.parcelURI;
        } 

        validateXRef(m_xContext, "ScriptInfo::prepareForInvocation(): invalid context");
        Any aAny=m_xContext->getValueByName(
        OUString::createFromAscii(
        "/singletons/drafts.com.sun.star.script.framework.storage.theScriptStorageManager"));
        Reference <XInterface> xx;
        if ((aAny >>= xx) == sal_False) {
        throw RuntimeException(OUSTR("ScriptInfo::prepareForInvocation(): could not get ScriptStorageManager"), Reference< XInterface >());
        }

        validateXRef(xx, "ScriptInfo::prepareForInvocation(): could not get XInterface");
        Reference<XScriptStorageManager> xSSM(xx,UNO_QUERY);
        validateXRef(xSSM, "ScriptInfo::prepareForInvocation(): could not get XScriptStorageManager");
        xx = xSSM->getScriptStorage(m_storageID);
        validateXRef(xx, "ScriptInfo::prepareForInvocation(): could not get XInterface");
        Reference <XParcelInvocationPrep> xPIP(xx, UNO_QUERY);
        validateXRef(xPIP, "ScriptInfo::prepareForInvocation(): could not get XParcelInvocationPrep");
        return xPIP->prepareForInvocation(m_scriptImplInfo.parcelURI);
    }
    catch(RuntimeException &e)
    {
        OUString temp = OUSTR(
            "ScriptInfo::prepareForInvocation RuntimeException: ");
        throw RuntimeException(temp.concat(e.Message),
                               Reference<XInterface> ());
    }
#ifdef _DEBUG
    catch ( ... )
    {
        throw RuntimeException(OUSTR(
            "ScriptInfo::prepareForInvocation UnknownException: "),
                         Reference<XInterface> ());
    }
#endif
}

//*************************************************************************
sal_Bool SAL_CALL ScriptInfo::supportsService( const OUString& serviceName )
throw(RuntimeException)
{
    OUString const * pNames = si_serviceNames.getConstArray();
    for ( sal_Int32 nPos = si_serviceNames.getLength(); nPos--; )
    {
        if (serviceName.equals( pNames[ nPos ] ))
        {
            return sal_True;
        }
    }
    return sal_False;
}

//*************************************************************************
Sequence<OUString> SAL_CALL ScriptInfo::getSupportedServiceNames(  )
throw(RuntimeException)
{
    return si_serviceNames;
}
//*************************************************************************
Reference<XInterface> SAL_CALL si_create(
    const Reference< XComponentContext > & xCompC )
{
    return (cppu::OWeakObject *)new ScriptInfo( xCompC );
}

//*************************************************************************
Sequence<OUString> si_getSupportedServiceNames(  )
SAL_THROW( () )
{
    return si_serviceNames;
}

//*************************************************************************
OUString si_getImplementationName(  )
SAL_THROW( () )
{
    return si_implName;
}
}
