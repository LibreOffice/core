/*************************************************************************
 *
 *  $RCSfile: stdtabcontroller.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:02:09 $
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

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XVCLCONTAINERPEER_HPP_
#include <com/sun/star/awt/XVclContainerPeer.hpp>
#endif

#include <toolkit/controls/stdtabcontroller.hxx>
#include <toolkit/controls/stdtabcontrollermodel.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/macros.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/memory.h>
#include <rtl/uuid.h>

#include <tools/debug.hxx>
#include <vcl/window.hxx>

#include <unotools/sequence.hxx>

//  ----------------------------------------------------
//  class StdTabController
//  ----------------------------------------------------
StdTabController::StdTabController()
{
}

StdTabController::~StdTabController()
{
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >  StdTabController::ImplFindControl( ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > >& rCtrls,
 const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > & rxCtrlModel ) const
{

/*
     // MT: Funktioniert nicht mehr, weil ich nicht mehr bei mir angemeldet bin,
    // weil DG das abfaengt.

    // #54677# Beim Laden eines HTML-Dokuments wird nach jedem Control ein
    // activateTabOrder gerufen und jede Menge Zeit in dieser Methode verbraten.
    // Die Anzahl dieser Schleifendurchlaufe steigt quadratisch, also versuchen
    // das Control direkt vom Model zu erhalten.
    // => Wenn genau ein Control als PropertyChangeListener angemeldet ist,
    // dann muss das auch das richtige sein.

    UnoControlModel* pUnoCtrlModel = UnoControlModel::GetImplementation( rxCtrlModel );


    if ( pUnoCtrlModel )
    {
        ListenerIterator aIt( pUnoCtrlModel->maPropertiesListeners );
        while( aIt.hasMoreElements() )
        {
            XEventListener* pL = aIt.next();
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >  xC( pL, ::com::sun::star::uno::UNO_QUERY );
            if ( xC.is() )
            {
                if( xC->getContext() == mxControlContainer )
                {
                    xCtrl = xC;
                    break;
                }
            }
        }
    }
    if ( !xCtrl.is() && rxCtrlModel.is())
*/
    DBG_ASSERT( rxCtrlModel.is(), "ImplFindControl - welches ?!" );

    const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > * pCtrls = rCtrls.getConstArray();
    sal_uInt32 nCtrls = rCtrls.getLength();
    for ( sal_uInt32 n = 0; n < nCtrls; n++ )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >  xModel(pCtrls[n].is() ? pCtrls[n]->getModel() : ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > ());
        if ( (::com::sun::star::awt::XControlModel*)xModel.get() == (::com::sun::star::awt::XControlModel*)rxCtrlModel.get() )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >  xCtrl( pCtrls[n] );
            ::utl::removeElementAt( rCtrls, n );
            return xCtrl;
        }
    }
    return ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > ();
}

sal_Bool StdTabController::ImplCreateComponentSequence( ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > >& rControls, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& rModels, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > >& rComponents, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any>* pTabStops, sal_Bool bPeerComponent ) const
{
    sal_Bool bOK = sal_True;

    // nur die wirklich geforderten Controls
    sal_uInt32 nModels = rModels.getLength();
    if (nModels != rControls.getLength())
    {
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > > aSeq = ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > >(nModels);
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > * pModels = rModels.getConstArray();
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >  xCurrentModel;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >  xCurrentControl;

        sal_uInt32 nRealControls = 0;
        for (sal_uInt32 n = 0; n < nModels; ++n)
        {
            xCurrentModel = pModels[n];
            xCurrentControl = ImplFindControl(rControls, xCurrentModel);
            if (xCurrentControl.is())
                aSeq.getArray()[nRealControls++] = xCurrentControl;
        }
        aSeq.realloc(nRealControls);
        rControls = aSeq;
    }


    const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > * pControls = rControls.getConstArray();
    sal_uInt32 nCtrls = rControls.getLength();
    rComponents = ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > >( nCtrls );
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > * pComps = rComponents.getArray();
    ::com::sun::star::uno::Any* pTabs = NULL;


    if ( pTabStops )
    {
        *pTabStops = ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any>( nCtrls );
        pTabs = pTabStops->getArray();
    }

    for ( sal_uInt32 n = 0; bOK && ( n < nCtrls ); n++ )
    {
        // Zum Model passendes Control suchen
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >  xCtrl(pControls[n]);
        if ( xCtrl.is() )
        {
            if (bPeerComponent)
                pComps[n] = ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > (xCtrl->getPeer(), ::com::sun::star::uno::UNO_QUERY);
            else
                pComps[n] = ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > (xCtrl, ::com::sun::star::uno::UNO_QUERY);

            // TabStop-::com::sun::star::beans::Property
            if ( pTabs )
            {
                // opt: String fuer TabStop als Konstante
                static const ::rtl::OUString aTabStopName( ::rtl::OUString::createFromAscii( "Tabstop" ) );

                ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xPSet( xCtrl->getModel(), ::com::sun::star::uno::UNO_QUERY );
                ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >  xInfo = xPSet->getPropertySetInfo();
                if( xInfo->hasPropertyByName( aTabStopName ) )
                    pTabs[n] = xPSet->getPropertyValue( aTabStopName );
            }
        }
        else
        {
            DBG_TRACE( "ImplCreateComponentSequence: Control not found" );
            bOK = sal_False;
        }
    }
    return bOK;
}

