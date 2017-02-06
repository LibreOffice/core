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
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/util/XModifyListener.hpp>

#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/sequence.hxx>

#include "svx/unoprov.hxx"
#include "svx/sdr/table/tabledesign.hxx"
#include "svx/dialmgr.hxx"
#include "svx/dialogs.hrc"

#include "celltypes.hxx"

#include <vector>
#include <map>


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::container;

using ::osl::MutexGuard;
using ::osl::ClearableMutexGuard;
using ::cppu::OInterfaceContainerHelper;

namespace sdr { namespace table {

typedef std::map< OUString, sal_Int32 > CellStyleNameMap;

typedef ::cppu::WeakComponentImplHelper< XStyle, XNameReplace, XServiceInfo, XIndexAccess, XModifyBroadcaster, XModifyListener > TableDesignStyleBase;

class TableDesignStyle : private ::cppu::BaseMutex, public TableDesignStyleBase
{
public:
    TableDesignStyle();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XStyle
    virtual sal_Bool SAL_CALL isUserDefined() override;
    virtual sal_Bool SAL_CALL isInUse() override;
    virtual OUString SAL_CALL getParentStyle() override;
    virtual void SAL_CALL setParentStyle( const OUString& aParentStyle ) override;

    // XNamed
    virtual OUString SAL_CALL getName() override;
    virtual void SAL_CALL setName( const OUString& aName ) override;

    // XNameAccess
    virtual Any SAL_CALL getByName( const OUString& aName ) override;
    virtual Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override ;
    virtual Any SAL_CALL getByIndex( sal_Int32 Index ) override;

    // XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const Any& aElement ) override;

    // XModifyBroadcaster
    virtual void SAL_CALL addModifyListener( const Reference< XModifyListener >& aListener ) override;
    virtual void SAL_CALL removeModifyListener( const Reference< XModifyListener >& aListener ) override;

    // XModifyListener
    virtual void SAL_CALL modified( const css::lang::EventObject& aEvent ) override;
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

    void notifyModifyListener();

    // this function is called upon disposing the component
    virtual void SAL_CALL disposing() override;

    static const CellStyleNameMap& getCellStyleNameMap();

    OUString msName;
    Reference< XStyle > maCellStyles[style_count];
};

typedef std::vector< Reference< XStyle > > TableDesignStyleVector;

class TableDesignFamily : public ::cppu::WeakImplHelper< XNameContainer, XNamed, XIndexAccess, XSingleServiceFactory,  XServiceInfo, XComponent, XPropertySet >
{
public:
    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XNamed
    virtual OUString SAL_CALL getName(  ) override;
    virtual void SAL_CALL setName( const OUString& aName ) override;

    // XNameAccess
    virtual Any SAL_CALL getByName( const OUString& aName ) override;
    virtual Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

    // XElementAccess
    virtual Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override ;
    virtual Any SAL_CALL getByIndex( sal_Int32 Index ) override;

    // XNameContainer
    virtual void SAL_CALL insertByName( const OUString& aName, const Any& aElement ) override;
    virtual void SAL_CALL removeByName( const OUString& Name ) override;

    // XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const Any& aElement ) override;

    // XSingleServiceFactory
    virtual Reference< XInterface > SAL_CALL createInstance(  ) override;
    virtual Reference< XInterface > SAL_CALL createInstanceWithArguments( const Sequence< Any >& aArguments ) override;

    // XComponent
    virtual void SAL_CALL dispose(  ) override;
    virtual void SAL_CALL addEventListener( const Reference< XEventListener >& xListener ) override;
    virtual void SAL_CALL removeEventListener( const Reference< XEventListener >& aListener ) override;

