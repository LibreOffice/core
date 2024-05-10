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
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/form/XReset.hpp>

#include <vcl/svapp.hxx>

#include <comphelper/compbase.hxx>
#include <comphelper/interfacecontainer4.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/sequence.hxx>

#include <svx/sdr/table/tabledesign.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>

#include "sdrtableobjimpl.hxx"

#include <vector>
#include <map>


using namespace css;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::container;

namespace sdr::table {

typedef std::map< OUString, sal_Int32 > CellStyleNameMap;

typedef ::comphelper::WeakComponentImplHelper< XStyle, XNameReplace, XServiceInfo, XIndexReplace, XModifiable, XModifyListener, XPropertySet > TableDesignStyleBase;

namespace {

class TableDesignStyle : public TableDesignStyleBase
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

    // XIndexReplace
    virtual void SAL_CALL replaceByIndex( sal_Int32 Index, const Any& Element ) override;

    // XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const Any& aElement ) override;

    // XPropertySet
    virtual Reference<XPropertySetInfo> SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const Any& aValue ) override;
    virtual Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const Reference<XPropertyChangeListener>& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const Reference<XPropertyChangeListener>& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener(const OUString& PropertyName, const Reference<XVetoableChangeListener>& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener(const OUString& PropertyName,const Reference<XVetoableChangeListener>&aListener ) override;

    // XModifiable
    virtual sal_Bool SAL_CALL isModified() override;
    virtual void SAL_CALL setModified( sal_Bool bModified ) override;

    // XModifyBroadcaster
    virtual void SAL_CALL addModifyListener( const Reference< XModifyListener >& aListener ) override;
    virtual void SAL_CALL removeModifyListener( const Reference< XModifyListener >& aListener ) override;

    // XModifyListener
    virtual void SAL_CALL modified( const css::lang::EventObject& aEvent ) override;
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

    void notifyModifyListener();
    void resetUserDefined();

    // this function is called upon disposing the component
    virtual void disposing(std::unique_lock<std::mutex>& aGuard) override;

    static const CellStyleNameMap& getCellStyleNameMap();

    bool mbUserDefined, mbModified;
    OUString msName;
    Reference< XStyle > maCellStyles[style_count];
    comphelper::OInterfaceContainerHelper4<XModifyListener> maModifyListeners;
};

}

typedef std::vector< Reference< XStyle > > TableDesignStyleVector;

namespace {

class TableDesignFamily : public ::cppu::WeakImplHelper< XNameContainer, XNamed, XIndexAccess, XSingleServiceFactory,  XServiceInfo, XComponent, XPropertySet, form::XReset >
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

    // XReset
    virtual void SAL_CALL reset() override;
    virtual void SAL_CALL addResetListener( const Reference<form::XResetListener>& aListener ) override;
    virtual void SAL_CALL removeResetListener( const Reference<form::XResetListener>& aListener ) override;

    TableDesignStyleVector  maDesigns;
};

}

TableDesignStyle::TableDesignStyle()
    : mbUserDefined(true)
    , mbModified(false)
{
}

const CellStyleNameMap& TableDesignStyle::getCellStyleNameMap()
{
    static CellStyleNameMap const aMap
    {
         { u"first-row"_ustr    , first_row_style },
         { u"last-row"_ustr     , last_row_style },
         { u"first-column"_ustr , first_column_style },
         { u"last-column"_ustr  , last_column_style },
         { u"body"_ustr         , body_style },
         { u"even-rows"_ustr    , even_rows_style },
         { u"odd-rows"_ustr     , odd_rows_style },
         { u"even-columns"_ustr , even_columns_style },
         { u"odd-columns"_ustr  , odd_columns_style },
         { u"background"_ustr   , background_style },
    };

    return aMap;
}

// XServiceInfo
OUString SAL_CALL TableDesignStyle::getImplementationName()
{
    return u"TableDesignStyle"_ustr;
}

sal_Bool SAL_CALL TableDesignStyle::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

Sequence< OUString > SAL_CALL TableDesignStyle::getSupportedServiceNames()
{
    return { u"com.sun.star.style.Style"_ustr };
}

// XStyle
sal_Bool SAL_CALL TableDesignStyle::isUserDefined()
{
    return mbUserDefined;
}

void TableDesignStyle::resetUserDefined()
{
    mbUserDefined = false;
}

