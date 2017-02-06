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

#include <sal/config.h>

#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/awt/VclWindowPeerAttribute.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/resource/XStringResourceResolver.hpp>
#include <toolkit/controls/unocontrol.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/uuid.h>
#include <osl/mutex.hxx>
#include <tools/date.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <comphelper/processfactory.hxx>
#include <toolkit/helper/property.hxx>
#include <toolkit/helper/servicenames.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/controls/accessiblecontrolcontext.hxx>
#include <comphelper/container.hxx>

#include <algorithm>
#include <map>
#include <set>
#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;

using ::com::sun::star::accessibility::XAccessibleContext;
using ::com::sun::star::accessibility::XAccessible;

struct LanguageDependentProp
{
    const char* pPropName;
    sal_Int32   nPropNameLength;
};

static const LanguageDependentProp aLanguageDependentProp[] =
{
    { "Text",            4 },
    { "Label",           5 },
    { "Title",           5 },
    { "HelpText",        8 },
    { "CurrencySymbol", 14 },
    { "StringItemList", 14 },
    { nullptr, 0                 }
};

static Sequence< OUString> lcl_ImplGetPropertyNames( const Reference< XMultiPropertySet > & rxModel )
{
    Sequence< OUString> aNames;
    Reference< XPropertySetInfo >  xPSInf = rxModel->getPropertySetInfo();
    DBG_ASSERT( xPSInf.is(), "UpdateFromModel: No PropertySetInfo!" );
    if ( xPSInf.is() )
    {
        Sequence< Property> aProps = xPSInf->getProperties();
        sal_Int32 nLen = aProps.getLength();
        aNames = Sequence< OUString>( nLen );
        OUString* pNames = aNames.getArray();
        const Property* pProps = aProps.getConstArray();
        for ( sal_Int32 n = 0; n < nLen; ++n, ++pProps, ++pNames)
            *pNames = pProps->Name;
    }
    return aNames;
}


class VclListenerLock
{
private:
    VCLXWindow*  m_pLockWindow;

public:
    explicit VclListenerLock( VCLXWindow* _pLockWindow )
        : m_pLockWindow( _pLockWindow )
    {
        if ( m_pLockWindow )
            m_pLockWindow->suspendVclEventListening( );
    }
    ~VclListenerLock()
    {
        if ( m_pLockWindow )
            m_pLockWindow->resumeVclEventListening( );
    }
    VclListenerLock(const VclListenerLock&) = delete;
    VclListenerLock& operator=(const VclListenerLock&) = delete;
};

typedef ::std::map< OUString, sal_Int32 >    MapString2Int;
struct UnoControl_Data
{
    MapString2Int   aSuspendedPropertyNotifications;
    /// true if and only if our model has a property ResourceResolver
    bool            bLocalizationSupport;

    UnoControl_Data()
        :aSuspendedPropertyNotifications()
        ,bLocalizationSupport( false )
    {
    }
};

UnoControl::UnoControl() :
      maDisposeListeners( *this )
    , maWindowListeners( *this )
    , maFocusListeners( *this )
    , maKeyListeners( *this )
    , maMouseListeners( *this )
    , maMouseMotionListeners( *this )
    , maPaintListeners( *this )
    , maModeChangeListeners( GetMutex() )
    , mpData( new UnoControl_Data )
{
    mbDisposePeer = true;
    mbRefeshingPeer = false;
    mbCreatingPeer = false;
    mbCreatingCompatiblePeer = false;
    mbDesignMode = false;
}

UnoControl::~UnoControl()
{
    DELETEZ( mpData );
}

OUString UnoControl::GetComponentServiceName()
{
    return OUString();
}

Reference< XWindowPeer >    UnoControl::ImplGetCompatiblePeer()
{
    DBG_ASSERT( !mbCreatingCompatiblePeer, "ImplGetCompatiblePeer - rekursive?" );

    mbCreatingCompatiblePeer = true;

    Reference< XWindowPeer > xCompatiblePeer = getPeer();

    if ( !xCompatiblePeer.is() )
    {
        // Create the pair as invisible
        bool bVis = maComponentInfos.bVisible;
        if( bVis )
            maComponentInfos.bVisible = false;

        Reference< XWindowPeer >    xCurrentPeer = getPeer();
        setPeer( nullptr );

        // queryInterface ourself, to allow aggregation
        Reference< XControl > xMe;
        OWeakAggObject::queryInterface( cppu::UnoType<decltype(xMe)>::get() ) >>= xMe;

        vcl::Window* pParentWindow( nullptr );
        {
            SolarMutexGuard aGuard;
            pParentWindow = dynamic_cast< vcl::Window* >( Application::GetDefaultDevice() );
            ENSURE_OR_THROW( pParentWindow != nullptr, "could obtain a default parent window!" );
        }
        try
        {
            xMe->createPeer( nullptr, pParentWindow->GetComponentInterface() );
        }
        catch( const Exception& )
        {
            mbCreatingCompatiblePeer = false;
            throw;
        }
        xCompatiblePeer = getPeer();
        setPeer( xCurrentPeer );

        if ( xCompatiblePeer.is() && mxGraphics.is() )
        {
            Reference< XView > xPeerView( xCompatiblePeer, UNO_QUERY );
            if ( xPeerView.is() )
                xPeerView->setGraphics( mxGraphics );
        }

        if( bVis )
            maComponentInfos.bVisible = true;
    }

    mbCreatingCompatiblePeer = false;

    return xCompatiblePeer;
}

