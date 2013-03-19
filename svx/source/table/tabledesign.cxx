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


#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <comphelper/serviceinfohelper.hxx>

#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase6.hxx>
#include <cppuhelper/implbase7.hxx>
#include <cppuhelper/interfacecontainer.h>

#include "svx/unoprov.hxx"
#include "svx/sdr/table/tabledesign.hxx"
#include "svx/dialmgr.hxx"
#include "svx/dialogs.hrc"

#include "celltypes.hxx"

#include <vector>
#include <map>

#include <boost/bind.hpp>

// -----------------------------------------------------------------------------

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::container;

using ::rtl::OUString;
using ::osl::MutexGuard;
using ::osl::ClearableMutexGuard;
using ::cppu::OInterfaceContainerHelper;

namespace sdr { namespace table {

typedef std::map< OUString, sal_Int32 > CellStyleNameMap;

typedef ::cppu::WeakComponentImplHelper6< XStyle, XNameReplace, XServiceInfo, XIndexAccess, XModifyBroadcaster, XModifyListener > TableDesignStyleBase;

class TableDesignStyle : private ::cppu::BaseMutex, public TableDesignStyleBase
{
public:
    TableDesignStyle();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(RuntimeException);

    // XStyle
    virtual ::sal_Bool SAL_CALL isUserDefined() throw (RuntimeException);
    virtual ::sal_Bool SAL_CALL isInUse() throw (RuntimeException);
    virtual OUString SAL_CALL getParentStyle() throw (RuntimeException);
    virtual void SAL_CALL setParentStyle( const OUString& aParentStyle ) throw (NoSuchElementException, RuntimeException);

    // XNamed
    virtual OUString SAL_CALL getName() throw (RuntimeException);
    virtual void SAL_CALL setName( const OUString& aName ) throw (RuntimeException);

