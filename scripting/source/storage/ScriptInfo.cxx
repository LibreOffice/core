/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_scripting.hxx"
#include <cppuhelper/implementationentry.hxx>

#include <boost/unordered_map.hpp>

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

typedef ::boost::unordered_map < ::rtl::OUString, css::uno::Any, ::rtl::OUStringHash,
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
    friend class ScriptInfo;

    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    void PropertySetImpl::privateSetPropertyValue( const ::rtl::OUString& aPropertyName, const Any& aValue )
    throw ( beans::UnknownPropertyException, beans::PropertyVetoException,
            lang::IllegalArgumentException, lang::WrappedTargetException,
            RuntimeException );

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
    throw RuntimeException(
        OUSTR( "PropertySetImpl::setPropertyValue: method not supported. Read-only PropertySet" ),
        Reference< XInterface >() );
}

void PropertySetImpl::privateSetPropertyValue( const ::rtl::OUString& aPropertyName,
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


//*************************************************************************
ScriptInfo::ScriptInfo( const ScriptData & scriptData, sal_Int32 storageID )
        : m_scriptData( scriptData ), m_storageID( storageID )
{
    OSL_TRACE( "< ++++++ ScriptInfo ctor called >\n" );
    OSL_TRACE( "< ++++++ parcelURI=%s>\n",::rtl::OUStringToOString(m_scriptData.parcelURI , RTL_TEXTENCODING_ASCII_US ).pData->buffer );
}
//*************************************************************************
ScriptInfo::~ScriptInfo()
{
    OSL_TRACE( "< ScriptInfo dtor called >\n" );
}
//*************************************************************************
OUString SAL_CALL ScriptInfo::getLogicalName(  ) throw ( RuntimeException )
{
    OSL_TRACE( "ScriptInfo::getLogicalName() " );
    return m_scriptData.logicalname;
}

//*************************************************************************
OUString SAL_CALL ScriptInfo::getDescription(  ) throw ( RuntimeException )
{
    OUString rs_desc;
    // TDB need to determine locale here, hardcoded at the moment
    // to english

    OUString localeLang(RTL_CONSTASCII_USTRINGPARAM("en"));
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
    OSL_TRACE( "ScriptInfo::getLanguage() " );
    return m_scriptData.language;
}

//*************************************************************************
OUString SAL_CALL ScriptInfo::getFunctionName(  ) throw ( RuntimeException )
{
    OSL_TRACE( "ScriptInfo::getFunctionName() " );
    return m_scriptData.functionname;
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
    PropertySetImpl* propSetImpl = new PropertySetImpl();
    Reference< beans::XPropertySet > xPropSet = propSetImpl;

    props_vec::const_iterator pv_it = m_scriptData.languagedepprops.begin();
    props_vec::const_iterator pv_itend = m_scriptData.languagedepprops.end();

    for( ; pv_it != pv_itend; ++pv_it )
    {
        try
        {
            propSetImpl->privateSetPropertyValue( pv_it->first, makeAny( pv_it->second ) );
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
css::uno::Sequence< ::rtl::OUString > SAL_CALL ScriptInfo::getFileSetNames()
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
css::uno::Sequence< ::rtl::OUString > SAL_CALL
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
    if( spvm_it == spvm_itend )
    {
        OSL_TRACE( "ScriptInfo::getFilesInFileSet: no files in fileset %s",
            ::rtl::OUStringToOString( fileSetName,
                RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        return results;
    }
    results.realloc( files.size() );
    for( sal_Int32 count = 0; spvm_it != spvm_itend ; ++spvm_it )
    {
        OUString fileName = spvm_it->first;
        OSL_TRACE( "ScriptInfo::getFilesInFileSet: adding file %s",
            ::rtl::OUStringToOString( fileName,
                RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        results[ count++ ] = fileName;
    }
    return results;
}
//*************************************************************************
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