bool UnoControl::ImplCheckLocalize( OUString& _rPossiblyLocalizable )
{
    if  (   !mpData->bLocalizationSupport
        ||  ( _rPossiblyLocalizable.isEmpty() )
        ||  ( _rPossiblyLocalizable[0] != '&' )
            // TODO: make this reasonable. At the moment, everything which by accident starts with a & is considered
            // localizable, which is probably wrong.
        )
        return false;

    try
    {
        Reference< XPropertySet > xPropSet( mxModel, UNO_QUERY_THROW );
        Reference< resource::XStringResourceResolver > xStringResourceResolver(
            xPropSet->getPropertyValue("ResourceResolver"),
            UNO_QUERY
        );
        if ( xStringResourceResolver.is() )
        {
            OUString aLocalizationKey( _rPossiblyLocalizable.copy( 1 ) );
            _rPossiblyLocalizable = xStringResourceResolver->resolveString( aLocalizationKey );
            return true;
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return false;
}

void UnoControl::ImplSetPeerProperty( const OUString& rPropName, const Any& rVal )
{
    // since a change made in propertiesChange, we can't be sure that this is called with an valid getPeer(),
    // this assumption may be false in some (seldom) multi-threading scenarios (cause propertiesChange
    // releases our mutex before calling here in)
    // That's why this additional check

    if ( mxVclWindowPeer.is() )
    {
        Any aConvertedValue( rVal );

        if ( mpData->bLocalizationSupport )
        {
            // We now support a mapping for language dependent properties. This is the
            // central method to implement it.
            if( rPropName == "Text"            ||
                rPropName == "Label"           ||
                rPropName == "Title"           ||
                rPropName == "HelpText"        ||
                rPropName == "CurrencySymbol"  ||
                rPropName == "StringItemList"  )
            {
                OUString aValue;
                uno::Sequence< OUString > aSeqValue;
                if ( aConvertedValue >>= aValue )
                {
                    if ( ImplCheckLocalize( aValue ) )
                        aConvertedValue <<= aValue;
                }
                else if ( aConvertedValue >>= aSeqValue )
                {
                    for ( sal_Int32 i = 0; i < aSeqValue.getLength(); i++ )
                        ImplCheckLocalize( aSeqValue[i] );
                    aConvertedValue <<= aSeqValue;
                }
            }
        }

        mxVclWindowPeer->setProperty( rPropName, aConvertedValue );
    }
}

void UnoControl::PrepareWindowDescriptor( WindowDescriptor& )
{
}

Reference< XWindow >    UnoControl::getParentPeer() const
{
    Reference< XWindow > xPeer;
    if( mxContext.is() )
    {
        Reference< XControl > xContComp( mxContext, UNO_QUERY );
        if ( xContComp.is() )
        {
            Reference< XWindowPeer > xP = xContComp->getPeer();
            if ( xP.is() )
                xPeer.set( xP, UNO_QUERY );
        }
    }
    return xPeer;
}

void UnoControl::updateFromModel()
{
    // Read default properties and hand over to peer
    if( getPeer().is() )
    {
        Reference< XMultiPropertySet >  xPropSet( mxModel, UNO_QUERY );
        if( xPropSet.is() )
        {
            Sequence< OUString> aNames = lcl_ImplGetPropertyNames( xPropSet );
            xPropSet->firePropertiesChangeEvent( aNames, this );
        }
    }
}


// XTypeProvider
IMPL_IMPLEMENTATION_ID( UnoControl )

void
UnoControl::DisposeAccessibleContext(Reference<XComponent> const& xContextComp)
{
    if (xContextComp.is())
    {
        try
        {
            xContextComp->removeEventListener( this );
            xContextComp->dispose();
        }
        catch( const Exception& )
        {
            OSL_FAIL( "UnoControl::disposeAccessibleContext: could not dispose my AccessibleContext!" );
        }
    }
}

void UnoControl::dispose(  )
{
    Reference< XWindowPeer > xPeer;
    Reference<XComponent> xAccessibleComp;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        if( mbDisposePeer )
        {
            xPeer = mxPeer;
        }
        setPeer( nullptr );
        xAccessibleComp.set(maAccessibleContext, UNO_QUERY);
        maAccessibleContext.clear();
    }
    if( xPeer.is() )
    {
        xPeer->dispose();
    }

    // dispose our AccessibleContext - without Mutex locked
    DisposeAccessibleContext(xAccessibleComp);

    EventObject aDisposeEvent;
    aDisposeEvent.Source = static_cast< XAggregation* >( this );

    maDisposeListeners.disposeAndClear( aDisposeEvent );
    maWindowListeners.disposeAndClear( aDisposeEvent );
    maFocusListeners.disposeAndClear( aDisposeEvent );
    maKeyListeners.disposeAndClear( aDisposeEvent );
    maMouseListeners.disposeAndClear( aDisposeEvent );
    maMouseMotionListeners.disposeAndClear( aDisposeEvent );
    maPaintListeners.disposeAndClear( aDisposeEvent );
    maModeChangeListeners.disposeAndClear( aDisposeEvent );

    // release Model again
    setModel( Reference< XControlModel > () );
    setContext( Reference< XInterface > () );
}

void UnoControl::addEventListener( const Reference< XEventListener >& rxListener )
{
    ::osl::MutexGuard aGuard( GetMutex() );

    maDisposeListeners.addInterface( rxListener );
}

void UnoControl::removeEventListener( const Reference< XEventListener >& rxListener )
{
    ::osl::MutexGuard aGuard( GetMutex() );

    maDisposeListeners.removeInterface( rxListener );
}

bool UnoControl::requiresNewPeer( const OUString& /* _rPropertyName */ ) const
{
    return false;
}

// XPropertiesChangeListener
void UnoControl::propertiesChange( const Sequence< PropertyChangeEvent >& rEvents )
{
    Sequence< PropertyChangeEvent > aEvents( rEvents );
    {
        ::osl::MutexGuard aGuard( GetMutex() );

        if ( !mpData->aSuspendedPropertyNotifications.empty() )
        {
            // strip the property which we are currently updating (somewhere up the stack)
            PropertyChangeEvent* pEvents = aEvents.getArray();
            PropertyChangeEvent* pEventsEnd = pEvents + aEvents.getLength();
            for ( ; pEvents < pEventsEnd; )
                if ( mpData->aSuspendedPropertyNotifications.find( pEvents->PropertyName ) != mpData->aSuspendedPropertyNotifications.end() )
                {
                    if ( pEvents != pEventsEnd )
                        ::std::copy( pEvents + 1, pEventsEnd, pEvents );
                    --pEventsEnd;
                }
                else
                    ++pEvents;
            aEvents.realloc( pEventsEnd - aEvents.getConstArray() );

            if ( !aEvents.getLength() )
                return;
        }
    }

    ImplModelPropertiesChanged( aEvents );
}

void UnoControl::ImplLockPropertyChangeNotification( const OUString& rPropertyName, bool bLock )
{
    MapString2Int::iterator pos = mpData->aSuspendedPropertyNotifications.find( rPropertyName );
    if ( bLock )
    {
        if ( pos == mpData->aSuspendedPropertyNotifications.end() )
            pos = mpData->aSuspendedPropertyNotifications.insert( MapString2Int::value_type( rPropertyName, 0 ) ).first;
        ++pos->second;
    }
    else
    {
        OSL_ENSURE( pos != mpData->aSuspendedPropertyNotifications.end(), "UnoControl::ImplLockPropertyChangeNotification: property not locked!" );
        if ( pos != mpData->aSuspendedPropertyNotifications.end() )
        {
            OSL_ENSURE( pos->second > 0, "UnoControl::ImplLockPropertyChangeNotification: invalid suspension counter!" );
            if ( 0 == --pos->second )
                mpData->aSuspendedPropertyNotifications.erase( pos );
        }
    }
}

void UnoControl::ImplLockPropertyChangeNotifications( const Sequence< OUString >& rPropertyNames, bool bLock )
{
    for (   const OUString* pPropertyName = rPropertyNames.getConstArray();
            pPropertyName != rPropertyNames.getConstArray() + rPropertyNames.getLength();
            ++pPropertyName
        )
        ImplLockPropertyChangeNotification( *pPropertyName, bLock );
}

void UnoControl::ImplModelPropertiesChanged( const Sequence< PropertyChangeEvent >& rEvents )
{
    ::osl::ClearableGuard< ::osl::Mutex > aGuard( GetMutex() );

    if( getPeer().is() )
    {
        std::vector< PropertyValue > aPeerPropertiesToSet;
        sal_Int32               nIndependentPos = 0;
        bool                    bResourceResolverSet( false );
            // position where to insert the independent properties into aPeerPropertiesToSet,
            // dependent ones are inserted at the end of the vector

        bool bNeedNewPeer = false;
            // some properties require a re-creation of the peer, 'cause they can't be changed on the fly

        Reference< XControlModel > xOwnModel( getModel(), UNO_QUERY );
            // our own model for comparison
        Reference< XPropertySet > xPS( xOwnModel, UNO_QUERY );
        Reference< XPropertySetInfo > xPSI( xPS->getPropertySetInfo(), UNO_QUERY );
        OSL_ENSURE( xPSI.is(), "UnoControl::ImplModelPropertiesChanged: should have property set meta data!" );

        const PropertyChangeEvent* pEvents = rEvents.getConstArray();

        sal_Int32 nLen = rEvents.getLength();
        aPeerPropertiesToSet.reserve(nLen);

        for( sal_Int32 i = 0; i < nLen; ++i, ++pEvents )
        {
            Reference< XControlModel > xModel( pEvents->Source, UNO_QUERY );
            bool bOwnModel = xModel.get() == xOwnModel.get();
            if ( !bOwnModel )
                continue;

            // Detect changes on our resource resolver which invalidates
            // automatically some language dependent properties.
            if ( pEvents->PropertyName == "ResourceResolver" )
            {
                Reference< resource::XStringResourceResolver > xStrResolver;
                if ( pEvents->NewValue >>= xStrResolver )
                    bResourceResolverSet = xStrResolver.is();
            }

            sal_uInt16 nPType = GetPropertyId( pEvents->PropertyName );
            if ( mbDesignMode && mbDisposePeer && !mbRefeshingPeer && !mbCreatingPeer )
            {
                // if we're in design mode, then some properties can change which
                // require creating a *new* peer (since these properties cannot
                // be switched at existing peers)
                if ( nPType )
                    bNeedNewPeer = ( nPType == BASEPROPERTY_BORDER )
                                || ( nPType == BASEPROPERTY_MULTILINE )
                                || ( nPType == BASEPROPERTY_DROPDOWN )
                                || ( nPType == BASEPROPERTY_HSCROLL )
                                || ( nPType == BASEPROPERTY_VSCROLL )
                                || ( nPType == BASEPROPERTY_AUTOHSCROLL )
                                || ( nPType == BASEPROPERTY_AUTOVSCROLL )
                                || ( nPType == BASEPROPERTY_ORIENTATION )
                                || ( nPType == BASEPROPERTY_SPIN )
                                || ( nPType == BASEPROPERTY_ALIGN )
                                || ( nPType == BASEPROPERTY_PAINTTRANSPARENT );
                else
                    bNeedNewPeer = requiresNewPeer( pEvents->PropertyName );

                if ( bNeedNewPeer )
                    break;
            }

            if ( nPType && ( nLen > 1 ) && DoesDependOnOthers( nPType ) )
            {
                // Add properties with dependencies on other properties last
                // since they're dependent on properties added later (such as
                // VALUE dependency on VALUEMIN/MAX)
                aPeerPropertiesToSet.push_back(PropertyValue(pEvents->PropertyName, 0, pEvents->NewValue, PropertyState_DIRECT_VALUE));
            }
            else
            {
                if ( bResourceResolverSet )
                {
                    // The resource resolver property change should be one of the first ones.
                    // All language dependent properties are dependent on this property.
                    // As BASEPROPERTY_NATIVE_WIDGET_LOOK is not dependent on resource
                    // resolver. We don't need to handle a special order for these two props.
                    aPeerPropertiesToSet.insert(
                        aPeerPropertiesToSet.begin(),
                        PropertyValue( pEvents->PropertyName, 0, pEvents->NewValue, PropertyState_DIRECT_VALUE ) );
                    ++nIndependentPos;
                }
                else if ( nPType == BASEPROPERTY_NATIVE_WIDGET_LOOK )
                {
                    // since *a lot* of other properties might be overruled by this one, we need
                    // a special handling:
                    // NativeWidgetLook needs to be set first: If it is set to ON, all other
                    // properties describing the look (e.g. BackgroundColor) are ignored, anyway.
                    // If it is switched OFF, then we need to do it first because else it will
                    // overrule other look-related properties, and re-initialize them from system
                    // defaults.
                    aPeerPropertiesToSet.insert(
                        aPeerPropertiesToSet.begin(),
                        PropertyValue( pEvents->PropertyName, 0, pEvents->NewValue, PropertyState_DIRECT_VALUE ) );
                    ++nIndependentPos;
                }
                else
                {
                    aPeerPropertiesToSet.insert(aPeerPropertiesToSet.begin() + nIndependentPos,
                        PropertyValue(pEvents->PropertyName, 0, pEvents->NewValue, PropertyState_DIRECT_VALUE));
                    ++nIndependentPos;
                }
            }
        }

        Reference< XWindow >    xParent = getParentPeer();
        Reference< XControl > xThis( static_cast<XAggregation*>(static_cast<cppu::OWeakAggObject*>(this)), UNO_QUERY );
        // call createPeer via a interface got from queryInterface, so the aggregating class can intercept it

        DBG_ASSERT( !bNeedNewPeer || xParent.is(), "Need new peer, but don't have a parent!" );

        // Check if we have to update language dependent properties
        if ( !bNeedNewPeer && bResourceResolverSet )
        {
            // Add language dependent properties into the peer property set.
            // Our resource resolver has been changed and we must be sure
            // that language dependent props use the new resolver.
            const LanguageDependentProp* pLangDepProp = aLanguageDependentProp;
            while ( pLangDepProp->pPropName != nullptr )
            {
                bool bMustBeInserted( true );
                for (PropertyValue & i : aPeerPropertiesToSet)
                {
                    if ( i.Name.equalsAsciiL(
                            pLangDepProp->pPropName, pLangDepProp->nPropNameLength ))
                    {
                        bMustBeInserted = false;
                        break;
                    }
                }

                if ( bMustBeInserted )
                {
                    // Add language dependent props at the end
                    OUString aPropName( OUString::createFromAscii( pLangDepProp->pPropName ));
                    if ( xPSI.is() && xPSI->hasPropertyByName( aPropName ) )
                    {
                        aPeerPropertiesToSet.push_back(
                            PropertyValue( aPropName, 0, xPS->getPropertyValue( aPropName ), PropertyState_DIRECT_VALUE ) );
                    }
                }

                ++pLangDepProp;
            }
        }
        aGuard.clear();

        // clear the guard before creating a new peer - as usual, our peer implementations use the SolarMutex

        if (bNeedNewPeer && xParent.is())
        {
            SolarMutexGuard aVclGuard;
                // and now this is the final withdrawal:
                // I have no other idea than locking the SolarMutex here....
                // I really hate the fact that VCL is not threadsafe....

            // Doesn't work for Container!
            getPeer()->dispose();
            mxPeer.clear();
            mxVclWindowPeer = nullptr;
            mbRefeshingPeer = true;
            Reference< XWindowPeer >    xP( xParent, UNO_QUERY );
            xThis->createPeer( Reference< XToolkit > (), xP );
            mbRefeshingPeer = false;
            aPeerPropertiesToSet.clear();
        }

        // lock the multiplexing of VCL events to our UNO listeners
        // this is for compatibility reasons: in OOo 1.0.x, changes which were done at the
        // model did not cause the listeners of the controls/peers to be called
        // Since the implementations for the listeners changed a lot towards 1.1, this
        // would not be the case anymore, if we would not do this listener-lock below
        // #i14703#
        VCLXWindow* pPeer;
        {
            SolarMutexGuard g;
            VclPtr<vcl::Window> pVclPeer = VCLUnoHelper::GetWindow( getPeer() );
            pPeer = pVclPeer ? pVclPeer->GetWindowPeer() : nullptr;
        }
        VclListenerLock aNoVclEventMultiplexing( pPeer );

        // setting peer properties may result in an attempt to acquire the solar mutex, 'cause the peers
        // usually don't have an own mutex but use the SolarMutex instead.
        // To prevent deadlocks resulting from this, we do this without our own mutex locked
        std::vector< PropertyValue >::iterator aEnd = aPeerPropertiesToSet.end();
        for (   std::vector< PropertyValue >::iterator aLoop = aPeerPropertiesToSet.begin();
                aLoop != aEnd;
                ++aLoop
            )
        {
            ImplSetPeerProperty( aLoop->Name, aLoop->Value );
        }
    }
}

void UnoControl::disposing( const EventObject& rEvt )
{
    ::osl::ClearableMutexGuard aGuard( GetMutex() );
    // do not compare differing types in case of multiple inheritance

    if ( maAccessibleContext.get() == rEvt.Source )
    {
        // just in case the context is disposed, but not released - ensure that we do not re-use it in the future
        maAccessibleContext = nullptr;
    }
    else if( mxModel.get() == Reference< XControlModel >(rEvt.Source,UNO_QUERY).get() )
    {
        // #62337# if the model dies, it does not make sense for us to live ...
        Reference< XControl >  xThis = this;

        aGuard.clear();
        xThis->dispose();

        DBG_ASSERT( !mxModel.is(), "UnoControl::disposing: invalid dispose behaviour!" );
        mxModel.clear();
    }
}


void SAL_CALL UnoControl::setOutputSize( const awt::Size& aSize )
{
    Reference< XWindow2 > xPeerWindow;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        xPeerWindow.set(getPeer(), css::uno::UNO_QUERY);
    }

    if ( xPeerWindow.is() )
        xPeerWindow->setOutputSize( aSize );
}