    // XNameAccess
    virtual Any SAL_CALL getByName( const OUString& aName ) throw(NoSuchElementException, WrappedTargetException, RuntimeException);
    virtual Sequence< OUString > SAL_CALL getElementNames() throw(RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw(RuntimeException);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw(RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(RuntimeException);

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(RuntimeException) ;
    virtual Any SAL_CALL getByIndex( sal_Int32 Index ) throw(IndexOutOfBoundsException, WrappedTargetException, RuntimeException);

    // XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const Any& aElement ) throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException);

    // XModifyBroadcaster
    virtual void SAL_CALL addModifyListener( const Reference< XModifyListener >& aListener ) throw (RuntimeException);
    virtual void SAL_CALL removeModifyListener( const Reference< XModifyListener >& aListener ) throw (RuntimeException);

    // XModifyListener
    virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

    void notifyModifyListener();

    // this function is called upon disposing the component
    virtual void SAL_CALL disposing();

    static const CellStyleNameMap& getCellStyleNameMap();

    OUString msName;
    Reference< XStyle > maCellStyles[style_count];
};

typedef std::vector< Reference< XStyle > > TableDesignStyleVector;

class TableDesignFamily : public ::cppu::WeakImplHelper7< XNameContainer, XNamed, XIndexAccess, XSingleServiceFactory,  XServiceInfo, XComponent, XPropertySet >
{
public:
    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(RuntimeException);

    // XNamed
    virtual OUString SAL_CALL getName(  ) throw (RuntimeException);
    virtual void SAL_CALL setName( const OUString& aName ) throw (RuntimeException);

    // XNameAccess
    virtual Any SAL_CALL getByName( const OUString& aName ) throw(NoSuchElementException, WrappedTargetException, RuntimeException);
    virtual Sequence< OUString > SAL_CALL getElementNames() throw(RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw(RuntimeException);

    // XElementAccess
    virtual Type SAL_CALL getElementType() throw(RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(RuntimeException);

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(RuntimeException) ;
    virtual Any SAL_CALL getByIndex( sal_Int32 Index ) throw(IndexOutOfBoundsException, WrappedTargetException, RuntimeException);

    // XNameContainer
    virtual void SAL_CALL insertByName( const OUString& aName, const Any& aElement ) throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException);
    virtual void SAL_CALL removeByName( const OUString& Name ) throw(NoSuchElementException, WrappedTargetException, RuntimeException);

    // XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const Any& aElement ) throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException);

    // XSingleServiceFactory
    virtual Reference< XInterface > SAL_CALL createInstance(  ) throw(Exception, RuntimeException);
    virtual Reference< XInterface > SAL_CALL createInstanceWithArguments( const Sequence< Any >& aArguments ) throw(Exception, RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (RuntimeException);
    virtual void SAL_CALL addEventListener( const Reference< XEventListener >& xListener ) throw (RuntimeException);
    virtual void SAL_CALL removeEventListener( const Reference< XEventListener >& aListener ) throw (RuntimeException);

    // XPropertySet
    virtual Reference<XPropertySetInfo> SAL_CALL getPropertySetInfo() throw (RuntimeException);
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const Any& aValue ) throw (UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException);
    virtual Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const Reference<XPropertyChangeListener>& xListener ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const Reference<XPropertyChangeListener>& aListener ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener(const OUString& PropertyName, const Reference<XVetoableChangeListener>& aListener ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener(const OUString& PropertyName,const Reference<XVetoableChangeListener>&aListener ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException);

    TableDesignStyleVector  maDesigns;
};

//------------------------------------------------------------------------
// TableDesignStyle
//------------------------------------------------------------------------

TableDesignStyle::TableDesignStyle()
: TableDesignStyleBase(m_aMutex)
{
}

const CellStyleNameMap& TableDesignStyle::getCellStyleNameMap()
{
    static CellStyleNameMap aMap;
    if( aMap.empty() )
    {
        CellStyleNameMap aNewMap;
        aNewMap[ OUString( "first-row" ) ] = first_row_style;
        aNewMap[ OUString( "last-row" ) ] = last_row_style;
        aNewMap[ OUString( "first-column" ) ] = first_column_style;
        aNewMap[ OUString( "last-column" ) ] = last_column_style;
        aNewMap[ OUString( "body" ) ] = body_style;
        aNewMap[ OUString( "even-rows" ) ] = even_rows_style;
        aNewMap[ OUString( "odd-rows" ) ] = odd_rows_style;
        aNewMap[ OUString( "even-columns" ) ] = even_columns_style;
        aNewMap[ OUString( "odd-columns" ) ] = odd_columns_style;
        aNewMap[ OUString( "background" ) ] = background_style;
        aMap.swap( aNewMap );
    }

    return aMap;
}

// ----------------------------------------------------------
// XServiceInfo
// ----------------------------------------------------------

OUString SAL_CALL TableDesignStyle::getImplementationName() throw(RuntimeException)
{
    return OUString("TableDesignStyle");
}

// ----------------------------------------------------------

sal_Bool SAL_CALL TableDesignStyle::supportsService( const OUString& ServiceName ) throw(RuntimeException)
{
    return comphelper::ServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames() );
}

// ----------------------------------------------------------

Sequence< OUString > SAL_CALL TableDesignStyle::getSupportedServiceNames() throw(RuntimeException)
{
    OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.style.Style") );
    Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

// ----------------------------------------------------------
// XStyle
// ----------------------------------------------------------

sal_Bool SAL_CALL TableDesignStyle::isUserDefined() throw (RuntimeException)
{
    return sal_False;
}

// ----------------------------------------------------------

sal_Bool SAL_CALL TableDesignStyle::isInUse() throw (RuntimeException)
{
    ClearableMutexGuard aGuard( rBHelper.rMutex );
    OInterfaceContainerHelper * pContainer = rBHelper.getContainer( XModifyListener::static_type() );
    if( pContainer )
    {
        Sequence< Reference< XInterface > > aListener( pContainer->getElements() );
        aGuard.clear();

        sal_Int32 nIndex = aListener.getLength();
        while( --nIndex >= 0 )
        {
            TableDesignUser* pUser = dynamic_cast< TableDesignUser* >( aListener[nIndex].get() );
            if( pUser && pUser->isInUse() )
                return sal_True;
        }
    }
    return sal_False;
}

// ----------------------------------------------------------

OUString SAL_CALL TableDesignStyle::getParentStyle() throw (RuntimeException)
{
    return OUString();
}

// ----------------------------------------------------------

void SAL_CALL TableDesignStyle::setParentStyle( const OUString& ) throw (NoSuchElementException, RuntimeException)
{
}

// ----------------------------------------------------------
// XNamed
// ----------------------------------------------------------

OUString SAL_CALL TableDesignStyle::getName() throw (RuntimeException)
{
    return msName;
}

// ----------------------------------------------------------

void SAL_CALL TableDesignStyle::setName( const OUString& rName ) throw (RuntimeException)
{
    msName = rName;
}

// ----------------------------------------------------------
// XNameAccess
// ----------------------------------------------------------

Any SAL_CALL TableDesignStyle::getByName( const OUString& rName ) throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;

    const CellStyleNameMap& rMap = getCellStyleNameMap();

    CellStyleNameMap::const_iterator iter = rMap.find( rName );
    if( iter == rMap.end() )
        throw NoSuchElementException();

    return Any( maCellStyles[(*iter).second] );
}

// ----------------------------------------------------------

Sequence< OUString > SAL_CALL TableDesignStyle::getElementNames() throw(RuntimeException)
{
    SolarMutexGuard aGuard;

    const CellStyleNameMap& rMap = getCellStyleNameMap();
    Sequence< OUString > aRet( rMap.size() );
    OUString* pName = aRet.getArray();

    CellStyleNameMap::const_iterator iter = rMap.begin();
    while( iter != rMap.end() )
        *pName++ = (*iter++).first;

    return aRet;
}

// ----------------------------------------------------------

sal_Bool SAL_CALL TableDesignStyle::hasByName( const OUString& rName )  throw(RuntimeException)
{
    SolarMutexGuard aGuard;

    const CellStyleNameMap& rMap = getCellStyleNameMap();

    CellStyleNameMap::const_iterator iter = rMap.find( rName );
    return ( iter != rMap.end() ) ? sal_True : sal_False;
}

// ----------------------------------------------------------
// XElementAccess
// ----------------------------------------------------------

Type SAL_CALL TableDesignStyle::getElementType() throw(RuntimeException)
{
    return XStyle::static_type();
}

// ----------------------------------------------------------

sal_Bool SAL_CALL TableDesignStyle::hasElements() throw(RuntimeException)
{
    return sal_True;
}

// ----------------------------------------------------------
// XIndexAccess
// ----------------------------------------------------------

sal_Int32 SAL_CALL TableDesignStyle::getCount() throw(RuntimeException)
{
    return style_count;
}

// ----------------------------------------------------------

Any SAL_CALL TableDesignStyle::getByIndex( sal_Int32 Index ) throw(IndexOutOfBoundsException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;

    if( (Index < 0) || (Index >= style_count) )
        throw IndexOutOfBoundsException();

    return Any( maCellStyles[Index] );
}

// ----------------------------------------------------------
// XNameReplace
// ----------------------------------------------------------

void SAL_CALL TableDesignStyle::replaceByName( const OUString& rName, const Any& aElement ) throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;

    const CellStyleNameMap& rMap = getCellStyleNameMap();
    CellStyleNameMap::const_iterator iter = rMap.find( rName );
    if( iter == rMap.end() )
        throw NoSuchElementException();


    Reference< XStyle > xNewStyle;
    if( !(aElement >>= xNewStyle) )
        throw IllegalArgumentException();

    const sal_Int32 nIndex = (*iter).second;

    Reference< XStyle > xOldStyle( maCellStyles[nIndex] );

    if( xNewStyle != xOldStyle )
    {
        Reference< XModifyListener > xListener( this );

        // end listening to old style, if possible
        Reference< XModifyBroadcaster > xOldBroadcaster( xOldStyle, UNO_QUERY );
        if( xOldBroadcaster.is() )
            xOldBroadcaster->removeModifyListener( xListener );

        // start listening to new style, if possible
        Reference< XModifyBroadcaster > xNewBroadcaster( xNewStyle, UNO_QUERY );
        if( xNewBroadcaster.is() )
            xNewBroadcaster->addModifyListener( xListener );

        maCellStyles[nIndex] = xNewStyle;
    }
}

// ----------------------------------------------------------
// XComponent
// ----------------------------------------------------------

void SAL_CALL TableDesignStyle::disposing()
{
    for( sal_Int32 nIndex = 0; nIndex < style_count; nIndex++ )
        maCellStyles[nIndex].clear();
}

//------------------------------------------------------------------------
// XModifyBroadcaster
//------------------------------------------------------------------------

void SAL_CALL TableDesignStyle::addModifyListener( const Reference< XModifyListener >& xListener ) throw (RuntimeException)
{
    ClearableMutexGuard aGuard( rBHelper.rMutex );
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        aGuard.clear();
        EventObject aEvt( static_cast< OWeakObject * >( this ) );
        xListener->disposing( aEvt );
    }
    else
    {
        rBHelper.addListener( XModifyListener::static_type(), xListener );
    }
}

//------------------------------------------------------------------------

void SAL_CALL TableDesignStyle::removeModifyListener( const Reference< XModifyListener >& xListener ) throw (RuntimeException)
{
    rBHelper.removeListener( XModifyListener::static_type(), xListener );
}

//------------------------------------------------------------------------

void TableDesignStyle::notifyModifyListener()
{
    MutexGuard aGuard( rBHelper.rMutex );

    OInterfaceContainerHelper * pContainer = rBHelper.getContainer( XModifyListener::static_type() );
    if( pContainer )
    {
        EventObject aEvt( static_cast< OWeakObject * >( this ) );
        pContainer->forEach<XModifyListener>( boost::bind( &XModifyListener::modified, _1, boost::cref( aEvt ) ) );
    }
}

//------------------------------------------------------------------------
// XModifyListener
//------------------------------------------------------------------------

// if we get a modify hint from a style, notify all registered XModifyListener
void SAL_CALL TableDesignStyle::modified( const ::com::sun::star::lang::EventObject& ) throw (::com::sun::star::uno::RuntimeException)
{
    notifyModifyListener();
}

//------------------------------------------------------------------------

void SAL_CALL TableDesignStyle::disposing( const ::com::sun::star::lang::EventObject& ) throw (::com::sun::star::uno::RuntimeException)
{
}

//------------------------------------------------------------------------
// TableStyle
//------------------------------------------------------------------------

// ----------------------------------------------------------
// XServiceInfo
// ----------------------------------------------------------

OUString SAL_CALL TableDesignFamily::getImplementationName() throw(RuntimeException)
{
    return OUString("TableDesignFamily");
}

// ----------------------------------------------------------

sal_Bool SAL_CALL TableDesignFamily::supportsService( const OUString& ServiceName ) throw(RuntimeException)
{
    return comphelper::ServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames() );
}

// ----------------------------------------------------------

Sequence< OUString > SAL_CALL TableDesignFamily::getSupportedServiceNames() throw(RuntimeException)
{
    OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.style.StyleFamily") );
    Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

// ----------------------------------------------------------
// XNamed
// ----------------------------------------------------------

OUString SAL_CALL TableDesignFamily::getName() throw (RuntimeException)
{
    return OUString( "table" );
}

// ----------------------------------------------------------

void SAL_CALL TableDesignFamily::setName( const OUString& ) throw (RuntimeException)
{
}

// ----------------------------------------------------------
// XNameAccess
// ----------------------------------------------------------

Any SAL_CALL TableDesignFamily::getByName( const OUString& rName ) throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;

    for( TableDesignStyleVector::iterator iter( maDesigns.begin() ); iter != maDesigns.end(); iter++ )
    {
        if( (*iter)->getName() == rName )
            return Any( (*iter) );
    }

    throw NoSuchElementException();
}

// ----------------------------------------------------------

Sequence< OUString > SAL_CALL TableDesignFamily::getElementNames() throw(RuntimeException)
{
    SolarMutexGuard aGuard;

    Sequence< OUString > aRet( maDesigns.size() );
    OUString* pNames = aRet.getArray();

    for( TableDesignStyleVector::iterator iter( maDesigns.begin() ); iter != maDesigns.end(); iter++ )
        *pNames++ = (*iter)->getName();

    return aRet;
}

// ----------------------------------------------------------

sal_Bool SAL_CALL TableDesignFamily::hasByName( const OUString& aName ) throw(RuntimeException)
{
    SolarMutexGuard aGuard;

    for( TableDesignStyleVector::iterator iter( maDesigns.begin() ); iter != maDesigns.end(); iter++ )
        if( (*iter)->getName() == aName )
            return sal_True;

    return sal_False;
}

// ----------------------------------------------------------
// XElementAccess
// ----------------------------------------------------------

Type SAL_CALL TableDesignFamily::getElementType() throw(RuntimeException)
{
    return XStyle::static_type();
}

// ----------------------------------------------------------

sal_Bool SAL_CALL TableDesignFamily::hasElements() throw(RuntimeException)
{
    SolarMutexGuard aGuard;

    return maDesigns.empty() ? sal_False : sal_True;
}

// ----------------------------------------------------------
// XIndexAccess
// ----------------------------------------------------------

sal_Int32 SAL_CALL TableDesignFamily::getCount() throw(RuntimeException)
{
    SolarMutexGuard aGuard;

    return sal::static_int_cast< sal_Int32 >( maDesigns.size() );
}

// ----------------------------------------------------------

Any SAL_CALL TableDesignFamily::getByIndex( sal_Int32 Index ) throw(IndexOutOfBoundsException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;

    if( (Index >= 0) && (Index < sal::static_int_cast< sal_Int32 >( maDesigns.size() ) ) )
        return Any( maDesigns[Index] );

    throw IndexOutOfBoundsException();
}

// ----------------------------------------------------------
// XNameContainer
// ----------------------------------------------------------

void SAL_CALL TableDesignFamily::insertByName( const OUString& rName, const Any& rElement ) throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;

