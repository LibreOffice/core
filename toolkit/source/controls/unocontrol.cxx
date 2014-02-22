/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/awt/VclWindowPeerAttribute.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
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
    { 0, 0                 }
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
    inline VclListenerLock( VCLXWindow* _pLockWindow )
        :m_pLockWindow( _pLockWindow )
    {
        if ( m_pLockWindow )
            m_pLockWindow->suspendVclEventListening( );
    }
    inline ~VclListenerLock( )
    {
        if ( m_pLockWindow )
            m_pLockWindow->resumeVclEventListening( );
    }

private:
    VclListenerLock();                                      
    VclListenerLock( const VclListenerLock& );              
    VclListenerLock& operator=( const VclListenerLock& );   
};

typedef ::std::map< OUString, sal_Int32 >    MapString2Int;
struct UnoControl_Data
{
    MapString2Int   aSuspendedPropertyNotifications;
    /
    bool            bLocalizationSupport;

    UnoControl_Data()
        :aSuspendedPropertyNotifications()
        ,bLocalizationSupport( false )
    {
    }
};




DBG_NAME( UnoControl )
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
    DBG_CTOR( UnoControl, NULL );
    mbDisposePeer = sal_True;
    mbRefeshingPeer = sal_False;
    mbCreatingPeer = sal_False;
    mbCreatingCompatiblePeer = sal_False;
    mbDesignMode = sal_False;
}

UnoControl::~UnoControl()
{
    DELETEZ( mpData );
    DBG_DTOR( UnoControl, NULL );
}

OUString UnoControl::GetComponentServiceName()
{
    return OUString();
}