sal_Bool SAL_CALL TableDesignStyle::isInUse()
{
    std::unique_lock aGuard( m_aMutex );
    if (maModifyListeners.getLength(aGuard))
    {
        comphelper::OInterfaceIteratorHelper4 it(aGuard, maModifyListeners);
        while ( it.hasMoreElements() )
        {
            SdrTableObjImpl* pUser = dynamic_cast< SdrTableObjImpl* >( it.next().get() );
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
    const CellStyleNameMap& rMap = getCellStyleNameMap();

    CellStyleNameMap::const_iterator iter = rMap.find( rName );
    if( iter == rMap.end() )
        throw NoSuchElementException();

    return Any( maCellStyles[(*iter).second] );
}


Sequence< OUString > SAL_CALL TableDesignStyle::getElementNames()
{
    return comphelper::mapKeysToSequence( getCellStyleNameMap() );
}


sal_Bool SAL_CALL TableDesignStyle::hasByName( const OUString& rName )
{
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
    if( (Index < 0) || (Index >= style_count) )
        throw IndexOutOfBoundsException();

    std::unique_lock aGuard( m_aMutex );
    return Any( maCellStyles[Index] );
}


// XIndexReplace

void SAL_CALL TableDesignStyle::replaceByIndex( sal_Int32 Index, const Any& aElement )
{
    if( (Index < 0) || (Index >= style_count) )
        throw IndexOutOfBoundsException();

    const CellStyleNameMap& rMap = getCellStyleNameMap();
    auto iter = std::find_if(rMap.begin(), rMap.end(),
        [&Index](const auto& item) { return Index == item.second; });
    if (iter != rMap.end())
        replaceByName(iter->first, aElement);
}


// XNameReplace


void SAL_CALL TableDesignStyle::replaceByName( const OUString& rName, const Any& aElement )
{
    const CellStyleNameMap& rMap = getCellStyleNameMap();
    CellStyleNameMap::const_iterator iter = rMap.find( rName );
    if( iter == rMap.end() )
        throw NoSuchElementException();


    Reference< XStyle > xNewStyle;
    if( !(aElement >>= xNewStyle) )
        throw IllegalArgumentException();

    const sal_Int32 nIndex = (*iter).second;

    std::unique_lock aGuard( m_aMutex );

    Reference< XStyle > xOldStyle( maCellStyles[nIndex] );

    if( xNewStyle == xOldStyle )
        return;

    Reference< XModifyListener > xListener( this );

    // end listening to old style, if possible
    Reference< XModifyBroadcaster > xOldBroadcaster( xOldStyle, UNO_QUERY );
    if( xOldBroadcaster.is() )
        xOldBroadcaster->removeModifyListener( xListener );

    // start listening to new style, if possible
    Reference< XModifyBroadcaster > xNewBroadcaster( xNewStyle, UNO_QUERY );
    if( xNewBroadcaster.is() )
        xNewBroadcaster->addModifyListener( xListener );

    if (xNewStyle && xNewStyle->isUserDefined())
        mbModified = true;

    maCellStyles[nIndex] = xNewStyle;
}


// XComponent


void TableDesignStyle::disposing(std::unique_lock<std::mutex>& aGuard)
{
    maModifyListeners.disposeAndClear(aGuard, EventObject(Reference<XComponent>(this)));

    for(Reference<XStyle> & rCellStyle : maCellStyles)
    {
        Reference<XModifyBroadcaster> xBroadcaster(rCellStyle, UNO_QUERY);
        if (xBroadcaster)
            xBroadcaster->removeModifyListener(this);
        rCellStyle.clear();
    }
}

// XPropertySet

Reference<XPropertySetInfo> TableDesignStyle::getPropertySetInfo()
{
    return {};
}

void TableDesignStyle::setPropertyValue( const OUString&, const Any& )
{
}

Any TableDesignStyle::getPropertyValue( const OUString& PropertyName )
{
    if (PropertyName != "IsPhysical")
        throw UnknownPropertyException("unknown property: " + PropertyName, getXWeak());

    return Any(mbModified || mbUserDefined);
}

void TableDesignStyle::addPropertyChangeListener( const OUString&, const Reference<XPropertyChangeListener>& )
{
}

void TableDesignStyle::removePropertyChangeListener( const OUString&, const Reference<XPropertyChangeListener>& )
{
}

void TableDesignStyle::addVetoableChangeListener( const OUString&, const Reference<XVetoableChangeListener>& )
{
}

void TableDesignStyle::removeVetoableChangeListener( const OUString&,const Reference<XVetoableChangeListener>& )
{
}

// XModifiable

sal_Bool TableDesignStyle::isModified()
{
    return mbModified;
}

void TableDesignStyle::setModified( sal_Bool bModified )
{
    mbModified = bModified;
    notifyModifyListener();
}


// XModifyBroadcaster


void SAL_CALL TableDesignStyle::addModifyListener( const Reference< XModifyListener >& xListener )
{
    std::unique_lock aGuard( m_aMutex );
    if (m_bDisposed)
    {
        aGuard.unlock();
        EventObject aEvt( getXWeak() );
        xListener->disposing( aEvt );
    }
    else
    {
        maModifyListeners.addInterface( aGuard, xListener );
    }
}


void SAL_CALL TableDesignStyle::removeModifyListener( const Reference< XModifyListener >& xListener )
{
    std::unique_lock aGuard( m_aMutex );
    maModifyListeners.removeInterface( aGuard, xListener );
}


void TableDesignStyle::notifyModifyListener()
{
    std::unique_lock aGuard( m_aMutex );

    if( maModifyListeners.getLength(aGuard) )
    {
        EventObject aEvt( getXWeak() );
        maModifyListeners.forEach(aGuard,
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
    return u"TableDesignFamily"_ustr;
}

sal_Bool SAL_CALL TableDesignFamily::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

Sequence< OUString > SAL_CALL TableDesignFamily::getSupportedServiceNames()
{
    return { u"com.sun.star.style.StyleFamily"_ustr };
}

// XNamed
OUString SAL_CALL TableDesignFamily::getName()
{
    return u"table"_ustr;
}

void SAL_CALL TableDesignFamily::setName( const OUString& )
{
}

// XNameAccess
Any SAL_CALL TableDesignFamily::getByName( const OUString& rName )
{
    SolarMutexGuard aGuard;

    auto iter = std::find_if(maDesigns.begin(), maDesigns.end(),
        [&rName](const Reference<XStyle>& rpStyle) { return rpStyle->getName() == rName; });
    if (iter != maDesigns.end())
        return Any( (*iter) );

    throw NoSuchElementException();
}


Sequence< OUString > SAL_CALL TableDesignFamily::getElementNames()
{
    SolarMutexGuard aGuard;

    Sequence< OUString > aRet( maDesigns.size() );
    OUString* pNames = aRet.getArray();

    for( const auto& rpStyle : maDesigns )
        *pNames++ = rpStyle->getName();

    return aRet;
}


sal_Bool SAL_CALL TableDesignFamily::hasByName( const OUString& aName )
{
    SolarMutexGuard aGuard;

    return std::any_of(maDesigns.begin(), maDesigns.end(),
        [&aName](const Reference<XStyle>& rpStyle) { return rpStyle->getName() == aName; });
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
    if (std::any_of(maDesigns.begin(), maDesigns.end(),
            [&rName](const Reference<XStyle>& rpStyle) { return rpStyle->getName() == rName; }))
        throw ElementExistException();

    maDesigns.push_back( xStyle );
}


void SAL_CALL TableDesignFamily::removeByName( const OUString& rName )
{
    SolarMutexGuard aGuard;

    auto iter = std::find_if(maDesigns.begin(), maDesigns.end(),
        [&rName](const Reference<XStyle>& rpStyle) { return rpStyle->getName() == rName; });
    if (iter != maDesigns.end())
    {
        Reference<XComponent> xComponent(*iter, UNO_QUERY);
        if (xComponent)
            xComponent->dispose();
        maDesigns.erase( iter );
        return;
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

    auto iter = std::find_if(maDesigns.begin(), maDesigns.end(),
        [&rName](const Reference<XStyle>& rpStyle) { return rpStyle->getName() == rName; });
    if (iter != maDesigns.end())
    {
        if (!(*iter)->isUserDefined())
            static_cast<TableDesignStyle*>(xStyle.get())->resetUserDefined();

        Reference<XComponent> xComponent(*iter, UNO_QUERY);
        if (xComponent)
            xComponent->dispose();
        (*iter) = xStyle;
        xStyle->setName( rName );
        return;
    }

    throw NoSuchElementException();
}


// XSingleServiceFactory


Reference< XInterface > SAL_CALL TableDesignFamily::createInstance()
{
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

    for( const auto& rStyle : aDesigns )
    {
        Reference< XComponent > xComp( rStyle, UNO_QUERY );
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
    if ( PropertyName != "DisplayName" )
    {
        throw UnknownPropertyException( "unknown property: " + PropertyName, getXWeak() );
    }

    OUString sDisplayName( SvxResId( RID_SVXSTR_STYLEFAMILY_TABLEDESIGN ) );
    return Any( sDisplayName );
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

// XReset

void TableDesignFamily::reset()
{
    for (const auto& aDesign : maDesigns)
    {
        auto aStyle = static_cast<TableDesignStyle*>(aDesign.get());
        aStyle->resetUserDefined();
        aStyle->setModified(false);
    }
}

void TableDesignFamily::addResetListener( const Reference<form::XResetListener>& )
{
}

void TableDesignFamily::removeResetListener( const Reference<form::XResetListener>& )
{
}

Reference< XNameAccess > CreateTableDesignFamily()
{
    return new TableDesignFamily;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
