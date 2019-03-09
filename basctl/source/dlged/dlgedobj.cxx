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
#include <sal/log.hxx>

#include <cassert>

#include <dlged.hxx>
#include <dlgeddef.hxx>
#include <dlgedlist.hxx>
#include <dlgedobj.hxx>
#include <dlgedpage.hxx>
#include <dlgedview.hxx>
#include <localizationmgr.hxx>
#include <strings.hxx>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/form/binding/XBindableValue.hpp>
#include <com/sun/star/form/binding/XValueBinding.hpp>
#include <com/sun/star/form/binding/XListEntrySink.hpp>
#include <com/sun/star/awt/XUnoControlContainer.hpp>
#include <com/sun/star/awt/XVclContainerPeer.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/script/XScriptEventsSupplier.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <cppuhelper/exc_hlp.hxx>
#include <o3tl/functional.hxx>
#include <unotools/sharedunocomponent.hxx>
#include <vcl/svapp.hxx>

namespace basctl
{

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::script;


DlgEditor& DlgEdObj::GetDialogEditor ()
{
    if (DlgEdForm* pFormThis = dynamic_cast<DlgEdForm*>(this))
        return pFormThis->GetDlgEditor();
    else
        return pDlgEdForm->GetDlgEditor();
}

DlgEdObj::DlgEdObj(SdrModel& rSdrModel)
:   SdrUnoObj(rSdrModel, OUString())
    ,bIsListening(false)
    ,pDlgEdForm( nullptr )
{
}

DlgEdObj::DlgEdObj(
    SdrModel& rSdrModel,
    const OUString& rModelName,
    const css::uno::Reference< css::lang::XMultiServiceFactory >& rxSFac)
:   SdrUnoObj(rSdrModel, rModelName, rxSFac)
    ,bIsListening(false)
    ,pDlgEdForm( nullptr )
{
}

DlgEdObj::~DlgEdObj()
{
    if ( isListening() )
        EndListening(true);
}

namespace
{
    /* returns the DlgEdForm which the given DlgEdObj belongs to
        (which might in fact be the object itself)

        Failure to obtain the form will be reported with an assertion in the non-product
        version.
     */
    bool lcl_getDlgEdForm( DlgEdObj* _pObject, DlgEdForm*& _out_pDlgEdForm )
    {
        _out_pDlgEdForm = dynamic_cast< DlgEdForm* >( _pObject );
        if ( !_out_pDlgEdForm )
            _out_pDlgEdForm = _pObject->GetDlgEdForm();
        DBG_ASSERT( _out_pDlgEdForm, "lcl_getDlgEdForm: no form!" );
        return ( _out_pDlgEdForm != nullptr );
    }
}

uno::Reference< awt::XControl > DlgEdObj::GetControl() const
{
    uno::Reference< awt::XControl > xControl;
    if (DlgEdForm const* pForm = GetDlgEdForm())
    {
        DlgEditor const& rEditor = pForm->GetDlgEditor();
        xControl = GetUnoControl(rEditor.GetView(), rEditor.GetWindow());
    }
    return xControl;
}

bool DlgEdObj::TransformSdrToControlCoordinates(
    sal_Int32 nXIn, sal_Int32 nYIn, sal_Int32 nWidthIn, sal_Int32 nHeightIn,
    sal_Int32& nXOut, sal_Int32& nYOut, sal_Int32& nWidthOut, sal_Int32& nHeightOut )
{
    // input position and size
    Size aPos( nXIn, nYIn );
    Size aSize( nWidthIn, nHeightIn );

    // form position
    DlgEdForm* pForm = nullptr;
    if ( !lcl_getDlgEdForm( this, pForm ) )
        return false;
    tools::Rectangle aFormRect = pForm->GetSnapRect();
    Size aFormPos( aFormRect.Left(), aFormRect.Top() );

    // convert 100th_mm to pixel
    OutputDevice* pDevice = Application::GetDefaultDevice();
    DBG_ASSERT( pDevice, "DlgEdObj::TransformSdrToControlCoordinates: missing default device!" );
    if ( !pDevice )
        return false;
    aPos = pDevice->LogicToPixel( aPos, MapMode( MapUnit::Map100thMM ) );
    aSize = pDevice->LogicToPixel( aSize, MapMode( MapUnit::Map100thMM ) );
    aFormPos = pDevice->LogicToPixel( aFormPos, MapMode( MapUnit::Map100thMM ) );

    // subtract form position
    aPos.AdjustWidth( -(aFormPos.Width()) );
    aPos.AdjustHeight( -(aFormPos.Height()) );

    // take window borders into account
    Reference< beans::XPropertySet > xPSetForm( pForm->GetUnoControlModel(), UNO_QUERY );
    DBG_ASSERT( xPSetForm.is(), "DlgEdObj::TransformFormToSdrCoordinates: no form property set!" );
    if ( !xPSetForm.is() )
        return false;
    bool bDecoration = true;
    xPSetForm->getPropertyValue( DLGED_PROP_DECORATION ) >>= bDecoration;
    if( bDecoration )
    {
        awt::DeviceInfo aDeviceInfo = pForm->getDeviceInfo();
        aPos.AdjustWidth( -(aDeviceInfo.LeftInset) );
        aPos.AdjustHeight( -(aDeviceInfo.TopInset) );
    }

    // convert pixel to logic units
    aPos = pDevice->PixelToLogic(aPos, MapMode(MapUnit::MapAppFont));
    aSize = pDevice->PixelToLogic(aSize, MapMode(MapUnit::MapAppFont));

    // set out parameters
    nXOut = aPos.Width();
    nYOut = aPos.Height();
    nWidthOut = aSize.Width();
    nHeightOut = aSize.Height();

    return true;
}

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
    aPos = pDevice->LogicToPixel( aPos, MapMode( MapUnit::Map100thMM ) );
    aSize = pDevice->LogicToPixel( aSize, MapMode( MapUnit::Map100thMM ) );

    // take window borders into account
    DlgEdForm* pForm = nullptr;
    if ( !lcl_getDlgEdForm( this, pForm ) )
        return false;