void StdTabController::ImplActivateControl( sal_Bool bFirst ) const
{
    // HACK wegen #53688#, muss auf ein Interface abgebildet werden, wenn Controls Remote liegen koennen.
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabController >  xTabController(const_cast< ::cppu::OWeakObject* >(static_cast< const ::cppu::OWeakObject* >(this)), ::com::sun::star::uno::UNO_QUERY);
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > > aCtrls = xTabController->getControls();
    const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > * pControls = aCtrls.getConstArray();
    sal_uInt32 nCount = aCtrls.getLength();

    for ( sal_uInt32 n = bFirst ? 0 : nCount; bFirst ? ( n < nCount ) : n; )
    {
        sal_uInt32 nCtrl = bFirst ? n++ : --n;
        DBG_ASSERT( pControls[nCtrl].is(), "Control nicht im ::com::sun::star::sdbcx::Container!" );
        if ( pControls[nCtrl].is() )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >  xCP = pControls[nCtrl]->getPeer();
            if ( xCP.is() )
            {
                VCLXWindow* pC = VCLXWindow::GetImplementation( xCP );
                if ( pC && pC->GetWindow() && ( pC->GetWindow()->GetStyle() & WB_TABSTOP ) )
                {
                    pC->GetWindow()->GrabFocus();
                    break;
                }
            }
        }
    }
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any StdTabController::queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XTabController*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XTypeProvider*, this ) );
    return (aRet.hasValue() ? aRet : OWeakAggObject::queryAggregation( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( StdTabController )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabController>* ) NULL )
IMPL_XTYPEPROVIDER_END