    // XPropertySet
    virtual Reference<XPropertySetInfo> SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const Any& aValue ) override;
    virtual Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const Reference<XPropertyChangeListener>& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const Reference<XPropertyChangeListener>& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener(const OUString& PropertyName, const Reference<XVetoableChangeListener>& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener(const OUString& PropertyName,const Reference<XVetoableChangeListener>&aListener ) override;

    TableDesignStyleVector  maDesigns;
};

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

// XServiceInfo
OUString SAL_CALL TableDesignStyle::getImplementationName()
{
    return OUString("TableDesignStyle");
}

sal_Bool SAL_CALL TableDesignStyle::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

Sequence< OUString > SAL_CALL TableDesignStyle::getSupportedServiceNames()
{
    OUString aServiceName("com.sun.star.style.Style");
    Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

// XStyle
sal_Bool SAL_CALL TableDesignStyle::isUserDefined()
{
    return false;
}

sal_Bool SAL_CALL TableDesignStyle::isInUse()
{
    ClearableMutexGuard aGuard( rBHelper.rMutex );
    OInterfaceContainerHelper * pContainer = rBHelper.getContainer( cppu::UnoType<XModifyListener>::get() );
    if( pContainer )
    {
        Sequence< Reference< XInterface > > aListener( pContainer->getElements() );
        aGuard.clear();

        sal_Int32 nIndex = aListener.getLength();
        while( --nIndex >= 0 )
        {
            TableDesignUser* pUser = dynamic_cast< TableDesignUser* >( aListener[nIndex].get() );
            if( pUser && pUser->isInUse() )
                return true;
        }
    }
    return false;
}


OUString SAL_CALL TableDesignStyle::getParentStyle()
{
    return OUString();
}


void SAL_CALL TableDesignStyle::setParentStyle( const OUString& )
{
}


// XNamed


OUString SAL_CALL TableDesignStyle::getName()
{
    return msName;
}


void SAL_CALL TableDesignStyle::setName( const OUString& rName )
{
    msName = rName;
}


// XNameAccess


Any SAL_CALL TableDesignStyle::getByName( const OUString& rName )
{
    SolarMutexGuard aGuard;

    const CellStyleNameMap& rMap = getCellStyleNameMap();

    CellStyleNameMap::const_iterator iter = rMap.find( rName );
    if( iter == rMap.end() )
        throw NoSuchElementException();

    return Any( maCellStyles[(*iter).second] );
}


Sequence< OUString > SAL_CALL TableDesignStyle::getElementNames()
{
    SolarMutexGuard aGuard;

    return comphelper::mapKeysToSequence( getCellStyleNameMap() );
}


sal_Bool SAL_CALL TableDesignStyle::hasByName( const OUString& rName )
{
    SolarMutexGuard aGuard;

    const CellStyleNameMap& rMap = getCellStyleNameMap();

    CellStyleNameMap::const_iterator iter = rMap.find( rName );
    return iter != rMap.end();
}


// XElementAccess


Type SAL_CALL TableDesignStyle::getElementType()
{
    return cppu::UnoType<XStyle>::get();
}


sal_Bool SAL_CALL TableDesignStyle::hasElements()
{
    return true;
}


// XIndexAccess


sal_Int32 SAL_CALL TableDesignStyle::getCount()
{
    return style_count;
}


Any SAL_CALL TableDesignStyle::getByIndex( sal_Int32 Index )
{
    SolarMutexGuard aGuard;

    if( (Index < 0) || (Index >= style_count) )
        throw IndexOutOfBoundsException();

    return Any( maCellStyles[Index] );
}


// XNameReplace


void SAL_CALL TableDesignStyle::replaceByName( const OUString& rName, const Any& aElement )
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


// XComponent


void SAL_CALL TableDesignStyle::disposing()
{
    for(Reference<XStyle> & rCellStyle : maCellStyles)
        rCellStyle.clear();
}


// XModifyBroadcaster


void SAL_CALL TableDesignStyle::addModifyListener( const Reference< XModifyListener >& xListener )
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
        rBHelper.addListener( cppu::UnoType<XModifyListener>::get(), xListener );
    }
}


void SAL_CALL TableDesignStyle::removeModifyListener( const Reference< XModifyListener >& xListener )
{
    rBHelper.removeListener( cppu::UnoType<XModifyListener>::get(), xListener );
}


void TableDesignStyle::notifyModifyListener()
{
    MutexGuard aGuard( rBHelper.rMutex );

    OInterfaceContainerHelper * pContainer = rBHelper.getContainer( cppu::UnoType<XModifyListener>::get() );
    if( pContainer )
    {
        EventObject aEvt( static_cast< OWeakObject * >( this ) );
        pContainer->forEach<XModifyListener>(
            [&] (Reference<XModifyListener> const& xListener)
                { return xListener->modified(aEvt); });
    }
}


// XModifyListener


// if we get a modify hint from a style, notify all registered XModifyListener
void SAL_CALL TableDesignStyle::modified( const css::lang::EventObject& )
{
    notifyModifyListener();
}


void SAL_CALL TableDesignStyle::disposing( const css::lang::EventObject& )
{
}


// TableStyle


// XServiceInfo
OUString SAL_CALL TableDesignFamily::getImplementationName()
{
    return OUString("TableDesignFamily");
}

sal_Bool SAL_CALL TableDesignFamily::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

Sequence< OUString > SAL_CALL TableDesignFamily::getSupportedServiceNames()
{
    OUString aServiceName("com.sun.star.style.StyleFamily");
    Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

// XNamed
OUString SAL_CALL TableDesignFamily::getName()
{
    return OUString( "table" );
}

void SAL_CALL TableDesignFamily::setName( const OUString& )
{
}

// XNameAccess
Any SAL_CALL TableDesignFamily::getByName( const OUString& rName )
{
    SolarMutexGuard aGuard;

    const TableDesignStyleVector::const_iterator aEnd( maDesigns.end() );
    for( TableDesignStyleVector::const_iterator iter( maDesigns.begin() );
        iter != aEnd; ++iter)
    {
        if( (*iter)->getName() == rName )
            return Any( (*iter) );
    }

    throw NoSuchElementException();
}


Sequence< OUString > SAL_CALL TableDesignFamily::getElementNames()
{
    SolarMutexGuard aGuard;

    Sequence< OUString > aRet( maDesigns.size() );
    OUString* pNames = aRet.getArray();

    const TableDesignStyleVector::const_iterator aEnd( maDesigns.end() );
    for( TableDesignStyleVector::const_iterator iter( maDesigns.begin() );
         iter != aEnd; ++iter)
        *pNames++ = (*iter)->getName();

    return aRet;
}


sal_Bool SAL_CALL TableDesignFamily::hasByName( const OUString& aName )
{
    SolarMutexGuard aGuard;

    const TableDesignStyleVector::const_iterator aEnd( maDesigns.end() );
    for( TableDesignStyleVector::const_iterator iter( maDesigns.begin() );
        iter != aEnd; ++iter)
        if( (*iter)->getName() == aName )
            return true;

    return false;
}


// XElementAccess


Type SAL_CALL TableDesignFamily::getElementType()
{
    return cppu::UnoType<XStyle>::get();
}


sal_Bool SAL_CALL TableDesignFamily::hasElements()
{
    SolarMutexGuard aGuard;

    return !maDesigns.empty();
}


// XIndexAccess


sal_Int32 SAL_CALL TableDesignFamily::getCount()
{
    SolarMutexGuard aGuard;

    return sal::static_int_cast< sal_Int32 >( maDesigns.size() );
}


Any SAL_CALL TableDesignFamily::getByIndex( sal_Int32 Index )
{
    SolarMutexGuard aGuard;

    if( (Index >= 0) && (Index < sal::static_int_cast< sal_Int32 >( maDesigns.size() ) ) )
        return Any( maDesigns[Index] );

    throw IndexOutOfBoundsException();
}


// XNameContainer


void SAL_CALL TableDesignFamily::insertByName( const OUString& rName, const Any& rElement )
{
    SolarMutexGuard aGuard;

    Reference< XStyle > xStyle( rElement, UNO_QUERY );
    if( !xStyle.is() )
        throw IllegalArgumentException();

    xStyle->setName( rName );
    const TableDesignStyleVector::const_iterator aEnd( maDesigns.end() );
    for( TableDesignStyleVector::const_iterator iter( maDesigns.begin() );
        iter != aEnd; ++iter)
        if( (*iter)->getName() == rName )
            throw ElementExistException();

    maDesigns.push_back( xStyle );
}


void SAL_CALL TableDesignFamily::removeByName( const OUString& rName )
{
    SolarMutexGuard aGuard;

    const TableDesignStyleVector::const_iterator aEnd( maDesigns.end() );
    for( TableDesignStyleVector::iterator iter( maDesigns.begin() );
        iter != aEnd; ++iter)
    {
        if( (*iter)->getName() == rName )
        {
            maDesigns.erase( iter );
            return;
        }
    }


    throw NoSuchElementException();
}


// XNameReplace


void SAL_CALL TableDesignFamily::replaceByName( const OUString& rName, const Any& aElement )
{
    SolarMutexGuard aGuard;

    Reference< XStyle > xStyle( aElement, UNO_QUERY );
    if( !xStyle.is() )
        throw IllegalArgumentException();

    const TableDesignStyleVector::const_iterator aEnd( maDesigns.end() );
    for( TableDesignStyleVector::iterator iter( maDesigns.begin() );
        iter != aEnd; ++iter)
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


// XSingleServiceFactory


Reference< XInterface > SAL_CALL TableDesignFamily::createInstance()
{
    SolarMutexGuard aGuard;

    return Reference< XInterface >( static_cast< XStyle* >( new TableDesignStyle ) );
}


Reference< XInterface > SAL_CALL TableDesignFamily::createInstanceWithArguments( const Sequence< Any >&  )
{
    return createInstance();
}


// XComponent


void SAL_CALL TableDesignFamily::dispose(  )
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


void SAL_CALL TableDesignFamily::addEventListener( const Reference< XEventListener >&  )
{
}


void SAL_CALL TableDesignFamily::removeEventListener( const Reference< XEventListener >&  )
{
}


// XPropertySet


Reference<XPropertySetInfo> TableDesignFamily::getPropertySetInfo()
{
    OSL_FAIL( "###unexpected!" );
    return Reference<XPropertySetInfo>();
}


void TableDesignFamily::setPropertyValue( const OUString& , const Any&  )
{
    OSL_FAIL( "###unexpected!" );
}


Any TableDesignFamily::getPropertyValue( const OUString& PropertyName )
{
    if ( PropertyName == "DisplayName" )
    {
        OUString sDisplayName( SVX_RESSTR( RID_SVXSTR_STYLEFAMILY_TABLEDESIGN ) );
        return Any( sDisplayName );
    }
    else
    {
        throw UnknownPropertyException( "unknown property: " + PropertyName, static_cast<OWeakObject *>(this) );
    }
}


void TableDesignFamily::addPropertyChangeListener( const OUString& , const Reference<XPropertyChangeListener>&  )
{
    OSL_FAIL( "###unexpected!" );
}


void TableDesignFamily::removePropertyChangeListener( const OUString& , const Reference<XPropertyChangeListener>&  )
{
    OSL_FAIL( "###unexpected!" );
}


void TableDesignFamily::addVetoableChangeListener( const OUString& , const Reference<XVetoableChangeListener>& )
{
    OSL_FAIL( "###unexpected!" );
}


void TableDesignFamily::removeVetoableChangeListener( const OUString& , const Reference<XVetoableChangeListener>&  )
{
    OSL_FAIL( "###unexpected!" );
}


Reference< XNameAccess > CreateTableDesignFamily()
{
    return new TableDesignFamily();
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
