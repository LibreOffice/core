/*************************************************************************
 *
 *  $RCSfile: unocontrolcontainer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-11-02 11:07:20 $
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


#ifndef _COM_SUN_STAR_AWT_XVCLCONTAINERPEER_HPP_
#include <com/sun/star/awt/XVclContainerPeer.hpp>
#endif

#include <cppuhelper/typeprovider.hxx>
#include <rtl/memory.h>
#include <rtl/uuid.h>

#include <toolkit/controls/unocontrolcontainer.hxx>
#include <toolkit/helper/property.hxx>
#include <toolkit/helper/servicenames.hxx>

#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif

#include <tools/debug.hxx>
#include <tools/list.hxx>

//  ----------------------------------------------------
//  class UnoControlHolder
//  ----------------------------------------------------
struct UnoControlHolder
{
public:
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >     xCtrl;
    ::rtl::OUString aName;

    UnoControlHolder( const ::rtl::OUString& rName, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > & rControl )
        : aName( rName )
    {
        xCtrl = rControl;

    }
};

DECLARE_LIST( UnoControlHolderList, UnoControlHolder* );


//  ----------------------------------------------------
//  class UnoControlContainer
//  ----------------------------------------------------
UnoControlContainer::UnoControlContainer(): maCListeners( *this )
{
    mpControls = new UnoControlHolderList;
}

UnoControlContainer::UnoControlContainer( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >  xP )
    :   maCListeners( *this )
{
    mxPeer = xP;
    mbDisposePeer = sal_False;
    mpControls = new UnoControlHolderList;
}

UnoControlContainer::~UnoControlContainer()
{
    for ( sal_uInt32 n = mpControls->Count(); n; )
    {
        UnoControlHolder* pHolder = mpControls->GetObject( --n );
        delete pHolder;
    }
    mpControls->Clear();
    delete mpControls;
}

void UnoControlContainer::ImplActivateTabControllers()
{
    sal_uInt32 nCount = maTabControllers.getLength();
    for ( sal_uInt32 n = 0; n < nCount; n++ )
    {
         maTabControllers.getArray()[n]->setContainer( this );
         maTabControllers.getArray()[n]->activateTabOrder();
    }
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any UnoControlContainer::queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XUnoControlContainer*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XControlContainer*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::container::XContainer*, this ) );
    return (aRet.hasValue() ? aRet : UnoControl::queryAggregation( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoControlContainer )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XView>* ) NULL ),
    UnoControl::getTypes()
IMPL_XTYPEPROVIDER_END

// ::com::sun::star::lang::XComponent
void UnoControlContainer::dispose(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    // DG: zuerst der Welt mitteilen, daﬂ der Container wegfliegt. Dieses ist um einiges
    // schneller wenn die Welt sowohl an den Controls als auch am Container horcht
    ::com::sun::star::lang::EventObject aEvt;
    aEvt.Source = (::com::sun::star::uno::XAggregation*)(::cppu::OWeakAggObject*)this;
    maDisposeListeners.disposeAndClear(aEvt);

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > > aCtrls = getControls();
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > * pCtrls = aCtrls.getArray();
    sal_uInt32 nCtrls = aCtrls.getLength();

    // alle Strukturen entfernen
    sal_uInt32 n;
    for ( n = mpControls->Count(); n; )
        delete mpControls->GetObject( --n );
    mpControls->Clear();

    for( n = 0; n < nCtrls; n++ )
    {
        pCtrls[n]->removeEventListener(this);

        // Control wegwerfen
        pCtrls[n]->dispose();
    }

    UnoControl::dispose();
}

void UnoControlContainer::addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

}

void UnoControlContainer::removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

}


// ::com::sun::star::lang::XEventListener
void UnoControlContainer::disposing( const ::com::sun::star::lang::EventObject& rEvt ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >  xControl(rEvt.Source, ::com::sun::star::uno::UNO_QUERY );
    if (xControl.is())
        removeControl(xControl);
}

// ::com::sun::star::container::XContainer
void UnoControlContainer::addContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maCListeners.addInterface( rxListener );
}

void UnoControlContainer::removeContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maCListeners.removeInterface( rxListener );
}


// ::com::sun::star::awt::XControlContainer
void UnoControlContainer::setStatusText( const ::rtl::OUString& rStatusText ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    // In der Parenthierarchie nach unten gehen
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >  xContainer( mxContext, ::com::sun::star::uno::UNO_QUERY );
    if( xContainer.is() )
        xContainer->setStatusText( rStatusText );
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > > UnoControlContainer::getControls(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_uInt32 nCtrls = mpControls->Count();
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > > aD( nCtrls );
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >  * pDest = aD.getArray();
    for( sal_uInt32 n = 0; n < nCtrls; n++ )
    {
        UnoControlHolder* pHolder= mpControls->GetObject( n );
        pDest[n] = pHolder->xCtrl;
    }

    return aD;
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > UnoControlContainer::getControl( const ::rtl::OUString& rName ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >  xCtrl;

    sal_uInt32 nCtrls = mpControls->Count();
    for( sal_uInt32 n = 0; n < nCtrls; n++ )
    {
        UnoControlHolder* pHolder = mpControls->GetObject( n );
        if ( pHolder->aName == rName )
        {
            xCtrl = pHolder->xCtrl;
            break;
        }
    }
    return xCtrl;
}

void UnoControlContainer::addControl( const ::rtl::OUString& rName, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& rControl ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    UnoControlHolder* pHolder = new UnoControlHolder( rName, rControl );
    mpControls->Insert( pHolder, LIST_APPEND );

    ::com::sun::star::uno::Any aAny = OWeakAggObject::queryInterface( ::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>*)0) );
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xThis;
    aAny >>= xThis;
    rControl->setContext( xThis );
    rControl->addEventListener(this);

    if( mxPeer.is() )
    {
        // Hat der Container ein Peer, dann auch gleich im Child erzeugen
        rControl->createPeer( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit > (), mxPeer );
        ImplActivateTabControllers();
    }

    if ( maCListeners.getLength() )
    {
        ::com::sun::star::container::ContainerEvent aEvent;
        aEvent.Source = *this;
        aEvent.Element <<= rControl;
        maCListeners.elementInserted( aEvent );
    }
}

void UnoControlContainer::removeControl( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& rControl ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( rControl.is() )
    {
        sal_uInt32 nCtrls = mpControls->Count();
        for( sal_uInt32 n = 0; n < nCtrls; n++ )
        {
            UnoControlHolder* pHolder = mpControls->GetObject( n );
            if ( (::com::sun::star::awt::XControl*)rControl.get() == (::com::sun::star::awt::XControl*)pHolder->xCtrl.get() )
            {
                rControl->removeEventListener(this);
                rControl->setContext( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > () );

                delete pHolder;
                mpControls->Remove( n );
                if ( maCListeners.getLength() )
                {
                    ::com::sun::star::container::ContainerEvent aEvent;
                    aEvent.Source = *this;
                    aEvent.Element <<= rControl;
                    maCListeners.elementRemoved( aEvent );
                }
                break;
            }
        }
    }
}



// ::com::sun::star::awt::XUnoControlContainer
void UnoControlContainer::setTabControllers( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabController > >& TabControllers ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maTabControllers = TabControllers;
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabController > > UnoControlContainer::getTabControllers(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return maTabControllers;
}

void UnoControlContainer::addTabController( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabController >& TabController ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_uInt32 nOldCount = maTabControllers.getLength();
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabController > > aNewSeq( nOldCount + 1 );
    for ( sal_uInt32 n = 0; n < nOldCount; n++ )
        aNewSeq.getArray()[n] = maTabControllers.getConstArray()[n];
    aNewSeq.getArray()[nOldCount] = TabController;
    maTabControllers = aNewSeq;
}

void UnoControlContainer::removeTabController( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabController >& TabController ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_uInt32 nCount = maTabControllers.getLength();
    for ( sal_uInt32 n = 0; n < nCount; n++ )
    {
        if( maTabControllers.getConstArray()[n] == TabController )
        {
            ::comphelper::removeElementAt( maTabControllers, n );
            break;
        }
    }
}

// ::com::sun::star::awt::XControl
void UnoControlContainer::createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& rxToolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& rParent ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if( !mxPeer.is() )
    {
        sal_Bool bVis = maComponentInfos.bVisible;
        if( bVis )
         UnoControl::setVisible( sal_False );
        // eigenes Peer erzeugen
        UnoControl::createPeer( rxToolkit, rParent );

        // alle Peers der Childs erzeugen
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > > aCtrls = getControls();
        sal_uInt32 nCtrls = aCtrls.getLength();
        for( sal_uInt32 n = 0; n < nCtrls; n++ )
            aCtrls.getArray()[n]->createPeer( rxToolkit, mxPeer );

        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XVclContainerPeer >  xC( mxPeer, ::com::sun::star::uno::UNO_QUERY );

        xC->enableDialogControl( sal_True );
        ImplActivateTabControllers();

        if( bVis )
         UnoControl::setVisible( sal_True );
    }
}


// ::com::sun::star::awt::XWindow
void UnoControlContainer::setVisible( sal_Bool bVisible ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    UnoControl::setVisible( bVisible );
    if( !mxContext.is() && bVisible )
        // Es ist ein TopWindow, also automatisch anzeigen
        createPeer( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit > (), ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > () );
}