namespace
{
    template < typename RETVALTYPE, typename DEFAULTTYPE >
    RETVALTYPE lcl_askPeer( const uno::Reference< awt::XWindowPeer >& _rxPeer, RETVALTYPE (SAL_CALL XWindow2::*_pMethod)(), DEFAULTTYPE _aDefault )
    {
        RETVALTYPE aReturn( _aDefault );

        Reference< XWindow2 > xPeerWindow( _rxPeer, UNO_QUERY );
        if ( xPeerWindow.is() )
            aReturn = (xPeerWindow.get()->*_pMethod)();

        return aReturn;
    }
}

awt::Size SAL_CALL UnoControl::getOutputSize(  )
{
    return lcl_askPeer( getPeer(), &XWindow2::getOutputSize, awt::Size() );
}

sal_Bool SAL_CALL UnoControl::isVisible(  )
{
    return lcl_askPeer( getPeer(), &XWindow2::isVisible, maComponentInfos.bVisible );
}

sal_Bool SAL_CALL UnoControl::isActive(  )
{
    return lcl_askPeer( getPeer(), &XWindow2::isActive, false );
}

sal_Bool SAL_CALL UnoControl::isEnabled(  )
{
    return lcl_askPeer( getPeer(), &XWindow2::isEnabled, maComponentInfos.bEnable );
}

