/*************************************************************************
 *
 *  $RCSfile: unocontrol.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 15:10:35 $
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

#ifndef _COM_SUN_STAR_AWT_XCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XControlContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_WINDOWATTRIBUTE_HPP_
#include <com/sun/star/awt/WindowAttribute.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_VCLWINDOWPEERATTRIBUTE_HPP_
#include <com/sun/star/awt/VclWindowPeerAttribute.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif

#ifndef _COM_SUN_STAR_LAN_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _TOOLKIT_CONTROLS_UNOCONTROL_HXX_
#include <toolkit/controls/unocontrol.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif
#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _TOOLS_TABLE_HXX
#include <tools/table.hxx>
#endif
#ifndef _DATE_HXX
#include <tools/date.hxx>
#endif
#ifndef _TOOLS_TIME_HXX
#include <tools/time.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

#ifndef _TOOLKIT_HELPER_PROPERTY_HXX_
#include <toolkit/helper/property.hxx>
#endif
#ifndef _TOOLKIT_HELPER_SERVICENAMES_HXX_
#include <toolkit/helper/servicenames.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef TOOLKIT_ACCESSIBLE_CONTROL_CONTEXT_HXX
#include <toolkit/controls/accessiblecontrolcontext.hxx>
#endif
#ifndef _COMPHELPER_CONTAINER_HXX_
#include <comphelper/container.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;

using ::com::sun::star::accessibility::XAccessibleContext;
using ::com::sun::star::accessibility::XAccessible;

WorkWindow* lcl_GetDefaultWindow()
{
    static WorkWindow* pW = NULL;
    if ( !pW )
        pW = new WorkWindow( NULL, 0 );
    return pW;
}

static Sequence< ::rtl::OUString> lcl_ImplGetPropertyNames( const Reference< XMultiPropertySet > & rxModel )
{
    Sequence< ::rtl::OUString> aNames;
    Reference< XPropertySetInfo >  xPSInf = rxModel->getPropertySetInfo();
    DBG_ASSERT( xPSInf.is(), "UpdateFromModel: No PropertySetInfo!" );
    if ( xPSInf.is() )
    {
        Sequence< Property> aProps = xPSInf->getProperties();
        sal_Int32 nLen = aProps.getLength();
        aNames = Sequence< ::rtl::OUString>( nLen );
        ::rtl::OUString* pNames = aNames.getArray() + nLen - 1;
        const Property* pProps = aProps.getConstArray() + nLen - 1;
        for ( sal_uInt32 n = nLen; n; --n, --pProps, --pNames)
            *pNames = pProps->Name;
    }
    return aNames;
}

//  ----------------------------------------------------
//  class UnoControl
//  ----------------------------------------------------
UnoControl::UnoControl()
    : maDisposeListeners( *this )
    , maWindowListeners( *this )
    , maFocusListeners( *this )
    , maKeyListeners( *this )
    , maMouseListeners( *this )
    , maMouseMotionListeners( *this )
    , maPaintListeners( *this )
    , maModeChangeListeners( GetMutex() )
{
    mbUpdatingModel = sal_False;
    mbDisposePeer = sal_True;
    mbRefeshingPeer = sal_False;
    mbCreatingPeer = sal_False;
    mbCreatingCompatiblePeer = sal_False;
    mbDesignMode = sal_False;
}

UnoControl::~UnoControl()
{
}

::rtl::OUString UnoControl::GetComponentServiceName()
{
    return ::rtl::OUString();
}

Reference< XWindowPeer >    UnoControl::ImplGetCompatiblePeer( sal_Bool bAcceptExistingPeer )
{
    DBG_ASSERT( !mbCreatingCompatiblePeer, "ImplGetCompatiblePeer - rekursive?" );

    mbCreatingCompatiblePeer = sal_True;

    Reference< XWindowPeer >    xP;

    if ( bAcceptExistingPeer )
        xP = getPeer();

    if ( !xP.is() )
    {
        // Peer unsichtbar erzeugen...
        sal_Bool bVis = maComponentInfos.bVisible;
        if( bVis )
            maComponentInfos.bVisible = sal_False;

        Reference< XWindowPeer >    xCurrentPeer = getPeer();
        setPeer( NULL );

        // Ueber queryInterface, wegen Aggregation...
        Any aAny = OWeakAggObject::queryInterface( ::getCppuType((const Reference< XControl>*)0) );
        Reference< XControl > xMe;
        aAny >>= xMe;

        WorkWindow* pWW;
        {
        osl::Guard< vos::IMutex > aGuard( Application::GetSolarMutex() );
        pWW = lcl_GetDefaultWindow();
        }
        xMe->createPeer( Reference< XToolkit >(), pWW->GetComponentInterface( sal_True ) );
        xP = getPeer();
        setPeer( xCurrentPeer );

        if( bVis )
            maComponentInfos.bVisible = sal_True;
    }

    mbCreatingCompatiblePeer = sal_False;

    return xP;
}

void UnoControl::ImplSetPeerProperty( const ::rtl::OUString& rPropName, const Any& rVal )
{
    if ( mxVclWindowPeer.is() )
        // since a change made in propertiesChange, we can't be sure that this is called with an valid getPeer(),
        // this assumption may be false in some (seldom) multi-threading scenarios (cause propertiesChange
        // releases our mutex before calling here in)
        // That's why this additional check
        mxVclWindowPeer->setProperty( rPropName, rVal );
}

void UnoControl::PrepareWindowDescriptor( WindowDescriptor& rDesc )
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
                xP->queryInterface( ::getCppuType((const Reference< XWindow >*)0) ) >>= xPeer;
        }
    }
    return xPeer;
}

void UnoControl::updateFromModel()
{
    // Alle standard Properties werden ausgelesen und in das Peer uebertragen
    if( getPeer().is() )
    {
        Reference< XMultiPropertySet >  xPropSet( mxModel, UNO_QUERY );
        Sequence< ::rtl::OUString> aNames = lcl_ImplGetPropertyNames( xPropSet );
        xPropSet->firePropertiesChangeEvent( aNames, this );
    }
}


// XTypeProvider
IMPL_IMPLEMENTATION_ID( UnoControl )

void UnoControl::disposeAccessibleContext()
{
    Reference< XComponent > xContextComp( maAccessibleContext.get(), UNO_QUERY );
    if ( xContextComp.is() )
    {
        maAccessibleContext = NULL;
        try
        {
            xContextComp->removeEventListener( this );
            xContextComp->dispose();
        }
        catch( const Exception& )
        {
            DBG_ERROR( "UnoControl::disposeAccessibleContext: could not dispose my AccessibleContext!" );
        }
    }
}

void UnoControl::dispose(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( GetMutex() );

    if( getPeer().is() && mbDisposePeer )
    {
        getPeer()->dispose();
        setPeer( NULL);
    }

    // dispose and release our AccessibleContext
    disposeAccessibleContext();

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

    // Model wieder freigeben
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

// XPropertiesChangeListener
void UnoControl::propertiesChange( const Sequence< PropertyChangeEvent >& rEvents ) throw(RuntimeException)
{
    ::osl::ClearableGuard< ::osl::Mutex > aGuard( GetMutex() );

    // kommt von xModel
    if( !IsUpdatingModel() && getPeer().is() )
    {
        DECLARE_STL_VECTOR( PropertyValue, PropertyValueVector);
        PropertyValueVector     aPeerPropertiesToSet;
        sal_Int32               nIndependentPos = 0;
            // position where to insert the independent properties, dependent ones are inserted at the end of the vector

        sal_Bool bNeedNewPeer = sal_False;
            // some properties require a re-creation of the peer, 'cause they can't be changed on the fly

        Reference< XControlModel > xOwnModel( getModel(), UNO_QUERY );
            // our own model for comparison

        const PropertyChangeEvent* pEvents = rEvents.getConstArray();

        sal_Int32 nLen = rEvents.getLength();
        aPeerPropertiesToSet.reserve(nLen);

        for( sal_Int32 i = 0; i < nLen; ++i, ++pEvents )
        {
            Reference< XControlModel > xModel( pEvents->Source, UNO_QUERY );
            sal_Bool bOwnModel = xModel.get() == xOwnModel.get();
            if ( bOwnModel )
            {
                sal_uInt16 nPType = GetPropertyId( pEvents->PropertyName );
                if ( nPType && mbDesignMode && mbDisposePeer && !mbRefeshingPeer && !mbCreatingPeer )
                {
                    // Im Design-Mode koennen sich Props aendern, die eine
                    // Neuerzeugung der Peer erfordern...
                    if ( ( nPType == BASEPROPERTY_BORDER ) ||
                            ( nPType == BASEPROPERTY_MULTILINE ) ||
                            ( nPType == BASEPROPERTY_DROPDOWN ) ||
                            ( nPType == BASEPROPERTY_HSCROLL ) ||
                            ( nPType == BASEPROPERTY_VSCROLL ) ||
                            ( nPType == BASEPROPERTY_ORIENTATION ) ||
                            ( nPType == BASEPROPERTY_SPIN ) ||
                            ( nPType == BASEPROPERTY_ALIGN ) )
                    {
                        bNeedNewPeer = sal_True;
                        break;
                    }
                }
                if ( nPType && ( nLen > 1 ) && DoesDependOnOthers( nPType ) )
                {
                    // Properties die von anderen abhaengen erst hinterher einstellen,
                    // weil sie von anderen Properties abhaengig sind, die aber erst spaeter
                    // eingestellt werden, z.B. VALUE nach VALUEMIN/MAX.
                    aPeerPropertiesToSet.push_back(PropertyValue(pEvents->PropertyName, 0, pEvents->NewValue, PropertyState_DIRECT_VALUE));
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
        // call createPeer via a interface got from queryInterface, so the aggregating class can intercept it

        DBG_ASSERT( !bNeedNewPeer || xParent.is(), "Need new peer, but don't have a parent!" );

        aGuard.clear();
        // clear the guard before creating a new peer - as usual, our peer implementations use the SolarMutex
        // 82300 - 12/21/00 - FS
        if (bNeedNewPeer && xParent.is())
        {
            NAMESPACE_VOS(OGuard) aVclGuard( Application::GetSolarMutex() );
                // and now this is the final withdrawal:
                // With 83561, I have no other idea than locking the SolarMutex here ....
                // I really hate the fact that VCL is not theadsafe ....
                // 01.03.2001 - FS

            // Funktioniert beim Container nicht!
            getPeer()->dispose();
            mxPeer.clear();
            mxVclWindowPeer = NULL;
            mbRefeshingPeer = sal_True;
            Reference< XWindowPeer >    xP( xParent, UNO_QUERY );
            xThis->createPeer( Reference< XToolkit > (), xP );
            mbRefeshingPeer = sal_False;
            aPeerPropertiesToSet.clear();
        }

        // setting peer properties may result in an attemp to acquire the solar mutex, 'cause the peers
        // usually don't have an own mutex but use the SolarMutex instead.
        // To prevent deadlocks resulting from this, we do this without our own mutex locked
        // FS - 11/03/2000
        PropertyValueVectorIterator aEnd = aPeerPropertiesToSet.end();
        for (   PropertyValueVectorIterator aLoop = aPeerPropertiesToSet.begin();
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
    ::osl::MutexGuard aGuard( GetMutex() );
    // bei "Multible Inheritance" nicht unterschiedliche Typen vergleichen.

    if( mxModel.get() == Reference< XControlModel >(rEvt.Source,UNO_QUERY).get() )
    {
        // #62337# Ohne Model wollen wir nicht weiterleben
        Reference< XControl >  xThis = this;
        xThis->dispose();

        DBG_ASSERT( !mxModel.is(), "UnoControl::disposing: invalid dispose behaviour!" );
        mxModel.clear();
    }
    else if ( maAccessibleContext.get() == rEvt.Source )
    {
        // just in case the context is disposed, but not released - ensure that we do not re-use it in the future
        maAccessibleContext = NULL;
    }
}


// XWindow
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
    ::osl::MutexGuard aGuard( GetMutex() );

    awt::Rectangle aRect( maComponentInfos.nX, maComponentInfos.nY, maComponentInfos.nWidth, maComponentInfos.nHeight);

    Reference< XWindow > xWindow( getPeer(), uno::UNO_QUERY );
    if( xWindow.is() )
        aRect = xWindow->getPosSize();

    return aRect;
}

void UnoControl::setVisible( sal_Bool bVisible ) throw(RuntimeException)
{
    Reference< XWindow > xWindow;
    {
        ::osl::MutexGuard aGuard( GetMutex() );

        // Visible status ist Sache der View
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

        // Enable status ist Sache der View
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
    ::osl::MutexGuard aGuard( GetMutex() );

    maWindowListeners.addInterface( rxListener );
    if( getPeer().is() && maWindowListeners.getLength() == 1 )
    {
        // erster Focus Listener, also am Peer anmelden
        Reference< XWindow >    xW( getPeer(), UNO_QUERY );
        xW->addWindowListener( &maWindowListeners );
    }
}

void UnoControl::removeWindowListener( const Reference< XWindowListener >& rxListener ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( GetMutex() );

    if( getPeer().is() && maWindowListeners.getLength() == 1 )
    {
        // letzter Focus Listener, also am Peer abmelden
        Reference< XWindow >    xW( getPeer(), UNO_QUERY );
        xW->removeWindowListener( &maWindowListeners );
    }
    maWindowListeners.removeInterface( rxListener );
}

void UnoControl::addFocusListener( const Reference< XFocusListener >& rxListener ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( GetMutex() );

    maFocusListeners.addInterface( rxListener );
    if( getPeer().is() && maFocusListeners.getLength() == 1 )
    {
        Reference< XWindow >    xW( getPeer(), UNO_QUERY );
        xW->addFocusListener( &maFocusListeners );
    }
}

void UnoControl::removeFocusListener( const Reference< XFocusListener >& rxListener ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( GetMutex() );

    if( getPeer().is() && maFocusListeners.getLength() == 1 )
    {
        Reference< XWindow >    xW( getPeer(), UNO_QUERY );
        xW->removeFocusListener( &maFocusListeners );
    }
    maFocusListeners.removeInterface( rxListener );
}

void UnoControl::addKeyListener( const Reference< XKeyListener >& rxListener ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( GetMutex() );

    maKeyListeners.addInterface( rxListener );
    if( getPeer().is() && maKeyListeners.getLength() == 1 )
    {
        Reference< XWindow >    xW( getPeer(), UNO_QUERY );
        xW->addKeyListener( &maKeyListeners);
    }
}

void UnoControl::removeKeyListener( const Reference< XKeyListener >& rxListener ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( GetMutex() );

    if( getPeer().is() && maKeyListeners.getLength() == 1 )
    {
        Reference< XWindow >    xW( getPeer(), UNO_QUERY );
        xW->removeKeyListener( &maKeyListeners);
    }
    maKeyListeners.removeInterface( rxListener );
}

void UnoControl::addMouseListener( const Reference< XMouseListener >& rxListener ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( GetMutex() );

    maMouseListeners.addInterface( rxListener );
    if( getPeer().is() && maMouseListeners.getLength() == 1 )
    {
        Reference< XWindow >    xW( getPeer(), UNO_QUERY );;
        xW->addMouseListener( &maMouseListeners);
    }
}

void UnoControl::removeMouseListener( const Reference< XMouseListener >& rxListener ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( GetMutex() );

    if( getPeer().is() && maMouseListeners.getLength() == 1 )
    {
        Reference< XWindow >    xW( getPeer(), UNO_QUERY );
        xW->removeMouseListener( &maMouseListeners);
    }
    maMouseListeners.removeInterface( rxListener );
}

void UnoControl::addMouseMotionListener( const Reference< XMouseMotionListener >& rxListener ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( GetMutex() );

    maMouseMotionListeners.addInterface( rxListener );
    if( getPeer().is() && maMouseMotionListeners.getLength() == 1 )
    {
        Reference< XWindow >    xW( getPeer(), UNO_QUERY );
        xW->addMouseMotionListener( &maMouseMotionListeners);
    }
}

void UnoControl::removeMouseMotionListener( const Reference< XMouseMotionListener >& rxListener ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( GetMutex() );

    if( getPeer().is() && maMouseMotionListeners.getLength() == 1 )
    {
        Reference< XWindow >    xW( getPeer(), UNO_QUERY );
        xW->removeMouseMotionListener( &maMouseMotionListeners);
    }
    maMouseMotionListeners.removeInterface( rxListener );
}

void UnoControl::addPaintListener( const Reference< XPaintListener >& rxListener ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( GetMutex() );

    maPaintListeners.addInterface( rxListener );
    if( getPeer().is() && maPaintListeners.getLength() == 1 )
    {
        Reference< XWindow >    xW( getPeer(), UNO_QUERY );
        xW->addPaintListener( &maPaintListeners );
    }
}

void UnoControl::removePaintListener( const Reference< XPaintListener >& rxListener ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( GetMutex() );

    if( getPeer().is() && maPaintListeners.getLength() == 1 )
    {
        Reference< XWindow >    xW( getPeer(), UNO_QUERY );
        xW->removePaintListener( &maPaintListeners );
    }
    maPaintListeners.removeInterface( rxListener );
}

// XView
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
    ::osl::MutexGuard aGuard( GetMutex() );

    Reference< XWindowPeer >    xP = ImplGetCompatiblePeer( sal_True );
    DBG_ASSERT( xP.is(), "Layout: No Peer!" );
    if ( xP.is() )
    {
        Reference< XView >  xV( xP, UNO_QUERY );
        xV->draw( x, y );

        if ( !getPeer().is() || ( getPeer() != xP ) )
            xP->dispose();
    }
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

// XControl
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

void UnoControl::createPeer( const Reference< XToolkit >& rxToolkit, const Reference< XWindowPeer >& rParentPeer ) throw(RuntimeException)
{
    ::osl::ClearableMutexGuard aGuard( GetMutex() );

    if ( !mxModel.is() )
    {
        RuntimeException aException;
        aException.Message = ::rtl::OUString::createFromAscii( "createPeer: no model!" );
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
            // kein TopWindow
            if ( !xToolkit.is() )
                xToolkit = rParentPeer->getToolkit();
            Any aAny = OWeakAggObject::queryInterface( ::getCppuType((const Reference< XControlContainer>*)0) );
            Reference< XControlContainer > xC;
            aAny >>= xC;
            if( xC.is() )
                // Es ist ein Container
                eType = WindowClass_CONTAINER;
            else
                eType = WindowClass_SIMPLE;
        }
        else
        { // Nur richtig, wenn es sich um ein Top Window handelt
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
        ::rtl::OUString aPropName = GetPropertyName( BASEPROPERTY_BORDER );
        if ( xInfo->hasPropertyByName( aPropName ) )
        {
            aVal = xPSet->getPropertyValue( aPropName );
            sal_Int16 n;
            if ( aVal >>= n )
            {
                if ( n )
                    aDescr.WindowAttributes |= WindowAttribute::BORDER;
                else
                    aDescr.WindowAttributes |= VclWindowPeerAttribute::NOBORDER;
            }
        }

        // Moveable
        aPropName = GetPropertyName( BASEPROPERTY_MOVEABLE );
        if ( xInfo->hasPropertyByName( aPropName ) )
        {
            aVal = xPSet->getPropertyValue( aPropName );
            sal_Bool b;
            if ( ( aVal >>= b ) && b)
                aDescr.WindowAttributes |= WindowAttribute::MOVEABLE;
        }

        // Closeable
        aPropName = GetPropertyName( BASEPROPERTY_CLOSEABLE );
        if ( xInfo->hasPropertyByName( aPropName ) )
        {
            aVal = xPSet->getPropertyValue( aPropName );
            sal_Bool b;
            if ( ( aVal >>= b ) && b)
                aDescr.WindowAttributes |= WindowAttribute::CLOSEABLE;
        }

        // Dropdown
        aPropName = GetPropertyName( BASEPROPERTY_DROPDOWN );
        if ( xInfo->hasPropertyByName( aPropName ) )
        {
            aVal = xPSet->getPropertyValue( aPropName );
            sal_Bool b;
            if ( ( aVal >>= b ) && b)
                aDescr.WindowAttributes |= VclWindowPeerAttribute::DROPDOWN;
        }

        // Spin
        aPropName = GetPropertyName( BASEPROPERTY_SPIN );
        if ( xInfo->hasPropertyByName( aPropName ) )
        {
            aVal = xPSet->getPropertyValue( aPropName );
            sal_Bool b;
            if ( ( aVal >>= b ) && b)
                aDescr.WindowAttributes |= VclWindowPeerAttribute::SPIN;
        }

        // HScroll
        aPropName = GetPropertyName( BASEPROPERTY_HSCROLL );
        if ( xInfo->hasPropertyByName( aPropName ) )
        {
            aVal = xPSet->getPropertyValue( aPropName );
            sal_Bool b;
            if ( ( aVal >>= b ) && b)
                aDescr.WindowAttributes |= VclWindowPeerAttribute::HSCROLL;
        }

        // VScroll
        aPropName = GetPropertyName( BASEPROPERTY_VSCROLL );
        if ( xInfo->hasPropertyByName( aPropName ) )
        {
            aVal = xPSet->getPropertyValue( aPropName );
            sal_Bool b;
            if ( ( aVal >>= b ) && b)
                aDescr.WindowAttributes |= VclWindowPeerAttribute::VSCROLL;
        }

        // Align
        aPropName = GetPropertyName( BASEPROPERTY_ALIGN );
        if ( xInfo->hasPropertyByName( aPropName ) )
        {
            aVal = xPSet->getPropertyValue( aPropName );
            sal_Int16 n;
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

        // Ableitungen die Moeglichkeit geben die Attribute zu manipulieren
        PrepareWindowDescriptor(aDescr);

        // create the peer
        setPeer( xToolkit->createWindow( aDescr ) );

        // release the mutex guard (and work with copies of our members)
        // this is necessary as our peer may lock the SolarMutex (actually, all currently known peers do), so calling
        // into the peer with our own mutex locked may cause deadlocks
        // (We _really_ need peers which do not use the SolarMutex. It's really pissing me off that from time to
        // time deadlocks pop up because the low-level components like our peers use a mutex which ususally
        // is locked at the top of the stack (it protects the global message looping). This is always dangerous, and
        // can not always be solved by tampering with other mutexes.
        // Unfortunately, the VCL used in the peers is not threadsafe, and by definition needs a locked SolarMutex.)
        // 82300 - 12/21/00 - FS
        UnoControlComponentInfos aComponentInfos(maComponentInfos);
        sal_Bool bDesignMode(mbDesignMode);
        Reference< XGraphics > xGraphics( mxGraphics );
        Reference< XView >  xV(getPeer(), UNO_QUERY);
        Reference< XWindow >    xW(getPeer(), UNO_QUERY);

        aGuard.clear();

        // the updateFromModel is done without a locked mutex, too.
        // The reason is that the only thing this method does  is firing property changes, and this in general has
        // to be done without locked mutexes (as every notification to external listeners).
        // 82300 - 12/21/00 - FS
        updateFromModel();

        xV->setZoom( aComponentInfos.nZoomX, aComponentInfos.nZoomY );

        setPosSize( maComponentInfos.nX, maComponentInfos.nY, maComponentInfos.nWidth, maComponentInfos.nHeight, maComponentInfos.nFlags );

        if( aComponentInfos.bVisible && !bDesignMode )
            // Erst nach dem setzen der Daten anzeigen
            xW->setVisible( aComponentInfos.bVisible );

        if( !aComponentInfos.bEnable )
            xW->setEnable( aComponentInfos.bEnable );

        if ( maWindowListeners.getLength() )
            xW->addWindowListener( &maWindowListeners );

        if ( maFocusListeners.getLength() )
            xW->addFocusListener( &maFocusListeners );

        if ( maKeyListeners.getLength() )
            xW->addKeyListener( &maKeyListeners );

        if ( maMouseListeners.getLength() )
            xW->addMouseListener( &maMouseListeners );

        if ( maMouseMotionListeners.getLength() )
            xW->addMouseMotionListener( &maMouseMotionListeners );

        if ( maPaintListeners.getLength() )
            xW->addPaintListener( &maPaintListeners );

        xV->setGraphics( xGraphics );

        mbCreatingPeer = sal_False;
    }
}

Reference< XWindowPeer > UnoControl::getPeer(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( GetMutex() );
    return mxPeer;
}

sal_Bool UnoControl::setModel( const Reference< XControlModel >& rxModel ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( GetMutex() );

    Reference< XMultiPropertySet > xPropSet( mxModel, UNO_QUERY );

    if( xPropSet.is() )
        xPropSet->removePropertiesChangeListener( this );

    mxModel = rxModel;
    if( mxModel.is() )
    {
        xPropSet = Reference< XMultiPropertySet > ( mxModel, UNO_QUERY );
        if( xPropSet.is() )
        {
            Sequence< ::rtl::OUString> aNames = lcl_ImplGetPropertyNames( xPropSet );
            xPropSet->addPropertiesChangeListener( aNames, this );
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
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        if ( bOn == mbDesignMode )
            return;

        // remember this
        mbDesignMode = bOn;
        xWindow = xWindow.query( getPeer() );

        // dispose our current AccessibleContext, if we have one
        // (changing the design mode implies having a new implementation for this context,
        // so the old one must be declared DEFUNC)
        disposeAccessibleContext();

        aModeChangeEvent.Source = *this;
        aModeChangeEvent.NewMode = ::rtl::OUString::createFromAscii( mbDesignMode ? "design" : "alive" );
    }

    // ajust the visibility of our window
    if ( xWindow.is() )
        xWindow->setVisible( !bOn );

    // and notify our mode listeners
    NOTIFY_LISTENERS( maModeChangeListeners, XModeChangeListener, modeChanged, aModeChangeEvent );
}

sal_Bool UnoControl::isDesignMode(  ) throw(RuntimeException)
{
    return mbDesignMode;
}

sal_Bool UnoControl::isTransparent(  ) throw(RuntimeException)
{
    return sal_False;
}

// XServiceInfo
::rtl::OUString UnoControl::getImplementationName(  ) throw(RuntimeException)
{
    DBG_ERROR( "This method should be overloaded!" );
    return ::rtl::OUString();
}

sal_Bool UnoControl::supportsService( const ::rtl::OUString& rServiceName ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( GetMutex() );

    Sequence< ::rtl::OUString > aSNL = getSupportedServiceNames();
    const ::rtl::OUString* pArray = aSNL.getConstArray();
    const ::rtl::OUString* pArrayEnd = aSNL.getConstArray();
    for (; pArray != pArrayEnd; ++pArray )
        if( *pArray == rServiceName )
            break;

    return pArray != pArrayEnd;
}

Sequence< ::rtl::OUString > UnoControl::getSupportedServiceNames(  ) throw(RuntimeException)
{
    ::rtl::OUString sName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControl" ) );
    return Sequence< ::rtl::OUString >( &sName, 1 );
}

// ------------------------------------------------------------------------
Reference< XAccessibleContext > SAL_CALL UnoControl::getAccessibleContext(  ) throw (RuntimeException)
{
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

void SAL_CALL UnoControl::addModeChangeApproveListener( const Reference< XModeChangeApproveListener >& _rxListener ) throw (NoSupportException, RuntimeException)
{
    throw NoSupportException( );
}

void SAL_CALL UnoControl::removeModeChangeApproveListener( const Reference< XModeChangeApproveListener >& _rxListener ) throw (NoSupportException, RuntimeException)
{
    throw NoSupportException( );
}