    Reference< XStyle > xStyle( rElement, UNO_QUERY );
    if( !xStyle.is() )
        throw IllegalArgumentException();

    xStyle->setName( rName );
    for( TableDesignStyleVector::iterator iter( maDesigns.begin() ); iter != maDesigns.end(); iter++ )
        if( (*iter)->getName() == rName )
            throw ElementExistException();

    maDesigns.push_back( xStyle );
}

// ----------------------------------------------------------

void SAL_CALL TableDesignFamily::removeByName( const OUString& rName ) throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;

    for( TableDesignStyleVector::iterator iter( maDesigns.begin() ); iter != maDesigns.end(); iter++ )
    {
        if( (*iter)->getName() == rName )
        {
            maDesigns.erase( iter );
            return;
        }
    }


    throw NoSuchElementException();
}

// ----------------------------------------------------------
// XNameReplace
// ----------------------------------------------------------

void SAL_CALL TableDesignFamily::replaceByName( const OUString& rName, const Any& aElement ) throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;

    Reference< XStyle > xStyle( aElement, UNO_QUERY );
    if( !xStyle.is() )
        throw IllegalArgumentException();

    for( TableDesignStyleVector::iterator iter( maDesigns.begin() ); iter != maDesigns.end(); iter++ )
    {
        if( (*iter)->getName() == rName )
        {
            (*iter) = xStyle;
            xStyle->setName( rName );
            return;
        }
    }

    throw NoSuchElementException();
}