Reference< XWindowPeer >    UnoControl::ImplGetCompatiblePeer( sal_Bool bAcceptExistingPeer )
{
    DBG_ASSERT( !mbCreatingCompatiblePeer, "ImplGetCompatiblePeer - rekursive?" );

    mbCreatingCompatiblePeer = sal_True;

    Reference< XWindowPeer > xCompatiblePeer;

    if ( bAcceptExistingPeer )
        xCompatiblePeer = getPeer();

    if ( !xCompatiblePeer.is() )
    {
        
        sal_Bool bVis = maComponentInfos.bVisible;
        if( bVis )
            maComponentInfos.bVisible = sal_False;

        Reference< XWindowPeer >    xCurrentPeer = getPeer();
        setPeer( NULL );

        
        Reference< XControl > xMe;
        OWeakAggObject::queryInterface( ::getCppuType( &xMe ) ) >>= xMe;

        Window* pParentWindow( NULL );
        {
            SolarMutexGuard aGuard;
            pParentWindow = dynamic_cast< Window* >( Application::GetDefaultDevice() );
            ENSURE_OR_THROW( pParentWindow != NULL, "could obtain a default parent window!" );
        }
        try
        {
            xMe->createPeer( NULL, pParentWindow->GetComponentInterface( sal_True ) );
        }
        catch( const Exception& )
        {
            mbCreatingCompatiblePeer = sal_False;
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
            maComponentInfos.bVisible = sal_True;
    }

    mbCreatingCompatiblePeer = sal_False;

    return xCompatiblePeer;
}

bool UnoControl::ImplCheckLocalize( OUString& _rPossiblyLocalizable )
{
    if  (   !mpData->bLocalizationSupport
        ||  ( _rPossiblyLocalizable.isEmpty() )
        ||  ( _rPossiblyLocalizable[0] != '&' )
            
            
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
    
    
    
    

    if ( mxVclWindowPeer.is() )
    {
        Any aConvertedValue( rVal );

        if ( mpData->bLocalizationSupport )
        {
            
            
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

void UnoControl::dispose(  ) throw(RuntimeException)
{
    Reference< XWindowPeer > xPeer;
    Reference<XComponent> xAccessibleComp;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        if( mbDisposePeer )
        {
            xPeer = mxPeer;
        }
        setPeer( NULL );
        xAccessibleComp.set(maAccessibleContext, UNO_QUERY);
        maAccessibleContext.clear();
    }
    if( xPeer.is() )
    {
        xPeer->dispose();
    }

    
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

    
    setModel( Reference< XControlModel > () );
    setContext( Reference< XInterface > () );
}

void UnoControl::addEventListener( const Reference< XEventListener >& rxListener ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( GetMutex() );

    maDisposeListeners.addInterface( rxListener );
}

void UnoControl::removeEventListener( const Reference< XEventListener >& rxListener ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( GetMutex() );

    maDisposeListeners.removeInterface( rxListener );
}

sal_Bool UnoControl::requiresNewPeer( const OUString& /* _rPropertyName */ ) const
{
    return sal_False;
}


void UnoControl::propertiesChange( const Sequence< PropertyChangeEvent >& rEvents ) throw(RuntimeException)
{
    Sequence< PropertyChangeEvent > aEvents( rEvents );
    {
        ::osl::MutexGuard aGuard( GetMutex() );

        if ( !mpData->aSuspendedPropertyNotifications.empty() )
        {
            
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
            
            

        sal_Bool bNeedNewPeer = sal_False;
            

        Reference< XControlModel > xOwnModel( getModel(), UNO_QUERY );
            
        Reference< XPropertySet > xPS( xOwnModel, UNO_QUERY );
        Reference< XPropertySetInfo > xPSI( xPS->getPropertySetInfo(), UNO_QUERY );
        OSL_ENSURE( xPSI.is(), "UnoControl::ImplModelPropertiesChanged: should have property set meta data!" );

        const PropertyChangeEvent* pEvents = rEvents.getConstArray();

        sal_Int32 nLen = rEvents.getLength();
        aPeerPropertiesToSet.reserve(nLen);

        for( sal_Int32 i = 0; i < nLen; ++i, ++pEvents )
        {
            Reference< XControlModel > xModel( pEvents->Source, UNO_QUERY );
            sal_Bool bOwnModel = xModel.get() == xOwnModel.get();
            if ( !bOwnModel )
                continue;

            
            
            if ( pEvents->PropertyName == "ResourceResolver" )
            {
                Reference< resource::XStringResourceResolver > xStrResolver;
                if ( pEvents->NewValue >>= xStrResolver )
                    bResourceResolverSet = xStrResolver.is();
            }

            sal_uInt16 nPType = GetPropertyId( pEvents->PropertyName );
            if ( mbDesignMode && mbDisposePeer && !mbRefeshingPeer && !mbCreatingPeer )
            {
                
                
                
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
                
                
                
                aPeerPropertiesToSet.push_back(PropertyValue(pEvents->PropertyName, 0, pEvents->NewValue, PropertyState_DIRECT_VALUE));
            }
            else
            {
                if ( bResourceResolverSet )
                {
                    
                    
                    
                    
                    aPeerPropertiesToSet.insert(
                        aPeerPropertiesToSet.begin(),
                        PropertyValue( pEvents->PropertyName, 0, pEvents->NewValue, PropertyState_DIRECT_VALUE ) );
                    ++nIndependentPos;
                }
                else if ( nPType == BASEPROPERTY_NATIVE_WIDGET_LOOK )
                {
                    
                    
                    
                    
                    
                    
                    
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
        Reference< XControl > xThis( (XAggregation*)(::cppu::OWeakAggObject*)this, UNO_QUERY );
        

        DBG_ASSERT( !bNeedNewPeer || xParent.is(), "Need new peer, but don't have a parent!" );

        
        if ( !bNeedNewPeer && bResourceResolverSet )
        {
            
            
            
            const LanguageDependentProp* pLangDepProp = aLanguageDependentProp;
            while ( pLangDepProp->pPropName != 0 )
            {
                bool bMustBeInserted( true );
                for ( sal_uInt32 i = 0; i < aPeerPropertiesToSet.size(); i++ )
                {
                    if ( aPeerPropertiesToSet[i].Name.equalsAsciiL(
                            pLangDepProp->pPropName, pLangDepProp->nPropNameLength ))
                    {
                        bMustBeInserted = false;
                        break;
                    }
                }

                if ( bMustBeInserted )
                {
                    
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

        

        if (bNeedNewPeer && xParent.is())
        {
            SolarMutexGuard aVclGuard;
                
                
                

            
            getPeer()->dispose();
            mxPeer.clear();
            mxVclWindowPeer = NULL;
            mbRefeshingPeer = sal_True;
            Reference< XWindowPeer >    xP( xParent, UNO_QUERY );
            xThis->createPeer( Reference< XToolkit > (), xP );
            mbRefeshingPeer = sal_False;
            aPeerPropertiesToSet.clear();
        }

        
        
        
        
        
        
        Window* pVclPeer = VCLUnoHelper::GetWindow( getPeer() );
        VCLXWindow* pPeer = pVclPeer ? pVclPeer->GetWindowPeer() : NULL;
        VclListenerLock aNoVclEventMultiplexing( pPeer );

        
        
        
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

void UnoControl::disposing( const EventObject& rEvt ) throw(RuntimeException)
{
    ::osl::ClearableMutexGuard aGuard( GetMutex() );
    

    if ( maAccessibleContext.get() == rEvt.Source )
    {
        
        maAccessibleContext = NULL;
    }
    else if( mxModel.get() == Reference< XControlModel >(rEvt.Source,UNO_QUERY).get() )
    {
        
        Reference< XControl >  xThis = this;

        aGuard.clear();
        xThis->dispose();

        DBG_ASSERT( !mxModel.is(), "UnoControl::disposing: invalid dispose behaviour!" );
        mxModel.clear();
    }
}


void SAL_CALL UnoControl::setOutputSize( const awt::Size& aSize ) throw (RuntimeException)
{
    Reference< XWindow2 > xPeerWindow;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        xPeerWindow = xPeerWindow.query( getPeer() );
    }

    if ( xPeerWindow.is() )
        xPeerWindow->setOutputSize( aSize );
}

namespace
{
    template < typename RETVALTYPE >
    RETVALTYPE lcl_askPeer( const uno::Reference< awt::XWindowPeer >& _rxPeer, RETVALTYPE (SAL_CALL XWindow2::*_pMethod)(), RETVALTYPE _aDefault )
    {
        RETVALTYPE aReturn( _aDefault );

        Reference< XWindow2 > xPeerWindow( _rxPeer, UNO_QUERY );
        if ( xPeerWindow.is() )
            aReturn = (xPeerWindow.get()->*_pMethod)();

        return aReturn;
    }
}

awt::Size SAL_CALL UnoControl::getOutputSize(  ) throw (RuntimeException)
{
    return lcl_askPeer( getPeer(), &XWindow2::getOutputSize, awt::Size() );
}

::sal_Bool SAL_CALL UnoControl::isVisible(  ) throw (RuntimeException)
{
    return lcl_askPeer( getPeer(), &XWindow2::isVisible, maComponentInfos.bVisible );
}

::sal_Bool SAL_CALL UnoControl::isActive(  ) throw (RuntimeException)
{
    return lcl_askPeer( getPeer(), &XWindow2::isActive, sal_False );
}

::sal_Bool SAL_CALL UnoControl::isEnabled(  ) throw (RuntimeException)
{
    return lcl_askPeer( getPeer(), &XWindow2::isEnabled, maComponentInfos.bEnable );
}

::sal_Bool SAL_CALL UnoControl::hasFocus(  ) throw (RuntimeException)
{
    return lcl_askPeer( getPeer(), &XWindow2::hasFocus, sal_False );
}


void UnoControl::setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags ) throw(RuntimeException)
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

        xWindow = xWindow.query( getPeer() );
    }

    if( xWindow.is() )
        xWindow->setPosSize( X, Y, Width, Height, Flags );
}

awt::Rectangle UnoControl::getPosSize(  ) throw(RuntimeException)
{
    awt::Rectangle aRect( maComponentInfos.nX, maComponentInfos.nY, maComponentInfos.nWidth, maComponentInfos.nHeight);
    Reference< XWindow > xWindow;

    {
        ::osl::MutexGuard aGuard( GetMutex() );
        xWindow = xWindow.query( getPeer() );
    }

    if( xWindow.is() )
        aRect = xWindow->getPosSize();
    return aRect;
}

void UnoControl::setVisible( sal_Bool bVisible ) throw(RuntimeException)
{
    Reference< XWindow > xWindow;
    {
        ::osl::MutexGuard aGuard( GetMutex() );

        
        maComponentInfos.bVisible = bVisible;
        xWindow = xWindow.query( getPeer() );
    }
    if ( xWindow.is() )
        xWindow->setVisible( bVisible );
}

void UnoControl::setEnable( sal_Bool bEnable ) throw(RuntimeException)
{
    Reference< XWindow > xWindow;
    {
        ::osl::MutexGuard aGuard( GetMutex() );

        
        maComponentInfos.bEnable = bEnable;
        xWindow = xWindow.query( getPeer() );
    }
    if ( xWindow.is() )
        xWindow->setEnable( bEnable );
}

void UnoControl::setFocus(  ) throw(RuntimeException)
{
    Reference< XWindow > xWindow;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        xWindow = xWindow.query( getPeer() );
    }
    if ( xWindow.is() )
        xWindow->setFocus();
}

void UnoControl::addWindowListener( const Reference< XWindowListener >& rxListener ) throw(RuntimeException)
{
    Reference< XWindow > xPeerWindow;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        maWindowListeners.addInterface( rxListener );
        if ( maWindowListeners.getLength() == 1 )
            xPeerWindow = xPeerWindow.query( getPeer() );
    }
    if ( xPeerWindow.is() )
        xPeerWindow->addWindowListener( &maWindowListeners );
}

void UnoControl::removeWindowListener( const Reference< XWindowListener >& rxListener ) throw(RuntimeException)
{
    Reference< XWindow > xPeerWindow;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        if ( maWindowListeners.getLength() == 1 )
            xPeerWindow = xPeerWindow.query( getPeer() );
        maWindowListeners.removeInterface( rxListener );
    }
    if ( xPeerWindow.is() )
        xPeerWindow->removeWindowListener( &maWindowListeners );
}

void UnoControl::addFocusListener( const Reference< XFocusListener >& rxListener ) throw(RuntimeException)
{
    Reference< XWindow > xPeerWindow;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        maFocusListeners.addInterface( rxListener );
        if ( maFocusListeners.getLength() == 1 )
            xPeerWindow = xPeerWindow.query( getPeer() );
    }
    if ( xPeerWindow.is() )
        xPeerWindow->addFocusListener( &maFocusListeners );
}

void UnoControl::removeFocusListener( const Reference< XFocusListener >& rxListener ) throw(RuntimeException)
{
    Reference< XWindow > xPeerWindow;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        if ( maFocusListeners.getLength() == 1 )
            xPeerWindow = xPeerWindow.query( getPeer() );
        maFocusListeners.removeInterface( rxListener );
    }
    if ( xPeerWindow.is() )
        xPeerWindow->removeFocusListener( &maFocusListeners );
}

void UnoControl::addKeyListener( const Reference< XKeyListener >& rxListener ) throw(RuntimeException)
{
    Reference< XWindow > xPeerWindow;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        maKeyListeners.addInterface( rxListener );
        if ( maKeyListeners.getLength() == 1 )
            xPeerWindow = xPeerWindow.query( getPeer() );
    }
    if ( xPeerWindow.is() )
        xPeerWindow->addKeyListener( &maKeyListeners);
}

void UnoControl::removeKeyListener( const Reference< XKeyListener >& rxListener ) throw(RuntimeException)
{
    Reference< XWindow > xPeerWindow;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        if ( maKeyListeners.getLength() == 1 )
            xPeerWindow = xPeerWindow.query( getPeer() );
        maKeyListeners.removeInterface( rxListener );
    }
    if ( xPeerWindow.is() )
        xPeerWindow->removeKeyListener( &maKeyListeners);
}

void UnoControl::addMouseListener( const Reference< XMouseListener >& rxListener ) throw(RuntimeException)
{
    Reference< XWindow > xPeerWindow;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        maMouseListeners.addInterface( rxListener );
        if ( maMouseListeners.getLength() == 1 )
            xPeerWindow = xPeerWindow.query( getPeer() );
    }
    if ( xPeerWindow.is() )
        xPeerWindow->addMouseListener( &maMouseListeners);
}

void UnoControl::removeMouseListener( const Reference< XMouseListener >& rxListener ) throw(RuntimeException)
{
    Reference< XWindow > xPeerWindow;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        if ( maMouseListeners.getLength() == 1 )
            xPeerWindow = xPeerWindow.query( getPeer() );
        maMouseListeners.removeInterface( rxListener );
    }
    if ( xPeerWindow.is() )
        xPeerWindow->removeMouseListener( &maMouseListeners );
}