sal_Bool SAL_CALL UnoControl::hasFocus(  )
{
    return lcl_askPeer( getPeer(), &XWindow2::hasFocus, false );
}

// XWindow
void UnoControl::setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags )
{
    Reference< XWindow > xWindow;
    {
        ::osl::MutexGuard aGuard( GetMutex() );

        if ( Flags & awt::PosSize::X )
            maComponentInfos.nX = X;
        if ( Flags & awt::PosSize::Y )
            maComponentInfos.nY = Y;
        if ( Flags & awt::PosSize::WIDTH )
            maComponentInfos.nWidth = Width;
        if ( Flags & awt::PosSize::HEIGHT )
            maComponentInfos.nHeight = Height;
        maComponentInfos.nFlags |= Flags;

        xWindow.set(getPeer(), css::uno::UNO_QUERY);
    }

    if( xWindow.is() )
        xWindow->setPosSize( X, Y, Width, Height, Flags );
}

awt::Rectangle UnoControl::getPosSize(  )
{
    awt::Rectangle aRect( maComponentInfos.nX, maComponentInfos.nY, maComponentInfos.nWidth, maComponentInfos.nHeight);
    Reference< XWindow > xWindow;

    {
        ::osl::MutexGuard aGuard( GetMutex() );
        xWindow.set(getPeer(), css::uno::UNO_QUERY);
    }

    if( xWindow.is() )
        aRect = xWindow->getPosSize();
    return aRect;
}