    // take window borders into account
    Reference< beans::XPropertySet > xPSetForm( pForm->GetUnoControlModel(), UNO_QUERY );
    DBG_ASSERT( xPSetForm.is(), "DlgEdObj::TransformFormToSdrCoordinates: no form property set!" );
    if ( !xPSetForm.is() )
        return false;
    bool bDecoration = true;
    xPSetForm->getPropertyValue( DLGED_PROP_DECORATION ) >>= bDecoration;
    if( bDecoration )
    {
        awt::DeviceInfo aDeviceInfo = pForm->getDeviceInfo();
        aSize.AdjustWidth( -(aDeviceInfo.LeftInset + aDeviceInfo.RightInset) );
        aSize.AdjustHeight( -(aDeviceInfo.TopInset + aDeviceInfo.BottomInset) );
    }
    // convert pixel to logic units
    aPos = pDevice->PixelToLogic(aPos, MapMode(MapUnit::MapAppFont));
    aSize = pDevice->PixelToLogic(aSize, MapMode(MapUnit::MapAppFont));

    // set out parameters
    nXOut = aPos.Width();
    nYOut = aPos.Height();
    nWidthOut = aSize.Width();
    nHeightOut = aSize.Height();

    return true;
}

bool DlgEdObj::TransformControlToSdrCoordinates(
    sal_Int32 nXIn, sal_Int32 nYIn, sal_Int32 nWidthIn, sal_Int32 nHeightIn,
    sal_Int32& nXOut, sal_Int32& nYOut, sal_Int32& nWidthOut, sal_Int32& nHeightOut )
{
    // input position and size
    Size aPos( nXIn, nYIn );
    Size aSize( nWidthIn, nHeightIn );

    // form position
    DlgEdForm* pForm = nullptr;
    if ( !lcl_getDlgEdForm( this, pForm ) )
        return false;

    Reference< beans::XPropertySet > xPSetForm( pForm->GetUnoControlModel(), UNO_QUERY );
    DBG_ASSERT( xPSetForm.is(), "DlgEdObj::TransformControlToSdrCoordinates: no form property set!" );
    if ( !xPSetForm.is() )
        return false;
    sal_Int32 nFormX = 0, nFormY = 0;
    xPSetForm->getPropertyValue( DLGED_PROP_POSITIONX ) >>= nFormX;
    xPSetForm->getPropertyValue( DLGED_PROP_POSITIONY ) >>= nFormY;
    Size aFormPos( nFormX, nFormY );

    // convert logic units to pixel
    OutputDevice* pDevice = Application::GetDefaultDevice();
    DBG_ASSERT( pDevice, "DlgEdObj::TransformControlToSdrCoordinates: missing default device!" );
    if ( !pDevice )
        return false;
    aPos = pDevice->LogicToPixel(aPos, MapMode(MapUnit::MapAppFont));
    aSize = pDevice->LogicToPixel(aSize, MapMode(MapUnit::MapAppFont));
    aFormPos = pDevice->LogicToPixel(aFormPos, MapMode(MapUnit::MapAppFont));

    // add form position
    aPos.AdjustWidth(aFormPos.Width() );
    aPos.AdjustHeight(aFormPos.Height() );

    // take window borders into account
    bool bDecoration = true;
    xPSetForm->getPropertyValue( DLGED_PROP_DECORATION ) >>= bDecoration;
    if( bDecoration )
    {
        awt::DeviceInfo aDeviceInfo = pForm->getDeviceInfo();
        aPos.AdjustWidth(aDeviceInfo.LeftInset );
        aPos.AdjustHeight(aDeviceInfo.TopInset );
    }

    // convert pixel to 100th_mm
    aPos = pDevice->PixelToLogic( aPos, MapMode( MapUnit::Map100thMM ) );
    aSize = pDevice->PixelToLogic( aSize, MapMode( MapUnit::Map100thMM ) );

    // set out parameters
    nXOut = aPos.Width();
    nYOut = aPos.Height();
    nWidthOut = aSize.Width();
    nHeightOut = aSize.Height();

    return true;
}

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

    // take window borders into account
    DlgEdForm* pForm = nullptr;
    if ( !lcl_getDlgEdForm( this, pForm ) )
        return false;

    aPos = pDevice->LogicToPixel(aPos, MapMode(MapUnit::MapAppFont));
    aSize = pDevice->LogicToPixel(aSize, MapMode(MapUnit::MapAppFont));

    // take window borders into account
    Reference< beans::XPropertySet > xPSetForm( pForm->GetUnoControlModel(), UNO_QUERY );
    DBG_ASSERT( xPSetForm.is(), "DlgEdObj::TransformFormToSdrCoordinates: no form property set!" );
    if ( !xPSetForm.is() )
        return false;
    bool bDecoration = true;
    xPSetForm->getPropertyValue( DLGED_PROP_DECORATION ) >>= bDecoration;
    if( bDecoration )
    {
        awt::DeviceInfo aDeviceInfo = pForm->getDeviceInfo();
        aSize.AdjustWidth(aDeviceInfo.LeftInset + aDeviceInfo.RightInset );
        aSize.AdjustHeight(aDeviceInfo.TopInset + aDeviceInfo.BottomInset );
    }

    // convert pixel to 100th_mm
    aPos = pDevice->PixelToLogic( aPos, MapMode( MapUnit::Map100thMM ) );
    aSize = pDevice->PixelToLogic( aSize, MapMode( MapUnit::Map100thMM ) );

    // set out parameters
    nXOut = aPos.Width();
    nYOut = aPos.Height();
    nWidthOut = aSize.Width();
    nHeightOut = aSize.Height();

    return true;
}

void DlgEdObj::SetRectFromProps()
{
    // get control position and size from properties
    Reference< beans::XPropertySet > xPSet( GetUnoControlModel(), UNO_QUERY );
    if ( xPSet.is() )
    {
        sal_Int32 nXIn = 0, nYIn = 0, nWidthIn = 0, nHeightIn = 0;
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
            SetSnapRect( tools::Rectangle( aPoint, aSize ) );
        }
    }
}

