/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgedobj.cxx,v $
 *
 *  $Revision: 1.39 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:15:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include <vector>
#include <algorithm>

#ifndef _BASCTL_DLGEDDEF_HXX
#include <dlgeddef.hxx>
#endif

#ifndef _BASCTL_DLGEDOBJ_HXX
#include "dlgedobj.hxx"
#endif

#ifndef _BASCTL_DLGED_HXX
#include "dlged.hxx"
#endif

#ifndef _BASCTL_DLGEDMOD_HXX
#include "dlgedmod.hxx"
#endif

#ifndef _BASCTL_DLGEDPAGE_HXX
#include "dlgedpage.hxx"
#endif

#ifndef _BASCTL_DLGEDVIEW_HXX
#include "dlgedview.hxx"
#endif

#ifndef _BASCTL_DLGEDLIST_HXX
#include "dlgedlist.hxx"
#endif

#ifndef _IDERID_HXX
#include <iderid.hxx>
#endif

#ifndef _BASCTL_DLGRESID_HRC
#include <dlgresid.hrc>
#endif

#ifndef _TOOLS_RESMGR_HXX
#include <tools/resmgr.hxx>
#endif

#ifndef _SVDIO_HXX //autogen
#include <svx/svdio.hxx>
#endif

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

#ifndef _COM_SUN_STAR_AWT_XTABCONTROLLERMODEL_HPP_
#include <com/sun/star/awt/XTabControllerModel.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XUNOCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XUnoControlContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XVCLCONTAINERPEER_HPP_
#include <com/sun/star/awt/XVclContainerPeer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XSCRIPTEVENTSSUPPLIER_HPP_
#include <com/sun/star/script/XScriptEventsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#include <algorithm>
#include <functional>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::script;
using namespace ::rtl;


TYPEINIT1(DlgEdObj, SdrUnoObj);
DBG_NAME(DlgEdObj);

//----------------------------------------------------------------------------

DlgEdObj::DlgEdObj()
          :SdrUnoObj(String(), sal_False)
          ,bIsListening(sal_False)
{
    DBG_CTOR(DlgEdObj, NULL);
}

//----------------------------------------------------------------------------

DlgEdObj::DlgEdObj(const ::rtl::OUString& rModelName)
          :SdrUnoObj(rModelName, sal_False)
          ,bIsListening(sal_False)
{
    DBG_CTOR(DlgEdObj, NULL);
}

//----------------------------------------------------------------------------

DlgEdObj::DlgEdObj(const ::rtl::OUString& rModelName,
                   const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxSFac)
          :SdrUnoObj(rModelName, rxSFac, sal_False)
          ,bIsListening(sal_False)
{
    DBG_CTOR(DlgEdObj, NULL);
}

//----------------------------------------------------------------------------

DlgEdObj::~DlgEdObj()
{
    DBG_DTOR(DlgEdObj, NULL);

    if ( isListening() )
        EndListening();
}

//----------------------------------------------------------------------------

void DlgEdObj::SetPage(SdrPage* _pNewPage)
{
    // now set the page
    SdrUnoObj::SetPage(_pNewPage);
}

//----------------------------------------------------------------------------

bool DlgEdObj::TransformSdrToControlCoordinates(
    sal_Int32 nXIn, sal_Int32 nYIn, sal_Int32 nWidthIn, sal_Int32 nHeightIn,
    sal_Int32& nXOut, sal_Int32& nYOut, sal_Int32& nWidthOut, sal_Int32& nHeightOut )
{
    // input position and size
    Size aPos( nXIn, nYIn );
    Size aSize( nWidthIn, nHeightIn );

    // form position
    DlgEdForm* pForm = 0;
    if ( ISA( DlgEdForm ) )
        pForm = (DlgEdForm*)this;
    else
        pForm = GetDlgEdForm();
    DBG_ASSERT( pForm, "DlgEdObj::TransformSdrToControlCoordinates: no form!" );
    if ( !pForm )
        return false;
    Rectangle aFormRect = pForm->GetSnapRect();
    Size aFormPos( aFormRect.Left(), aFormRect.Top() );

    // convert 100th_mm to pixel
    OutputDevice* pDevice = Application::GetDefaultDevice();
    DBG_ASSERT( pDevice, "DlgEdObj::TransformSdrToControlCoordinates: missing default device!" );
    if ( !pDevice )
        return false;
    aPos = pDevice->LogicToPixel( aPos, MapMode( MAP_100TH_MM ) );
    aSize = pDevice->LogicToPixel( aSize, MapMode( MAP_100TH_MM ) );
    aFormPos = pDevice->LogicToPixel( aFormPos, MapMode( MAP_100TH_MM ) );

    // subtract form position
    aPos.Width() -= aFormPos.Width();
    aPos.Height() -= aFormPos.Height();

    // take window borders into account
    Reference< awt::XDevice > xDev;
    DlgEditor* pEditor = pForm->GetDlgEditor();
    if ( pEditor )
    {
        Window* pWindow = pEditor->GetWindow();
        DBG_ASSERT( pWindow, "DlgEdObj::TransformSdrToControlCoordinates: no window!" );
        if ( pWindow )
        {
            Reference< awt::XControl > xDlg( pForm->GetUnoControl( pWindow ), UNO_QUERY );
            if ( xDlg.is() )
                xDev.set( xDlg->getPeer(), UNO_QUERY );
        }
    }
    DBG_ASSERT( xDev.is(), "DlgEdObj::TransformSdrToControlCoordinates: no device!" );
    if ( xDev.is() )
    {
        awt::DeviceInfo aDeviceInfo = xDev->getInfo();
        aPos.Width() -= aDeviceInfo.LeftInset;
        aPos.Height() -= aDeviceInfo.TopInset;
    }
    else
    {
        return false;
    }

    // convert pixel to logic units
    aPos = pDevice->PixelToLogic( aPos, MapMode( MAP_APPFONT ) );
    aSize = pDevice->PixelToLogic( aSize, MapMode( MAP_APPFONT ) );

    // set out parameters
    nXOut = aPos.Width();
    nYOut = aPos.Height();
    nWidthOut = aSize.Width();
    nHeightOut = aSize.Height();

    return true;
}

//----------------------------------------------------------------------------

bool DlgEdObj::TransformSdrToFormCoordinates(
    sal_Int32 nXIn, sal_Int32 nYIn, sal_Int32 nWidthIn, sal_Int32 nHeightIn,
    sal_Int32& nXOut, sal_Int32& nYOut, sal_Int32& nWidthOut, sal_Int32& nHeightOut )
{
    // input position and size
    Size aPos( nXIn, nYIn );
    Size aSize( nWidthIn, nHeightIn );

    // convert 100th_mm to pixel
    OutputDevice* pDevice = Application::GetDefaultDevice();
    DBG_ASSERT( pDevice, "DlgEdObj::TransformSdrToFormCoordinates: missing default device!" );
    if ( !pDevice )
        return false;
    aPos = pDevice->LogicToPixel( aPos, MapMode( MAP_100TH_MM ) );
    aSize = pDevice->LogicToPixel( aSize, MapMode( MAP_100TH_MM ) );

    // take window borders into account
    Reference< awt::XDevice > xDev;
    DlgEdForm* pForm = 0;
    if ( ISA( DlgEdForm ) )
        pForm = (DlgEdForm*)this;
    else
        pForm = GetDlgEdForm();
    DBG_ASSERT( pForm, "DlgEdObj::TransformSdrToFormCoordinates: no form!" );
    if ( !pForm )
        return false;
    DlgEditor* pEditor = pForm->GetDlgEditor();
    if ( pEditor )
    {
        Window* pWindow = pEditor->GetWindow();
        DBG_ASSERT( pWindow, "DlgEdObj::TransformSdrToFormCoordinates: no window!" );
        if ( pWindow )
        {
            Reference< awt::XControl > xDlg( pForm->GetUnoControl( pWindow ), UNO_QUERY );
            if ( xDlg.is() )
                xDev.set( xDlg->getPeer(), UNO_QUERY );
        }
    }
    DBG_ASSERT( xDev.is(), "DlgEdObj::TransformSdrToFormCoordinates: no device!" );
    if ( xDev.is() )
    {
        awt::DeviceInfo aDeviceInfo = xDev->getInfo();
        aSize.Width() -= aDeviceInfo.LeftInset + aDeviceInfo.RightInset;
        aSize.Height() -= aDeviceInfo.TopInset + aDeviceInfo.BottomInset;
    }
    else
    {
        return false;
    }

    // convert pixel to logic units
    aPos = pDevice->PixelToLogic( aPos, MapMode( MAP_APPFONT ) );
    aSize = pDevice->PixelToLogic( aSize, MapMode( MAP_APPFONT ) );

    // set out parameters
    nXOut = aPos.Width();
    nYOut = aPos.Height();
    nWidthOut = aSize.Width();
    nHeightOut = aSize.Height();

    return true;
}