void UnoControl::addMouseMotionListener( const Reference< XMouseMotionListener >& rxListener ) throw(RuntimeException)
{
    Reference< XWindow > xPeerWindow;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        maMouseMotionListeners.addInterface( rxListener );
        if ( maMouseMotionListeners.getLength() == 1 )
            xPeerWindow = xPeerWindow.query( getPeer() );
    }
    if ( xPeerWindow.is() )
        xPeerWindow->addMouseMotionListener( &maMouseMotionListeners);
}

void UnoControl::removeMouseMotionListener( const Reference< XMouseMotionListener >& rxListener ) throw(RuntimeException)
{
    Reference< XWindow > xPeerWindow;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        if ( maMouseMotionListeners.getLength() == 1 )
            xPeerWindow = xPeerWindow.query( getPeer() );
        maMouseMotionListeners.removeInterface( rxListener );
    }
    if ( xPeerWindow.is() )
        xPeerWindow->removeMouseMotionListener( &maMouseMotionListeners );
}

void UnoControl::addPaintListener( const Reference< XPaintListener >& rxListener ) throw(RuntimeException)
{
    Reference< XWindow > xPeerWindow;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        maPaintListeners.addInterface( rxListener );
        if ( maPaintListeners.getLength() == 1 )
            xPeerWindow = xPeerWindow.query( getPeer() );
    }
    if ( xPeerWindow.is() )
        xPeerWindow->addPaintListener( &maPaintListeners);
}

