/*************************************************************************
 *
 *  $RCSfile: dlgedobj.cxx,v $
 *
 *  $Revision: 1.33 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 12:40:17 $
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

void DlgEdObj::SetRectFromProps()
{
    // get property sets
    Reference< beans::XPropertySet >  xPSet( GetUnoControlModel(), UNO_QUERY );
    Reference< beans::XPropertySet >  xPSetForm( GetDlgEdForm()->GetUnoControlModel(), UNO_QUERY );

    if (xPSet.is() && xPSetForm.is())
    {
        // get control position and size from properties
        sal_Int32 nX, nY, nWidth, nHeight;
        xPSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PositionX" ) ) ) >>= nX;
        xPSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PositionY" ) ) ) >>= nY;
        xPSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Width" ) ) ) >>= nWidth;
        xPSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Height" ) ) ) >>= nHeight;

        // get form position and size from properties
        sal_Int32 nFormX, nFormY, nFormWidth, nFormHeight;
        xPSetForm->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PositionX" ) ) ) >>= nFormX;
        xPSetForm->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PositionY" ) ) ) >>= nFormY;
        xPSetForm->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Width" ) ) ) >>= nFormWidth;
        xPSetForm->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Height" ) ) ) >>= nFormHeight;

        Size aPos( nX, nY );
        Size aSize( nWidth, nHeight );
        Size aFormPos( nFormX, nFormY );
        Size aFormSize( nFormWidth, nFormHeight );

        // convert logic units to pixel
        OutputDevice* pDevice = Application::GetDefaultDevice();
        DBG_ASSERT( pDevice, "Missing Default Device!" );
        if( pDevice )
        {
            aPos = pDevice->LogicToPixel( aPos, MapMode(MAP_APPFONT) );
            aSize = pDevice->LogicToPixel( aSize, MapMode(MAP_APPFONT) );
            aFormPos = pDevice->LogicToPixel( aFormPos, MapMode(MAP_APPFONT) );
            aFormSize = pDevice->LogicToPixel( aFormSize, MapMode(MAP_APPFONT) );
        }

        // take window borders into account
        awt::DeviceInfo aDeviceInfo;
        Window* pWindow = GetDlgEdForm()->GetDlgEditor()->GetWindow();
        Reference< awt::XControl > xDlg( GetDlgEdForm()->GetUnoControl( pWindow ), UNO_QUERY );
        if ( xDlg.is() )
        {
            Reference< awt::XDevice > xDev( xDlg->getPeer(), UNO_QUERY );
            if ( xDev.is() )
                aDeviceInfo = xDev->getInfo();
        }
        aPos.Width() += aFormPos.Width() + aDeviceInfo.LeftInset;
        aPos.Height() += aFormPos.Height() + aDeviceInfo.TopInset;

        // convert pixel to 100th_mm
        if( pDevice )
        {
            aPos = pDevice->PixelToLogic( aPos, MapMode(MAP_100TH_MM) );
            aSize = pDevice->PixelToLogic( aSize, MapMode(MAP_100TH_MM) );
        }

        Point aPoint;
        aPoint.X() = aPos.Width();
        aPoint.Y() = aPos.Height();

        // set rectangle position and size
        SetSnapRect( Rectangle( aPoint, aSize ) );
    }
}

//----------------------------------------------------------------------------

void DlgEdObj::SetPropsFromRect()
{
    // get control property set
    Reference< beans::XPropertySet >  xPSet( GetUnoControlModel(), UNO_QUERY );
    if (xPSet.is())
    {
        // get control position and size from rectangle
        Rectangle aRect = GetSnapRect();
        Point aPoint = aRect.TopLeft();
        Size aSize = aRect.GetSize();
        Size aPos;
        aPos.Width() = aPoint.X();
        aPos.Height() = aPoint.Y();

        // get form position and size from rectangle
        Rectangle aFormRect = GetDlgEdForm()->GetSnapRect();
        Point aFormPoint = aFormRect.TopLeft();
        Size aFormSize = aFormRect.GetSize();
        Size aFormPos;
        aFormPos.Width() = aFormPoint.X();
        aFormPos.Height() = aFormPoint.Y();

        // convert 100th_mm to pixel
        OutputDevice* pDevice = Application::GetDefaultDevice();
        DBG_ASSERT( pDevice, "Missing Default Device!" );
        if( pDevice )
        {
            aPos = pDevice->LogicToPixel( aPos, MapMode(MAP_100TH_MM) );
            aSize = pDevice->LogicToPixel( aSize, MapMode(MAP_100TH_MM) );
            aFormPos = pDevice->LogicToPixel( aFormPos, MapMode(MAP_100TH_MM) );
            aFormSize = pDevice->LogicToPixel( aFormSize, MapMode(MAP_100TH_MM) );
        }

        // take window borders into account
        awt::DeviceInfo aDeviceInfo;
        Window* pWindow = GetDlgEdForm()->GetDlgEditor()->GetWindow();
        Reference< awt::XControl > xDlg( GetDlgEdForm()->GetUnoControl( pWindow ), UNO_QUERY );
        if ( xDlg.is() )
        {
            Reference< awt::XDevice > xDev( xDlg->getPeer(), UNO_QUERY );
            if ( xDev.is() )
                aDeviceInfo = xDev->getInfo();
        }
        aPos.Width() -= aFormPos.Width() + aDeviceInfo.LeftInset;
        aPos.Height() -= aFormPos.Height() + aDeviceInfo.TopInset;

        // convert pixel to logic units
        if( pDevice )
        {
            aPos = pDevice->PixelToLogic( aPos, MapMode(MAP_APPFONT) );
            aSize = pDevice->PixelToLogic( aSize, MapMode(MAP_APPFONT) );
        }

        // set properties
        Any aValue;
        aValue <<= aPos.Width();
        xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "PositionX" ) ), aValue );
        aValue <<= aPos.Height();
        xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "PositionY" ) ), aValue );
        aValue <<= aSize.Width();
        xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Width" ) ), aValue );
        aValue <<= aSize.Height();
        xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Height" ) ), aValue );
    }
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
                xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Name" ) ), aName );
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
        xPSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Step" ) ) ) >>= nStep;
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

            // create a map of tab indices and control names, sorted by tab index
            IndexToNameMap aIndexToNameMap;
            for ( sal_Int16 i = 0; i < nCtrls; ++i )
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
                    xPSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TabIndex" ) ) ) >>= nTabIndex;

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
                    xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "TabIndex" ) ), aTabIndex );
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
        aDefaultName = ::rtl::OUString( IDEResId(nResId) );
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
        xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Name" ) ), aUniqueName );

        Reference< container::XNameContainer > xCont( GetDlgEdForm()->GetUnoControlModel() , UNO_QUERY );
        if ( xCont.is() )
        {
            // set tabindex
               Sequence< OUString > aNames = xCont->getElementNames();
            Any aTabIndex;
            aTabIndex <<= (sal_Int16) aNames.getLength();
            xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "TabIndex" ) ), aTabIndex );

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
    SdrObject* pReturn = SdrUnoObj::Clone(_pPage, _pModel);
    if (!pReturn)
        return pReturn;

    return pReturn;
}

//----------------------------------------------------------------------------

void DlgEdObj::operator= (const SdrObject& rObj)
{
    SdrUnoObj::operator= (rObj);
}

//----------------------------------------------------------------------------

void DlgEdObj::WriteData(SvStream& rOut) const  // not working yet
{
    SdrUnoObj::WriteData(rOut);
}

//----------------------------------------------------------------------------

void DlgEdObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn) // not working yet
{
    SdrUnoObj::ReadData(rHead,rIn);
}

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
            xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Name" ) ), aUniqueName );

            // set labels
            if ( supportsService( "com.sun.star.awt.UnoControlButtonModel" ) ||
                supportsService( "com.sun.star.awt.UnoControlRadioButtonModel" ) ||
                supportsService( "com.sun.star.awt.UnoControlCheckBoxModel" ) ||
                supportsService( "com.sun.star.awt.UnoControlGroupBoxModel" ) ||
                supportsService( "com.sun.star.awt.UnoControlFixedTextModel" ) )
            {
                xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Label" ) ), aUniqueName );
            }

            // set number formats supplier for formatted field
            if ( supportsService( "com.sun.star.awt.UnoControlFormattedFieldModel" ) )
            {
                Reference< util::XNumberFormatsSupplier > xSupplier = GetDlgEdForm()->GetDlgEditor()->GetNumberFormatsSupplier();
                if ( xSupplier.is() )
                {
                    Any aSupplier;
                    aSupplier <<= xSupplier;
                    xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "FormatsSupplier" ) ), aSupplier );
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
                xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "TabIndex" ) ), aTabIndex );

                // set step
                Reference< beans::XPropertySet > xPSetForm( xCont, UNO_QUERY );
                if ( xPSetForm.is() )
                {
                    Any aStep = xPSetForm->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Step" ) ) );
                    xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Step" ) ), aStep );
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

        // set rectangle, if geometry information in the model changed
        if ( evt.PropertyName == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Width")) ||
             evt.PropertyName == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Height")) ||
             evt.PropertyName == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PositionX")) ||
             evt.PropertyName == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PositionY")) )
        {
            if ( !ISA(DlgEdForm) )
            {
                SetRectFromProps();
            }
            else
            {
                // set rectangle of form
                SetRectFromProps();

                // set rectangles of all childs
                ::std::vector<DlgEdObj*>::iterator aIter;
                ::std::vector<DlgEdObj*> aChildList = ((DlgEdForm*)this)->GetChilds();
                for ( aIter = aChildList.begin() ; aIter != aChildList.end() ; aIter++ )
                {
                    (*aIter)->SetRectFromProps();
                }
            }
        }
        // change name of control in dialog model
        else if ( evt.PropertyName == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Name")) )
        {
            if ( !ISA(DlgEdForm) )
            {
                NameChange(evt);
            }
        }
        // update step
        else if ( evt.PropertyName == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Step")) )
        {
            UpdateStep();
        }
        // change tabindex
        else if ( evt.PropertyName == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TabIndex")) )
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
    // get property set
    Reference< beans::XPropertySet >  xPSetForm( GetUnoControlModel(), UNO_QUERY );

    if (xPSetForm.is())
    {
        // get form position and size from properties
        sal_Int32 nFormX, nFormY, nFormWidth, nFormHeight;
        xPSetForm->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PositionX" ) ) ) >>= nFormX;
        xPSetForm->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PositionY" ) ) ) >>= nFormY;
        xPSetForm->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Width" ) ) ) >>= nFormWidth;
        xPSetForm->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Height" ) ) ) >>= nFormHeight;

        Size aFormPos( nFormX, nFormY );
        Size aFormSize( nFormWidth, nFormHeight );

        // convert logic units to pixel
        OutputDevice* pDevice = Application::GetDefaultDevice();
        DBG_ASSERT( pDevice, "Missing Default Device!" );
        if( pDevice )
        {
            aFormPos = pDevice->LogicToPixel( aFormPos, MapMode(MAP_APPFONT) );
            aFormSize = pDevice->LogicToPixel( aFormSize, MapMode(MAP_APPFONT) );
        }

        // take window borders into account
        awt::DeviceInfo aDeviceInfo;
        Window* pWindow = GetDlgEditor()->GetWindow();
        Reference< awt::XControl > xDlg( GetUnoControl( pWindow ), UNO_QUERY );
        if ( xDlg.is() )
        {
            Reference< awt::XDevice > xDev( xDlg->getPeer(), UNO_QUERY );
            if ( xDev.is() )
                aDeviceInfo = xDev->getInfo();
        }
        aFormSize.Width() += aDeviceInfo.LeftInset + aDeviceInfo.RightInset;
        aFormSize.Height() += aDeviceInfo.TopInset + aDeviceInfo.BottomInset;

        // convert pixel to 100th_mm
        if( pDevice )
        {
            aFormPos = pDevice->PixelToLogic( aFormPos, MapMode(MAP_100TH_MM) );
            aFormSize = pDevice->PixelToLogic( aFormSize, MapMode(MAP_100TH_MM) );
        }

        Point aFormPoint;
        aFormPoint.X() = aFormPos.Width();
        aFormPoint.Y() = aFormPos.Height();

        // set rectangle position and size
        SetSnapRect( Rectangle( aFormPoint, aFormSize ) );
    }
}

//----------------------------------------------------------------------------

void DlgEdForm::SetPropsFromRect()
{
    //EndListening(sal_False);

    // get control property set
    Reference< beans::XPropertySet >  xPSetForm( GetUnoControlModel(), UNO_QUERY );
    if (xPSetForm.is())
    {
        // get form position and size from rectangle
        Rectangle aFormRect = GetSnapRect();
        Point aFormPoint = aFormRect.TopLeft();
        Size aFormSize = aFormRect.GetSize();
        Size aFormPos;
        aFormPos.Width() = aFormPoint.X();
        aFormPos.Height() = aFormPoint.Y();

        // convert 100th_mm to pixel
        OutputDevice* pDevice = Application::GetDefaultDevice();
        DBG_ASSERT( pDevice, "Missing Default Device!" );
        if( pDevice )
        {
            aFormPos = pDevice->LogicToPixel( aFormPos, MapMode(MAP_100TH_MM) );
            aFormSize = pDevice->LogicToPixel( aFormSize, MapMode(MAP_100TH_MM) );
        }

        // take window borders into account
        awt::DeviceInfo aDeviceInfo;
        Window* pWindow = GetDlgEditor()->GetWindow();
        Reference< awt::XControl > xDlg( GetUnoControl( pWindow ), UNO_QUERY );
        if ( xDlg.is() )
        {
            Reference< awt::XDevice > xDev( xDlg->getPeer(), UNO_QUERY );
            if ( xDev.is() )
                aDeviceInfo = xDev->getInfo();
        }
        aFormSize.Width() -= aDeviceInfo.LeftInset + aDeviceInfo.RightInset;
        aFormSize.Height() -= aDeviceInfo.TopInset + aDeviceInfo.BottomInset;

        // convert pixel to logic units
        if( pDevice )
        {
            aFormPos = pDevice->PixelToLogic( aFormPos, MapMode(MAP_APPFONT) );
            aFormSize = pDevice->PixelToLogic( aFormSize, MapMode(MAP_APPFONT) );
        }

        // set properties
        Any aValue;
        aValue <<= aFormPos.Width();
        xPSetForm->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "PositionX" ) ), aValue );
        aValue <<= aFormPos.Height();
        xPSetForm->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "PositionY" ) ), aValue );
        aValue <<= aFormSize.Width();
        xPSetForm->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Width" ) ), aValue );
        aValue <<= aFormSize.Height();
        xPSetForm->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Height" ) ), aValue );
    }

    //StartListening();
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
                xPSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TabIndex" ) ) ) >>= nTabIndex;

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
                xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "TabIndex" ) ), aTabIndex );
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