void DlgEdObj::SetPropsFromRect()
{
    // get control position and size from rectangle
    tools::Rectangle aRect_ = GetSnapRect();
    sal_Int32 nXIn = aRect_.Left();
    sal_Int32 nYIn = aRect_.Top();
    sal_Int32 nWidthIn = aRect_.GetWidth();
    sal_Int32 nHeightIn = aRect_.GetHeight();

    // transform coordinates
    sal_Int32 nXOut, nYOut, nWidthOut, nHeightOut;
    if ( TransformSdrToControlCoordinates( nXIn, nYIn, nWidthIn, nHeightIn, nXOut, nYOut, nWidthOut, nHeightOut ) )
    {
        // set properties
        Reference< beans::XPropertySet > xPSet( GetUnoControlModel(), UNO_QUERY );
        if ( xPSet.is() )
        {
            xPSet->setPropertyValue( DLGED_PROP_POSITIONX, Any(nXOut) );
            xPSet->setPropertyValue( DLGED_PROP_POSITIONY, Any(nYOut) );
            xPSet->setPropertyValue( DLGED_PROP_WIDTH, Any(nWidthOut) );
            xPSet->setPropertyValue( DLGED_PROP_HEIGHT, Any(nHeightOut) );
        }
    }
}

void DlgEdObj::PositionAndSizeChange( const beans::PropertyChangeEvent& evt )
{
    DBG_ASSERT( pDlgEdForm, "DlgEdObj::PositionAndSizeChange: no form!" );
    DlgEdPage& rPage = pDlgEdForm->GetDlgEditor().GetPage();
    {
        Size aPageSize = rPage.GetSize();
        sal_Int32 nPageWidthIn = aPageSize.Width();
        sal_Int32 nPageHeightIn = aPageSize.Height();
        sal_Int32 nPageX, nPageY, nPageWidth, nPageHeight;
        if ( TransformSdrToControlCoordinates( 0/*nPageXIn*/, 0/*nPageYIn*/, nPageWidthIn, nPageHeightIn, nPageX, nPageY, nPageWidth, nPageHeight ) )
        {
            Reference< beans::XPropertySet > xPSet( GetUnoControlModel(), UNO_QUERY );
            if ( xPSet.is() )
            {
                sal_Int32 nX = 0, nY = 0, nWidth = 0, nHeight = 0;
                xPSet->getPropertyValue( DLGED_PROP_POSITIONX ) >>= nX;
                xPSet->getPropertyValue( DLGED_PROP_POSITIONY ) >>= nY;
                xPSet->getPropertyValue( DLGED_PROP_WIDTH ) >>= nWidth;
                xPSet->getPropertyValue( DLGED_PROP_HEIGHT ) >>= nHeight;

                sal_Int32 nValue = 0;
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
                    EndListening( false );
                    xPSet->setPropertyValue( evt.PropertyName, Any(nNewValue) );
                    StartListening();
                }
            }
        }
    }

    SetRectFromProps();
}

void DlgEdObj::NameChange( const  css::beans::PropertyChangeEvent& evt )
{
    // get old name
    OUString aOldName;
    evt.OldValue >>= aOldName;

    // get new name
    OUString aNewName;
    evt.NewValue >>= aNewName;

    if ( aNewName != aOldName )
    {
        Reference< container::XNameAccess > xNameAcc((GetDlgEdForm()->GetUnoControlModel()), UNO_QUERY);
        if ( xNameAcc.is() && xNameAcc->hasByName(aOldName) )
        {
            if (!xNameAcc->hasByName(aNewName) && !aNewName.isEmpty())
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

                    LocalizationMgr::renameControlResourceIDsForEditorObject(
                        &GetDialogEditor(), aAny, aNewName
                    );
                }
            }
            else
            {
                // set old name property
                EndListening(false);
                Reference< beans::XPropertySet >  xPSet(GetUnoControlModel(), UNO_QUERY);
                xPSet->setPropertyValue( DLGED_PROP_NAME, Any(aOldName) );
                StartListening();
            }
        }
    }
}

sal_Int32 DlgEdObj::GetStep() const
{
    // get step property
    sal_Int32 nStep = 0;
    uno::Reference< beans::XPropertySet >  xPSet( GetUnoControlModel(), uno::UNO_QUERY );
    if (xPSet.is())
    {
        xPSet->getPropertyValue( DLGED_PROP_STEP ) >>= nStep;
    }
    return nStep;
}

void DlgEdObj::UpdateStep()
{
    sal_Int32 nCurStep = GetDlgEdForm()->GetStep();
    sal_Int32 nStep = GetStep();

    SdrLayerAdmin& rLayerAdmin(getSdrModelFromSdrObject().GetLayerAdmin());
    SdrLayerID nHiddenLayerId   = rLayerAdmin.GetLayerID( "HiddenLayer" );
    SdrLayerID nControlLayerId   = rLayerAdmin.GetLayerID( rLayerAdmin.GetControlLayerName() );

    if( nCurStep )
    {
        if ( nStep && (nStep != nCurStep) )
        {
            SetLayer( nHiddenLayerId );
        }
        else
        {
            SetLayer( nControlLayerId );
        }
    }
    else
    {
        SetLayer( nControlLayerId );
    }
}

