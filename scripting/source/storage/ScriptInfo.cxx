/*************************************************************************
 *
 *  $RCSfile: ScriptInfo.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: dfoster $ $Date: 2002-10-30 16:12:40 $
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
#include <cppuhelper/implementationentry.hxx>

#include <hash_map>

#include <osl/file.hxx>
#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <drafts/com/sun/star/script/framework/storage/XScriptStorageManager.hpp>

#include <util/util.hxx>
#include "ScriptInfo.hxx"

using namespace ::rtl;
using namespace com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::drafts::com::sun::star::script::framework;
using namespace ::drafts::com::sun::star::script::framework::storage;

namespace scripting_impl
{

typedef ::std::hash_map < ::rtl::OUString, css::uno::Any, ::rtl::OUStringHash,
    ::std::equal_to< ::rtl::OUString > > PropertySet_hash;

class PropertySetImpl : public ::cppu::WeakImplHelper1< css::beans::XPropertySet >
{

public:
    PropertySetImpl();
    ~PropertySetImpl();

// XPropertySet implementation
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw ( css::uno::RuntimeException );
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName,
        const css::uno::Any& aValue )
        throw ( css::beans::UnknownPropertyException,
            css::beans::PropertyVetoException,
            css::lang::IllegalArgumentException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException );
    virtual css::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName )
        throw ( css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException );
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
        const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener )
        throw ( css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException );
    virtual void SAL_CALL removePropertyChangeListener(
        const ::rtl::OUString& aPropertyName,
        const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener )
        throw ( css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException );
    virtual void SAL_CALL addVetoableChangeListener(
        const ::rtl::OUString& PropertyName,
        const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
        throw ( css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException );
    virtual void SAL_CALL removeVetoableChangeListener(
        const ::rtl::OUString& PropertyName,
        const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
        throw ( css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException );

private:
    css::uno::Reference< css::uno::XComponentContext > m_xContext;

    osl::Mutex m_mutex;
    PropertySet_hash m_propertyMap;
};

PropertySetImpl::PropertySetImpl()
{
   OSL_TRACE( "<PropertySetImpl ctor called\n" );
}

PropertySetImpl::~PropertySetImpl()
{
    OSL_TRACE( "<PropertySetImpl dtor called\n>" );
}

Reference< beans::XPropertySetInfo > SAL_CALL PropertySetImpl::getPropertySetInfo( )
    throw ( RuntimeException )
{
    return Reference< beans::XPropertySetInfo > (); // Not supported
}

void SAL_CALL PropertySetImpl::setPropertyValue( const ::rtl::OUString& aPropertyName,
    const Any& aValue )
    throw ( beans::UnknownPropertyException, beans::PropertyVetoException,
            lang::IllegalArgumentException, lang::WrappedTargetException,
            RuntimeException )
{
    ::osl::Guard< osl::Mutex > aGuard( m_mutex );
    m_propertyMap[ aPropertyName ] = aValue;
}

//*************************************************************************
Any SAL_CALL PropertySetImpl::getPropertyValue( const ::rtl::OUString& PropertyName )
    throw ( beans::UnknownPropertyException,
            lang::WrappedTargetException, RuntimeException )
{
    if ( m_propertyMap.find( PropertyName ) == m_propertyMap.end() )
    {
        throw RuntimeException(

            OUSTR( "PropertySetImpl::getPropertyValue: invalid PropertyName ").concat(
                PropertyName),
            Reference< XInterface >() );
    }

    ::osl::Guard< osl::Mutex > aGuard( m_mutex );
    Any returnValue = m_propertyMap[ PropertyName ];

    return returnValue;
}

//*************************************************************************
void SAL_CALL PropertySetImpl::addPropertyChangeListener(
    const ::rtl::OUString& aPropertyName,
    const Reference< beans::XPropertyChangeListener >& xListener )
    throw ( beans::UnknownPropertyException, lang::WrappedTargetException,
            RuntimeException )
{
    throw RuntimeException(
        OUSTR( "PropertySetImpl::addPropertyChangeListener: method not supported" ),
        Reference< XInterface >() );
}

//*************************************************************************
void SAL_CALL PropertySetImpl::removePropertyChangeListener(
    const ::rtl::OUString& aPropertyName,
    const Reference< beans::XPropertyChangeListener >& aListener )
    throw ( beans::UnknownPropertyException, lang::WrappedTargetException,
            RuntimeException )
{
    throw RuntimeException(
        OUSTR( "PropertySetImpl::removePropertyChangeListener: method not supported" ),
        Reference< XInterface >() );
}

//*************************************************************************
void SAL_CALL PropertySetImpl::addVetoableChangeListener(
    const ::rtl::OUString& PropertyName,
    const Reference< beans::XVetoableChangeListener >& aListener )
    throw ( beans::UnknownPropertyException, lang::WrappedTargetException,
            RuntimeException )
{
    throw RuntimeException(
        OUSTR( "PropertySetImpl::addVetoableChangeListener: method not supported" ),
        Reference< XInterface >() );
}

//*************************************************************************
void SAL_CALL PropertySetImpl::removeVetoableChangeListener(
    const ::rtl::OUString& PropertyName,
    const Reference< beans::XVetoableChangeListener >& aListener )
    throw ( beans::UnknownPropertyException, lang::WrappedTargetException,
            RuntimeException )
{
    throw RuntimeException(
        OUSTR( "PropertySetImpl::removeVetoableChangeListener: method not supported" ),
        Reference< XInterface >() );
}


const sal_Char* const SI_SERVICE_NAME="drafts.com.sun.star.script.framework.storage.ScriptInfo";
const sal_Char* const SI_IMPL_NAME="drafts.com.sun.star.script.framework.storage.ScriptInfo";

static OUString si_implName = OUString::createFromAscii( SI_IMPL_NAME );
static OUString si_serviceName = OUString::createFromAscii( SI_SERVICE_NAME );
static Sequence< OUString > si_serviceNames = Sequence< OUString >( &si_serviceName, 1 );

extern ::rtl_StandardModuleCount s_moduleCount;

static sal_Char docUriPrefix [] = "vnd.sun.star.pkg";

//*************************************************************************
ScriptInfo::ScriptInfo( const Reference< XComponentContext > & xContext )
        : m_xContext( xContext )
{
    OSL_TRACE( "< ++++++ ScriptInfo ctor called >\n" );
    validateXRef( m_xContext, "ScriptInfo::ScriptInfo : cannot get component context" );

    m_xMgr = m_xContext->getServiceManager();
    validateXRef( m_xMgr, "ScriptInfo::ScriptInfo : cannot get service manager" );

    s_moduleCount.modCnt.acquire( &s_moduleCount.modCnt );
}

ScriptInfo::ScriptInfo( const Reference< XComponentContext > & xContext,
        const ScriptData & scriptData, sal_Int32 storageID )
        : m_xContext( xContext ), m_scriptData( scriptData ),
            m_storageID( storageID )
{
    OSL_TRACE( "< ++++++ ScriptInfo ctor called >\n" );
    OSL_TRACE( "< ++++++ parcelURI=%s>\n",::rtl::OUStringToOString(m_scriptData.parcelURI , RTL_TEXTENCODING_ASCII_US ).pData->buffer );
    validateXRef( m_xContext, "ScriptInfo::ScriptInfo : cannot get component context" );

    m_xMgr = m_xContext->getServiceManager();
    validateXRef( m_xMgr, "ScriptInfo::ScriptInfo : cannot get service manager" );
    s_moduleCount.modCnt.acquire( &s_moduleCount.modCnt );
}
//*************************************************************************
ScriptInfo::~ScriptInfo()
{
    OSL_TRACE( "< ScriptInfo dtor called >\n" );
    s_moduleCount.modCnt.release( &s_moduleCount.modCnt );
}


//*************************************************************************
OUString SAL_CALL ScriptInfo::getLogicalName(  ) throw ( RuntimeException )
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_mutex );
    OSL_TRACE( "ScriptInfo::getLogicalName() " );
    return m_scriptData.logicalname;
}

//*************************************************************************
OUString SAL_CALL ScriptInfo::getDescription(  ) throw ( RuntimeException )
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_mutex );
    OUString rs_desc;
    // TDB need to determine locale here, hardcoded at the moment
    // to english

    OUString localeLang = OUString::createFromAscii( "en" );
    strpair_map::const_iterator str_it =
        m_scriptData.locales.find( localeLang );

    if( str_it == m_scriptData.locales.end() )
    {
        OSL_TRACE( "No description set in meta-data" );
        return rs_desc;
    }
    rs_desc = str_it->second.second;
    return rs_desc;
}

//*************************************************************************
OUString SAL_CALL ScriptInfo::getLanguage(  ) throw ( RuntimeException )
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_mutex );
    OSL_TRACE( "ScriptInfo::getLanguage() " );
    return m_scriptData.language;
}

//*************************************************************************
OUString SAL_CALL ScriptInfo::getScriptLocation()
    throw ( RuntimeException )
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_mutex );
    return OUString::createFromAscii( "Not done at the moment" );
}

//*************************************************************************
OUString SAL_CALL ScriptInfo::getFunctionName(  ) throw ( RuntimeException )
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_mutex );
    OSL_TRACE( "ScriptInfo::getFunctionName() " );
    return m_scriptData.functionname;
}

//*************************************************************************
OUString SAL_CALL ScriptInfo::getLocation(  ) throw ( RuntimeException )
{
    OUString location = OUString::createFromAscii( "need to be done" );

    return location;
}
//*************************************************************************
OUString SAL_CALL ScriptInfo::getParcelURI(  ) throw ( RuntimeException )
{
    return m_scriptData.parcelURI;
}

//*************************************************************************
Reference< beans::XPropertySet > SAL_CALL ScriptInfo::getLanguageProperties(  )
    throw ( RuntimeException )
{
    Reference< beans::XPropertySet > xPropSet = new PropertySetImpl();

    props_vec::const_iterator pv_it = m_scriptData.languagedepprops.begin();
    props_vec::const_iterator pv_itend = m_scriptData.languagedepprops.end();

    for( ; pv_it != pv_itend; ++pv_it )
    {
        try
        {
            xPropSet->setPropertyValue( pv_it->first, makeAny( pv_it->second ) );
        }
        catch( Exception& e )
        {
            OUString msg = OUSTR(
                "ScriptInfo::getLanguage caught exception while setting property," );
            msg = msg.concat( OUSTR( " PropertryName: " ) ).concat( pv_it->first );
            msg = msg.concat( OUSTR( " \nException message is: " ) );
            msg = msg.concat( e.Message );
            throw RuntimeException( msg , Reference< XInterface >() );
        }
    }

    return xPropSet;
}
//*************************************************************************
css::uno::Sequence< ::rtl::OUString > ScriptInfo::getFileSetNames()
throw ( css::uno::RuntimeException )
{
    OSL_TRACE("ScriptInfo::getFileSetNames");
    Sequence< OUString > results;
    filesets_map::iterator fsm_it = m_scriptData.filesets.begin();
    filesets_map::iterator fsm_itend = m_scriptData.filesets.end();
    if( fsm_it == fsm_itend )
    {
        OSL_TRACE( "ScriptInfo::getFileSetNames: no filesets" );
        return results;
    }
    results.realloc( m_scriptData.filesets.size() );
    for ( sal_Int32 count = 0; fsm_it != fsm_itend; ++fsm_it )
    {
        OUString fileSetName = fsm_it->first;
        OSL_TRACE( "ScriptInfo::getFileSetNames: adding name %s",
            ::rtl::OUStringToOString( fileSetName,
                RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        results[ count++ ] = fileSetName;
    }
    return results;
}
//*************************************************************************
css::uno::Sequence< ::rtl::OUString >
ScriptInfo::getFilesInFileSet( const ::rtl::OUString & fileSetName )
throw ( css::uno::RuntimeException )
{
    Sequence< OUString > results;
    filesets_map::iterator fsm_it = m_scriptData.filesets.find( fileSetName );
    filesets_map::iterator fsm_itend = m_scriptData.filesets.end();
    if( fsm_it == fsm_itend )
    {
        OSL_TRACE( "ScriptInfo::getFilesInFileSet: no fileset named %s",
            ::rtl::OUStringToOString( fileSetName,
                RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        return results;
    }

    strpairvec_map files = fsm_it->second.second;
    strpairvec_map::iterator spvm_it = files.begin();
    strpairvec_map::iterator spvm_itend = files.end();
    if( spvm_it != spvm_itend )
    {
        OSL_TRACE( "ScriptInfo::getFilesInFileSet: no files in fileset %s",
            ::rtl::OUStringToOString( fileSetName,
                RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        return results;
    }
    results.realloc( files.size() );
    for( sal_Int32 count = 0; spvm_it != spvm_itend ; ++spvm_it )
    {
        OUString fileName = fsm_it->first;
        OSL_TRACE( "ScriptInfo::getFilesInFileSet: adding file %s",
            ::rtl::OUStringToOString( fileName,
                RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        results[ count++ ] = fileSetName;
    }
    return results;
}
//*************************************************************************
OUString SAL_CALL ScriptInfo::getImplementationName(  )
throw( RuntimeException )
{
    return si_implName;
}

//*************************************************************************
sal_Bool SAL_CALL ScriptInfo::supportsService( const OUString& serviceName )
throw( RuntimeException )
{
    OUString const * pNames = si_serviceNames.getConstArray();
    for ( sal_Int32 nPos = si_serviceNames.getLength(); nPos--; )
    {
        if ( serviceName.equals( pNames[ nPos ] ) )
        {
            return sal_True;
        }
    }
    return sal_False;
}

//*************************************************************************
Sequence<OUString> SAL_CALL ScriptInfo::getSupportedServiceNames(  )
throw( RuntimeException )
{
    return si_serviceNames;
}
//*************************************************************************
Reference< XInterface > SAL_CALL si_create(
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
