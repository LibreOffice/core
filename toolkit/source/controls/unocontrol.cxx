/*************************************************************************
 *
 *  $RCSfile: unocontrol.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: mt $ $Date: 2001-03-13 15:42:53 $
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

#ifndef _COM_SUN_STAR_AWT_XVCLWINDOWPEER_HPP_
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XControlContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_WINDOWATTRIBUTE_HPP_
#include <com/sun/star/awt/WindowAttribute.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_VCLWINDOWPEERATTRIBUTE_HPP_
#include <com/sun/star/awt/VclWindowPeerAttribute.hpp>
#endif

#ifndef _COM_SUN_STAR_LAN_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#include <com/sun/star/beans/PropertyValue.hpp>

#include <toolkit/controls/unocontrol.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/memory.h>
#include <rtl/uuid.h>

#include <vos/mutex.hxx>
#include <tools/string.hxx>
#include <tools/table.hxx>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <tools/urlobj.hxx>
#include <tools/debug.hxx>

#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

#include <toolkit/helper/property.hxx>
#include <toolkit/helper/servicenames.hxx>

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

WorkWindow* lcl_GetDefaultWindow()
{
    static WorkWindow* pW = NULL;
    if ( !pW )
        pW = new WorkWindow( NULL, 0 );
    return pW;
}

static ::com::sun::star::uno::Sequence< ::rtl::OUString> lcl_ImplGetPropertyNames( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XMultiPropertySet > & rxModel )
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString> aNames;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >  xPSInf = rxModel->getPropertySetInfo();
    DBG_ASSERT( xPSInf.is(), "UpdateFromModel: No PropertySetInfo!" );
    if ( xPSInf.is() )
    {
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property> aProps = xPSInf->getProperties();
        sal_Int32 nLen = aProps.getLength();
        aNames = ::com::sun::star::uno::Sequence< ::rtl::OUString>( nLen );
        ::rtl::OUString* pNames = aNames.getArray() + nLen - 1;
        const ::com::sun::star::beans::Property* pProps = aProps.getConstArray() + nLen - 1;
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
{
    mbUpdatingModel = sal_False;
    mbDisposePeer = sal_True;
    mbRefeshingPeer = sal_False;
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

::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >  UnoControl::ImplGetCompatiblePeer( sal_Bool bAcceptExistingPeer )
{
    DBG_ASSERT( !mbCreatingCompatiblePeer, "ImplGetCompatiblePeer - rekursive?" );

    mbCreatingCompatiblePeer = sal_True;

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >  xP;

    if ( bAcceptExistingPeer )
        xP = mxPeer;

    if ( !xP.is() )
    {
        // Peer unsichtbar erzeugen...
        sal_Bool bVis = maComponentInfos.bVisible;
        if( bVis )
            maComponentInfos.bVisible = sal_False;

        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >  xCurrentPeer = mxPeer;
        mxPeer = NULL;

        // Ueber queryInterface, wegen Aggregation...
        ::com::sun::star::uno::Any aAny = OWeakAggObject::queryInterface( ::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>*)0) );
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > xMe;
        aAny >>= xMe;

        WorkWindow* pWW;
        {
        osl::Guard< vos::IMutex > aGuard( Application::GetSolarMutex() );
        pWW = lcl_GetDefaultWindow();
        }
        xMe->createPeer( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >(), pWW->GetComponentInterface( sal_True ) );
        xP = mxPeer;
        mxPeer = xCurrentPeer;

        if( bVis )
            maComponentInfos.bVisible = sal_True;
    }

    mbCreatingCompatiblePeer = sal_False;

    return xP;
}

void UnoControl::ImplSetPeerProperty( const ::rtl::OUString& rPropName, const ::com::sun::star::uno::Any& rVal )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XVclWindowPeer >  xW( mxPeer, ::com::sun::star::uno::UNO_QUERY );
    if (xW.is())
        // since a change made in propertiesChange, we can't be sure that this is called with an valid mxPeer,
        // this assumption may be false in some (seldom) multi-threading scenarios (cause propertiesChange
        // releases our mutex before calling here in)
        // That's why this additional check
        xW->setProperty( rPropName, rVal );
}

void UnoControl::PrepareWindowDescriptor( ::com::sun::star::awt::WindowDescriptor& rDesc )
{
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >  UnoControl::getParentPeer() const
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > xPeer;
    if( mxContext.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > xContComp( mxContext, ::com::sun::star::uno::UNO_QUERY );
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > xP = xContComp->getPeer();
        if ( xP.is() )
            xP->queryInterface( ::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >*)0) ) >>= xPeer;
    }
    return xPeer;
}

void UnoControl::updateFromModel()
{
    // Alle standard Properties werden ausgelesen und in das Peer uebertragen
    if( mxPeer.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XMultiPropertySet >  xPropSet( mxModel, ::com::sun::star::uno::UNO_QUERY );
        ::com::sun::star::uno::Sequence< ::rtl::OUString> aNames = lcl_ImplGetPropertyNames( xPropSet );
        xPropSet->firePropertiesChangeEvent( aNames, this );
    }
}


// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any UnoControl::queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XControl*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XWindow*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XComponent*, SAL_STATIC_CAST( ::com::sun::star::awt::XControl*, this ) ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XView*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::beans::XPropertiesChangeListener*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XEventListener*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XServiceInfo*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XTypeProvider*, this ) );
    return (aRet.hasValue() ? aRet : OWeakAggObject::queryAggregation( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoControl )
getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>* ) NULL ),
getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow>* ) NULL ),
getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XView>* ) NULL ),
getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener>* ) NULL ),
getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XServiceInfo>* ) NULL )
IMPL_XTYPEPROVIDER_END

void UnoControl::dispose(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if( mxPeer.is() && mbDisposePeer )
    {
        mxPeer->dispose();
        mxPeer = NULL;
    }

    ::com::sun::star::lang::EventObject aEvt;
    aEvt.Source = (::com::sun::star::uno::XAggregation*)(::cppu::OWeakAggObject*)this;

    maDisposeListeners.disposeAndClear(aEvt);
    maWindowListeners.disposeAndClear(aEvt);
    maFocusListeners.disposeAndClear(aEvt);
    maKeyListeners.disposeAndClear(aEvt);
    maMouseListeners.disposeAndClear(aEvt);
    maMouseMotionListeners.disposeAndClear(aEvt);
    maPaintListeners.disposeAndClear(aEvt);

    // Model wieder freigeben
    setModel( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > () );
    setContext( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > () );
}

void UnoControl::addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maDisposeListeners.addInterface( rxListener );
}

void UnoControl::removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maDisposeListeners.removeInterface( rxListener );
}

// ::com::sun::star::beans::XPropertiesChangeListener
void UnoControl::propertiesChange( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyChangeEvent >& rEvents ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::ClearableGuard< ::osl::Mutex > aGuard( GetMutex() );

    // kommt von xModel
    if( !IsUpdatingModel() && mxPeer.is() )
    {
        DECLARE_STL_VECTOR( ::com::sun::star::beans::PropertyValue, PropertyValueVector);
        PropertyValueVector     aPeerPropertiesToSet;
        sal_Int32               nIndependentPos = 0;
            // position where to insert the independent properties, dependent ones are inserted at the end of the vector

        sal_Bool bNeedNewPeer = sal_False;
            // some properties require a re-creation of the peer, 'cause they can't be changed on the fly

        sal_Int32 nLen = rEvents.getLength();
        for( sal_Int32 i = 0; i < nLen; i++ )
        {
            const ::com::sun::star::beans::PropertyChangeEvent& rEvt = rEvents.getConstArray()[i];
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > xModel( rEvt.Source, ::com::sun::star::uno::UNO_QUERY );
            sal_Bool bOwnModel = (::com::sun::star::awt::XControlModel*)xModel.get() == (::com::sun::star::awt::XControlModel*)getModel().get();
            if ( bOwnModel )
            {
                sal_uInt16 nPType = GetPropertyId( rEvt.PropertyName );
                if ( nPType && mbDesignMode && mbDisposePeer && !mbRefeshingPeer )
                {
                    // Im Design-Mode koennen sich Props aendern, die eine
                    // Neuerzeugung der Peer erfordern...
                    if ( ( nPType == BASEPROPERTY_BORDER ) ||
                            ( nPType == BASEPROPERTY_MULTILINE ) ||
                            ( nPType == BASEPROPERTY_DROPDOWN ) ||
                            ( nPType == BASEPROPERTY_HSCROLL ) ||
                            ( nPType == BASEPROPERTY_VSCROLL ) ||
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
                    aPeerPropertiesToSet.push_back(::com::sun::star::beans::PropertyValue(rEvt.PropertyName, 0, rEvt.NewValue, ::com::sun::star::beans::PropertyState_DIRECT_VALUE));
                }
                else
                {
                    aPeerPropertiesToSet.insert(aPeerPropertiesToSet.begin() + nIndependentPos,
                        ::com::sun::star::beans::PropertyValue(rEvt.PropertyName, 0, rEvt.NewValue, ::com::sun::star::beans::PropertyState_DIRECT_VALUE));
                    ++nIndependentPos;
                }
            }
        }

        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >  xParent = getParentPeer();
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > xThis( (::com::sun::star::uno::XAggregation*)(::cppu::OWeakAggObject*)this, ::com::sun::star::uno::UNO_QUERY );
            // call createPeer via a interface got from queryInterface, so the aggregating class can intercept it

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
            mxPeer->dispose();
            mxPeer.clear();
            mbRefeshingPeer = sal_True;
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >  xP( xParent, ::com::sun::star::uno::UNO_QUERY );
            xThis->createPeer( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit > (), xP );
            mbRefeshingPeer = sal_False;
        }

        // setting peer properties may result in an attemp to acquire the solar mutex, 'cause the peers
        // usually don't have an own mutex but use the SolarMutex instead.
        // To prevent deadlocks resulting from this, we do this without our own mutex locked
        // FS - 11/03/2000
        for (   PropertyValueVectorIterator aLoop = aPeerPropertiesToSet.begin();
                aLoop != aPeerPropertiesToSet.end();
                ++aLoop
            )
        {
            ImplSetPeerProperty( aLoop->Name, aLoop->Value );
        }
    }
}

void UnoControl::disposing( const ::com::sun::star::lang::EventObject& rEvt ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    // kommt von xModel, bei "Multible Inheritance" nicht unterschiedliche
    // Typen vergleichen.

    if( mxModel == rEvt.Source )
    {
        mxModel = NULL;

        // #62337# Ohne Model wollen wir nicht weiterleben
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >  xThis = this;
        xThis->dispose();
    }
}


// ::com::sun::star::awt::XWindow
void UnoControl::setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maComponentInfos.nX = X;
    maComponentInfos.nY = Y;
    maComponentInfos.nWidth = Width;
    maComponentInfos.nHeight = Height;
    maComponentInfos.nFlags = Flags;

    if( mxPeer.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >  xW( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        xW->setPosSize( X, Y, Width, Height, Flags );
    }
}

::com::sun::star::awt::Rectangle UnoControl::getPosSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return ::com::sun::star::awt::Rectangle( maComponentInfos.nX, maComponentInfos.nY, maComponentInfos.nWidth, maComponentInfos.nHeight);
}

void UnoControl::setVisible( sal_Bool bVisible ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    // Visible status ist Sache der ::com::sun::star::sdbcx::View
    maComponentInfos.bVisible = bVisible;
    if( mxPeer.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >  xW( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        xW->setVisible( maComponentInfos.bVisible );
    }
}

void UnoControl::setEnable( sal_Bool bEnable ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    // Enable status ist Sache der ::com::sun::star::sdbcx::View
    maComponentInfos.bEnable = bEnable;
    if( mxPeer.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > xW( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        xW->setEnable( maComponentInfos.bEnable );
    }
}

void UnoControl::setFocus(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if( mxPeer.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >  xW( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        xW->setFocus();
    }
}

void UnoControl::addWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maWindowListeners.addInterface( rxListener );
    if( mxPeer.is() && maWindowListeners.getLength() == 1 )
    {
        // erster Focus Listener, also am Peer anmelden
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >  xW( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        xW->addWindowListener( &maWindowListeners );
    }
}

void UnoControl::removeWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if( mxPeer.is() && maWindowListeners.getLength() == 1 )
    {
        // letzter Focus Listener, also am Peer abmelden
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >  xW( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        xW->removeWindowListener( &maWindowListeners );
    }
    maWindowListeners.removeInterface( rxListener );
}

void UnoControl::addFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maFocusListeners.addInterface( rxListener );
    if( mxPeer.is() && maFocusListeners.getLength() == 1 )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >  xW( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        xW->addFocusListener( &maFocusListeners );
    }
}

void UnoControl::removeFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if( mxPeer.is() && maFocusListeners.getLength() == 1 )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >  xW( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        xW->removeFocusListener( &maFocusListeners );
    }
    maFocusListeners.removeInterface( rxListener );
}

void UnoControl::addKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maKeyListeners.addInterface( rxListener );
    if( mxPeer.is() && maKeyListeners.getLength() == 1 )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >  xW( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        xW->addKeyListener( &maKeyListeners);
    }
}

void UnoControl::removeKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if( mxPeer.is() && maKeyListeners.getLength() == 1 )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >  xW( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        xW->removeKeyListener( &maKeyListeners);
    }
    maKeyListeners.removeInterface( rxListener );
}

void UnoControl::addMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maMouseListeners.addInterface( rxListener );
    if( mxPeer.is() && maMouseListeners.getLength() == 1 )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >  xW( mxPeer, ::com::sun::star::uno::UNO_QUERY );;
        xW->addMouseListener( &maMouseListeners);
    }
}

void UnoControl::removeMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if( mxPeer.is() && maMouseListeners.getLength() == 1 )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >  xW( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        xW->removeMouseListener( &maMouseListeners);
    }
    maMouseListeners.removeInterface( rxListener );
}

void UnoControl::addMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maMouseMotionListeners.addInterface( rxListener );
    if( mxPeer.is() && maMouseMotionListeners.getLength() == 1 )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >  xW( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        xW->addMouseMotionListener( &maMouseMotionListeners);
    }
}

void UnoControl::removeMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if( mxPeer.is() && maMouseMotionListeners.getLength() == 1 )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >  xW( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        xW->removeMouseMotionListener( &maMouseMotionListeners);
    }
    maMouseMotionListeners.removeInterface( rxListener );
}

void UnoControl::addPaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maPaintListeners.addInterface( rxListener );
    if( mxPeer.is() && maPaintListeners.getLength() == 1 )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >  xW( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        xW->addPaintListener( &maPaintListeners );
    }
}

void UnoControl::removePaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if( mxPeer.is() && maPaintListeners.getLength() == 1 )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >  xW( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        xW->removePaintListener( &maPaintListeners );
    }
    maPaintListeners.removeInterface( rxListener );
}

// ::com::sun::star::awt::XView
sal_Bool UnoControl::setGraphics( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >& rDevice ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    mxGraphics = rDevice;
    if ( mxPeer.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XView >  xV( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        if ( xV.is() )
            xV->setGraphics( mxGraphics );
    }
    return sal_True;
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics > UnoControl::getGraphics(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mxGraphics;
}

::com::sun::star::awt::Size UnoControl::getSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    return ::com::sun::star::awt::Size( maComponentInfos.nWidth, maComponentInfos.nHeight );
}

void UnoControl::draw( sal_Int32 x, sal_Int32 y ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >  xP = ImplGetCompatiblePeer( sal_True );
    DBG_ASSERT( xP.is(), "Layout: No Peer!" );
    if ( xP.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XView >  xV( xP, ::com::sun::star::uno::UNO_QUERY );
        xV->draw( x, y );

        if ( !mxPeer.is() || ( mxPeer != xP ) )
            xP->dispose();
    }
}

void UnoControl::setZoom( float fZoomX, float fZoomY ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maComponentInfos.nZoomX = fZoomX;
    maComponentInfos.nZoomY = fZoomY;
    if ( mxPeer.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XView >  xV( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        xV->setZoom( fZoomX, fZoomY );
    }
}

// ::com::sun::star::awt::XControl
void UnoControl::setContext( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxContext ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    mxContext = rxContext;
}

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > UnoControl::getContext(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mxContext;
}

void UnoControl::createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& rxToolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& rParentPeer ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::ClearableMutexGuard aGuard( GetMutex() );

    if( !mxPeer.is() )
    {
        ::com::sun::star::awt::WindowClass eType;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >  xToolkit = rxToolkit;
        if( rParentPeer.is() && mxContext.is() )
        {
            // kein TopWindow
            if ( !xToolkit.is() )
                xToolkit = rParentPeer->getToolkit();
            ::com::sun::star::uno::Any aAny = OWeakAggObject::queryInterface( ::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer>*)0) );
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer > xC;
            aAny >>= xC;
            if( xC.is() )
                // Es ist ein Container
                eType = ::com::sun::star::awt::WindowClass_CONTAINER;
            else
                eType = ::com::sun::star::awt::WindowClass_SIMPLE;
        }
        else
        { // Nur richtig, wenn es sich um ein Top Window handelt
            if( rParentPeer.is() )
            {
                if ( !xToolkit.is() )
                    xToolkit = rParentPeer->getToolkit();
                eType = ::com::sun::star::awt::WindowClass_CONTAINER;
            }
            else
            {
                if ( !xToolkit.is() )
                    xToolkit = VCLUnoHelper::CreateToolkit();
                eType = ::com::sun::star::awt::WindowClass_TOP;
            }
        }
         ::com::sun::star::awt::WindowDescriptor aDescr;
        aDescr.Type = eType;
        aDescr.WindowServiceName = GetComponentServiceName();
        aDescr.Parent = rParentPeer;
        aDescr.Bounds = getPosSize();
        aDescr.WindowAttributes = 0;

        // ::com::sun::star::sheet::Border
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xPSet( mxModel, ::com::sun::star::uno::UNO_QUERY );
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >  xInfo = xPSet->getPropertySetInfo();

        ::com::sun::star::uno::Any aVal;
        ::rtl::OUString aPropName = GetPropertyName( BASEPROPERTY_BORDER );
        if ( xInfo->hasPropertyByName( aPropName ) )
        {
            aVal = xPSet->getPropertyValue( aPropName );
            sal_Int16 n;
            if ( aVal >>= n )
            {
                if ( n )
                    aDescr.WindowAttributes |= ::com::sun::star::awt::WindowAttribute::BORDER;
                else
                    aDescr.WindowAttributes |= ::com::sun::star::awt::VclWindowPeerAttribute::NOBORDER;
            }
        }

        // Moveable
        aPropName = GetPropertyName( BASEPROPERTY_MOVEABLE );
        if ( xInfo->hasPropertyByName( aPropName ) )
        {
            aVal = xPSet->getPropertyValue( aPropName );
            sal_Bool b;
            if ( ( aVal >>= b ) && b)
                aDescr.WindowAttributes |= ::com::sun::star::awt::WindowAttribute::MOVEABLE;
        }

        // Closeable
        aPropName = GetPropertyName( BASEPROPERTY_CLOSEABLE );
        if ( xInfo->hasPropertyByName( aPropName ) )
        {
            aVal = xPSet->getPropertyValue( aPropName );
            sal_Bool b;
            if ( ( aVal >>= b ) && b)
                aDescr.WindowAttributes |= ::com::sun::star::awt::WindowAttribute::CLOSEABLE;
        }

        // Dropdown
        aPropName = GetPropertyName( BASEPROPERTY_DROPDOWN );
        if ( xInfo->hasPropertyByName( aPropName ) )
        {
            aVal = xPSet->getPropertyValue( aPropName );
            sal_Bool b;
            if ( ( aVal >>= b ) && b)
                aDescr.WindowAttributes |= ::com::sun::star::awt::VclWindowPeerAttribute::DROPDOWN;
        }

        // Spin
        aPropName = GetPropertyName( BASEPROPERTY_SPIN );
        if ( xInfo->hasPropertyByName( aPropName ) )
        {
            aVal = xPSet->getPropertyValue( aPropName );
            sal_Bool b;
            if ( ( aVal >>= b ) && b)
                aDescr.WindowAttributes |= ::com::sun::star::awt::VclWindowPeerAttribute::SPIN;
        }

        // HScroll
        aPropName = GetPropertyName( BASEPROPERTY_HSCROLL );
        if ( xInfo->hasPropertyByName( aPropName ) )
        {
            aVal = xPSet->getPropertyValue( aPropName );
            sal_Bool b;
            if ( ( aVal >>= b ) && b)
                aDescr.WindowAttributes |= ::com::sun::star::awt::VclWindowPeerAttribute::HSCROLL;
        }

        // VScroll
        aPropName = GetPropertyName( BASEPROPERTY_VSCROLL );
        if ( xInfo->hasPropertyByName( aPropName ) )
        {
            aVal = xPSet->getPropertyValue( aPropName );
            sal_Bool b;
            if ( ( aVal >>= b ) && b)
                aDescr.WindowAttributes |= ::com::sun::star::awt::VclWindowPeerAttribute::VSCROLL;
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
                    aDescr.WindowAttributes |= ::com::sun::star::awt::VclWindowPeerAttribute::LEFT;
                else if ( n == PROPERTY_ALIGN_CENTER )
                    aDescr.WindowAttributes |= ::com::sun::star::awt::VclWindowPeerAttribute::CENTER;
                else
                    aDescr.WindowAttributes |= ::com::sun::star::awt::VclWindowPeerAttribute::RIGHT;
            }
        }

        // Ableitungen die Moeglichkeit geben die Attribute zu manipulieren
        PrepareWindowDescriptor(aDescr);

        // create the peer
        mxPeer = xToolkit->createWindow( aDescr );

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
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics > xGraphics( mxGraphics );
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XView >  xV(mxPeer, ::com::sun::star::uno::UNO_QUERY);
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >  xW(mxPeer, ::com::sun::star::uno::UNO_QUERY);

        aGuard.clear();

        // the updateFromModel is done without a locked mutex, too.
        // The reason is that the only thing this method does  is firing property changes, and this in general has
        // to be done without locked mutexes (as every notification to external listeners).
        // 82300 - 12/21/00 - FS
        updateFromModel();

        xV->setZoom( aComponentInfos.nZoomX, aComponentInfos.nZoomY );

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
    }
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > UnoControl::getPeer(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return mxPeer;
}

sal_Bool UnoControl::setModel( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& rxModel ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XMultiPropertySet > xPropSet( mxModel, ::com::sun::star::uno::UNO_QUERY );

    if( xPropSet.is() )
        xPropSet->removePropertiesChangeListener( this );

    mxModel = rxModel;
    if( mxModel.is() )
    {
        xPropSet = ::com::sun::star::uno::Reference< ::com::sun::star::beans::XMultiPropertySet > ( mxModel, ::com::sun::star::uno::UNO_QUERY );
        if( xPropSet.is() )
        {
            ::com::sun::star::uno::Sequence< ::rtl::OUString> aNames = lcl_ImplGetPropertyNames( xPropSet );
            xPropSet->addPropertiesChangeListener( aNames, this );
        }
    }
    return mxModel.is();
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > UnoControl::getModel(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mxModel;
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XView > UnoControl::getView(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return  (::com::sun::star::awt::XView*)this;
}

void UnoControl::setDesignMode( sal_Bool bOn ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    mbDesignMode = bOn;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >  xW( mxPeer, ::com::sun::star::uno::UNO_QUERY );
    if ( xW.is() )
        xW->setVisible( !bOn );
}

sal_Bool UnoControl::isDesignMode(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mbDesignMode;
}

sal_Bool UnoControl::isTransparent(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return sal_False;
}

// ::com::sun::star::lang::XServiceInfo
::rtl::OUString UnoControl::getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    DBG_ERROR( "This method should be overloaded!" );
    return ::rtl::OUString();
}

sal_Bool UnoControl::supportsService( const ::rtl::OUString& rServiceName ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::uno::Sequence< ::rtl::OUString > aSNL = getSupportedServiceNames();
    const ::rtl::OUString * pArray = aSNL.getConstArray();
    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == rServiceName )
            return sal_True;
    return sal_False;
}

::com::sun::star::uno::Sequence< ::rtl::OUString > UnoControl::getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    DBG_ERROR( "This method should be overloaded!" );
    return ::com::sun::star::uno::Sequence< ::rtl::OUString >();
}