void DlgEdObj::TabIndexChange( const beans::PropertyChangeEvent& evt )
{
    DlgEdForm* pForm = GetDlgEdForm();
    if ( pForm )
    {
        // stop listening with all children
        std::vector<DlgEdObj*> aChildList = pForm->GetChildren();
        for (auto const& child : aChildList)
        {
            child->EndListening( false );
        }

        Reference< container::XNameAccess > xNameAcc( pForm->GetUnoControlModel() , UNO_QUERY );
        if ( xNameAcc.is() )
        {
            // get sequence of control names
            Sequence< OUString > aNames = xNameAcc->getElementNames();
            const OUString* pNames = aNames.getConstArray();
            sal_Int32 nCtrls = aNames.getLength();

            // create a map of tab indices and control names, sorted by tab index
            IndexToNameMap aIndexToNameMap;
            for ( sal_Int32 i = 0; i < nCtrls; ++i )
            {
                // get control name
                OUString aName( pNames[i] );

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
                aIndexToNameMap.emplace( nTabIndex, aName );
            }

            // create a helper list of control names, sorted by tab index
            std::vector< OUString > aNameList( aIndexToNameMap.size() );
            std::transform(
                    aIndexToNameMap.begin(), aIndexToNameMap.end(),
                    aNameList.begin(),
                    ::o3tl::select2nd< IndexToNameMap::value_type >( )
                );

            // check tab index
            sal_Int16 nOldTabIndex = 0;
            evt.OldValue >>= nOldTabIndex;
            sal_Int16 nNewTabIndex = 0;
            evt.NewValue >>= nNewTabIndex;
            if ( nNewTabIndex < 0 )
                nNewTabIndex = 0;
            else if ( nNewTabIndex > nCtrls - 1 )
                nNewTabIndex = sal::static_int_cast<sal_Int16>( nCtrls - 1 );

            // reorder helper list
            OUString aCtrlName = aNameList[nOldTabIndex];
            aNameList.erase( aNameList.begin() + nOldTabIndex );
            aNameList.insert( aNameList.begin() + nNewTabIndex , aCtrlName );

            // set new tab indices
            for ( sal_Int32 i = 0; i < nCtrls; ++i )
            {
                Any aCtrl = xNameAcc->getByName( aNameList[i] );
                Reference< beans::XPropertySet > xPSet;
                aCtrl >>= xPSet;
                if ( xPSet.is() )
                {
                    assert(i >= SAL_MIN_INT16);
                    if (i > SAL_MAX_INT16)
                    {
                        SAL_WARN("basctl", "tab " << i << " > SAL_MAX_INT16");
                        continue;
                    }
                    xPSet->setPropertyValue( DLGED_PROP_TABINDEX, Any(static_cast<sal_Int16>(i)) );
                }
            }

            // reorder objects in drawing page
            getSdrModelFromSdrObject().GetPage(0)->SetObjectOrdNum( nOldTabIndex + 1, nNewTabIndex + 1 );

            pForm->UpdateTabOrderAndGroups();
        }

        // start listening with all children
        for (auto const& child : aChildList)
        {
            child->StartListening();
        }
    }
}

bool DlgEdObj::supportsService( OUString const & serviceName ) const
{
    bool bSupports = false;

    Reference< lang::XServiceInfo > xServiceInfo( GetUnoControlModel() , UNO_QUERY );
        // TODO: cache xServiceInfo as member?
    if ( xServiceInfo.is() )
        bSupports = xServiceInfo->supportsService( serviceName );

    return bSupports;
}