// ----------------------------------------------------------
// XSingleServiceFactory
// ----------------------------------------------------------

Reference< XInterface > SAL_CALL TableDesignFamily::createInstance() throw(Exception, RuntimeException)
{
    SolarMutexGuard aGuard;

    return Reference< XInterface >( static_cast< XStyle* >( new TableDesignStyle ) );
}

// ----------------------------------------------------------

Reference< XInterface > SAL_CALL TableDesignFamily::createInstanceWithArguments( const Sequence< Any >&  ) throw(Exception, RuntimeException)
{
    return createInstance();
}

// ----------------------------------------------------------
// XComponent
// ----------------------------------------------------------

void SAL_CALL TableDesignFamily::dispose(  ) throw (RuntimeException)
{
    TableDesignStyleVector aDesigns;
    aDesigns.swap( maDesigns );

    for( TableDesignStyleVector::iterator iter( aDesigns.begin() ); iter != aDesigns.end(); ++iter )
    {
        Reference< XComponent > xComp( (*iter), UNO_QUERY );
        if( xComp.is() )
            xComp->dispose();
    }
}

// ----------------------------------------------------------

void SAL_CALL TableDesignFamily::addEventListener( const Reference< XEventListener >&  ) throw (RuntimeException)
{
}

// ----------------------------------------------------------