//----------------------------------------------------------------------------

bool DlgEdObj::TransformControlToSdrCoordinates(
    sal_Int32 nXIn, sal_Int32 nYIn, sal_Int32 nWidthIn, sal_Int32 nHeightIn,
    sal_Int32& nXOut, sal_Int32& nYOut, sal_Int32& nWidthOut, sal_Int32& nHeightOut )
{
    // input position and size
    Size aPos( nXIn, nYIn );
    Size aSize( nWidthIn, nHeightIn );

    // form position
    DlgEdForm* pForm = 0;
    if ( ISA( DlgEdForm ) )
        pForm = (DlgEdForm*)this;
    else
        pForm = GetDlgEdForm();
    DBG_ASSERT( pForm, "DlgEdObj::TransformControlToSdrCoordinates: no form!" );
    if ( !pForm )
        return false;
    Reference< beans::XPropertySet > xPSetForm( pForm->GetUnoControlModel(), UNO_QUERY );
    DBG_ASSERT( xPSetForm.is(), "DlgEdObj::TransformControlToSdrCoordinates: no form property set!" );
    if ( !xPSetForm.is() )
        return false;
    sal_Int32 nFormX, nFormY, nFormWidth, nFormHeight;
    xPSetForm->getPropertyValue( DLGED_PROP_POSITIONX ) >>= nFormX;
    xPSetForm->getPropertyValue( DLGED_PROP_POSITIONY ) >>= nFormY;
    xPSetForm->getPropertyValue( DLGED_PROP_WIDTH ) >>= nFormWidth;
    xPSetForm->getPropertyValue( DLGED_PROP_HEIGHT ) >>= nFormHeight;
    Size aFormPos( nFormX, nFormY );

    // convert logic units to pixel
    OutputDevice* pDevice = Application::GetDefaultDevice();
    DBG_ASSERT( pDevice, "DlgEdObj::TransformControlToSdrCoordinates: missing default device!" );
    if ( !pDevice )
        return false;
    aPos = pDevice->LogicToPixel( aPos, MapMode( MAP_APPFONT ) );
    aSize = pDevice->LogicToPixel( aSize, MapMode( MAP_APPFONT ) );
    aFormPos = pDevice->LogicToPixel( aFormPos, MapMode( MAP_APPFONT ) );

    // add form position
    aPos.Width() += aFormPos.Width();
    aPos.Height() += aFormPos.Height();

    // take window borders into account
    Reference< awt::XDevice > xDev;
    DlgEditor* pEditor = pForm->GetDlgEditor();
    if ( pEditor )
    {
        Window* pWindow = pEditor->GetWindow();
        DBG_ASSERT( pWindow, "DlgEdObj::TransformControlToSdrCoordinates: no window!" );
        if ( pWindow )
        {
            Reference< awt::XControl > xDlg( pForm->GetUnoControl( pWindow ), UNO_QUERY );
            if ( xDlg.is() )
                xDev.set( xDlg->getPeer(), UNO_QUERY );
        }
    }
    DBG_ASSERT( xDev.is(), "DlgEdObj::TransformControlTSdrCoordinates: no device!" );
    if ( xDev.is() )
    {
        awt::DeviceInfo aDeviceInfo = xDev->getInfo();
        aPos.Width() += aDeviceInfo.LeftInset;
        aPos.Height() += aDeviceInfo.TopInset;
    }
    else
    {
        return false;
    }

    // convert pixel to 100th_mm
    aPos = pDevice->PixelToLogic( aPos, MapMode( MAP_100TH_MM ) );
    aSize = pDevice->PixelToLogic( aSize, MapMode( MAP_100TH_MM ) );

    // set out parameters
    nXOut = aPos.Width();
    nYOut = aPos.Height();
    nWidthOut = aSize.Width();
    nHeightOut = aSize.Height();

    return true;
}

//----------------------------------------------------------------------------

bool DlgEdObj::TransformFormToSdrCoordinates(
    sal_Int32 nXIn, sal_Int32 nYIn, sal_Int32 nWidthIn, sal_Int32 nHeightIn,
    sal_Int32& nXOut, sal_Int32& nYOut, sal_Int32& nWidthOut, sal_Int32& nHeightOut )
{
    // input position and size
    Size aPos( nXIn, nYIn );
    Size aSize( nWidthIn, nHeightIn );

    // convert logic units to pixel
    OutputDevice* pDevice = Application::GetDefaultDevice();
    DBG_ASSERT( pDevice, "DlgEdObj::TransformFormToSdrCoordinates: missing default device!" );
    if ( !pDevice )
        return false;
    aPos = pDevice->LogicToPixel( aPos, MapMode( MAP_APPFONT ) );
    aSize = pDevice->LogicToPixel( aSize, MapMode( MAP_APPFONT ) );

    // take window borders into account
    Reference< awt::XDevice > xDev;
    DlgEdForm* pForm = 0;
    if ( ISA( DlgEdForm ) )
        pForm = (DlgEdForm*)this;
    else
        pForm = GetDlgEdForm();
    DBG_ASSERT( pForm, "DlgEdObj::TransformFormToSdrCoordinates: no form!" );
    if ( !pForm )
        return false;
    DlgEditor* pEditor = pForm->GetDlgEditor();
    if ( pEditor )
    {
        Window* pWindow = pEditor->GetWindow();
        DBG_ASSERT( pWindow, "DlgEdObj::TransformFormToSdrCoordinates: no window!" );
        if ( pWindow )
        {
            Reference< awt::XControl > xDlg( pForm->GetUnoControl( pWindow ), UNO_QUERY );
            if ( xDlg.is() )
                xDev.set( xDlg->getPeer(), UNO_QUERY );
        }
    }
    DBG_ASSERT( xDev.is(), "DlgEdObj::TransformFormToSdrCoordinates: no device!" );
    if ( xDev.is() )
    {
        awt::DeviceInfo aDeviceInfo = xDev->getInfo();
        aSize.Width() += aDeviceInfo.LeftInset + aDeviceInfo.RightInset;
        aSize.Height() += aDeviceInfo.TopInset + aDeviceInfo.BottomInset;
    }
    else
    {
        return false;
    }

    // convert pixel to 100th_mm
    aPos = pDevice->PixelToLogic( aPos, MapMode( MAP_100TH_MM ) );
    aSize = pDevice->PixelToLogic( aSize, MapMode( MAP_100TH_MM ) );

    // set out parameters
    nXOut = aPos.Width();
    nYOut = aPos.Height();
    nWidthOut = aSize.Width();
    nHeightOut = aSize.Height();

    return true;
}

//----------------------------------------------------------------------------

void DlgEdObj::SetRectFromProps()
{
    // get control position and size from properties
    Reference< beans::XPropertySet > xPSet( GetUnoControlModel(), UNO_QUERY );
    if ( xPSet.is() )
    {
        sal_Int32 nXIn, nYIn, nWidthIn, nHeightIn;
        xPSet->getPropertyValue( DLGED_PROP_POSITIONX ) >>= nXIn;
        xPSet->getPropertyValue( DLGED_PROP_POSITIONY ) >>= nYIn;
        xPSet->getPropertyValue( DLGED_PROP_WIDTH ) >>= nWidthIn;
        xPSet->getPropertyValue( DLGED_PROP_HEIGHT ) >>= nHeightIn;

        // transform coordinates
        sal_Int32 nXOut, nYOut, nWidthOut, nHeightOut;
        if ( TransformControlToSdrCoordinates( nXIn, nYIn, nWidthIn, nHeightIn, nXOut, nYOut, nWidthOut, nHeightOut ) )
        {
            // set rectangle position and size
            Point aPoint( nXOut, nYOut );
            Size aSize( nWidthOut, nHeightOut );
            SetSnapRect( Rectangle( aPoint, aSize ) );
        }
    }
}

//----------------------------------------------------------------------------