void UnoControl::setVisible( sal_Bool bVisible )
{
    Reference< XWindow > xWindow;
    {
        ::osl::MutexGuard aGuard( GetMutex() );

        // Visible status is handled by View
        maComponentInfos.bVisible = bVisible;
        xWindow.set(getPeer(), css::uno::UNO_QUERY);
    }
    if ( xWindow.is() )
        xWindow->setVisible( bVisible );
}

void UnoControl::setEnable( sal_Bool bEnable )
{
    Reference< XWindow > xWindow;
    {
        ::osl::MutexGuard aGuard( GetMutex() );

        // Enable status is handled by View
        maComponentInfos.bEnable = bEnable;
        xWindow.set(getPeer(), css::uno::UNO_QUERY);
    }
    if ( xWindow.is() )
        xWindow->setEnable( bEnable );
}

void UnoControl::setFocus(  )
{
    Reference< XWindow > xWindow;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        xWindow.set(getPeer(), css::uno::UNO_QUERY);
    }
    if ( xWindow.is() )
        xWindow->setFocus();
}

void UnoControl::addWindowListener( const Reference< XWindowListener >& rxListener )
{
    Reference< XWindow > xPeerWindow;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        maWindowListeners.addInterface( rxListener );
        if ( maWindowListeners.getLength() == 1 )
            xPeerWindow.set(getPeer(), css::uno::UNO_QUERY);
    }
    if ( xPeerWindow.is() )
        xPeerWindow->addWindowListener( &maWindowListeners );
}

void UnoControl::removeWindowListener( const Reference< XWindowListener >& rxListener )
{
    Reference< XWindow > xPeerWindow;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        if ( maWindowListeners.getLength() == 1 )
            xPeerWindow.set(getPeer(), css::uno::UNO_QUERY);
        maWindowListeners.removeInterface( rxListener );
    }
    if ( xPeerWindow.is() )
        xPeerWindow->removeWindowListener( &maWindowListeners );
}

void UnoControl::addFocusListener( const Reference< XFocusListener >& rxListener )
{
    Reference< XWindow > xPeerWindow;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        maFocusListeners.addInterface( rxListener );
        if ( maFocusListeners.getLength() == 1 )
            xPeerWindow.set(getPeer(), css::uno::UNO_QUERY);
    }
    if ( xPeerWindow.is() )
        xPeerWindow->addFocusListener( &maFocusListeners );
}

void UnoControl::removeFocusListener( const Reference< XFocusListener >& rxListener )
{
    Reference< XWindow > xPeerWindow;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        if ( maFocusListeners.getLength() == 1 )
            xPeerWindow.set(getPeer(), css::uno::UNO_QUERY);
        maFocusListeners.removeInterface( rxListener );
    }
    if ( xPeerWindow.is() )
        xPeerWindow->removeFocusListener( &maFocusListeners );
}

void UnoControl::addKeyListener( const Reference< XKeyListener >& rxListener )
{
    Reference< XWindow > xPeerWindow;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        maKeyListeners.addInterface( rxListener );
        if ( maKeyListeners.getLength() == 1 )
            xPeerWindow.set(getPeer(), css::uno::UNO_QUERY);
    }
    if ( xPeerWindow.is() )
        xPeerWindow->addKeyListener( &maKeyListeners);
}

void UnoControl::removeKeyListener( const Reference< XKeyListener >& rxListener )
{
    Reference< XWindow > xPeerWindow;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        if ( maKeyListeners.getLength() == 1 )
            xPeerWindow.set(getPeer(), css::uno::UNO_QUERY);
        maKeyListeners.removeInterface( rxListener );
    }
    if ( xPeerWindow.is() )
        xPeerWindow->removeKeyListener( &maKeyListeners);
}

void UnoControl::addMouseListener( const Reference< XMouseListener >& rxListener )
{
    Reference< XWindow > xPeerWindow;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        maMouseListeners.addInterface( rxListener );
        if ( maMouseListeners.getLength() == 1 )
            xPeerWindow.set(getPeer(), css::uno::UNO_QUERY);
    }
    if ( xPeerWindow.is() )
        xPeerWindow->addMouseListener( &maMouseListeners);
}

void UnoControl::removeMouseListener( const Reference< XMouseListener >& rxListener )
{
    Reference< XWindow > xPeerWindow;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        if ( maMouseListeners.getLength() == 1 )
            xPeerWindow.set(getPeer(), css::uno::UNO_QUERY);
        maMouseListeners.removeInterface( rxListener );
    }
    if ( xPeerWindow.is() )
        xPeerWindow->removeMouseListener( &maMouseListeners );
}

void UnoControl::addMouseMotionListener( const Reference< XMouseMotionListener >& rxListener )
{
    Reference< XWindow > xPeerWindow;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        maMouseMotionListeners.addInterface( rxListener );
        if ( maMouseMotionListeners.getLength() == 1 )
            xPeerWindow.set(getPeer(), css::uno::UNO_QUERY);
    }
    if ( xPeerWindow.is() )
        xPeerWindow->addMouseMotionListener( &maMouseMotionListeners);
}

void UnoControl::removeMouseMotionListener( const Reference< XMouseMotionListener >& rxListener )
{
    Reference< XWindow > xPeerWindow;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        if ( maMouseMotionListeners.getLength() == 1 )
            xPeerWindow.set(getPeer(), css::uno::UNO_QUERY);
        maMouseMotionListeners.removeInterface( rxListener );
    }
    if ( xPeerWindow.is() )
        xPeerWindow->removeMouseMotionListener( &maMouseMotionListeners );
}

void UnoControl::addPaintListener( const Reference< XPaintListener >& rxListener )
{
    Reference< XWindow > xPeerWindow;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        maPaintListeners.addInterface( rxListener );
        if ( maPaintListeners.getLength() == 1 )
            xPeerWindow.set(getPeer(), css::uno::UNO_QUERY);
    }
    if ( xPeerWindow.is() )
        xPeerWindow->addPaintListener( &maPaintListeners);
}

void UnoControl::removePaintListener( const Reference< XPaintListener >& rxListener )
{
    Reference< XWindow > xPeerWindow;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        if ( maPaintListeners.getLength() == 1 )
            xPeerWindow.set(getPeer(), css::uno::UNO_QUERY);
        maPaintListeners.removeInterface( rxListener );
    }
    if ( xPeerWindow.is() )
        xPeerWindow->removePaintListener( &maPaintListeners );
}

// XView
sal_Bool UnoControl::setGraphics( const Reference< XGraphics >& rDevice )
{
    Reference< XView > xView;
    {
        ::osl::MutexGuard aGuard( GetMutex() );

        mxGraphics = rDevice;
        xView.set(getPeer(), css::uno::UNO_QUERY);
    }
    return !xView.is() || xView->setGraphics( rDevice );
}