void UnoControl::removePaintListener( const Reference< XPaintListener >& rxListener ) throw(RuntimeException)
{
    Reference< XWindow > xPeerWindow;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        if ( maPaintListeners.getLength() == 1 )
            xPeerWindow = xPeerWindow.query( getPeer() );
        maPaintListeners.removeInterface( rxListener );
    }
    if ( xPeerWindow.is() )
        xPeerWindow->removePaintListener( &maPaintListeners );
}


sal_Bool UnoControl::setGraphics( const Reference< XGraphics >& rDevice ) throw(RuntimeException)
{
    Reference< XView > xView;
    {
        ::osl::MutexGuard aGuard( GetMutex() );

        mxGraphics = rDevice;
        xView = xView.query( getPeer() );
    }
    return xView.is() ? xView->setGraphics( rDevice ) : sal_True;
}

Reference< XGraphics > UnoControl::getGraphics(  ) throw(RuntimeException)
{
    return mxGraphics;
}

awt::Size UnoControl::getSize(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( GetMutex() );
    return awt::Size( maComponentInfos.nWidth, maComponentInfos.nHeight );
}

void UnoControl::draw( sal_Int32 x, sal_Int32 y ) throw(RuntimeException)
{
    Reference< XWindowPeer > xDrawPeer;
    Reference< XView > xDrawPeerView;

    bool bDisposeDrawPeer( false );
    {
        ::osl::MutexGuard aGuard( GetMutex() );

        xDrawPeer = ImplGetCompatiblePeer( sal_True );
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

void UnoControl::setZoom( float fZoomX, float fZoomY ) throw(RuntimeException)
{
    Reference< XView > xView;
    {
        ::osl::MutexGuard aGuard( GetMutex() );

        maComponentInfos.nZoomX = fZoomX;
        maComponentInfos.nZoomY = fZoomY;

        xView = xView.query( getPeer() );
    }
    if ( xView.is() )
        xView->setZoom( fZoomX, fZoomY );
}


void UnoControl::setContext( const Reference< XInterface >& rxContext ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( GetMutex() );

    mxContext = rxContext;
}

Reference< XInterface > UnoControl::getContext(  ) throw(RuntimeException)
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

void UnoControl::createPeer( const Reference< XToolkit >& rxToolkit, const Reference< XWindowPeer >& rParentPeer ) throw(RuntimeException)
{
    ::osl::ClearableMutexGuard aGuard( GetMutex() );
    if ( !mxModel.is() )
    {
        RuntimeException aException;
        aException.Message = "createPeer: no model!";
        aException.Context = (XAggregation*)(::cppu::OWeakAggObject*)this;
        throw( aException );
    }

    if( !getPeer().is() )
    {
        mbCreatingPeer = sal_True;

        WindowClass eType;
        Reference< XToolkit >  xToolkit = rxToolkit;
        if( rParentPeer.is() && mxContext.is() )
        {
            
            if ( !xToolkit.is() )
                xToolkit = rParentPeer->getToolkit();
            Any aAny = OWeakAggObject::queryInterface( ::getCppuType((const Reference< XControlContainer>*)0) );
            Reference< XControlContainer > xC;
            aAny >>= xC;
            if( xC.is() )
                
                eType = WindowClass_CONTAINER;
            else
                eType = WindowClass_SIMPLE;
        }
        else
        { 
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

        
        if ( aDescr.Type == WindowClass_TOP )
        {
            aPropName = GetPropertyName( BASEPROPERTY_DESKTOP_AS_PARENT );
            if ( xInfo->hasPropertyByName( aPropName ) )
            {
                aVal = xPSet->getPropertyValue( aPropName );
                sal_Bool b = sal_Bool();
                if ( ( aVal >>= b ) && b)
                    aDescr.ParentIndex = -1;
            }
        }
        
        aPropName = GetPropertyName( BASEPROPERTY_MOVEABLE );
        if ( xInfo->hasPropertyByName( aPropName ) )
        {
            aVal = xPSet->getPropertyValue( aPropName );
            sal_Bool b = sal_Bool();
            if ( ( aVal >>= b ) && b)
                aDescr.WindowAttributes |= WindowAttribute::MOVEABLE;
        }

        
        aPropName = GetPropertyName( BASEPROPERTY_CLOSEABLE );
        if ( xInfo->hasPropertyByName( aPropName ) )
        {
            aVal = xPSet->getPropertyValue( aPropName );
            sal_Bool b = sal_Bool();
            if ( ( aVal >>= b ) && b)
                aDescr.WindowAttributes |= WindowAttribute::CLOSEABLE;
        }

        
        aPropName = GetPropertyName( BASEPROPERTY_DROPDOWN );
        if ( xInfo->hasPropertyByName( aPropName ) )
        {
            aVal = xPSet->getPropertyValue( aPropName );
            sal_Bool b = sal_Bool();
            if ( ( aVal >>= b ) && b)
                aDescr.WindowAttributes |= VclWindowPeerAttribute::DROPDOWN;
        }

        
        aPropName = GetPropertyName( BASEPROPERTY_SPIN );
        if ( xInfo->hasPropertyByName( aPropName ) )
        {
            aVal = xPSet->getPropertyValue( aPropName );
            sal_Bool b = sal_Bool();
            if ( ( aVal >>= b ) && b)
                aDescr.WindowAttributes |= VclWindowPeerAttribute::SPIN;
        }

        
        aPropName = GetPropertyName( BASEPROPERTY_HSCROLL );
        if ( xInfo->hasPropertyByName( aPropName ) )
        {
            aVal = xPSet->getPropertyValue( aPropName );
            sal_Bool b = sal_Bool();
            if ( ( aVal >>= b ) && b)
                aDescr.WindowAttributes |= VclWindowPeerAttribute::HSCROLL;
        }

        
        aPropName = GetPropertyName( BASEPROPERTY_VSCROLL );
        if ( xInfo->hasPropertyByName( aPropName ) )
        {
            aVal = xPSet->getPropertyValue( aPropName );
            sal_Bool b = sal_Bool();
            if ( ( aVal >>= b ) && b)
                aDescr.WindowAttributes |= VclWindowPeerAttribute::VSCROLL;
        }

        
        aPropName = GetPropertyName( BASEPROPERTY_AUTOHSCROLL );
        if ( xInfo->hasPropertyByName( aPropName ) )
        {
            aVal = xPSet->getPropertyValue( aPropName );
            sal_Bool b = sal_Bool();
            if ( ( aVal >>= b ) && b)
                aDescr.WindowAttributes |= VclWindowPeerAttribute::AUTOHSCROLL;
        }

        
        aPropName = GetPropertyName( BASEPROPERTY_AUTOVSCROLL );
        if ( xInfo->hasPropertyByName( aPropName ) )
        {
            aVal = xPSet->getPropertyValue( aPropName );
            sal_Bool b = sal_Bool();
            if ( ( aVal >>= b ) && b)
                aDescr.WindowAttributes |= VclWindowPeerAttribute::AUTOVSCROLL;
        }

        
        
        aPropName = GetPropertyName( BASEPROPERTY_NOLABEL );
        if ( xInfo->hasPropertyByName( aPropName ) )
        {
            aVal = xPSet->getPropertyValue( aPropName );
            sal_Bool b = sal_Bool();
            if ( ( aVal >>=b ) && b )
                aDescr.WindowAttributes |= VclWindowPeerAttribute::NOLABEL;
        }
        

        
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

        
        PrepareWindowDescriptor(aDescr);

        
        setPeer( xToolkit->createWindow( aDescr ) );

        
        
        
        
        
        
        
        
        
        UnoControlComponentInfos aComponentInfos(maComponentInfos);
        sal_Bool bDesignMode(mbDesignMode);

        Reference< XGraphics >  xGraphics( mxGraphics           );
        Reference< XView >      xView    ( getPeer(), UNO_QUERY_THROW );
        Reference< XWindow >    xWindow  ( getPeer(), UNO_QUERY_THROW );

        aGuard.clear();

        
        
        
        
        updateFromModel();

        xView->setZoom( aComponentInfos.nZoomX, aComponentInfos.nZoomY );

        setPosSize( aComponentInfos.nX, aComponentInfos.nY, aComponentInfos.nWidth, aComponentInfos.nHeight, aComponentInfos.nFlags );

        if( aComponentInfos.bVisible && !bDesignMode )
            
            xWindow->setVisible( aComponentInfos.bVisible );

        if( !aComponentInfos.bEnable )
            xWindow->setEnable( aComponentInfos.bEnable );

        xView->setGraphics( xGraphics );

        peerCreated();

        mbCreatingPeer = sal_False;
    }
}

Reference< XWindowPeer > UnoControl::getPeer() throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( GetMutex() );
    return mxPeer;
}

sal_Bool UnoControl::setModel( const Reference< XControlModel >& rxModel ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( GetMutex() );

    Reference< XMultiPropertySet > xPropSet( mxModel, UNO_QUERY );

    
    Reference< XPropertiesChangeListener > xListener;
    queryInterface( ::getCppuType( &xListener ) ) >>= xListener;

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

Reference< XControlModel > UnoControl::getModel(    ) throw(RuntimeException)
{
    return mxModel;
}

Reference< XView > UnoControl::getView(  ) throw(RuntimeException)
{
    return  static_cast< XView* >( this );
}

void UnoControl::setDesignMode( sal_Bool bOn ) throw(RuntimeException)
{
    ModeChangeEvent aModeChangeEvent;

    Reference< XWindow > xWindow;
    Reference<XComponent> xAccessibleComp;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        if ( bOn == mbDesignMode )
            return;

        
        mbDesignMode = bOn;
        xWindow = xWindow.query( getPeer() );

        xAccessibleComp.set(maAccessibleContext, UNO_QUERY);
        maAccessibleContext.clear();

        aModeChangeEvent.Source = *this;
        aModeChangeEvent.NewMode = mbDesignMode ? OUString("design") : OUString("alive" );
    }

    
    
    
    DisposeAccessibleContext(xAccessibleComp);

    
    if ( xWindow.is() )
        xWindow->setVisible( !bOn );

    
    maModeChangeListeners.notifyEach( &XModeChangeListener::modeChanged, aModeChangeEvent );
}

sal_Bool UnoControl::isDesignMode(  ) throw(RuntimeException)
{
    return mbDesignMode;
}

sal_Bool UnoControl::isTransparent(  ) throw(RuntimeException)
{
    return sal_False;
}


OUString UnoControl::getImplementationName(  ) throw(RuntimeException)
{
    OSL_FAIL( "This method should be overloaded!" );
    return OUString();
}

sal_Bool UnoControl::supportsService( const OUString& rServiceName ) throw(RuntimeException)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > UnoControl::getSupportedServiceNames(  ) throw(RuntimeException)
{
    OUString sName( "com.sun.star.awt.UnoControl" );
    return Sequence< OUString >( &sName, 1 );
}


Reference< XAccessibleContext > SAL_CALL UnoControl::getAccessibleContext(  ) throw (RuntimeException)
{
    
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( GetMutex() );

    Reference< XAccessibleContext > xCurrentContext( maAccessibleContext.get(), UNO_QUERY );
    if ( !xCurrentContext.is() )
    {
        if ( !mbDesignMode )
        {   
            Reference< XAccessible > xPeerAcc( getPeer(), UNO_QUERY );
            if ( xPeerAcc.is() )
                xCurrentContext = xPeerAcc->getAccessibleContext( );
        }
        else
            
            xCurrentContext = ::toolkit::OAccessibleControlContext::create( this );

        DBG_ASSERT( xCurrentContext.is(), "UnoControl::getAccessibleContext: invalid context (invalid peer?)!" );
        maAccessibleContext = xCurrentContext;

        
        Reference< XComponent > xContextComp( xCurrentContext, UNO_QUERY );
        if ( xContextComp.is() )
            xContextComp->addEventListener( this );
        
        
        
        
    }

    return xCurrentContext;
}

void SAL_CALL UnoControl::addModeChangeListener( const Reference< XModeChangeListener >& _rxListener ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( GetMutex() );
    maModeChangeListeners.addInterface( _rxListener );
}

void SAL_CALL UnoControl::removeModeChangeListener( const Reference< XModeChangeListener >& _rxListener ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( GetMutex() );
    maModeChangeListeners.removeInterface( _rxListener );
}

void SAL_CALL UnoControl::addModeChangeApproveListener( const Reference< XModeChangeApproveListener >& ) throw (NoSupportException, RuntimeException)
{
    throw NoSupportException( );
}

void SAL_CALL UnoControl::removeModeChangeApproveListener( const Reference< XModeChangeApproveListener >&  ) throw (NoSupportException, RuntimeException)
{
    throw NoSupportException( );
}


awt::Point SAL_CALL UnoControl::convertPointToLogic( const awt::Point& i_Point, ::sal_Int16 i_TargetUnit ) throw (IllegalArgumentException, RuntimeException)
{
    Reference< XUnitConversion > xPeerConversion;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        xPeerConversion = xPeerConversion.query( getPeer() );
    }
    if ( xPeerConversion.is() )
        return xPeerConversion->convertPointToLogic( i_Point, i_TargetUnit );
    return awt::Point( );
}


awt::Point SAL_CALL UnoControl::convertPointToPixel( const awt::Point& i_Point, ::sal_Int16 i_SourceUnit ) throw (IllegalArgumentException, RuntimeException)
{
    Reference< XUnitConversion > xPeerConversion;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        xPeerConversion = xPeerConversion.query( getPeer() );
    }
    if ( xPeerConversion.is() )
        return xPeerConversion->convertPointToPixel( i_Point, i_SourceUnit );
    return awt::Point( );
}