void DlgEdObj::SetPropsFromRect()
{
    // get control position and size from rectangle
    Rectangle aRect = GetSnapRect();
    sal_Int32 nXIn = aRect.Left();
    sal_Int32 nYIn = aRect.Top();
    sal_Int32 nWidthIn = aRect.GetWidth();
    sal_Int32 nHeightIn = aRect.GetHeight();

    // transform coordinates
    sal_Int32 nXOut, nYOut, nWidthOut, nHeightOut;
    if ( TransformSdrToControlCoordinates( nXIn, nYIn, nWidthIn, nHeightIn, nXOut, nYOut, nWidthOut, nHeightOut ) )
    {
        // set properties
        Reference< beans::XPropertySet > xPSet( GetUnoControlModel(), UNO_QUERY );
        if ( xPSet.is() )
        {
            Any aValue;
            aValue <<= nXOut;
            xPSet->setPropertyValue( DLGED_PROP_POSITIONX, aValue );
            aValue <<= nYOut;
            xPSet->setPropertyValue( DLGED_PROP_POSITIONY, aValue );
            aValue <<= nWidthOut;
            xPSet->setPropertyValue( DLGED_PROP_WIDTH, aValue );
            aValue <<= nHeightOut;
            xPSet->setPropertyValue( DLGED_PROP_HEIGHT, aValue );
        }
    }
}

//----------------------------------------------------------------------------

void DlgEdObj::PositionAndSizeChange( const beans::PropertyChangeEvent& evt )
{
    DlgEdPage* pPage = 0;
    if ( pDlgEdForm )
    {
        DlgEditor* pEditor = pDlgEdForm->GetDlgEditor();
        if ( pEditor )
            pPage = pEditor->GetPage();
    }
    DBG_ASSERT( pPage, "DlgEdObj::PositionAndSizeChange: no page!" );
    if ( pPage )
    {
        sal_Int32 nPageXIn = 0;
        sal_Int32 nPageYIn = 0;
        Size aPageSize = pPage->GetSize();
        sal_Int32 nPageWidthIn = aPageSize.Width();
        sal_Int32 nPageHeightIn = aPageSize.Height();
        sal_Int32 nPageX, nPageY, nPageWidth, nPageHeight;
        if ( TransformSdrToControlCoordinates( nPageXIn, nPageYIn, nPageWidthIn, nPageHeightIn, nPageX, nPageY, nPageWidth, nPageHeight ) )
        {
            Reference< beans::XPropertySet > xPSet( GetUnoControlModel(), UNO_QUERY );
            if ( xPSet.is() )
            {
                sal_Int32 nX, nY, nWidth, nHeight;
                xPSet->getPropertyValue( DLGED_PROP_POSITIONX ) >>= nX;
                xPSet->getPropertyValue( DLGED_PROP_POSITIONY ) >>= nY;
                xPSet->getPropertyValue( DLGED_PROP_WIDTH ) >>= nWidth;
                xPSet->getPropertyValue( DLGED_PROP_HEIGHT ) >>= nHeight;

                sal_Int32 nValue;
                evt.NewValue >>= nValue;
                sal_Int32 nNewValue = nValue;

                if ( evt.PropertyName == DLGED_PROP_POSITIONX )
                {
                    if ( nNewValue + nWidth > nPageX + nPageWidth )
                        nNewValue = nPageX + nPageWidth - nWidth;
                    if ( nNewValue < nPageX )
                        nNewValue = nPageX;
                }
                else if ( evt.PropertyName == DLGED_PROP_POSITIONY )
                {
                    if ( nNewValue + nHeight > nPageY + nPageHeight )
                        nNewValue = nPageY + nPageHeight - nHeight;
                    if ( nNewValue < nPageY )
                        nNewValue = nPageY;
                }
                else if ( evt.PropertyName == DLGED_PROP_WIDTH )
                {
                    if ( nX + nNewValue > nPageX + nPageWidth )
                        nNewValue = nPageX + nPageWidth - nX;
                    if ( nNewValue < 1 )
                        nNewValue = 1;
                }
                else if ( evt.PropertyName == DLGED_PROP_HEIGHT )
                {
                    if ( nY + nNewValue > nPageY + nPageHeight )
                        nNewValue = nPageY + nPageHeight - nY;
                    if ( nNewValue < 1 )
                        nNewValue = 1;
                }

                if ( nNewValue != nValue )
                {
                    Any aNewValue;
                    aNewValue <<= nNewValue;
                    EndListening( sal_False );
                    xPSet->setPropertyValue( evt.PropertyName, aNewValue );
                    StartListening();
                }
            }
        }
    }

    SetRectFromProps();
}

//----------------------------------------------------------------------------

void SAL_CALL DlgEdObj::NameChange( const  ::com::sun::star::beans::PropertyChangeEvent& evt ) throw( ::com::sun::star::uno::RuntimeException)
{
    // get old name
    ::rtl::OUString aOldName;
    evt.OldValue >>= aOldName;

    // get new name
    ::rtl::OUString aNewName;
    evt.NewValue >>= aNewName;

    if ( !aNewName.equals(aOldName) )
    {
        Reference< container::XNameAccess > xNameAcc((GetDlgEdForm()->GetUnoControlModel()), UNO_QUERY);
        if ( xNameAcc.is() && xNameAcc->hasByName(aOldName) )
        {
            if ( !xNameAcc->hasByName(aNewName) && aNewName.getLength() != 0 )
            {
                // remove the control by the old name and insert the control by the new name in the container
                Reference< container::XNameContainer > xCont(xNameAcc, UNO_QUERY );
                if ( xCont.is() )
                {
                    Reference< awt::XControlModel > xCtrl(GetUnoControlModel(), UNO_QUERY);
                    Any aAny;
                    aAny <<= xCtrl;
                    xCont->removeByName( aOldName );
                    xCont->insertByName( aNewName , aAny );
                }
            }
            else
            {
                // set old name property
                EndListening(sal_False);
                Reference< beans::XPropertySet >  xPSet(GetUnoControlModel(), UNO_QUERY);
                Any aName;
                aName <<= aOldName;
                xPSet->setPropertyValue( DLGED_PROP_NAME, aName );
                StartListening();
            }
        }
    }
}

//----------------------------------------------------------------------------

sal_Int32 DlgEdObj::GetStep() const
{
    // get step property
    sal_Int32 nStep;
    uno::Reference< beans::XPropertySet >  xPSet( GetUnoControlModel(), uno::UNO_QUERY );
    if (xPSet.is())
    {
        xPSet->getPropertyValue( DLGED_PROP_STEP ) >>= nStep;
    }
    return nStep;
}

//----------------------------------------------------------------------------

void DlgEdObj::UpdateStep()
{
    sal_Int32 nCurStep = GetDlgEdForm()->GetStep();
    sal_Int32 nStep = GetStep();

    if( nCurStep )
    {
        SdrLayerAdmin& rLayerAdmin = GetModel()->GetLayerAdmin();
        SdrLayerID      nLayerId   = rLayerAdmin.GetLayerID( String( RTL_CONSTASCII_USTRINGPARAM( "HiddenLayer" ) ), FALSE );
        if ( nStep && (nStep != nCurStep) )
        {
            SetLayer( nLayerId );
        }
        else
        {
            SetLayer( 0 );
        }
    }
    else
    {
        SetLayer( 0 );
    }
}

//----------------------------------------------------------------------------