Reference< XGraphics > UnoControl::getGraphics(  )
{
    return mxGraphics;
}

awt::Size UnoControl::getSize(  )
{
    ::osl::MutexGuard aGuard( GetMutex() );
    return awt::Size( maComponentInfos.nWidth, maComponentInfos.nHeight );
}

void UnoControl::draw( sal_Int32 x, sal_Int32 y )
{
    Reference< XWindowPeer > xDrawPeer;
    Reference< XView > xDrawPeerView;

    bool bDisposeDrawPeer( false );
    {
        ::osl::MutexGuard aGuard( GetMutex() );

        xDrawPeer = ImplGetCompatiblePeer();
        bDisposeDrawPeer = xDrawPeer.is() && ( xDrawPeer != getPeer() );

        xDrawPeerView.set( xDrawPeer, UNO_QUERY );
        DBG_ASSERT( xDrawPeerView.is(), "UnoControl::draw: no peer!" );
    }

    if ( xDrawPeerView.is() )
    {
        Reference< XVclWindowPeer > xWindowPeer;
        xWindowPeer.set( xDrawPeer, UNO_QUERY );
        if ( xWindowPeer.is() )
            xWindowPeer->setDesignMode( mbDesignMode );
        xDrawPeerView->draw( x, y );
    }

    if ( bDisposeDrawPeer )
        xDrawPeer->dispose();
}

void UnoControl::setZoom( float fZoomX, float fZoomY )
{
    Reference< XView > xView;
    {
        ::osl::MutexGuard aGuard( GetMutex() );

        maComponentInfos.nZoomX = fZoomX;
        maComponentInfos.nZoomY = fZoomY;

        xView.set(getPeer(), css::uno::UNO_QUERY);
    }
    if ( xView.is() )
        xView->setZoom( fZoomX, fZoomY );
}

// XControl
void UnoControl::setContext( const Reference< XInterface >& rxContext )
{
    ::osl::MutexGuard aGuard( GetMutex() );

    mxContext = rxContext;
}

Reference< XInterface > UnoControl::getContext(  )
{
    ::osl::MutexGuard aGuard( GetMutex() );

    return mxContext;
}

void UnoControl::peerCreated()
{
    Reference< XWindow > xWindow( getPeer(), UNO_QUERY );
    if ( !xWindow.is() )
        return;

    if ( maWindowListeners.getLength() )
        xWindow->addWindowListener( &maWindowListeners );

    if ( maFocusListeners.getLength() )
        xWindow->addFocusListener( &maFocusListeners );

    if ( maKeyListeners.getLength() )
        xWindow->addKeyListener( &maKeyListeners );

    if ( maMouseListeners.getLength() )
        xWindow->addMouseListener( &maMouseListeners );

    if ( maMouseMotionListeners.getLength() )
        xWindow->addMouseMotionListener( &maMouseMotionListeners );

    if ( maPaintListeners.getLength() )
        xWindow->addPaintListener( &maPaintListeners );
}

