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

#include "dlged.hxx"
#include "dlgeddef.hxx"
#include "dlgedlist.hxx"
#include "dlgedobj.hxx"
#include "dlgedpage.hxx"
#include "dlgedview.hxx"
#include "iderid.hxx"
#include "localizationmgr.hxx"

#include "dlgresid.hrc"

#include <com/sun/star/form/binding/XBindableValue.hpp>
#include <com/sun/star/form/binding/XValueBinding.hpp>
#include <com/sun/star/form/binding/XListEntrySink.hpp>
#include <com/sun/star/awt/XUnoControlContainer.hpp>
#include <com/sun/star/awt/XVclContainerPeer.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/script/XScriptEventsSupplier.hpp>
#include <o3tl/compat_functional.hxx>
#include <unotools/sharedunocomponent.hxx>
#include <vcl/svapp.hxx>

namespace basctl
{

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::script;

TYPEINIT1(DlgEdObj, SdrUnoObj);
DBG_NAME(DlgEdObj);

DlgEditor& DlgEdObj::GetDialogEditor ()
{
    if (DlgEdForm* pFormThis = dynamic_cast<DlgEdForm*>(this))
        return pFormThis->GetDlgEditor();
    else
        return pDlgEdForm->GetDlgEditor();
}

DlgEdObj::DlgEdObj()
          :SdrUnoObj(OUString(), false)
          ,bIsListening(false)
          ,pDlgEdForm( NULL )
{
    DBG_CTOR(DlgEdObj, NULL);
}

DlgEdObj::DlgEdObj(const OUString& rModelName,
                   const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxSFac)
          :SdrUnoObj(rModelName, rxSFac, false)
          ,bIsListening(false)
          ,pDlgEdForm( NULL )
{
    DBG_CTOR(DlgEdObj, NULL);
}

DlgEdObj::~DlgEdObj()
{
    DBG_DTOR(DlgEdObj, NULL);

    if ( isListening() )
        EndListening();
}

void DlgEdObj::SetPage(SdrPage* _pNewPage)
{
    
    SdrUnoObj::SetPage(_pNewPage);
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
        return ( _out_pDlgEdForm != NULL );
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
    
    Size aPos( nXIn, nYIn );
    Size aSize( nWidthIn, nHeightIn );

    
    DlgEdForm* pForm = NULL;
    if ( !lcl_getDlgEdForm( this, pForm ) )
        return false;
    Rectangle aFormRect = pForm->GetSnapRect();
    Size aFormPos( aFormRect.Left(), aFormRect.Top() );

    
    OutputDevice* pDevice = Application::GetDefaultDevice();
    DBG_ASSERT( pDevice, "DlgEdObj::TransformSdrToControlCoordinates: missing default device!" );
    if ( !pDevice )
        return false;
    aPos = pDevice->LogicToPixel( aPos, MapMode( MAP_100TH_MM ) );
    aSize = pDevice->LogicToPixel( aSize, MapMode( MAP_100TH_MM ) );
    aFormPos = pDevice->LogicToPixel( aFormPos, MapMode( MAP_100TH_MM ) );

    
    aPos.Width() -= aFormPos.Width();
    aPos.Height() -= aFormPos.Height();

    
    Reference< beans::XPropertySet > xPSetForm( pForm->GetUnoControlModel(), UNO_QUERY );
    DBG_ASSERT( xPSetForm.is(), "DlgEdObj::TransformFormToSdrCoordinates: no form property set!" );
    if ( !xPSetForm.is() )
        return false;
    bool bDecoration = true;
    xPSetForm->getPropertyValue( DLGED_PROP_DECORATION ) >>= bDecoration;
    if( bDecoration )
    {
        awt::DeviceInfo aDeviceInfo = pForm->getDeviceInfo();
        aPos.Width() -= aDeviceInfo.LeftInset;
        aPos.Height() -= aDeviceInfo.TopInset;
    }

    
    aPos = pDevice->PixelToLogic( aPos, MAP_APPFONT );
    aSize = pDevice->PixelToLogic( aSize, MAP_APPFONT );

    
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
    
    Size aPos( nXIn, nYIn );
    Size aSize( nWidthIn, nHeightIn );

    
    OutputDevice* pDevice = Application::GetDefaultDevice();
    DBG_ASSERT( pDevice, "DlgEdObj::TransformSdrToFormCoordinates: missing default device!" );
    if ( !pDevice )
        return false;
    aPos = pDevice->LogicToPixel( aPos, MapMode( MAP_100TH_MM ) );
    aSize = pDevice->LogicToPixel( aSize, MapMode( MAP_100TH_MM ) );

    
    DlgEdForm* pForm = NULL;
    if ( !lcl_getDlgEdForm( this, pForm ) )
        return false;

    
    Reference< beans::XPropertySet > xPSetForm( pForm->GetUnoControlModel(), UNO_QUERY );
    DBG_ASSERT( xPSetForm.is(), "DlgEdObj::TransformFormToSdrCoordinates: no form property set!" );
    if ( !xPSetForm.is() )
        return false;
    bool bDecoration = true;
    xPSetForm->getPropertyValue( DLGED_PROP_DECORATION ) >>= bDecoration;
    if( bDecoration )
    {
        awt::DeviceInfo aDeviceInfo = pForm->getDeviceInfo();
        aSize.Width() -= aDeviceInfo.LeftInset + aDeviceInfo.RightInset;
        aSize.Height() -= aDeviceInfo.TopInset + aDeviceInfo.BottomInset;
    }
    
    aPos = pDevice->PixelToLogic( aPos, MAP_APPFONT );
    aSize = pDevice->PixelToLogic( aSize, MAP_APPFONT );

    
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
    
    Size aPos( nXIn, nYIn );
    Size aSize( nWidthIn, nHeightIn );

    
    DlgEdForm* pForm = NULL;
    if ( !lcl_getDlgEdForm( this, pForm ) )
        return false;

    Reference< beans::XPropertySet > xPSetForm( pForm->GetUnoControlModel(), UNO_QUERY );
    DBG_ASSERT( xPSetForm.is(), "DlgEdObj::TransformControlToSdrCoordinates: no form property set!" );
    if ( !xPSetForm.is() )
        return false;
    sal_Int32 nFormX = 0, nFormY = 0, nFormWidth, nFormHeight;
    xPSetForm->getPropertyValue( DLGED_PROP_POSITIONX ) >>= nFormX;
    xPSetForm->getPropertyValue( DLGED_PROP_POSITIONY ) >>= nFormY;
    xPSetForm->getPropertyValue( DLGED_PROP_WIDTH ) >>= nFormWidth;
    xPSetForm->getPropertyValue( DLGED_PROP_HEIGHT ) >>= nFormHeight;
    Size aFormPos( nFormX, nFormY );

    
    OutputDevice* pDevice = Application::GetDefaultDevice();
    DBG_ASSERT( pDevice, "DlgEdObj::TransformControlToSdrCoordinates: missing default device!" );
    if ( !pDevice )
        return false;
    aPos = pDevice->LogicToPixel( aPos, MAP_APPFONT );
    aSize = pDevice->LogicToPixel( aSize, MAP_APPFONT );
    aFormPos = pDevice->LogicToPixel( aFormPos, MAP_APPFONT );

    
    aPos.Width() += aFormPos.Width();
    aPos.Height() += aFormPos.Height();

    
    bool bDecoration = true;
    xPSetForm->getPropertyValue( DLGED_PROP_DECORATION ) >>= bDecoration;
    if( bDecoration )
    {
        awt::DeviceInfo aDeviceInfo = pForm->getDeviceInfo();
        aPos.Width() += aDeviceInfo.LeftInset;
        aPos.Height() += aDeviceInfo.TopInset;
    }

    
    aPos = pDevice->PixelToLogic( aPos, MapMode( MAP_100TH_MM ) );
    aSize = pDevice->PixelToLogic( aSize, MapMode( MAP_100TH_MM ) );

    
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
    
    Size aPos( nXIn, nYIn );
    Size aSize( nWidthIn, nHeightIn );

    
    OutputDevice* pDevice = Application::GetDefaultDevice();
    DBG_ASSERT( pDevice, "DlgEdObj::TransformFormToSdrCoordinates: missing default device!" );
    if ( !pDevice )
        return false;

    
    DlgEdForm* pForm = NULL;
    if ( !lcl_getDlgEdForm( this, pForm ) )
        return false;

    aPos = pDevice->LogicToPixel( aPos, MAP_APPFONT );
    aSize = pDevice->LogicToPixel( aSize, MAP_APPFONT );

    
    Reference< beans::XPropertySet > xPSetForm( pForm->GetUnoControlModel(), UNO_QUERY );
    DBG_ASSERT( xPSetForm.is(), "DlgEdObj::TransformFormToSdrCoordinates: no form property set!" );
    if ( !xPSetForm.is() )
        return false;
    bool bDecoration = true;
    xPSetForm->getPropertyValue( DLGED_PROP_DECORATION ) >>= bDecoration;
    if( bDecoration )
    {
        awt::DeviceInfo aDeviceInfo = pForm->getDeviceInfo();
        aSize.Width() += aDeviceInfo.LeftInset + aDeviceInfo.RightInset;
        aSize.Height() += aDeviceInfo.TopInset + aDeviceInfo.BottomInset;
    }

    
    aPos = pDevice->PixelToLogic( aPos, MapMode( MAP_100TH_MM ) );
    aSize = pDevice->PixelToLogic( aSize, MapMode( MAP_100TH_MM ) );

    
    nXOut = aPos.Width();
    nYOut = aPos.Height();
    nWidthOut = aSize.Width();
    nHeightOut = aSize.Height();

    return true;
}

void DlgEdObj::SetRectFromProps()
{
    
    Reference< beans::XPropertySet > xPSet( GetUnoControlModel(), UNO_QUERY );
    if ( xPSet.is() )
    {
        sal_Int32 nXIn = 0, nYIn = 0, nWidthIn = 0, nHeightIn = 0;
        xPSet->getPropertyValue( DLGED_PROP_POSITIONX ) >>= nXIn;
        xPSet->getPropertyValue( DLGED_PROP_POSITIONY ) >>= nYIn;
        xPSet->getPropertyValue( DLGED_PROP_WIDTH ) >>= nWidthIn;
        xPSet->getPropertyValue( DLGED_PROP_HEIGHT ) >>= nHeightIn;

        
        sal_Int32 nXOut, nYOut, nWidthOut, nHeightOut;
        if ( TransformControlToSdrCoordinates( nXIn, nYIn, nWidthIn, nHeightIn, nXOut, nYOut, nWidthOut, nHeightOut ) )
        {
            
            Point aPoint( nXOut, nYOut );
            Size aSize( nWidthOut, nHeightOut );
            SetSnapRect( Rectangle( aPoint, aSize ) );
        }
    }
}

void DlgEdObj::SetPropsFromRect()
{
    
    Rectangle aRect_ = GetSnapRect();
    sal_Int32 nXIn = aRect_.Left();
    sal_Int32 nYIn = aRect_.Top();
    sal_Int32 nWidthIn = aRect_.GetWidth();
    sal_Int32 nHeightIn = aRect_.GetHeight();

    
    sal_Int32 nXOut, nYOut, nWidthOut, nHeightOut;
    if ( TransformSdrToControlCoordinates( nXIn, nYIn, nWidthIn, nHeightIn, nXOut, nYOut, nWidthOut, nHeightOut ) )
    {
        
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

void DlgEdObj::PositionAndSizeChange( const beans::PropertyChangeEvent& evt )
{
    DBG_ASSERT( pDlgEdForm, "DlgEdObj::PositionAndSizeChange: no form!" );
    DlgEdPage& rPage = pDlgEdForm->GetDlgEditor().GetPage();
    {
        sal_Int32 nPageXIn = 0;
        sal_Int32 nPageYIn = 0;
        Size aPageSize = rPage.GetSize();
        sal_Int32 nPageWidthIn = aPageSize.Width();
        sal_Int32 nPageHeightIn = aPageSize.Height();
        sal_Int32 nPageX, nPageY, nPageWidth, nPageHeight;
        if ( TransformSdrToControlCoordinates( nPageXIn, nPageYIn, nPageWidthIn, nPageHeightIn, nPageX, nPageY, nPageWidth, nPageHeight ) )
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
                    Any aNewValue;
                    aNewValue <<= nNewValue;
                    EndListening( false );
                    xPSet->setPropertyValue( evt.PropertyName, aNewValue );
                    StartListening();
                }
            }
        }
    }