void DlgEdObj::TabIndexChange( const beans::PropertyChangeEvent& evt ) throw (RuntimeException)
{
    DlgEdForm* pForm = GetDlgEdForm();
    if ( pForm )
    {
        // stop listening with all children
        ::std::vector<DlgEdObj*> aChildList = pForm->GetChilds();
        ::std::vector<DlgEdObj*>::iterator aIter;
        for ( aIter = aChildList.begin() ; aIter != aChildList.end() ; ++aIter )
        {
            (*aIter)->EndListening( sal_False );
        }

        Reference< container::XNameAccess > xNameAcc( pForm->GetUnoControlModel() , UNO_QUERY );
        if ( xNameAcc.is() )
        {
            // get sequence of control names
            Sequence< ::rtl::OUString > aNames = xNameAcc->getElementNames();
            const ::rtl::OUString* pNames = aNames.getConstArray();
            sal_Int32 nCtrls = aNames.getLength();
            sal_Int16 i;

            // create a map of tab indices and control names, sorted by tab index
            IndexToNameMap aIndexToNameMap;
            for ( i = 0; i < nCtrls; ++i )
            {
                // get control name
                ::rtl::OUString aName( pNames[i] );

                // get tab index
                sal_Int16 nTabIndex = -1;
                Any aCtrl = xNameAcc->getByName( aName );
                Reference< beans::XPropertySet > xPSet;
                   aCtrl >>= xPSet;
                if ( xPSet.is() && xPSet == Reference< beans::XPropertySet >( evt.Source, UNO_QUERY ) )
                    evt.OldValue >>= nTabIndex;
                else if ( xPSet.is() )
                    xPSet->getPropertyValue( DLGED_PROP_TABINDEX ) >>= nTabIndex;

                // insert into map
                aIndexToNameMap.insert( IndexToNameMap::value_type( nTabIndex, aName ) );
            }

            // create a helper list of control names, sorted by tab index
            ::std::vector< ::rtl::OUString > aNameList( aIndexToNameMap.size() );
            ::std::transform(
                    aIndexToNameMap.begin(), aIndexToNameMap.end(),
                    aNameList.begin(),
                    ::std::select2nd< IndexToNameMap::value_type >( )
                );

            // check tab index
            sal_Int16 nOldTabIndex;
            evt.OldValue >>= nOldTabIndex;
            sal_Int16 nNewTabIndex;
            evt.NewValue >>= nNewTabIndex;
            if ( nNewTabIndex < 0 )
                nNewTabIndex = 0;
            else if ( nNewTabIndex > nCtrls - 1 )
                nNewTabIndex = nCtrls - 1;

            // reorder helper list
            ::rtl::OUString aCtrlName = aNameList[nOldTabIndex];
            aNameList.erase( aNameList.begin() + nOldTabIndex );
            aNameList.insert( aNameList.begin() + nNewTabIndex , aCtrlName );

            // set new tab indices
            for ( i = 0; i < nCtrls; ++i )
            {
                Any aCtrl = xNameAcc->getByName( aNameList[i] );
                Reference< beans::XPropertySet > xPSet;
                   aCtrl >>= xPSet;
                if ( xPSet.is() )
                {
                    Any aTabIndex;
                    aTabIndex <<= (sal_Int16) i;
                    xPSet->setPropertyValue( DLGED_PROP_TABINDEX, aTabIndex );
                }
            }

            // reorder objects in drawing page
            GetModel()->GetPage(0)->SetObjectOrdNum( nOldTabIndex + 1, nNewTabIndex + 1 );

            // #110559#
            pForm->UpdateTabOrderAndGroups();
        }

        // start listening with all children
        for ( aIter = aChildList.begin() ; aIter != aChildList.end() ; ++aIter )
        {
            (*aIter)->StartListening();
        }
    }
}

//----------------------------------------------------------------------------

sal_Bool DlgEdObj::supportsService( const sal_Char* _pServiceName ) const
{
    sal_Bool bSupports = sal_False;

    Reference< lang::XServiceInfo > xServiceInfo( GetUnoControlModel() , UNO_QUERY );
        // TODO: cache xServiceInfo as member?
    if ( xServiceInfo.is() )
        bSupports = xServiceInfo->supportsService( ::rtl::OUString::createFromAscii( _pServiceName ) );

    return bSupports;
}

//----------------------------------------------------------------------------