void StdTabController::setModel( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabControllerModel >& Model ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    mxModel = Model;
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabControllerModel > StdTabController::getModel(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mxModel;
}

void StdTabController::setContainer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >& Container ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    mxControlContainer = Container;
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer > StdTabController::getContainer(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mxControlContainer;
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > > StdTabController::getControls(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > > aSeq;

    if ( mxControlContainer.is() )
    {
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > > aModels = mxModel->getControlModels();
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > * pModels = aModels.getConstArray();

        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > > xCtrls = mxControlContainer->getControls();

        sal_uInt32 nCtrls = aModels.getLength();
        aSeq = ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > >( nCtrls );
        for ( sal_uInt32 n = 0; n < nCtrls; n++ )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >  xCtrlModel = pModels[n];
            // Zum Model passendes Control suchen
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >  xCtrl = ImplFindControl( xCtrls, xCtrlModel );
            aSeq.getArray()[n] = xCtrl;
        }
    }
    return aSeq;
}

void StdTabController::autoTabOrder(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    DBG_ASSERT( mxControlContainer.is(), "autoTabOrder: No ControlContainer!" );
    if ( !mxControlContainer.is() )
        return;

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > > aSeq = mxModel->getControlModels();
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > > aCompSeq;

    // vieleicht erhalte ich hier einen TabController,
    // der schneller die Liste meiner Controls ermittelt
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabController >  xTabController(static_cast< ::cppu::OWeakObject* >(this), ::com::sun::star::uno::UNO_QUERY);
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > > aControls = xTabController->getControls();

    // #58317# Es sind ggf. noch nicht alle Controls fuer die Models im ::com::sun::star::sdbcx::Container,
    // dann kommt spaeter nochmal ein autoTabOrder...
    if( !ImplCreateComponentSequence( aControls, aSeq, aCompSeq, NULL, sal_False ) )
        return;

    sal_uInt32 nCtrls = aCompSeq.getLength();
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > * pComponents = aCompSeq.getArray();

    ComponentEntryList aCtrls;
    sal_uInt32 n;
    for ( n = 0; n < nCtrls; n++ )
    {
        ::com::sun::star::awt::XWindow* pC = (::com::sun::star::awt::XWindow*)pComponents[n].get();
        ComponentEntry* pE = new ComponentEntry;
        pE->pComponent = pC;
        ::com::sun::star::awt::Rectangle aPosSize = pC->getPosSize();
        pE->aPos.X() = aPosSize.X;
        pE->aPos.Y() = aPosSize.Y;

        sal_uInt16 nPos;
        for ( nPos = 0; nPos < aCtrls.Count(); nPos++ )
        {
            ComponentEntry* pEntry = aCtrls.GetObject( nPos );
            if ( pEntry->aPos.Y() >= pE->aPos.Y() )
            {
                while ( pEntry && ( pEntry->aPos.Y() == pE->aPos.Y() )
                                && ( pEntry->aPos.X() < pE->aPos.X() ) )
                {
                    pEntry = aCtrls.GetObject( ++nPos );
                }
                break;
            }
        }
        aCtrls.Insert( pE, nPos );
    }

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > > aNewSeq( nCtrls );
    for ( n = 0; n < nCtrls; n++ )
    {
        ComponentEntry* pE = aCtrls.GetObject( n );
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >  xUC( pE->pComponent, ::com::sun::star::uno::UNO_QUERY );
        aNewSeq.getArray()[n] = xUC->getModel();
        delete pE;
    }
    aCtrls.Clear();

    mxModel->setControlModels( aNewSeq );
}

void StdTabController::activateTabOrder(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    // Am ::com::sun::star::sdbcx::Container die Tab-Reihenfolge aktivieren...

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >  xC( mxControlContainer, ::com::sun::star::uno::UNO_QUERY );
     if ( !xC.is() || !xC->getPeer().is() )
        return;

    // vieleicht erhalte ich hier einen TabController,
    // der schneller die Liste meiner Controls ermittelt
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabController >  xTabController(static_cast< ::cppu::OWeakObject* >(this), ::com::sun::star::uno::UNO_QUERY);

    // Flache Liste besorgen...
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > > aSeq = mxModel->getControlModels();
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > > aCompSeq;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any> aTabSeq;

    // DG: Aus Optimierungsgruenden werden die Controls mittels getControls() geholt,
    // dieses hoert sich zwar wiedersinning an, fuehrt aber im konkreten Fall (Forms) zu sichtbaren
    // Geschwindigkeitsvorteilen
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > > aCtrls = xTabController->getControls();

    // #58317# Es sind ggf. noch nicht alle Controls fuer die Models im ::com::sun::star::sdbcx::Container,
    // dann kommt spaeter nochmal ein activateTabOrder...
    if( !ImplCreateComponentSequence( aCtrls, aSeq, aCompSeq, &aTabSeq, sal_True ) )
        return;

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XVclContainerPeer >  xVclContainerPeer( xC->getPeer(), ::com::sun::star::uno::UNO_QUERY );

    xVclContainerPeer->setTabOrder( aCompSeq, aTabSeq, mxModel->getGroupControl() );

    // Gruppen...
    aCtrls = xTabController->getControls();

    ::rtl::OUString aName;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > > aGroupSeq;
    sal_uInt32 nGroups = mxModel->getGroupCount();
    for ( sal_uInt32 nG = 0; nG < nGroups; nG++ )
    {
        mxModel->getGroup( nG, aGroupSeq, aName );
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > > aCompSeq;
        ImplCreateComponentSequence( aCtrls, aGroupSeq, aCompSeq, NULL, sal_True );
        xVclContainerPeer->setGroup( aCompSeq );
    }
}

void StdTabController::activateFirst(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ImplActivateControl( sal_True );
}

void StdTabController::activateLast(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ImplActivateControl( sal_False );
}