void SAL_CALL TableDesignFamily::removeEventListener( const Reference< XEventListener >&  ) throw (RuntimeException)
{
}

// ----------------------------------------------------------
// XPropertySet
// ----------------------------------------------------------

Reference<XPropertySetInfo> TableDesignFamily::getPropertySetInfo() throw (RuntimeException)
{
    OSL_FAIL( "###unexpected!" );
    return Reference<XPropertySetInfo>();
}

// ----------------------------------------------------------

void TableDesignFamily::setPropertyValue( const OUString& , const Any&  ) throw (UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    OSL_FAIL( "###unexpected!" );
}

// ----------------------------------------------------------

Any TableDesignFamily::getPropertyValue( const OUString& PropertyName ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    if ( PropertyName == "DisplayName" )
    {
        OUString sDisplayName( SVX_RESSTR( RID_SVXSTR_STYLEFAMILY_TABLEDESIGN ) );
        return Any( sDisplayName );
    }
    else
    {
        throw UnknownPropertyException( OUString("unknown property: ") + PropertyName, static_cast<OWeakObject *>(this) );
    }
}

// ----------------------------------------------------------

void TableDesignFamily::addPropertyChangeListener( const OUString& , const Reference<XPropertyChangeListener>&  ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    OSL_FAIL( "###unexpected!" );
}

// ----------------------------------------------------------

void TableDesignFamily::removePropertyChangeListener( const OUString& , const Reference<XPropertyChangeListener>&  ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    OSL_FAIL( "###unexpected!" );
}

// ----------------------------------------------------------

void TableDesignFamily::addVetoableChangeListener( const OUString& , const Reference<XVetoableChangeListener>& ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    OSL_FAIL( "###unexpected!" );
}

// ----------------------------------------------------------

void TableDesignFamily::removeVetoableChangeListener( const OUString& , const Reference<XVetoableChangeListener>&  ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    OSL_FAIL( "###unexpected!" );
}

// --------------------------------------------------------------------


SVX_DLLPUBLIC Reference< XNameAccess > CreateTableDesignFamily()
{
    return new TableDesignFamily();
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