void UnoControl::createPeer( const Reference< XToolkit >& rxToolkit, const Reference< XWindowPeer >& rParentPeer )
{
    ::osl::ClearableMutexGuard aGuard( GetMutex() );
    if ( !mxModel.is() )
    {
        RuntimeException aException;
        aException.Message = "createPeer: no model!";
        aException.Context = static_cast<XAggregation*>(static_cast<cppu::OWeakAggObject*>(this));
        throw( aException );
    }

    if( !getPeer().is() )
    {
        mbCreatingPeer = true;

        WindowClass eType;
        Reference< XToolkit >  xToolkit = rxToolkit;
        if( rParentPeer.is() && mxContext.is() )
        {
            // kein TopWindow
            if ( !xToolkit.is() )
                xToolkit = rParentPeer->getToolkit();
            Any aAny = OWeakAggObject::queryInterface( cppu::UnoType<XControlContainer>::get());
            Reference< XControlContainer > xC;
            aAny >>= xC;
            if( xC.is() )
                // It's a container
                eType = WindowClass_CONTAINER;
            else
                eType = WindowClass_SIMPLE;
        }
        else
        { // This is only correct for Top Window
            if( rParentPeer.is() )
            {
                if ( !xToolkit.is() )
                    xToolkit = rParentPeer->getToolkit();
                eType = WindowClass_CONTAINER;
            }
            else
            {
                if ( !xToolkit.is() )
                    xToolkit = VCLUnoHelper::CreateToolkit();
                eType = WindowClass_TOP;
            }
        }
        WindowDescriptor aDescr;
        aDescr.Type = eType;
        aDescr.WindowServiceName = GetComponentServiceName();
        aDescr.Parent = rParentPeer;
        aDescr.Bounds = getPosSize();
        aDescr.WindowAttributes = 0;

        // Border
        Reference< XPropertySet > xPSet( mxModel, UNO_QUERY );
        Reference< XPropertySetInfo >  xInfo = xPSet->getPropertySetInfo();

        Any aVal;
        OUString aPropName = GetPropertyName( BASEPROPERTY_BORDER );
        if ( xInfo->hasPropertyByName( aPropName ) )
        {
            aVal = xPSet->getPropertyValue( aPropName );
            sal_Int16 n = sal_Int16();
            if ( aVal >>= n )
            {
                if ( n )
                    aDescr.WindowAttributes |= WindowAttribute::BORDER;
                else
                    aDescr.WindowAttributes |= VclWindowPeerAttribute::NOBORDER;
            }
        }

        // DESKTOP_AS_PARENT
        if ( aDescr.Type == WindowClass_TOP )
        {
            aPropName = GetPropertyName( BASEPROPERTY_DESKTOP_AS_PARENT );
            if ( xInfo->hasPropertyByName( aPropName ) )
            {
                aVal = xPSet->getPropertyValue( aPropName );
                bool b = bool();
                if ( ( aVal >>= b ) && b)
                    aDescr.ParentIndex = -1;
            }
        }
        // Moveable
        aPropName = GetPropertyName( BASEPROPERTY_MOVEABLE );
        if ( xInfo->hasPropertyByName( aPropName ) )
        {
            aVal = xPSet->getPropertyValue( aPropName );
            bool b = bool();
            if ( ( aVal >>= b ) && b)
                aDescr.WindowAttributes |= WindowAttribute::MOVEABLE;
        }

        // Closeable
        aPropName = GetPropertyName( BASEPROPERTY_CLOSEABLE );
        if ( xInfo->hasPropertyByName( aPropName ) )
        {
            aVal = xPSet->getPropertyValue( aPropName );
            bool b = bool();
            if ( ( aVal >>= b ) && b)
                aDescr.WindowAttributes |= WindowAttribute::CLOSEABLE;
        }

        // Dropdown
        aPropName = GetPropertyName( BASEPROPERTY_DROPDOWN );
        if ( xInfo->hasPropertyByName( aPropName ) )
        {
            aVal = xPSet->getPropertyValue( aPropName );
            bool b = bool();
            if ( ( aVal >>= b ) && b)
                aDescr.WindowAttributes |= VclWindowPeerAttribute::DROPDOWN;
        }

        // Spin
        aPropName = GetPropertyName( BASEPROPERTY_SPIN );
        if ( xInfo->hasPropertyByName( aPropName ) )
        {
            aVal = xPSet->getPropertyValue( aPropName );
            bool b = bool();
            if ( ( aVal >>= b ) && b)
                aDescr.WindowAttributes |= VclWindowPeerAttribute::SPIN;
        }

        // HScroll
        aPropName = GetPropertyName( BASEPROPERTY_HSCROLL );
        if ( xInfo->hasPropertyByName( aPropName ) )
        {
            aVal = xPSet->getPropertyValue( aPropName );
            bool b = bool();
            if ( ( aVal >>= b ) && b)
                aDescr.WindowAttributes |= VclWindowPeerAttribute::HSCROLL;
        }

        // VScroll
        aPropName = GetPropertyName( BASEPROPERTY_VSCROLL );
        if ( xInfo->hasPropertyByName( aPropName ) )
        {
            aVal = xPSet->getPropertyValue( aPropName );
            bool b = bool();
            if ( ( aVal >>= b ) && b)
                aDescr.WindowAttributes |= VclWindowPeerAttribute::VSCROLL;
        }

        // AutoHScroll
        aPropName = GetPropertyName( BASEPROPERTY_AUTOHSCROLL );
        if ( xInfo->hasPropertyByName( aPropName ) )
        {
            aVal = xPSet->getPropertyValue( aPropName );
            bool b = bool();
            if ( ( aVal >>= b ) && b)
                aDescr.WindowAttributes |= VclWindowPeerAttribute::AUTOHSCROLL;
        }

        // AutoVScroll
        aPropName = GetPropertyName( BASEPROPERTY_AUTOVSCROLL );
        if ( xInfo->hasPropertyByName( aPropName ) )
        {
            aVal = xPSet->getPropertyValue( aPropName );
            bool b = bool();
            if ( ( aVal >>= b ) && b)
                aDescr.WindowAttributes |= VclWindowPeerAttribute::AUTOVSCROLL;
        }

        //added for issue79712
        //NoLabel
        aPropName = GetPropertyName( BASEPROPERTY_NOLABEL );
        if ( xInfo->hasPropertyByName( aPropName ) )
        {
            aVal = xPSet->getPropertyValue( aPropName );
            bool b = bool();
            if ( ( aVal >>=b ) && b )
                aDescr.WindowAttributes |= VclWindowPeerAttribute::NOLABEL;
        }
        //issue79712 ends

        // Align
        aPropName = GetPropertyName( BASEPROPERTY_ALIGN );
        if ( xInfo->hasPropertyByName( aPropName ) )
        {
            aVal = xPSet->getPropertyValue( aPropName );
            sal_Int16 n = sal_Int16();
            if ( aVal >>= n )
            {
                if ( n == PROPERTY_ALIGN_LEFT )
                    aDescr.WindowAttributes |= VclWindowPeerAttribute::LEFT;
                else if ( n == PROPERTY_ALIGN_CENTER )
                    aDescr.WindowAttributes |= VclWindowPeerAttribute::CENTER;
                else
                    aDescr.WindowAttributes |= VclWindowPeerAttribute::RIGHT;
            }
        }

        // Allow derivates to manipulate attributes
        PrepareWindowDescriptor(aDescr);

        // create the peer
        setPeer( xToolkit->createWindow( aDescr ) );

        // release the mutex guard (and work with copies of our members)
        // this is necessary as our peer may lock the SolarMutex (actually, all currently known peers do), so calling
        // into the peer with our own mutex locked may cause deadlocks
        // (We _really_ need peers which do not use the SolarMutex. It's really pissing me off that from time to
        // time deadlocks pop up because the low-level components like our peers use a mutex which usually
        // is locked at the top of the stack (it protects the global message looping). This is always dangerous, and
        // can not always be solved by tampering with other mutexes.
        // Unfortunately, the VCL used in the peers is not threadsafe, and by definition needs a locked SolarMutex.)
        // 82300 - 12/21/00 - FS
        UnoControlComponentInfos aComponentInfos(maComponentInfos);
        bool bDesignMode(mbDesignMode);

        Reference< XGraphics >  xGraphics( mxGraphics           );
        Reference< XView >      xView    ( getPeer(), UNO_QUERY_THROW );
        Reference< XWindow >    xWindow  ( getPeer(), UNO_QUERY_THROW );

        aGuard.clear();

        // the updateFromModel is done without a locked mutex, too.
        // The reason is that the only thing this method does  is firing property changes, and this in general has
        // to be done without locked mutexes (as every notification to external listeners).
        // 82300 - 12/21/00 - FS
        updateFromModel();

        xView->setZoom( aComponentInfos.nZoomX, aComponentInfos.nZoomY );

        setPosSize( aComponentInfos.nX, aComponentInfos.nY, aComponentInfos.nWidth, aComponentInfos.nHeight, aComponentInfos.nFlags );

        if( aComponentInfos.bVisible && !bDesignMode )
            // Show only after setting the data
            xWindow->setVisible( aComponentInfos.bVisible );

        if( !aComponentInfos.bEnable )
            xWindow->setEnable( aComponentInfos.bEnable );

        xView->setGraphics( xGraphics );

        peerCreated();

        mbCreatingPeer = false;
    }
}

Reference< XWindowPeer > UnoControl::getPeer()
{
    ::osl::MutexGuard aGuard( GetMutex() );
    return mxPeer;
}

sal_Bool UnoControl::setModel( const Reference< XControlModel >& rxModel )
{
    ::osl::MutexGuard aGuard( GetMutex() );

    Reference< XMultiPropertySet > xPropSet( mxModel, UNO_QUERY );

    // query for the XPropertiesChangeListener - our delegator is allowed to overwrite this interface
    Reference< XPropertiesChangeListener > xListener;
    queryInterface( cppu::UnoType<decltype(xListener)>::get() ) >>= xListener;

    if( xPropSet.is() )
        xPropSet->removePropertiesChangeListener( xListener );

    mpData->bLocalizationSupport = false;
    mxModel = rxModel;

    if( mxModel.is() )
    {
        try
        {
            xPropSet.set( mxModel, UNO_QUERY_THROW );
            Reference< XPropertySetInfo > xPSI( xPropSet->getPropertySetInfo(), UNO_SET_THROW );

            Sequence< OUString> aNames = lcl_ImplGetPropertyNames( xPropSet );
            xPropSet->addPropertiesChangeListener( aNames, xListener );

            mpData->bLocalizationSupport = xPSI->hasPropertyByName("ResourceResolver");
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
            mxModel.clear();
        }
    }

    return mxModel.is();
}