awt::Size SAL_CALL UnoControl::convertSizeToLogic( const awt::Size& i_Size, ::sal_Int16 i_TargetUnit ) throw (IllegalArgumentException, RuntimeException)
{
    Reference< XUnitConversion > xPeerConversion;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        xPeerConversion = xPeerConversion.query( getPeer() );
    }
    if ( xPeerConversion.is() )
        return xPeerConversion->convertSizeToLogic( i_Size, i_TargetUnit );
    return awt::Size( );
}


awt::Size SAL_CALL UnoControl::convertSizeToPixel( const awt::Size& i_Size, ::sal_Int16 i_SourceUnit ) throw (IllegalArgumentException, RuntimeException)
{
    Reference< XUnitConversion > xPeerConversion;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        xPeerConversion = xPeerConversion.query( getPeer() );
    }
    if ( xPeerConversion.is() )
        return xPeerConversion->convertSizeToPixel( i_Size, i_SourceUnit );
    return awt::Size( );
}


uno::Reference< awt::XStyleSettings > SAL_CALL UnoControl::getStyleSettings() throw (RuntimeException)
{
    Reference< awt::XStyleSettingsSupplier > xPeerSupplier;
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        xPeerSupplier = xPeerSupplier.query( getPeer() );
    }
    if ( xPeerSupplier.is() )
        return xPeerSupplier->getStyleSettings();
    return NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