    SetRectFromProps();
}

void SAL_CALL DlgEdObj::NameChange( const  ::com::sun::star::beans::PropertyChangeEvent& evt ) throw( ::com::sun::star::uno::RuntimeException)
{
    
    OUString aOldName;
    evt.OldValue >>= aOldName;

    
    OUString aNewName;
    evt.NewValue >>= aNewName;

    if ( !aNewName.equals(aOldName) )
    {
        Reference< container::XNameAccess > xNameAcc((GetDlgEdForm()->GetUnoControlModel()), UNO_QUERY);
        if ( xNameAcc.is() && xNameAcc->hasByName(aOldName) )
        {
            if (!xNameAcc->hasByName(aNewName) && !aNewName.isEmpty())
            {
                
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
                
                EndListening(false);
                Reference< beans::XPropertySet >  xPSet(GetUnoControlModel(), UNO_QUERY);
                Any aName;
                aName <<= aOldName;
                xPSet->setPropertyValue( DLGED_PROP_NAME, aName );
                StartListening();
            }
        }
    }
}

sal_Int32 DlgEdObj::GetStep() const
{
    
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

    SdrLayerAdmin& rLayerAdmin = GetModel()->GetLayerAdmin();
    SdrLayerID nHiddenLayerId   = rLayerAdmin.GetLayerID( OUString( "HiddenLayer" ), false );
    SdrLayerID nControlLayerId   = rLayerAdmin.GetLayerID( rLayerAdmin.GetControlLayerName(), false );

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

void DlgEdObj::TabIndexChange( const beans::PropertyChangeEvent& evt ) throw (RuntimeException)
{
    DlgEdForm* pForm = GetDlgEdForm();
    if ( pForm )
    {
        
        ::std::vector<DlgEdObj*> aChildList = pForm->GetChildren();
        ::std::vector<DlgEdObj*>::iterator aIter;
        for ( aIter = aChildList.begin() ; aIter != aChildList.end() ; ++aIter )
        {
            (*aIter)->EndListening( false );
        }

        Reference< container::XNameAccess > xNameAcc( pForm->GetUnoControlModel() , UNO_QUERY );
        if ( xNameAcc.is() )
        {
            
            Sequence< OUString > aNames = xNameAcc->getElementNames();
            const OUString* pNames = aNames.getConstArray();
            sal_Int32 nCtrls = aNames.getLength();
            sal_Int16 i;

            
            IndexToNameMap aIndexToNameMap;
            for ( i = 0; i < nCtrls; ++i )
            {
                
                OUString aName( pNames[i] );

                
                sal_Int16 nTabIndex = -1;
                Any aCtrl = xNameAcc->getByName( aName );
                Reference< beans::XPropertySet > xPSet;
                   aCtrl >>= xPSet;
                if ( xPSet.is() && xPSet == Reference< beans::XPropertySet >( evt.Source, UNO_QUERY ) )
                    evt.OldValue >>= nTabIndex;
                else if ( xPSet.is() )
                    xPSet->getPropertyValue( DLGED_PROP_TABINDEX ) >>= nTabIndex;

                
                aIndexToNameMap.insert( IndexToNameMap::value_type( nTabIndex, aName ) );
            }

            
            ::std::vector< OUString > aNameList( aIndexToNameMap.size() );
            ::std::transform(
                    aIndexToNameMap.begin(), aIndexToNameMap.end(),
                    aNameList.begin(),
                    ::o3tl::select2nd< IndexToNameMap::value_type >( )
                );

            
            sal_Int16 nOldTabIndex = 0;
            evt.OldValue >>= nOldTabIndex;
            sal_Int16 nNewTabIndex = 0;
            evt.NewValue >>= nNewTabIndex;
            if ( nNewTabIndex < 0 )
                nNewTabIndex = 0;
            else if ( nNewTabIndex > nCtrls - 1 )
                nNewTabIndex = sal::static_int_cast<sal_Int16>( nCtrls - 1 );

            
            OUString aCtrlName = aNameList[nOldTabIndex];
            aNameList.erase( aNameList.begin() + nOldTabIndex );
            aNameList.insert( aNameList.begin() + nNewTabIndex , aCtrlName );

            
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

            
            GetModel()->GetPage(0)->SetObjectOrdNum( nOldTabIndex + 1, nNewTabIndex + 1 );

            
            pForm->UpdateTabOrderAndGroups();
        }

        
        for ( aIter = aChildList.begin() ; aIter != aChildList.end() ; ++aIter )
        {
            (*aIter)->StartListening();
        }
    }
}

bool DlgEdObj::supportsService( OUString const & serviceName ) const
{
    bool bSupports = false;

    Reference< lang::XServiceInfo > xServiceInfo( GetUnoControlModel() , UNO_QUERY );
        
    if ( xServiceInfo.is() )
        bSupports = xServiceInfo->supportsService( serviceName );

    return bSupports;
}

OUString DlgEdObj::GetDefaultName() const
{
    sal_uInt16 nResId = 0;
    OUString aDefaultName;
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
    else if ( supportsService( "com.sun.star.awt.tree.TreeControlModel" ) )
    {
        nResId = RID_STR_CLASS_TREECONTROL;
    }
    else if ( supportsService( "com.sun.star.awt.UnoControlSpinButtonModel" ) )
    {
        nResId = RID_STR_CLASS_SPINCONTROL;
    }
    else
    {
        nResId = RID_STR_CLASS_CONTROL;
    }

    if (nResId)
    {
        aDefaultName = IDE_RESSTR(nResId);
    }

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

sal_uInt32 DlgEdObj::GetObjInventor()   const
{
    return DlgInventor;
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
    else
    {
        return OBJ_DLG_CONTROL;
    }
}

void DlgEdObj::clonedFrom(const DlgEdObj* _pSource)
{
    
    pDlgEdForm = _pSource->pDlgEdForm;

    
    pDlgEdForm->AddChild( this );

    Reference< beans::XPropertySet > xPSet( GetUnoControlModel(), UNO_QUERY );
    if ( xPSet.is() )
    {
        
        OUString aOUniqueName( GetUniqueName() );
        Any aUniqueName;
        aUniqueName <<= aOUniqueName;
        xPSet->setPropertyValue( DLGED_PROP_NAME, aUniqueName );

        Reference< container::XNameContainer > xCont( GetDlgEdForm()->GetUnoControlModel() , UNO_QUERY );
        if ( xCont.is() )
        {
            
               Sequence< OUString > aNames = xCont->getElementNames();
            Any aTabIndex;
            aTabIndex <<= (sal_Int16) aNames.getLength();
            xPSet->setPropertyValue( DLGED_PROP_TABINDEX, aTabIndex );

            
            Reference< awt::XControlModel > xCtrl( xPSet , UNO_QUERY );
            Any aCtrl;
            aCtrl <<= xCtrl;
            xCont->insertByName( aOUniqueName , aCtrl );

            
            pDlgEdForm->UpdateTabOrderAndGroups();
        }
    }

    
    StartListening();
}

DlgEdObj* DlgEdObj::Clone() const
{
    DlgEdObj* pDlgEdObj = CloneHelper< DlgEdObj >();
    DBG_ASSERT( pDlgEdObj != NULL, "DlgEdObj::Clone: invalid clone!" );
    if ( pDlgEdObj )
        pDlgEdObj->clonedFrom( this );

    return pDlgEdObj;
}

SdrObject* DlgEdObj::getFullDragClone() const
{
    
    
    SdrObject* pObj = new SdrUnoObj(OUString());
    *pObj = *((const SdrUnoObj*)this);

    return pObj;
}

void DlgEdObj::NbcMove( const Size& rSize )
{
    SdrUnoObj::NbcMove( rSize );

    
    EndListening(false);

    
    SetPropsFromRect();

    
    StartListening();

    
    GetDlgEdForm()->GetDlgEditor().SetDialogModelChanged(true);
}

void DlgEdObj::NbcResize(const Point& rRef, const Fraction& xFract, const Fraction& yFract)
{
    SdrUnoObj::NbcResize( rRef, xFract, yFract );

    
    EndListening(false);

    
    SetPropsFromRect();

    
    StartListening();

    
    GetDlgEdForm()->GetDlgEditor().SetDialogModelChanged(true);
}

bool DlgEdObj::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    bool bResult = SdrUnoObj::EndCreate(rStat, eCmd);

    SetDefaults();
    StartListening();

    return bResult;
}

void DlgEdObj::SetDefaults()
{
    
    pDlgEdForm = ((DlgEdPage*)GetPage())->GetDlgEdForm();

    if ( pDlgEdForm )
    {
        
        pDlgEdForm->AddChild( this );

        Reference< beans::XPropertySet > xPSet( GetUnoControlModel(), UNO_QUERY );
        if ( xPSet.is() )
        {
            
            OUString aOUniqueName( GetUniqueName() );

            
            Any aUniqueName;
            aUniqueName <<= aOUniqueName;
            xPSet->setPropertyValue( DLGED_PROP_NAME, aUniqueName );

            
            if ( supportsService( "com.sun.star.awt.UnoControlButtonModel" ) ||
                supportsService( "com.sun.star.awt.UnoControlRadioButtonModel" ) ||
                supportsService( "com.sun.star.awt.UnoControlCheckBoxModel" ) ||
                supportsService( "com.sun.star.awt.UnoControlGroupBoxModel" ) ||
                supportsService( "com.sun.star.awt.UnoControlFixedTextModel" ) )
            {
                xPSet->setPropertyValue( DLGED_PROP_LABEL, aUniqueName );
            }

            
            if ( supportsService( "com.sun.star.awt.UnoControlFormattedFieldModel" ) )
            {
                Reference< util::XNumberFormatsSupplier > xSupplier = GetDlgEdForm()->GetDlgEditor().GetNumberFormatsSupplier();
                if ( xSupplier.is() )
                {
                    Any aSupplier;
                    aSupplier <<= xSupplier;
                    xPSet->setPropertyValue( DLGED_PROP_FORMATSSUPPLIER, aSupplier );
                }
            }

            
            SetPropsFromRect();

            Reference< container::XNameContainer > xCont( GetDlgEdForm()->GetUnoControlModel() , UNO_QUERY );
            if ( xCont.is() )
            {
                
                   Sequence< OUString > aNames = xCont->getElementNames();
                uno::Any aTabIndex;
                aTabIndex <<= (sal_Int16) aNames.getLength();
                xPSet->setPropertyValue( DLGED_PROP_TABINDEX, aTabIndex );

                
                Reference< beans::XPropertySet > xPSetForm( xCont, UNO_QUERY );
                if ( xPSetForm.is() )
                {
                    Any aStep = xPSetForm->getPropertyValue( DLGED_PROP_STEP );
                    xPSet->setPropertyValue( DLGED_PROP_STEP, aStep );
                }

                
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

        
        pDlgEdForm->GetDlgEditor().SetDialogModelChanged(true);
    }
}

void DlgEdObj::StartListening()
{
    DBG_ASSERT(!isListening(), "DlgEdObj::StartListening: already listening!");

    if (!isListening())
    {
        bIsListening = true;

        
        Reference< XPropertySet > xControlModel( GetUnoControlModel() , UNO_QUERY );
        if (!m_xPropertyChangeListener.is() && xControlModel.is())
        {
            
            m_xPropertyChangeListener = new DlgEdPropListenerImpl(*this);

            
            xControlModel->addPropertyChangeListener( OUString() , m_xPropertyChangeListener );
        }

        
        Reference< XScriptEventsSupplier > xEventsSupplier( GetUnoControlModel() , UNO_QUERY );
        if( !m_xContainerListener.is() && xEventsSupplier.is() )
        {
            
            m_xContainerListener = new DlgEdEvtContListenerImpl(*this);

            
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
            
            Reference< XPropertySet > xControlModel(GetUnoControlModel(), UNO_QUERY);
            if ( m_xPropertyChangeListener.is() && xControlModel.is() )
            {
                
                xControlModel->removePropertyChangeListener( OUString() , m_xPropertyChangeListener );
            }
            m_xPropertyChangeListener.clear();

            
            Reference< XScriptEventsSupplier > xEventsSupplier( GetUnoControlModel() , UNO_QUERY );
            if( m_xContainerListener.is() && xEventsSupplier.is() )
            {
                
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

void SAL_CALL DlgEdObj::_propertyChange( const  ::com::sun::star::beans::PropertyChangeEvent& evt ) throw( ::com::sun::star::uno::RuntimeException)
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

        
        rDlgEditor.SetDialogModelChanged(true);

        
        if ( evt.PropertyName == DLGED_PROP_POSITIONX || evt.PropertyName == DLGED_PROP_POSITIONY ||
             evt.PropertyName == DLGED_PROP_WIDTH || evt.PropertyName == DLGED_PROP_HEIGHT ||
             evt.PropertyName == DLGED_PROP_DECORATION )
        {
            PositionAndSizeChange( evt );

            if ( evt.PropertyName == DLGED_PROP_DECORATION )
                GetDialogEditor().ResetDialog();
        }
        
        else if ( evt.PropertyName == DLGED_PROP_NAME )
        {
            if (!dynamic_cast<DlgEdForm*>(this))
                NameChange(evt);
        }
        
        else if ( evt.PropertyName == DLGED_PROP_STEP )
        {
            UpdateStep();
        }
        
        else if ( evt.PropertyName == DLGED_PROP_TABINDEX )
        {
            if (!dynamic_cast<DlgEdForm*>(this))
                TabIndexChange(evt);
        }
    }
}

void SAL_CALL DlgEdObj::_elementInserted(const ::com::sun::star::container::ContainerEvent& ) throw(::com::sun::star::uno::RuntimeException)
{
    if (isListening())
    {
        
        GetDialogEditor().SetDialogModelChanged(true);
    }
}

void SAL_CALL DlgEdObj::_elementReplaced(const ::com::sun::star::container::ContainerEvent& ) throw(::com::sun::star::uno::RuntimeException)
{
    if (isListening())
    {
        
        GetDialogEditor().SetDialogModelChanged(true);
    }
}

void SAL_CALL DlgEdObj::_elementRemoved(const ::com::sun::star::container::ContainerEvent& ) throw(::com::sun::star::uno::RuntimeException)
{
    if (isListening())
    {
        
        GetDialogEditor().SetDialogModelChanged(true);
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

TYPEINIT1(DlgEdForm, DlgEdObj);
DBG_NAME(DlgEdForm);

DlgEdForm::DlgEdForm (DlgEditor& rDlgEditor_) :
    rDlgEditor(rDlgEditor_)
{
    DBG_CTOR(DlgEdForm, NULL);
}

DlgEdForm::~DlgEdForm()
{
    DBG_DTOR(DlgEdForm, NULL);
}

void DlgEdForm::SetRectFromProps()
{
    
    Reference< beans::XPropertySet > xPSet( GetUnoControlModel(), UNO_QUERY );
    if ( xPSet.is() )
    {
        sal_Int32 nXIn = 0, nYIn = 0, nWidthIn = 0, nHeightIn = 0;
        xPSet->getPropertyValue( DLGED_PROP_POSITIONX ) >>= nXIn;
        xPSet->getPropertyValue( DLGED_PROP_POSITIONY ) >>= nYIn;
        xPSet->getPropertyValue( DLGED_PROP_WIDTH ) >>= nWidthIn;
        xPSet->getPropertyValue( DLGED_PROP_HEIGHT ) >>= nHeightIn;

        
        sal_Int32 nXOut, nYOut, nWidthOut, nHeightOut;
        if ( TransformFormToSdrCoordinates( nXIn, nYIn, nWidthIn, nHeightIn, nXOut, nYOut, nWidthOut, nHeightOut ) )
        {
            
            Point aPoint( nXOut, nYOut );
            Size aSize( nWidthOut, nHeightOut );
            SetSnapRect( Rectangle( aPoint, aSize ) );
        }
    }
}

void DlgEdForm::SetPropsFromRect()
{
    
    Rectangle aRect_ = GetSnapRect();
    sal_Int32 nXIn = aRect_.Left();
    sal_Int32 nYIn = aRect_.Top();
    sal_Int32 nWidthIn = aRect_.GetWidth();
    sal_Int32 nHeightIn = aRect_.GetHeight();

    
    sal_Int32 nXOut, nYOut, nWidthOut, nHeightOut;
    if ( TransformSdrToFormCoordinates( nXIn, nYIn, nWidthIn, nHeightIn, nXOut, nYOut, nWidthOut, nHeightOut ) )
    {
        
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

void DlgEdForm::AddChild( DlgEdObj* pDlgEdObj )
{
    pChildren.push_back( pDlgEdObj );
}

void DlgEdForm::RemoveChild( DlgEdObj* pDlgEdObj )
{
    pChildren.erase( ::std::find( pChildren.begin() , pChildren.end() , pDlgEdObj ) );
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
                Any aNewValue;
                aNewValue <<= nNewValue;
                EndListening( false );
                xPSetForm->setPropertyValue( evt.PropertyName, aNewValue );
                StartListening();
            }
        }
    }

    bool bAdjustedPageSize = rEditor.AdjustPageSize();
    SetRectFromProps();
    std::vector<DlgEdObj*> const& aChildList = ((DlgEdForm*)this)->GetChildren();
    std::vector<DlgEdObj*>::const_iterator aIter;

    if ( bAdjustedPageSize )
    {
        rEditor.InitScrollBars();
        aPageSize = rPage.GetSize();
        nPageWidthIn = aPageSize.Width();
        nPageHeightIn = aPageSize.Height();
        if ( TransformSdrToControlCoordinates( nPageXIn, nPageYIn, nPageWidthIn, nPageHeightIn, nPageX, nPageY, nPageWidth, nPageHeight ) )
        {
            for ( aIter = aChildList.begin(); aIter != aChildList.end(); ++aIter )
            {
                Reference< beans::XPropertySet > xPSet( (*aIter)->GetUnoControlModel(), UNO_QUERY );
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
                        Any aValue;
                        aValue <<= nNewX;
                        EndListening( false );
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
                        EndListening( false );
                        xPSet->setPropertyValue( DLGED_PROP_POSITIONY, aValue );
                        StartListening();
                    }
                }
            }
        }
    }

    for ( aIter = aChildList.begin(); aIter != aChildList.end(); ++aIter )
        (*aIter)->SetRectFromProps();
}

void DlgEdForm::UpdateStep()
{
    sal_uLong nObjCount;
    SdrPage* pSdrPage = GetPage();

    if ( pSdrPage && ( ( nObjCount = pSdrPage->GetObjCount() ) > 0 ) )
    {
        for ( sal_uLong i = 0 ; i < nObjCount ; i++ )
        {
            DlgEdObj* pDlgEdObj = dynamic_cast<DlgEdObj*>(pSdrPage->GetObj(i));
            if (pDlgEdObj && !dynamic_cast<DlgEdForm*>(pDlgEdObj))
                pDlgEdObj->UpdateStep();
        }
    }
}

void DlgEdForm::UpdateTabIndices()
{
    
    ::std::vector<DlgEdObj*>::iterator aIter;
    for ( aIter = pChildren.begin() ; aIter != pChildren.end() ; ++aIter )
    {
        (*aIter)->EndListening( false );
    }

    Reference< ::com::sun::star::container::XNameAccess > xNameAcc( GetUnoControlModel() , UNO_QUERY );
    if ( xNameAcc.is() )
    {
        
        Sequence< OUString > aNames = xNameAcc->getElementNames();
        const OUString* pNames = aNames.getConstArray();
        sal_Int32 nCtrls = aNames.getLength();

        
        IndexToNameMap aIndexToNameMap;
        for ( sal_Int16 i = 0; i < nCtrls; ++i )
        {
            
            OUString aName( pNames[i] );

            
            sal_Int16 nTabIndex = -1;
            Any aCtrl = xNameAcc->getByName( aName );
            Reference< ::com::sun::star::beans::XPropertySet > xPSet;
               aCtrl >>= xPSet;
            if ( xPSet.is() )
                xPSet->getPropertyValue( DLGED_PROP_TABINDEX ) >>= nTabIndex;

            
            aIndexToNameMap.insert( IndexToNameMap::value_type( nTabIndex, aName ) );
        }

        
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

        
        UpdateTabOrderAndGroups();
    }

    
    for ( aIter = pChildren.begin() ; aIter != pChildren.end() ; ++aIter )
    {
        (*aIter)->StartListening();
    }
}

void DlgEdForm::UpdateTabOrder()
{
    
    
    
    
    
    

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
    
    
    
    
    
    
    
    

    Reference< awt::XTabControllerModel > xTabModel( GetUnoControlModel() , UNO_QUERY );
    if ( xTabModel.is() )
    {
        
        ::std::vector<DlgEdObj*> aChildList = GetChildren();
        sal_uInt32 nSize = aChildList.size();
        Sequence< Reference< awt::XControl > > aSeqControls( nSize );
        for ( sal_uInt32 i = 0; i < nSize; ++i )
            aSeqControls.getArray()[i] = Reference< awt::XControl >( aChildList[i]->GetControl(), UNO_QUERY );

        sal_Int32 nGroupCount = xTabModel->getGroupCount();
        for ( sal_Int32 nGroup = 0; nGroup < nGroupCount; ++nGroup )
        {
            
            OUString aName;
            Sequence< Reference< awt::XControlModel > > aSeqModels;
            xTabModel->getGroup( nGroup, aSeqModels, aName );
            const Reference< awt::XControlModel >* pModels = aSeqModels.getConstArray();
            sal_Int32 nModelCount = aSeqModels.getLength();

            
            Sequence< Reference< awt::XWindow > > aSeqPeers( nModelCount );
            for ( sal_Int32 nModel = 0; nModel < nModelCount; ++nModel )
            {
                
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
                            
                            aSeqPeers.getArray()[ nModel ] = Reference< awt::XWindow >( xCtrl->getPeer(), UNO_QUERY );
                            break;
                        }
                    }
                }
            }

            
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

    
    EndListening(false);
    SetPropsFromRect();
    StartListening();

    
    ::std::vector<DlgEdObj*>::iterator aIter;
    for ( aIter = pChildren.begin() ; aIter != pChildren.end() ; ++aIter )
    {
        (*aIter)->EndListening(false);
        (*aIter)->SetPropsFromRect();
        (*aIter)->StartListening();
    }

    
    GetDlgEditor().SetDialogModelChanged(true);
}

void DlgEdForm::NbcResize(const Point& rRef, const Fraction& xFract, const Fraction& yFract)
{
    SdrUnoObj::NbcResize( rRef, xFract, yFract );

    
    EndListening(false);
    SetPropsFromRect();
    StartListening();

    
    ::std::vector<DlgEdObj*>::iterator aIter;
    for ( aIter = pChildren.begin() ; aIter != pChildren.end() ; ++aIter )
    {
        (*aIter)->EndListening(false);
        (*aIter)->SetPropsFromRect();
        (*aIter)->StartListening();
    }

    
    GetDlgEditor().SetDialogModelChanged(true);
}

bool DlgEdForm::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    bool bResult = SdrUnoObj::EndCreate(rStat, eCmd);

    
    EndListening(false);

    
    SetPropsFromRect();

    
    GetDlgEditor().SetDialogModelChanged(true);

    
    StartListening();

    return bResult;
}

awt::DeviceInfo DlgEdForm::getDeviceInfo() const
{
    awt::DeviceInfo aDeviceInfo;

    DlgEditor& rEditor = GetDlgEditor();
    Window& rWindow = rEditor.GetWindow();

    
    ::utl::SharedUNOComponent< awt::XControl > xDialogControl; 
    xDialogControl.reset( GetControl(), ::utl::SharedUNOComponent< awt::XControl >::NoTakeOwnership );
    if ( !xDialogControl.is() )
    {
        
        
        
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
bool DlgEdObj::MakeDataAware( const Reference< frame::XModel >& xModel )
{
    bool bRes = false;
    
    
    
    
    Reference< lang::XMultiServiceFactory > xFac( xModel, UNO_QUERY );
    Reference< form::binding::XBindableValue > xBindable( GetUnoControlModel(), UNO_QUERY );
    Reference< form::binding::XListEntrySink  > xListEntrySink( GetUnoControlModel(), UNO_QUERY );
    if ( xFac.is() )
    {
        if ( xBindable.is() )
        {
            Reference< form::binding::XValueBinding > xBinding( xFac->createInstance( "com.sun.star.table.CellValueBinding" ), UNO_QUERY );
            xBindable->setValueBinding( xBinding );
        }
        if ( xListEntrySink.is() )
        {
            Reference< form::binding::XListEntrySource > xSource( xFac->createInstance( "com.sun.star.table.CellRangeListSource" ), UNO_QUERY );
            xListEntrySink->setListEntrySource( xSource );
        }
        if ( xListEntrySink.is() || xBindable.is() )
            bRes = true;
    }
    return bRes;
}
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