OUString DlgEdObj::GetDefaultName() const
{
    OUString sResId;
    OUString aDefaultName;
    if ( supportsService( "com.sun.star.awt.UnoControlDialogModel" ) )
    {
        sResId = RID_STR_CLASS_DIALOG;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlButtonModel" ) )
    {
        sResId = RID_STR_CLASS_BUTTON;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlRadioButtonModel" ) )
    {
        sResId = RID_STR_CLASS_RADIOBUTTON;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlCheckBoxModel" ) )
    {
        sResId = RID_STR_CLASS_CHECKBOX;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlListBoxModel" ) )
    {
        sResId = RID_STR_CLASS_LISTBOX;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlComboBoxModel" ) )
    {
        sResId = RID_STR_CLASS_COMBOBOX;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlGroupBoxModel" ) )
    {
        sResId = RID_STR_CLASS_GROUPBOX;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlEditModel" ) )
    {
        sResId = RID_STR_CLASS_EDIT;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlFixedTextModel" ) )
    {
        sResId = RID_STR_CLASS_FIXEDTEXT;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlImageControlModel" ) )
    {
        sResId = RID_STR_CLASS_IMAGECONTROL;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlProgressBarModel" ) )
    {
        sResId = RID_STR_CLASS_PROGRESSBAR;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlScrollBarModel" ) )
    {
        sResId = RID_STR_CLASS_SCROLLBAR;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlFixedLineModel" ) )
    {
        sResId = RID_STR_CLASS_FIXEDLINE;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlDateFieldModel" ) )
    {
        sResId = RID_STR_CLASS_DATEFIELD;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlTimeFieldModel" ) )
    {
        sResId = RID_STR_CLASS_TIMEFIELD;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlNumericFieldModel" ) )
    {
        sResId = RID_STR_CLASS_NUMERICFIELD;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlCurrencyFieldModel" ) )
    {
        sResId = RID_STR_CLASS_CURRENCYFIELD;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlFormattedFieldModel" ) )
    {
        sResId = RID_STR_CLASS_FORMATTEDFIELD;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlPatternFieldModel" ) )
    {
        sResId = RID_STR_CLASS_PATTERNFIELD;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlFileControlModel" ) )
    {
        sResId = RID_STR_CLASS_FILECONTROL;
    }
    else if ( supportsService( "com.sun.star.awt.tree.TreeControlModel" ) )
    {
        sResId = RID_STR_CLASS_TREECONTROL;
    }
    else if ( supportsService( "com.sun.star.awt.grid.UnoControlGridModel" ) )
    {
        sResId = RID_STR_CLASS_GRIDCONTROL;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlFixedHyperlinkModel" ) )
    {
        sResId = RID_STR_CLASS_HYPERLINKCONTROL;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlSpinButtonModel" ) )
    {
        sResId = RID_STR_CLASS_SPINCONTROL;
    }
    else
    {
        sResId = RID_STR_CLASS_CONTROL;
    }

    if (!sResId.isEmpty())
        aDefaultName = sResId;

    return aDefaultName;
}

OUString DlgEdObj::GetUniqueName() const
{
    OUString aUniqueName;
    uno::Reference< container::XNameAccess > xNameAcc((GetDlgEdForm()->GetUnoControlModel()), uno::UNO_QUERY);

    if ( xNameAcc.is() )
    {
        sal_Int32 n = 0;
        OUString aDefaultName = GetDefaultName();

        do
        {
            aUniqueName = aDefaultName + OUString::number(++n);
        }   while (xNameAcc->hasByName(aUniqueName));
    }

    return aUniqueName;
}

SdrInventor DlgEdObj::GetObjInventor()   const
{
    return SdrInventor::BasicDialog;
}

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
    else if ( supportsService( "com.sun.star.awt.tree.TreeControlModel" ))
    {
        return OBJ_DLG_TREECONTROL;
    }
    else if ( supportsService( "com.sun.star.awt.grid.UnoControlGridModel" ))
    {
        return OBJ_DLG_GRIDCONTROL;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlFixedHyperlinkModel" ))
    {
        return OBJ_DLG_HYPERLINKCONTROL;
    }
    else
    {
        return OBJ_DLG_CONTROL;
    }
}

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
        OUString aOUniqueName( GetUniqueName() );
        Any aUniqueName;
        aUniqueName <<= aOUniqueName;
        xPSet->setPropertyValue( DLGED_PROP_NAME, aUniqueName );

        Reference< container::XNameContainer > xCont( GetDlgEdForm()->GetUnoControlModel() , UNO_QUERY );
        if ( xCont.is() )
        {
            // set tabindex
            Sequence< OUString > aNames = xCont->getElementNames();
            xPSet->setPropertyValue( DLGED_PROP_TABINDEX, Any(static_cast<sal_Int16>(aNames.getLength())) );

            // insert control model in dialog model
            Reference< awt::XControlModel > xCtrl( xPSet , UNO_QUERY );
            xCont->insertByName( aOUniqueName, Any(xCtrl) );

            pDlgEdForm->UpdateTabOrderAndGroups();
        }
    }

    // start listening
    StartListening();
}

DlgEdObj* DlgEdObj::CloneSdrObject(SdrModel& rTargetModel) const
{
    DlgEdObj* pDlgEdObj = CloneHelper< DlgEdObj >(rTargetModel);
    DBG_ASSERT( pDlgEdObj != nullptr, "DlgEdObj::Clone: invalid clone!" );
    if ( pDlgEdObj )
        pDlgEdObj->clonedFrom( this );

    return pDlgEdObj;
}

SdrObject* DlgEdObj::getFullDragClone() const
{
    // no need to really add the clone for dragging, it's a temporary
    // object
    SdrObject* pObj = new SdrUnoObj(
        getSdrModelFromSdrObject(),
        OUString());
    *pObj = *static_cast<const SdrUnoObj*>(this);

    return pObj;
}

void DlgEdObj::NbcMove( const Size& rSize )
{
    SdrUnoObj::NbcMove( rSize );

    // stop listening
    EndListening(false);

    // set geometry properties
    SetPropsFromRect();

    // start listening
    StartListening();

    // dialog model changed
    GetDlgEdForm()->GetDlgEditor().SetDialogModelChanged();
}

void DlgEdObj::NbcResize(const Point& rRef, const Fraction& xFract, const Fraction& yFract)
{
    SdrUnoObj::NbcResize( rRef, xFract, yFract );

    // stop listening
    EndListening(false);

    // set geometry properties
    SetPropsFromRect();

    // start listening
    StartListening();

    // dialog model changed
    GetDlgEdForm()->GetDlgEditor().SetDialogModelChanged();
}

bool DlgEdObj::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    bool bResult = SdrUnoObj::EndCreate(rStat, eCmd);

    // tdf#120674 after interactive creation, the SdrObject (this) has no SdrPage yet
    // due to not being inserted. Usually this should be handled in a ::handlePageChange
    // implementation. For historical reasons, the SdrPage (which is the DlgEdPage) was
    // already set. For now, get it from the SdrDragStat and use it to access and set
    // the local pDlgEdForm
    if(nullptr == pDlgEdForm && nullptr != rStat.GetPageView())
    {
        const DlgEdPage* pDlgEdPage(dynamic_cast<const DlgEdPage*>(rStat.GetPageView()->GetPage()));

        if(nullptr != pDlgEdPage)
        {
            // set parent form
            pDlgEdForm = pDlgEdPage->GetDlgEdForm();
        }
    }

    SetDefaults();
    StartListening();

    return bResult;
}

void DlgEdObj::SetDefaults()
{
    if ( pDlgEdForm )
    {
        // add child to parent form
        pDlgEdForm->AddChild( this );

        Reference< beans::XPropertySet > xPSet( GetUnoControlModel(), UNO_QUERY );
        if ( xPSet.is() )
        {
            // get unique name
            OUString aOUniqueName( GetUniqueName() );

            // set name property
            xPSet->setPropertyValue( DLGED_PROP_NAME, Any(aOUniqueName) );

            // set labels
            if ( supportsService( "com.sun.star.awt.UnoControlButtonModel" ) ||
                supportsService( "com.sun.star.awt.UnoControlRadioButtonModel" ) ||
                supportsService( "com.sun.star.awt.UnoControlCheckBoxModel" ) ||
                supportsService( "com.sun.star.awt.UnoControlGroupBoxModel" ) ||
                supportsService( "com.sun.star.awt.UnoControlFixedTextModel" ) )
            {
                xPSet->setPropertyValue( DLGED_PROP_LABEL, Any(aOUniqueName) );
            }

            // set number formats supplier for formatted field
            if ( supportsService( "com.sun.star.awt.UnoControlFormattedFieldModel" ) )
            {
                Reference< util::XNumberFormatsSupplier > xSupplier = GetDlgEdForm()->GetDlgEditor().GetNumberFormatsSupplier();
                if ( xSupplier.is() )
                {
                    xPSet->setPropertyValue( DLGED_PROP_FORMATSSUPPLIER, Any(xSupplier) );
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
                aTabIndex <<= static_cast<sal_Int16>(aNames.getLength());
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

                LocalizationMgr::setControlResourceIDsForNewEditorObject(
                    &GetDialogEditor(), aAny, aOUniqueName
                );

                pDlgEdForm->UpdateTabOrderAndGroups();
            }
        }

        // dialog model changed
        pDlgEdForm->GetDlgEditor().SetDialogModelChanged();
    }
}

void DlgEdObj::StartListening()
{
    DBG_ASSERT(!isListening(), "DlgEdObj::StartListening: already listening!");

    if (!isListening())
    {
        bIsListening = true;

        // XPropertyChangeListener
        Reference< XPropertySet > xControlModel( GetUnoControlModel() , UNO_QUERY );
        if (!m_xPropertyChangeListener.is() && xControlModel.is())
        {
            // create listener
            m_xPropertyChangeListener = new DlgEdPropListenerImpl(*this);

            // register listener to properties
            xControlModel->addPropertyChangeListener( OUString() , m_xPropertyChangeListener );
        }

        // XContainerListener
        Reference< XScriptEventsSupplier > xEventsSupplier( GetUnoControlModel() , UNO_QUERY );
        if( !m_xContainerListener.is() && xEventsSupplier.is() )
        {
            // create listener
            m_xContainerListener = new DlgEdEvtContListenerImpl(*this);

            // register listener to script event container
            Reference< XNameContainer > xEventCont = xEventsSupplier->getEvents();
            DBG_ASSERT(xEventCont.is(), "DlgEdObj::StartListening: control model has no script event container!");
            Reference< XContainer > xCont( xEventCont , UNO_QUERY );
            if (xCont.is())
                xCont->addContainerListener( m_xContainerListener );
        }
    }
}

void DlgEdObj::EndListening(bool bRemoveListener)
{
    DBG_ASSERT(isListening(), "DlgEdObj::EndListening: not listening currently!");

    if (isListening())
    {
        bIsListening = false;

        if (bRemoveListener)
        {
            // XPropertyChangeListener
            Reference< XPropertySet > xControlModel(GetUnoControlModel(), UNO_QUERY);
            if ( m_xPropertyChangeListener.is() && xControlModel.is() )
            {
                // remove listener
                xControlModel->removePropertyChangeListener( OUString() , m_xPropertyChangeListener );
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

void DlgEdObj::_propertyChange( const  css::beans::PropertyChangeEvent& evt )
{
    if (isListening())
    {
        DlgEdForm* pRealDlgEdForm = dynamic_cast<DlgEdForm*>(this);
        if (!pRealDlgEdForm)
            pRealDlgEdForm = GetDlgEdForm();
        if (!pRealDlgEdForm)
            return;
        DlgEditor& rDlgEditor = pRealDlgEdForm->GetDlgEditor();
        if (rDlgEditor.isInPaint())
            return;

        // dialog model changed
        rDlgEditor.SetDialogModelChanged();

        // update position and size
        if ( evt.PropertyName == DLGED_PROP_POSITIONX || evt.PropertyName == DLGED_PROP_POSITIONY ||
             evt.PropertyName == DLGED_PROP_WIDTH || evt.PropertyName == DLGED_PROP_HEIGHT ||
             evt.PropertyName == DLGED_PROP_DECORATION )
        {
            PositionAndSizeChange( evt );

            if ( evt.PropertyName == DLGED_PROP_DECORATION )
                GetDialogEditor().ResetDialog();
        }
        // change name of control in dialog model
        else if ( evt.PropertyName == DLGED_PROP_NAME )
        {
            if (!dynamic_cast<DlgEdForm*>(this))
            {
                try
                {
                    NameChange(evt);
                }
                catch (container::NoSuchElementException const&)
                {
                    css::uno::Any anyEx = cppu::getCaughtException();
                    throw lang::WrappedTargetRuntimeException("", nullptr,
                            anyEx);
                }
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
            if (!dynamic_cast<DlgEdForm*>(this))
                TabIndexChange(evt);
        }
    }
}

void DlgEdObj::_elementInserted()
{
    if (isListening())
    {
        // dialog model changed
        GetDialogEditor().SetDialogModelChanged();
    }
}

void DlgEdObj::_elementReplaced()
{
    if (isListening())
    {
        // dialog model changed
        GetDialogEditor().SetDialogModelChanged();
    }
}

void DlgEdObj::_elementRemoved()
{
    if (isListening())
    {
        // dialog model changed
        GetDialogEditor().SetDialogModelChanged();
    }
}

void DlgEdObj::SetLayer(SdrLayerID nLayer)
{
    SdrLayerID nOldLayer = GetLayer();

    if ( nLayer != nOldLayer )
    {
        SdrUnoObj::SetLayer( nLayer );

        DlgEdHint aHint( DlgEdHint::LAYERCHANGED, this );
        GetDlgEdForm()->GetDlgEditor().Broadcast( aHint );
    }
}

DlgEdForm::DlgEdForm(
    SdrModel& rSdrModel,
    DlgEditor& rDlgEditor_)
:   DlgEdObj(rSdrModel),
    rDlgEditor(rDlgEditor_)
{
}

DlgEdForm::~DlgEdForm()
{
}

void DlgEdForm::SetRectFromProps()
{
    // get form position and size from properties
    Reference< beans::XPropertySet > xPSet( GetUnoControlModel(), UNO_QUERY );
    if ( xPSet.is() )
    {
        sal_Int32 nXIn = 0, nYIn = 0, nWidthIn = 0, nHeightIn = 0;
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
            SetSnapRect( tools::Rectangle( aPoint, aSize ) );
        }
    }
}

void DlgEdForm::SetPropsFromRect()
{
    // get form position and size from rectangle
    tools::Rectangle aRect_ = GetSnapRect();
    sal_Int32 nXIn = aRect_.Left();
    sal_Int32 nYIn = aRect_.Top();
    sal_Int32 nWidthIn = aRect_.GetWidth();
    sal_Int32 nHeightIn = aRect_.GetHeight();

    // transform coordinates
    sal_Int32 nXOut, nYOut, nWidthOut, nHeightOut;
    if ( TransformSdrToFormCoordinates( nXIn, nYIn, nWidthIn, nHeightIn, nXOut, nYOut, nWidthOut, nHeightOut ) )
    {
        // set properties
        Reference< beans::XPropertySet > xPSet( GetUnoControlModel(), UNO_QUERY );
        if ( xPSet.is() )
        {
            xPSet->setPropertyValue( DLGED_PROP_POSITIONX, Any(nXOut) );
            xPSet->setPropertyValue( DLGED_PROP_POSITIONY, Any(nYOut) );
            xPSet->setPropertyValue( DLGED_PROP_WIDTH, Any(nWidthOut) );
            xPSet->setPropertyValue( DLGED_PROP_HEIGHT, Any(nHeightOut) );
        }
    }
}

void DlgEdForm::AddChild( DlgEdObj* pDlgEdObj )
{
    pChildren.push_back( pDlgEdObj );
}

void DlgEdForm::RemoveChild( DlgEdObj* pDlgEdObj )
{
    pChildren.erase( std::remove( pChildren.begin() , pChildren.end() , pDlgEdObj ) );
}

void DlgEdForm::PositionAndSizeChange( const beans::PropertyChangeEvent& evt )
{
    DlgEditor& rEditor = GetDlgEditor();
    DlgEdPage& rPage = rEditor.GetPage();

    sal_Int32 nPageXIn = 0;
    sal_Int32 nPageYIn = 0;
    Size aPageSize = rPage.GetSize();
    sal_Int32 nPageWidthIn = aPageSize.Width();
    sal_Int32 nPageHeightIn = aPageSize.Height();
    sal_Int32 nPageX, nPageY, nPageWidth, nPageHeight;
    if ( TransformSdrToFormCoordinates( nPageXIn, nPageYIn, nPageWidthIn, nPageHeightIn, nPageX, nPageY, nPageWidth, nPageHeight ) )
    {
        Reference< beans::XPropertySet > xPSetForm( GetUnoControlModel(), UNO_QUERY );
        if ( xPSetForm.is() )
        {
            sal_Int32 nValue = 0;
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
                EndListening( false );
                xPSetForm->setPropertyValue( evt.PropertyName, Any(nNewValue) );
                StartListening();
            }
        }
    }

    bool bAdjustedPageSize = rEditor.AdjustPageSize();
    SetRectFromProps();
    std::vector<DlgEdObj*> const& aChildList = GetChildren();

    if ( bAdjustedPageSize )
    {
        rEditor.InitScrollBars();
        aPageSize = rPage.GetSize();
        nPageWidthIn = aPageSize.Width();
        nPageHeightIn = aPageSize.Height();
        if ( TransformSdrToControlCoordinates( nPageXIn, nPageYIn, nPageWidthIn, nPageHeightIn, nPageX, nPageY, nPageWidth, nPageHeight ) )
        {
            for (auto const& child : aChildList)
            {
                Reference< beans::XPropertySet > xPSet( child->GetUnoControlModel(), UNO_QUERY );
                if ( xPSet.is() )
                {
                    sal_Int32 nX = 0, nY = 0, nWidth = 0, nHeight = 0;
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
                        EndListening( false );
                        xPSet->setPropertyValue( DLGED_PROP_POSITIONX, Any(nNewX) );
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
                        EndListening( false );
                        xPSet->setPropertyValue( DLGED_PROP_POSITIONY, Any(nNewY) );
                        StartListening();
                    }
                }
            }
        }
    }

    for (auto const& child : aChildList)
        child->SetRectFromProps();
}

void DlgEdForm::UpdateStep()
{
    SdrPage* pSdrPage = getSdrPageFromSdrObject();

    if ( pSdrPage )
    {
        const size_t nObjCount = pSdrPage->GetObjCount();
        for ( size_t i = 0 ; i < nObjCount ; i++ )
        {
            DlgEdObj* pDlgEdObj = dynamic_cast<DlgEdObj*>(pSdrPage->GetObj(i));
            if (pDlgEdObj && !dynamic_cast<DlgEdForm*>(pDlgEdObj))
                pDlgEdObj->UpdateStep();
        }
    }
}

void DlgEdForm::UpdateTabIndices()
{
    // stop listening with all children
    for (auto const& child : pChildren)
    {
        child->EndListening( false );
    }

    Reference< css::container::XNameAccess > xNameAcc( GetUnoControlModel() , UNO_QUERY );
    if ( xNameAcc.is() )
    {
        // get sequence of control names
        Sequence< OUString > aNames = xNameAcc->getElementNames();
        const OUString* pNames = aNames.getConstArray();
        sal_Int32 nCtrls = aNames.getLength();

        // create a map of tab indices and control names, sorted by tab index
        IndexToNameMap aIndexToNameMap;
        for ( sal_Int32 i = 0; i < nCtrls; ++i )
        {
            // get name
            OUString aName( pNames[i] );

            // get tab index
            sal_Int16 nTabIndex = -1;
            Any aCtrl = xNameAcc->getByName( aName );
            Reference< css::beans::XPropertySet > xPSet;
            aCtrl >>= xPSet;
            if ( xPSet.is() )
                xPSet->getPropertyValue( DLGED_PROP_TABINDEX ) >>= nTabIndex;

            // insert into map
            aIndexToNameMap.emplace( nTabIndex, aName );
        }

        // set new tab indices
        sal_Int16 nNewTabIndex = 0;
        for (auto const& indexToName : aIndexToNameMap)
        {
            Any aCtrl = xNameAcc->getByName( indexToName.second );
            Reference< beans::XPropertySet > xPSet;
            aCtrl >>= xPSet;
            if ( xPSet.is() )
            {
                xPSet->setPropertyValue( DLGED_PROP_TABINDEX, Any(nNewTabIndex) );
                nNewTabIndex++;
            }
        }

        UpdateTabOrderAndGroups();
    }

    // start listening with all children
    for (auto const& child : pChildren)
    {
        child->StartListening();
    }
}

void DlgEdForm::UpdateTabOrder()
{
    // When the tabindex of a control model changes, the dialog control is
    // notified about those changes. Due to #109067# (bad performance of
    // dialog editor) the dialog control doesn't activate the tab order
    // in design mode. When the dialog editor has reordered all
    // tabindices, this method allows to activate the taborder afterwards.

    Reference< awt::XUnoControlContainer > xCont( GetControl(), UNO_QUERY );
    if ( xCont.is() )
    {
        Sequence< Reference< awt::XTabController > > aSeqTabCtrls = xCont->getTabControllers();
        const Reference< awt::XTabController >* pTabCtrls = aSeqTabCtrls.getConstArray();
        sal_Int32 nCount = aSeqTabCtrls.getLength();
        for ( sal_Int32 i = 0; i < nCount; ++i )
            pTabCtrls[i]->activateTabOrder();
    }
}

void DlgEdForm::UpdateGroups()
{
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
        // create a global list of controls that belong to the dialog
        std::vector<DlgEdObj*> aChildList = GetChildren();
        sal_uInt32 nSize = aChildList.size();
        Sequence< Reference< awt::XControl > > aSeqControls( nSize );
        for ( sal_uInt32 i = 0; i < nSize; ++i )
            aSeqControls.getArray()[i].set( aChildList[i]->GetControl(), UNO_QUERY );

        sal_Int32 nGroupCount = xTabModel->getGroupCount();
        for ( sal_Int32 nGroup = 0; nGroup < nGroupCount; ++nGroup )
        {
            // get a list of control models that belong to this group
            OUString aName;
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
                        if ( xCtrlModel.get() == pModels[nModel].get() )
                        {
                            // get the control peer and insert into the list of peers
                            aSeqPeers.getArray()[ nModel ].set( xCtrl->getPeer(), UNO_QUERY );
                            break;
                        }
                    }
                }
            }

            // set the group at the dialog peer
            Reference< awt::XControl > xDlg( GetControl(), UNO_QUERY );
            if ( xDlg.is() )
            {
                Reference< awt::XVclContainerPeer > xDlgPeer( xDlg->getPeer(), UNO_QUERY );
                if ( xDlgPeer.is() )
                    xDlgPeer->setGroup( aSeqPeers );
            }
        }
    }
}

void DlgEdForm::UpdateTabOrderAndGroups()
{
    UpdateTabOrder();
    UpdateGroups();
}

void DlgEdForm::NbcMove( const Size& rSize )
{
    SdrUnoObj::NbcMove( rSize );

    // set geometry properties of form
    EndListening(false);
    SetPropsFromRect();
    StartListening();

    // set geometry properties of all children
    for (auto const& child : pChildren)
    {
        child->EndListening(false);
        child->SetPropsFromRect();
        child->StartListening();
    }

    // dialog model changed
    GetDlgEditor().SetDialogModelChanged();
}

void DlgEdForm::NbcResize(const Point& rRef, const Fraction& xFract, const Fraction& yFract)
{
    SdrUnoObj::NbcResize( rRef, xFract, yFract );

    // set geometry properties of form
    EndListening(false);
    SetPropsFromRect();
    StartListening();

    // set geometry properties of all children
    for (auto const& child : pChildren)
    {
        child->EndListening(false);
        child->SetPropsFromRect();
        child->StartListening();
    }

    // dialog model changed
    GetDlgEditor().SetDialogModelChanged();
}

bool DlgEdForm::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    bool bResult = SdrUnoObj::EndCreate(rStat, eCmd);

    // stop listening
    EndListening(false);

    // set geometry properties
    SetPropsFromRect();

    // dialog model changed
    GetDlgEditor().SetDialogModelChanged();

    // start listening
    StartListening();

    return bResult;
}

awt::DeviceInfo DlgEdForm::getDeviceInfo() const
{
    awt::DeviceInfo aDeviceInfo;

    DlgEditor& rEditor = GetDlgEditor();
    vcl::Window& rWindow = rEditor.GetWindow();

    // obtain an XControl
    ::utl::SharedUNOComponent< awt::XControl > xDialogControl; // ensures auto-disposal, if needed
    xDialogControl.reset( GetControl(), ::utl::SharedUNOComponent< awt::XControl >::NoTakeOwnership );
    if ( !xDialogControl.is() )
    {
        // don't create a temporary control all the time, this method here is called
        // way too often. Instead, use a cached DeviceInfo.
        // #i74065#
        if ( !!mpDeviceInfo )
            return *mpDeviceInfo;

        Reference< awt::XControlContainer > xEditorControlContainer( rEditor.GetWindowControlContainer() );
        xDialogControl.reset(
            GetTemporaryControlForWindow(rWindow, xEditorControlContainer),
            utl::SharedUNOComponent< awt::XControl >::TakeOwnership
        );
    }

    Reference< awt::XDevice > xDialogDevice;
    if ( xDialogControl.is() )
        xDialogDevice.set( xDialogControl->getPeer(), UNO_QUERY );
    DBG_ASSERT( xDialogDevice.is(), "DlgEdForm::getDeviceInfo: no device!" );
    if ( xDialogDevice.is() )
        aDeviceInfo = xDialogDevice->getInfo();

    mpDeviceInfo.reset( aDeviceInfo );

    return aDeviceInfo;
}
void DlgEdObj::MakeDataAware( const Reference< frame::XModel >& xModel )
{
    // Need to flesh this out, currently we will only support data-aware controls for calc
    // and only handle a subset of functionality e.g. linked-cell and cell range data source. Of course later
    // we need to disambiguate for writer ( and others ? ) and also support the generic form (db) bindings
    // we need some more work in xmlscript to be able to handle that
    Reference< lang::XMultiServiceFactory > xFac( xModel, UNO_QUERY );
    Reference< form::binding::XBindableValue > xBindable( GetUnoControlModel(), UNO_QUERY );
    Reference< form::binding::XListEntrySink  > xListEntrySink( GetUnoControlModel(), UNO_QUERY );
    if ( xFac.is() )
    {
        css::table::CellAddress aApiAddress;

        //tdf#90361 CellValueBinding and CellRangeListSource are unusable
        //without being initialized, so use createInstanceWithArguments with a
        //dummy BoundCell instead of createInstance. This at least results in
        //the dialog editor not falling.
        css::beans::NamedValue aValue;
        aValue.Name = "BoundCell";
        aValue.Value <<= aApiAddress;

        Sequence< Any > aArgs( 1 );
        aArgs[ 0 ] <<= aValue;

        if ( xBindable.is() )
        {
            Reference< form::binding::XValueBinding > xBinding( xFac->createInstanceWithArguments( "com.sun.star.table.CellValueBinding", aArgs ), UNO_QUERY );
            xBindable->setValueBinding( xBinding );
        }
        if ( xListEntrySink.is() )
        {
            Reference< form::binding::XListEntrySource > xSource( xFac->createInstanceWithArguments( "com.sun.star.table.CellRangeListSource", aArgs ), UNO_QUERY );
            xListEntrySink->setListEntrySource( xSource );
        }
    }
}
} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