Reference< XControlModel > UnoControl::getModel(    )
{
    return mxModel;
}

Reference< XView > UnoControl::getView(  )
{
    return  static_cast< XView* >( this );
}

void UnoControl::setDesignMode( sal_Bool bOn )
{
    ModeChangeEvent aModeChangeEvent;

    Reference< XWindow > xWindow;
    Reference<XComponent> xAccessibleComp;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        if ( bool(bOn) == mbDesignMode )
            return;

        // remember this
        mbDesignMode = bOn;
        xWindow.set(getPeer(), css::uno::UNO_QUERY);

        xAccessibleComp.set(maAccessibleContext, UNO_QUERY);
        maAccessibleContext.clear();

        aModeChangeEvent.Source = *this;
        aModeChangeEvent.NewMode = mbDesignMode ? OUString("design") : OUString("alive" );
    }

    // dispose current AccessibleContext, if we have one - without Mutex lock
    // (changing the design mode implies having a new implementation for this context,
    // so the old one must be declared DEFUNC)
    DisposeAccessibleContext(xAccessibleComp);

    // adjust the visibility of our window
    if ( xWindow.is() )
        xWindow->setVisible( !bOn );

    // and notify our mode listeners
    maModeChangeListeners.notifyEach( &XModeChangeListener::modeChanged, aModeChangeEvent );
}

sal_Bool UnoControl::isDesignMode(  )
{
    return mbDesignMode;
}

sal_Bool UnoControl::isTransparent(  )
{
    return false;
}

// XServiceInfo
OUString UnoControl::getImplementationName(  )
{
    OSL_FAIL( "This method should be overridden!" );
    return OUString();
}

sal_Bool UnoControl::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > UnoControl::getSupportedServiceNames(  )
{
    OUString sName( "com.sun.star.awt.UnoControl" );
    return Sequence< OUString >( &sName, 1 );
}


Reference< XAccessibleContext > SAL_CALL UnoControl::getAccessibleContext(  )
{
    // creation of the context will certainly require the SolarMutex ...
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( GetMutex() );

    Reference< XAccessibleContext > xCurrentContext( maAccessibleContext.get(), UNO_QUERY );
    if ( !xCurrentContext.is() )
    {
        if ( !mbDesignMode )
        {   // in alive mode, use the AccessibleContext of the peer
            Reference< XAccessible > xPeerAcc( getPeer(), UNO_QUERY );
            if ( xPeerAcc.is() )
                xCurrentContext = xPeerAcc->getAccessibleContext( );
        }
        else
            // in design mode, use a fallback
            xCurrentContext = ::toolkit::OAccessibleControlContext::create( this );

        DBG_ASSERT( xCurrentContext.is(), "UnoControl::getAccessibleContext: invalid context (invalid peer?)!" );
        maAccessibleContext = xCurrentContext;

        // get notified when the context is disposed
        Reference< XComponent > xContextComp( xCurrentContext, UNO_QUERY );
        if ( xContextComp.is() )
            xContextComp->addEventListener( this );
        // In an ideal world, this is not necessary - there the object would be released as soon as it has been
        // disposed, and thus our weak reference would be empty, too.
        // But 'til this ideal world comes (means 'til we do never have any refcount/lifetime bugs anymore), we
        // need to listen for disposal and reset our weak reference then.
    }

    return xCurrentContext;
}

void SAL_CALL UnoControl::addModeChangeListener( const Reference< XModeChangeListener >& _rxListener )
{
    ::osl::MutexGuard aGuard( GetMutex() );
    maModeChangeListeners.addInterface( _rxListener );
}

void SAL_CALL UnoControl::removeModeChangeListener( const Reference< XModeChangeListener >& _rxListener )
{
    ::osl::MutexGuard aGuard( GetMutex() );
    maModeChangeListeners.removeInterface( _rxListener );
}

void SAL_CALL UnoControl::addModeChangeApproveListener( const Reference< XModeChangeApproveListener >& )
{
    throw NoSupportException( );
}

void SAL_CALL UnoControl::removeModeChangeApproveListener( const Reference< XModeChangeApproveListener >&  )
{
    throw NoSupportException( );
}


awt::Point SAL_CALL UnoControl::convertPointToLogic( const awt::Point& i_Point, ::sal_Int16 i_TargetUnit )
{
    Reference< XUnitConversion > xPeerConversion;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        xPeerConversion.set(getPeer(), css::uno::UNO_QUERY);
    }
    if ( xPeerConversion.is() )
        return xPeerConversion->convertPointToLogic( i_Point, i_TargetUnit );
    return awt::Point( );
}


awt::Point SAL_CALL UnoControl::convertPointToPixel( const awt::Point& i_Point, ::sal_Int16 i_SourceUnit )
{
    Reference< XUnitConversion > xPeerConversion;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        xPeerConversion.set(getPeer(), css::uno::UNO_QUERY);
    }
    if ( xPeerConversion.is() )
        return xPeerConversion->convertPointToPixel( i_Point, i_SourceUnit );
    return awt::Point( );
}


awt::Size SAL_CALL UnoControl::convertSizeToLogic( const awt::Size& i_Size, ::sal_Int16 i_TargetUnit )
{
    Reference< XUnitConversion > xPeerConversion;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        xPeerConversion.set(getPeer(), css::uno::UNO_QUERY);
    }
    if ( xPeerConversion.is() )
        return xPeerConversion->convertSizeToLogic( i_Size, i_TargetUnit );
    return awt::Size( );
}


awt::Size SAL_CALL UnoControl::convertSizeToPixel( const awt::Size& i_Size, ::sal_Int16 i_SourceUnit )
{
    Reference< XUnitConversion > xPeerConversion;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        xPeerConversion.set(getPeer(), css::uno::UNO_QUERY);
    }
    if ( xPeerConversion.is() )
        return xPeerConversion->convertSizeToPixel( i_Size, i_SourceUnit );
    return awt::Size( );
}


uno::Reference< awt::XStyleSettings > SAL_CALL UnoControl::getStyleSettings()
{
    Reference< awt::XStyleSettingsSupplier > xPeerSupplier;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        xPeerSupplier.set(getPeer(), css::uno::UNO_QUERY);
    }
    if ( xPeerSupplier.is() )
        return xPeerSupplier->getStyleSettings();
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