::rtl::OUString DlgEdObj::GetDefaultName() const
{
    sal_uInt16 nResId = 0;
    ::rtl::OUString aDefaultName;
    if ( supportsService( "com.sun.star.awt.UnoControlDialogModel" ) )
    {
        nResId = RID_STR_CLASS_DIALOG;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlButtonModel" ) )
    {
        nResId = RID_STR_CLASS_BUTTON;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlRadioButtonModel" ) )
    {
        nResId = RID_STR_CLASS_RADIOBUTTON;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlCheckBoxModel" ) )
    {
        nResId = RID_STR_CLASS_CHECKBOX;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlListBoxModel" ) )
    {
        nResId = RID_STR_CLASS_LISTBOX;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlComboBoxModel" ) )
    {
        nResId = RID_STR_CLASS_COMBOBOX;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlGroupBoxModel" ) )
    {
        nResId = RID_STR_CLASS_GROUPBOX;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlEditModel" ) )
    {
        nResId = RID_STR_CLASS_EDIT;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlFixedTextModel" ) )
    {
        nResId = RID_STR_CLASS_FIXEDTEXT;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlImageControlModel" ) )
    {
        nResId = RID_STR_CLASS_IMAGECONTROL;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlProgressBarModel" ) )
    {
        nResId = RID_STR_CLASS_PROGRESSBAR;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlScrollBarModel" ) )
    {
        nResId = RID_STR_CLASS_SCROLLBAR;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlFixedLineModel" ) )
    {
        nResId = RID_STR_CLASS_FIXEDLINE;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlDateFieldModel" ) )
    {
        nResId = RID_STR_CLASS_DATEFIELD;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlTimeFieldModel" ) )
    {
        nResId = RID_STR_CLASS_TIMEFIELD;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlNumericFieldModel" ) )
    {
        nResId = RID_STR_CLASS_NUMERICFIELD;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlCurrencyFieldModel" ) )
    {
        nResId = RID_STR_CLASS_CURRENCYFIELD;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlFormattedFieldModel" ) )
    {
        nResId = RID_STR_CLASS_FORMATTEDFIELD;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlPatternFieldModel" ) )
    {
        nResId = RID_STR_CLASS_PATTERNFIELD;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlFileControlModel" ) )
    {
        nResId = RID_STR_CLASS_FILECONTROL;
    }
    else
    {
        nResId = RID_STR_CLASS_CONTROL;
    }

    if (nResId)
    {
        aDefaultName = ::rtl::OUString( String(IDEResId(nResId)) );
    }

    return aDefaultName;
}

//----------------------------------------------------------------------------

::rtl::OUString DlgEdObj::GetUniqueName() const
{
    ::rtl::OUString aUniqueName;
    uno::Reference< container::XNameAccess > xNameAcc((GetDlgEdForm()->GetUnoControlModel()), uno::UNO_QUERY);

    if ( xNameAcc.is() )
    {
        sal_Int32 n = 0;
        ::rtl::OUString aDefaultName = GetDefaultName();

        do
        {
            aUniqueName = aDefaultName + ::rtl::OUString::valueOf(++n);
        }   while (xNameAcc->hasByName(aUniqueName));
    }

    return aUniqueName;
}

//----------------------------------------------------------------------------

sal_uInt32 DlgEdObj::GetObjInventor()   const
{
    return DlgInventor;
}

//----------------------------------------------------------------------------

sal_uInt16 DlgEdObj::GetObjIdentifier() const
{
    if ( supportsService( "com.sun.star.awt.UnoControlDialogModel" ))
    {
        return OBJ_DLG_DIALOG;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlButtonModel" ))
    {
        return OBJ_DLG_PUSHBUTTON;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlRadioButtonModel" ))
    {
        return OBJ_DLG_RADIOBUTTON;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlCheckBoxModel" ))
    {
        return OBJ_DLG_CHECKBOX;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlListBoxModel" ))
    {
        return OBJ_DLG_LISTBOX;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlComboBoxModel" ))
    {
        return OBJ_DLG_COMBOBOX;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlGroupBoxModel" ))
    {
        return OBJ_DLG_GROUPBOX;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlEditModel" ))
    {
        return OBJ_DLG_EDIT;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlFixedTextModel" ))
    {
        return OBJ_DLG_FIXEDTEXT;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlImageControlModel" ))
    {
        return OBJ_DLG_IMAGECONTROL;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlProgressBarModel" ))
    {
        return OBJ_DLG_PROGRESSBAR;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlScrollBarModel" ))
    {
        return OBJ_DLG_HSCROLLBAR;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlFixedLineModel" ))
    {
        return OBJ_DLG_HFIXEDLINE;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlDateFieldModel" ))
    {
        return OBJ_DLG_DATEFIELD;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlTimeFieldModel" ))
    {
        return OBJ_DLG_TIMEFIELD;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlNumericFieldModel" ))
    {
        return OBJ_DLG_NUMERICFIELD;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlCurrencyFieldModel" ))
    {
        return OBJ_DLG_CURRENCYFIELD;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlFormattedFieldModel" ))
    {
        return OBJ_DLG_FORMATTEDFIELD;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlPatternFieldModel" ))
    {
        return OBJ_DLG_PATTERNFIELD;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlFileControlModel" ))
    {
        return OBJ_DLG_FILECONTROL;
    }
    else
    {
        return OBJ_DLG_CONTROL;
    }
}

//----------------------------------------------------------------------------

void DlgEdObj::clonedFrom(const DlgEdObj* _pSource)
{
    // set parent form
    pDlgEdForm = _pSource->pDlgEdForm;

    // add child to parent form
    pDlgEdForm->AddChild( this );

    Reference< beans::XPropertySet > xPSet( GetUnoControlModel(), UNO_QUERY );
    if ( xPSet.is() )
    {
        // set new name
        ::rtl::OUString aOUniqueName( GetUniqueName() );
        Any aUniqueName;
        aUniqueName <<= aOUniqueName;
        xPSet->setPropertyValue( DLGED_PROP_NAME, aUniqueName );

        Reference< container::XNameContainer > xCont( GetDlgEdForm()->GetUnoControlModel() , UNO_QUERY );
        if ( xCont.is() )
        {
            // set tabindex
               Sequence< OUString > aNames = xCont->getElementNames();
            Any aTabIndex;
            aTabIndex <<= (sal_Int16) aNames.getLength();
            xPSet->setPropertyValue( DLGED_PROP_TABINDEX, aTabIndex );

            // insert control model in dialog model
            Reference< awt::XControlModel > xCtrl( xPSet , UNO_QUERY );
            Any aCtrl;
            aCtrl <<= xCtrl;
            xCont->insertByName( aOUniqueName , aCtrl );

            // #110559#
            pDlgEdForm->UpdateTabOrderAndGroups();
        }
    }

    // start listening
    StartListening();
}

//----------------------------------------------------------------------------

SdrObject* DlgEdObj::Clone() const
{
    SdrObject* pReturn = SdrUnoObj::Clone();

    DlgEdObj* pDlgEdObj = PTR_CAST(DlgEdObj, pReturn);
    DBG_ASSERT( pDlgEdObj != NULL, "DlgEdObj::Clone: invalid clone!" );
    if ( pDlgEdObj )
        pDlgEdObj->clonedFrom( this );

    return pReturn;
}

//----------------------------------------------------------------------------

SdrObject* DlgEdObj::Clone(SdrPage* _pPage, SdrModel* _pModel) const // not working yet
{
    // #116235#
    //SdrObject* pReturn = SdrUnoObj::Clone(_pPage, _pModel);
    SdrObject* pReturn = SdrUnoObj::Clone();

    if(pReturn)
    {
        pReturn->SetModel(_pModel);
        pReturn->SetPage(_pPage);
    }

    return pReturn;
}

//----------------------------------------------------------------------------

void DlgEdObj::operator= (const SdrObject& rObj)
{
    SdrUnoObj::operator= (rObj);
}

//----------------------------------------------------------------------------

//BFS01void DlgEdObj::WriteData(SvStream& rOut) const   // not working yet
//BFS01{
//BFS01 SdrUnoObj::WriteData(rOut);
//BFS01}

//----------------------------------------------------------------------------

//BFS01void DlgEdObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)  // not working yet
//BFS01{
//BFS01 SdrUnoObj::ReadData(rHead,rIn);
//BFS01}

//----------------------------------------------------------------------------

void DlgEdObj::NbcMove( const Size& rSize )
{
    SdrUnoObj::NbcMove( rSize );

    // stop listening
    EndListening(sal_False);

    // set geometry properties
    SetPropsFromRect();

    // start listening
    StartListening();

    // dialog model changed
    GetDlgEdForm()->GetDlgEditor()->SetDialogModelChanged(TRUE);
}

//----------------------------------------------------------------------------

void DlgEdObj::NbcResize(const Point& rRef, const Fraction& xFract, const Fraction& yFract)
{
    SdrUnoObj::NbcResize( rRef, xFract, yFract );

    // stop listening
    EndListening(sal_False);

    // set geometry properties
    SetPropsFromRect();

    // start listening
    StartListening();

    // dialog model changed
    GetDlgEdForm()->GetDlgEditor()->SetDialogModelChanged(TRUE);
}

//----------------------------------------------------------------------------

FASTBOOL DlgEdObj::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    sal_Bool bResult = SdrUnoObj::EndCreate(rStat, eCmd);

    SetDefaults();
    StartListening();

    return bResult;
}

//----------------------------------------------------------------------------

void DlgEdObj::SetDefaults()
{
    // set parent form
    pDlgEdForm = ((DlgEdPage*)GetPage())->GetDlgEdForm();

    if ( pDlgEdForm )
    {
        // add child to parent form
        pDlgEdForm->AddChild( this );

        Reference< beans::XPropertySet > xPSet( GetUnoControlModel(), UNO_QUERY );
        if ( xPSet.is() )
        {
            // get unique name
            ::rtl::OUString aOUniqueName( GetUniqueName() );

            // set name property
            Any aUniqueName;
            aUniqueName <<= aOUniqueName;
            xPSet->setPropertyValue( DLGED_PROP_NAME, aUniqueName );

            // set labels
            if ( supportsService( "com.sun.star.awt.UnoControlButtonModel" ) ||
                supportsService( "com.sun.star.awt.UnoControlRadioButtonModel" ) ||
                supportsService( "com.sun.star.awt.UnoControlCheckBoxModel" ) ||
                supportsService( "com.sun.star.awt.UnoControlGroupBoxModel" ) ||
                supportsService( "com.sun.star.awt.UnoControlFixedTextModel" ) )
            {
                xPSet->setPropertyValue( DLGED_PROP_LABEL, aUniqueName );
            }

            // set number formats supplier for formatted field
            if ( supportsService( "com.sun.star.awt.UnoControlFormattedFieldModel" ) )
            {
                Reference< util::XNumberFormatsSupplier > xSupplier = GetDlgEdForm()->GetDlgEditor()->GetNumberFormatsSupplier();
                if ( xSupplier.is() )
                {
                    Any aSupplier;
                    aSupplier <<= xSupplier;
                    xPSet->setPropertyValue( DLGED_PROP_FORMATSSUPPLIER, aSupplier );
                }
            }

            // set geometry properties
            SetPropsFromRect();

            Reference< container::XNameContainer > xCont( GetDlgEdForm()->GetUnoControlModel() , UNO_QUERY );
            if ( xCont.is() )
            {
                // set tabindex
                   Sequence< OUString > aNames = xCont->getElementNames();
                uno::Any aTabIndex;
                aTabIndex <<= (sal_Int16) aNames.getLength();
                xPSet->setPropertyValue( DLGED_PROP_TABINDEX, aTabIndex );

                // set step
                Reference< beans::XPropertySet > xPSetForm( xCont, UNO_QUERY );
                if ( xPSetForm.is() )
                {
                    Any aStep = xPSetForm->getPropertyValue( DLGED_PROP_STEP );
                    xPSet->setPropertyValue( DLGED_PROP_STEP, aStep );
                }

                // insert control model in dialog model
                Reference< awt::XControlModel > xCtrl( xPSet , UNO_QUERY );
                Any aAny;
                aAny <<= xCtrl;
                xCont->insertByName( aOUniqueName , aAny );

                // #110559#
                pDlgEdForm->UpdateTabOrderAndGroups();
            }
        }

        // dialog model changed
        pDlgEdForm->GetDlgEditor()->SetDialogModelChanged( TRUE );
    }
}

//----------------------------------------------------------------------------

IMPL_LINK(DlgEdObj, OnCreate, void*, EMPTYTAG)
{
    /*
    if (pTempView)
        pTempView->ObjectCreated(this);
    */
    return 0;
}

//----------------------------------------------------------------------------

void DlgEdObj::StartListening()
{
    DBG_ASSERT(!isListening(), "DlgEdObj::StartListening: already listening!");

    if (!isListening())
    {
        bIsListening = sal_True;

        // XPropertyChangeListener
        Reference< XPropertySet > xControlModel( GetUnoControlModel() , UNO_QUERY );
        if (!m_xPropertyChangeListener.is() && xControlModel.is())
        {
            // create listener
            m_xPropertyChangeListener = static_cast< ::com::sun::star::beans::XPropertyChangeListener*>( new DlgEdPropListenerImpl( (DlgEdObj*)this ) );

            // register listener to properties
            xControlModel->addPropertyChangeListener( ::rtl::OUString() , m_xPropertyChangeListener );
        }

        // XContainerListener
        Reference< XScriptEventsSupplier > xEventsSupplier( GetUnoControlModel() , UNO_QUERY );
        if( !m_xContainerListener.is() && xEventsSupplier.is() )
        {
            // create listener
            m_xContainerListener = static_cast< ::com::sun::star::container::XContainerListener*>( new DlgEdEvtContListenerImpl( (DlgEdObj*)this ) );

            // register listener to script event container
            Reference< XNameContainer > xEventCont = xEventsSupplier->getEvents();
            DBG_ASSERT(xEventCont.is(), "DlgEdObj::StartListening: control model has no script event container!");
            Reference< XContainer > xCont( xEventCont , UNO_QUERY );
            if (xCont.is())
                xCont->addContainerListener( m_xContainerListener );
        }
    }
}

//----------------------------------------------------------------------------

void DlgEdObj::EndListening(sal_Bool bRemoveListener)
{
    DBG_ASSERT(isListening(), "DlgEdObj::EndListening: not listening currently!");

    if (isListening())
    {
        bIsListening = sal_False;

        if (bRemoveListener)
        {
            // XPropertyChangeListener
            Reference< XPropertySet > xControlModel(GetUnoControlModel(), UNO_QUERY);
            if ( m_xPropertyChangeListener.is() && xControlModel.is() )
            {
                // remove listener
                xControlModel->removePropertyChangeListener( ::rtl::OUString() , m_xPropertyChangeListener );
            }
            m_xPropertyChangeListener.clear();

            // XContainerListener
            Reference< XScriptEventsSupplier > xEventsSupplier( GetUnoControlModel() , UNO_QUERY );
            if( m_xContainerListener.is() && xEventsSupplier.is() )
            {
                // remove listener
                Reference< XNameContainer > xEventCont = xEventsSupplier->getEvents();
                DBG_ASSERT(xEventCont.is(), "DlgEdObj::EndListening: control model has no script event container!");
                Reference< XContainer > xCont( xEventCont , UNO_QUERY );
                if (xCont.is())
                    xCont->removeContainerListener( m_xContainerListener );
            }
            m_xContainerListener.clear();
        }
    }
}

//----------------------------------------------------------------------------

void SAL_CALL DlgEdObj::_propertyChange( const  ::com::sun::star::beans::PropertyChangeEvent& evt ) throw( ::com::sun::star::uno::RuntimeException)
{
    if (isListening())
    {
        // dialog model changed
        if ( ISA(DlgEdForm) )
        {
            ((DlgEdForm*)this)->GetDlgEditor()->SetDialogModelChanged(TRUE);
        }
        else
        {
            GetDlgEdForm()->GetDlgEditor()->SetDialogModelChanged(TRUE);
        }

        // update position and size
        if ( evt.PropertyName == DLGED_PROP_POSITIONX || evt.PropertyName == DLGED_PROP_POSITIONY ||
             evt.PropertyName == DLGED_PROP_WIDTH || evt.PropertyName == DLGED_PROP_HEIGHT )
        {
            PositionAndSizeChange( evt );
        }
        // change name of control in dialog model
        else if ( evt.PropertyName == DLGED_PROP_NAME )
        {
            if ( !ISA(DlgEdForm) )
            {
                NameChange(evt);
            }
        }
        // update step
        else if ( evt.PropertyName == DLGED_PROP_STEP )
        {
            UpdateStep();
        }
        // change tabindex
        else if ( evt.PropertyName == DLGED_PROP_TABINDEX )
        {
            if ( !ISA(DlgEdForm) )
            {
                TabIndexChange(evt);
            }
        }
    }
}

//----------------------------------------------------------------------------

void SAL_CALL DlgEdObj::_elementInserted(const ::com::sun::star::container::ContainerEvent& Event) throw(::com::sun::star::uno::RuntimeException)
{
    if (isListening())
    {
        // dialog model changed
        if ( ISA(DlgEdForm) )
        {
            ((DlgEdForm*)this)->GetDlgEditor()->SetDialogModelChanged(TRUE);
        }
        else
        {
            GetDlgEdForm()->GetDlgEditor()->SetDialogModelChanged(TRUE);
        }
    }
}

//----------------------------------------------------------------------------

void SAL_CALL DlgEdObj::_elementReplaced(const ::com::sun::star::container::ContainerEvent& Event) throw(::com::sun::star::uno::RuntimeException)
{
    if (isListening())
    {
        // dialog model changed
        if ( ISA(DlgEdForm) )
        {
            ((DlgEdForm*)this)->GetDlgEditor()->SetDialogModelChanged(TRUE);
        }
        else
        {
            GetDlgEdForm()->GetDlgEditor()->SetDialogModelChanged(TRUE);
        }
    }
}

//----------------------------------------------------------------------------

void SAL_CALL DlgEdObj::_elementRemoved(const ::com::sun::star::container::ContainerEvent& Event) throw(::com::sun::star::uno::RuntimeException)
{
    if (isListening())
    {
        // dialog model changed
        if ( ISA(DlgEdForm) )
        {
            ((DlgEdForm*)this)->GetDlgEditor()->SetDialogModelChanged(TRUE);
        }
        else
        {
            GetDlgEdForm()->GetDlgEditor()->SetDialogModelChanged(TRUE);
        }
    }
}

//----------------------------------------------------------------------------

void DlgEdObj::SetLayer(SdrLayerID nLayer)
{
    SdrLayerID nOldLayer = GetLayer();

    if ( nLayer != nOldLayer )
    {
        SdrUnoObj::SetLayer( nLayer );

        DlgEdHint aHint( DLGED_HINT_LAYERCHANGED, this );
        GetDlgEdForm()->GetDlgEditor()->Broadcast( aHint );
    }
}

//----------------------------------------------------------------------------

SdrObject* DlgEdObj::CheckHit( const Point& rPnt, USHORT nTol,const SetOfByte* pSet ) const
{
    // #109994# fixed here, because the drawing layer doesn't handle objects
    // with a width or height of 0 in a proper way
    Rectangle aRect( aOutRect );
    if ( aRect.IsEmpty() )
    {
        aRect.Left() -= nTol;
        aRect.Top() -= nTol;
        aRect.Right() = ( aRect.Right() == RECT_EMPTY ? aOutRect.Left() + nTol : aRect.Right() + nTol );
        aRect.Bottom() = ( aRect.Bottom() == RECT_EMPTY ? aOutRect.Top() + nTol : aRect.Bottom() + nTol );

        if ( aRect.IsInside( rPnt ) )
            return (SdrObject*)this;
        else
            return 0;
    }

    if ( supportsService( "com.sun.star.awt.UnoControlGroupBoxModel" ))
    {
        Rectangle aROuter = aOutRect;
        aROuter.Left()   -= nTol;
        aROuter.Right()  += nTol;
        aROuter.Top()    -= nTol;
        aROuter.Bottom() += nTol;

        Rectangle aRInner = aOutRect;
        if( (aRInner.GetSize().Height() > (long)nTol*2) &&
            (aRInner.GetSize().Width()  > (long)nTol*2)    )
        {
            aRInner.Left()   += nTol;
            aRInner.Right()  -= nTol;
            aRInner.Top()    += nTol;
            aRInner.Bottom() -= nTol;
        }

        if( aROuter.IsInside( rPnt ) && !aRInner.IsInside( rPnt ) )
            return (SdrObject*)this;
        else
            return 0;
    }
    else
        return SdrUnoObj::CheckHit( rPnt, nTol, pSet );
}


//----------------------------------------------------------------------------

TYPEINIT1(DlgEdForm, DlgEdObj);
DBG_NAME(DlgEdForm);

//----------------------------------------------------------------------------

DlgEdForm::DlgEdForm(const ::rtl::OUString& rModelName)
          :DlgEdObj(rModelName)
{
    DBG_CTOR(DlgEdForm, NULL);
}

//----------------------------------------------------------------------------

DlgEdForm::DlgEdForm(const ::rtl::OUString& rModelName,
                    const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxSFac)
          :DlgEdObj(rModelName, rxSFac)
{
    DBG_CTOR(DlgEdForm, NULL);
}

//----------------------------------------------------------------------------

DlgEdForm::DlgEdForm()
          :DlgEdObj()
{
    DBG_CTOR(DlgEdForm, NULL);
}

//----------------------------------------------------------------------------

DlgEdForm::~DlgEdForm()
{
    DBG_DTOR(DlgEdForm, NULL);
}

//----------------------------------------------------------------------------

void DlgEdForm::SetRectFromProps()
{
    // get form position and size from properties
    Reference< beans::XPropertySet > xPSet( GetUnoControlModel(), UNO_QUERY );
    if ( xPSet.is() )
    {
        sal_Int32 nXIn, nYIn, nWidthIn, nHeightIn;
        xPSet->getPropertyValue( DLGED_PROP_POSITIONX ) >>= nXIn;
        xPSet->getPropertyValue( DLGED_PROP_POSITIONY ) >>= nYIn;
        xPSet->getPropertyValue( DLGED_PROP_WIDTH ) >>= nWidthIn;
        xPSet->getPropertyValue( DLGED_PROP_HEIGHT ) >>= nHeightIn;

        // transform coordinates
        sal_Int32 nXOut, nYOut, nWidthOut, nHeightOut;
        if ( TransformFormToSdrCoordinates( nXIn, nYIn, nWidthIn, nHeightIn, nXOut, nYOut, nWidthOut, nHeightOut ) )
        {
            // set rectangle position and size
            Point aPoint( nXOut, nYOut );
            Size aSize( nWidthOut, nHeightOut );
            SetSnapRect( Rectangle( aPoint, aSize ) );
        }
    }
}

//----------------------------------------------------------------------------

void DlgEdForm::SetPropsFromRect()
{
    // get form position and size from rectangle
    Rectangle aRect = GetSnapRect();
    sal_Int32 nXIn = aRect.Left();
    sal_Int32 nYIn = aRect.Top();
    sal_Int32 nWidthIn = aRect.GetWidth();
    sal_Int32 nHeightIn = aRect.GetHeight();

    // transform coordinates
    sal_Int32 nXOut, nYOut, nWidthOut, nHeightOut;
    if ( TransformSdrToFormCoordinates( nXIn, nYIn, nWidthIn, nHeightIn, nXOut, nYOut, nWidthOut, nHeightOut ) )
    {
        // set properties
        Reference< beans::XPropertySet > xPSet( GetUnoControlModel(), UNO_QUERY );
        if ( xPSet.is() )
        {
            Any aValue;
            aValue <<= nXOut;
            xPSet->setPropertyValue( DLGED_PROP_POSITIONX, aValue );
            aValue <<= nYOut;
            xPSet->setPropertyValue( DLGED_PROP_POSITIONY, aValue );
            aValue <<= nWidthOut;
            xPSet->setPropertyValue( DLGED_PROP_WIDTH, aValue );
            aValue <<= nHeightOut;
            xPSet->setPropertyValue( DLGED_PROP_HEIGHT, aValue );
        }
    }
}

//----------------------------------------------------------------------------

void DlgEdForm::AddChild( DlgEdObj* pDlgEdObj )
{
    pChilds.push_back( pDlgEdObj );
}

//----------------------------------------------------------------------------

void DlgEdForm::RemoveChild( DlgEdObj* pDlgEdObj )
{
    pChilds.erase( ::std::find( pChilds.begin() , pChilds.end() , pDlgEdObj ) );
}

//----------------------------------------------------------------------------

void DlgEdForm::PositionAndSizeChange( const beans::PropertyChangeEvent& evt )
{
    DlgEditor* pEditor = GetDlgEditor();
    DBG_ASSERT( pEditor, "DlgEdForm::PositionAndSizeChange: no dialog editor!" );
    if ( pEditor )
    {
        DlgEdPage* pPage = pEditor->GetPage();
        DBG_ASSERT( pPage, "DlgEdForm::PositionAndSizeChange: no page!" );
        if ( pPage )
        {
            sal_Int32 nPageXIn = 0;
            sal_Int32 nPageYIn = 0;
            Size aPageSize = pPage->GetSize();
            sal_Int32 nPageWidthIn = aPageSize.Width();
            sal_Int32 nPageHeightIn = aPageSize.Height();
            sal_Int32 nPageX, nPageY, nPageWidth, nPageHeight;
            if ( TransformSdrToFormCoordinates( nPageXIn, nPageYIn, nPageWidthIn, nPageHeightIn, nPageX, nPageY, nPageWidth, nPageHeight ) )
            {
                Reference< beans::XPropertySet > xPSetForm( GetUnoControlModel(), UNO_QUERY );
                if ( xPSetForm.is() )
                {
                    sal_Int32 nValue;
                    evt.NewValue >>= nValue;
                    sal_Int32 nNewValue = nValue;

                    if ( evt.PropertyName == DLGED_PROP_POSITIONX )
                    {
                        if ( nNewValue < nPageX )
                            nNewValue = nPageX;
                    }
                    else if ( evt.PropertyName == DLGED_PROP_POSITIONY )
                    {
                        if ( nNewValue < nPageY )
                            nNewValue = nPageY;
                    }
                    else if ( evt.PropertyName == DLGED_PROP_WIDTH )
                    {
                        if ( nNewValue < 1 )
                            nNewValue = 1;
                    }
                    else if ( evt.PropertyName == DLGED_PROP_HEIGHT )
                    {
                        if ( nNewValue < 1 )
                            nNewValue = 1;
                    }

                    if ( nNewValue != nValue )
                    {
                        Any aNewValue;
                        aNewValue <<= nNewValue;
                        EndListening( sal_False );
                        xPSetForm->setPropertyValue( evt.PropertyName, aNewValue );
                        StartListening();
                    }
                }
            }

            bool bAdjustedPageSize = pEditor->AdjustPageSize();
            SetRectFromProps();
            ::std::vector< DlgEdObj* >::iterator aIter;
            ::std::vector< DlgEdObj* > aChildList = ((DlgEdForm*)this)->GetChilds();

            if ( bAdjustedPageSize )
            {
                pEditor->InitScrollBars();
                aPageSize = pPage->GetSize();
                nPageWidthIn = aPageSize.Width();
                nPageHeightIn = aPageSize.Height();
                if ( TransformSdrToControlCoordinates( nPageXIn, nPageYIn, nPageWidthIn, nPageHeightIn, nPageX, nPageY, nPageWidth, nPageHeight ) )
                {
                    for ( aIter = aChildList.begin(); aIter != aChildList.end(); aIter++ )
                    {
                        Reference< beans::XPropertySet > xPSet( (*aIter)->GetUnoControlModel(), UNO_QUERY );
                        if ( xPSet.is() )
                        {
                            sal_Int32 nX, nY, nWidth, nHeight;
                            xPSet->getPropertyValue( DLGED_PROP_POSITIONX ) >>= nX;
                            xPSet->getPropertyValue( DLGED_PROP_POSITIONY ) >>= nY;
                            xPSet->getPropertyValue( DLGED_PROP_WIDTH ) >>= nWidth;
                            xPSet->getPropertyValue( DLGED_PROP_HEIGHT ) >>= nHeight;

                            sal_Int32 nNewX = nX;
                            if ( nX + nWidth > nPageX + nPageWidth )
                            {
                                nNewX = nPageX + nPageWidth - nWidth;
                                if ( nNewX < nPageX )
                                    nNewX = nPageX;
                            }
                            if ( nNewX != nX )
                            {
                                Any aValue;
                                aValue <<= nNewX;
                                EndListening( sal_False );
                                xPSet->setPropertyValue( DLGED_PROP_POSITIONX, aValue );
                                StartListening();
                            }

                            sal_Int32 nNewY = nY;
                            if ( nY + nHeight > nPageY + nPageHeight )
                            {
                                nNewY = nPageY + nPageHeight - nHeight;
                                if ( nNewY < nPageY )
                                    nNewY = nPageY;
                            }
                            if ( nNewY != nY )
                            {
                                Any aValue;
                                aValue <<= nNewY;
                                EndListening( sal_False );
                                xPSet->setPropertyValue( DLGED_PROP_POSITIONY, aValue );
                                StartListening();
                            }
                        }
                    }
                }
            }

            for ( aIter = aChildList.begin(); aIter != aChildList.end(); aIter++ )
            {
                (*aIter)->SetRectFromProps();
            }
        }
    }
}

//----------------------------------------------------------------------------

void DlgEdForm::UpdateStep()
{
    ULONG nObjCount;
    SdrPage* pSdrPage = GetPage();

    if ( pSdrPage && ( ( nObjCount = pSdrPage->GetObjCount() ) > 0 ) )
    {
        for ( ULONG i = 0 ; i < nObjCount ; i++ )
        {
            SdrObject* pObj = pSdrPage->GetObj(i);
            DlgEdObj* pDlgEdObj = PTR_CAST(DlgEdObj, pObj);
            if ( pDlgEdObj && !pDlgEdObj->ISA(DlgEdForm) )
                pDlgEdObj->UpdateStep();
        }
    }
}

//----------------------------------------------------------------------------

void DlgEdForm::UpdateTabIndices()
{
    // stop listening with all children
    ::std::vector<DlgEdObj*>::iterator aIter;
    for ( aIter = pChilds.begin() ; aIter != pChilds.end() ; ++aIter )
    {
        (*aIter)->EndListening( sal_False );
    }

    Reference< ::com::sun::star::container::XNameAccess > xNameAcc( GetUnoControlModel() , UNO_QUERY );
    if ( xNameAcc.is() )
    {
        // get sequence of control names
        Sequence< ::rtl::OUString > aNames = xNameAcc->getElementNames();
        const ::rtl::OUString* pNames = aNames.getConstArray();
        sal_Int32 nCtrls = aNames.getLength();

        // create a map of tab indices and control names, sorted by tab index
        IndexToNameMap aIndexToNameMap;
        for ( sal_Int16 i = 0; i < nCtrls; ++i )
        {
            // get name
            ::rtl::OUString aName( pNames[i] );

            // get tab index
            sal_Int16 nTabIndex = -1;
            Any aCtrl = xNameAcc->getByName( aName );
            Reference< ::com::sun::star::beans::XPropertySet > xPSet;
               aCtrl >>= xPSet;
            if ( xPSet.is() )
                xPSet->getPropertyValue( DLGED_PROP_TABINDEX ) >>= nTabIndex;

            // insert into map
            aIndexToNameMap.insert( IndexToNameMap::value_type( nTabIndex, aName ) );
        }

        // set new tab indices
        sal_Int16 nNewTabIndex = 0;
        for ( IndexToNameMap::iterator aIt = aIndexToNameMap.begin(); aIt != aIndexToNameMap.end(); ++aIt )
        {
            Any aCtrl = xNameAcc->getByName( aIt->second );
            Reference< beans::XPropertySet > xPSet;
               aCtrl >>= xPSet;
            if ( xPSet.is() )
            {
                Any aTabIndex;
                aTabIndex <<= (sal_Int16) nNewTabIndex++;
                xPSet->setPropertyValue( DLGED_PROP_TABINDEX, aTabIndex );
            }
        }

        // #110559#
        UpdateTabOrderAndGroups();
    }

    // start listening with all children
    for ( aIter = pChilds.begin() ; aIter != pChilds.end() ; ++aIter )
    {
        (*aIter)->StartListening();
    }
}

//----------------------------------------------------------------------------

void DlgEdForm::UpdateTabOrder()
{
    // #110559#
    // When the tabindex of a control model changes, the dialog control is
    // notified about those changes. Due to #109067# (bad performance of
    // dialog editor) the dialog control doesn't activate the tab order
    // in design mode. When the dialog editor has reordered all
    // tabindices, this method allows to activate the taborder afterwards.

    Window* pWindow = GetDlgEditor() ? GetDlgEditor()->GetWindow() : NULL;
    if ( pWindow )
    {
        Reference< awt::XUnoControlContainer > xCont( GetUnoControl( pWindow ), UNO_QUERY );
        if ( xCont.is() )
        {
            Sequence< Reference< awt::XTabController > > aSeqTabCtrls = xCont->getTabControllers();
            const Reference< awt::XTabController >* pTabCtrls = aSeqTabCtrls.getConstArray();
            sal_Int32 nCount = aSeqTabCtrls.getLength();
            for ( sal_Int32 i = 0; i < nCount; ++i )
                pTabCtrls[i]->activateTabOrder();
        }
    }
}

//----------------------------------------------------------------------------

void DlgEdForm::UpdateGroups()
{
    // #110559#
    // The grouping of radio buttons in a dialog is done by vcl.
    // In the dialog editor we have two views (=controls) for one
    // radio button model. One control is owned by the dialog control,
    // but not visible in design mode. The other control is owned by
    // the drawing layer object. Whereas the grouping of the first
    // control is done by vcl, the grouping of the control in the
    // drawing layer has to be done here.

    Reference< awt::XTabControllerModel > xTabModel( GetUnoControlModel() , UNO_QUERY );
    if ( xTabModel.is() )
    {
        Window* pWindow = GetDlgEditor() ? GetDlgEditor()->GetWindow() : NULL;
        if ( pWindow )
        {
            // create a global list of controls that belong to the dialog
            ::std::vector<DlgEdObj*> aChildList = GetChilds();
            sal_uInt32 nSize = aChildList.size();
            Sequence< Reference< awt::XControl > > aSeqControls( nSize );
            for ( sal_uInt32 i = 0; i < nSize; ++i )
                aSeqControls.getArray()[i] = Reference< awt::XControl >( aChildList[i]->GetUnoControl( pWindow ), UNO_QUERY );

            sal_Int32 nGroupCount = xTabModel->getGroupCount();
            for ( sal_Int32 nGroup = 0; nGroup < nGroupCount; ++nGroup )
            {
                // get a list of control models that belong to this group
                ::rtl::OUString aName;
                Sequence< Reference< awt::XControlModel > > aSeqModels;
                xTabModel->getGroup( nGroup, aSeqModels, aName );
                const Reference< awt::XControlModel >* pModels = aSeqModels.getConstArray();
                sal_Int32 nModelCount = aSeqModels.getLength();

                // create a list of peers that belong to this group
                Sequence< Reference< awt::XWindow > > aSeqPeers( nModelCount );
                for ( sal_Int32 nModel = 0; nModel < nModelCount; ++nModel )
                {
                    // for each control model find the corresponding control in the global list
                    const Reference< awt::XControl >* pControls = aSeqControls.getConstArray();
                    sal_Int32 nControlCount = aSeqControls.getLength();
                    for ( sal_Int32 nControl = 0; nControl < nControlCount; ++nControl )
                    {
                        const Reference< awt::XControl > xCtrl( pControls[nControl] );
                        if ( xCtrl.is() )
                        {
                            Reference< awt::XControlModel > xCtrlModel( xCtrl->getModel() );
                            if ( (awt::XControlModel*)xCtrlModel.get() == (awt::XControlModel*)pModels[nModel].get() )
                            {
                                // get the control peer and insert into the list of peers
                                aSeqPeers.getArray()[ nModel ] = Reference< awt::XWindow >( xCtrl->getPeer(), UNO_QUERY );
                                break;
                            }
                        }
                    }
                }

                // set the group at the dialog peer
                Reference< awt::XControl > xDlg( GetUnoControl( pWindow ), UNO_QUERY );
                if ( xDlg.is() )
                {
                    Reference< awt::XVclContainerPeer > xDlgPeer( xDlg->getPeer(), UNO_QUERY );
                    if ( xDlgPeer.is() )
                        xDlgPeer->setGroup( aSeqPeers );
                }
            }
        }
    }
}

//----------------------------------------------------------------------------

void DlgEdForm::UpdateTabOrderAndGroups()
{
    UpdateTabOrder();
    UpdateGroups();
}

//----------------------------------------------------------------------------

SdrObject* DlgEdForm::CheckHit( const Point& rPnt, USHORT nTol,
    const SetOfByte*  pSet ) const
{
    Rectangle aROuter = aOutRect;
    aROuter.Left()   -= nTol;
    aROuter.Right()  += nTol;
    aROuter.Top()    -= nTol;
    aROuter.Bottom() += nTol;

    Rectangle aRInner = aOutRect;
    if( (aRInner.GetSize().Height() > (long)nTol*2) &&
        (aRInner.GetSize().Width()  > (long)nTol*2)    )
    {
        aRInner.Left()   += nTol;
        aRInner.Right()  -= nTol;
        aRInner.Top()    += nTol;
        aRInner.Bottom() -= nTol;
    }

    if( aROuter.IsInside( rPnt ) && !aRInner.IsInside( rPnt ) )
        return (SdrObject*)this;
    else
        return 0;
}

//----------------------------------------------------------------------------

void DlgEdForm::NbcMove( const Size& rSize )
{
    SdrUnoObj::NbcMove( rSize );

    // set geometry properties of form
    EndListening(sal_False);
    SetPropsFromRect();
    StartListening();

    // set geometry properties of all childs
    ::std::vector<DlgEdObj*>::iterator aIter;
    for ( aIter = pChilds.begin() ; aIter != pChilds.end() ; aIter++ )
    {
        (*aIter)->EndListening(sal_False);
        (*aIter)->SetPropsFromRect();
        (*aIter)->StartListening();
    }

    // dialog model changed
    GetDlgEditor()->SetDialogModelChanged(TRUE);
}

//----------------------------------------------------------------------------

void DlgEdForm::NbcResize(const Point& rRef, const Fraction& xFract, const Fraction& yFract)
{
    SdrUnoObj::NbcResize( rRef, xFract, yFract );

    // set geometry properties of form
    EndListening(sal_False);
    SetPropsFromRect();
    StartListening();

    // set geometry properties of all childs
    ::std::vector<DlgEdObj*>::iterator aIter;
    for ( aIter = pChilds.begin() ; aIter != pChilds.end() ; aIter++ )
    {
        (*aIter)->EndListening(sal_False);
        (*aIter)->SetPropsFromRect();
        (*aIter)->StartListening();
    }

    // dialog model changed
    GetDlgEditor()->SetDialogModelChanged(TRUE);
}

//----------------------------------------------------------------------------

FASTBOOL DlgEdForm::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    sal_Bool bResult = SdrUnoObj::EndCreate(rStat, eCmd);

    // stop listening
    EndListening(sal_False);

    // set geometry properties
    SetPropsFromRect();

    // dialog model changed
    GetDlgEditor()->SetDialogModelChanged(TRUE);

    // start listening
    StartListening();

    return bResult;
}

//----------------------------------------------------------------------------


