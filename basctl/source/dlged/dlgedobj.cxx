/*************************************************************************
 *
 *  $RCSfile: dlgedobj.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: tbe $ $Date: 2001-03-23 16:12:17 $
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

#include "vcsbxdef.hxx"

#ifndef _TOOLS_RESMGR_HXX
#include <tools/resmgr.hxx>
#endif

#ifndef _SVDIO_HXX //autogen
#include <svx/svdio.hxx>
#endif

#ifndef _SHL_HXX
#include <tools/shl.hxx>
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


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::script;
using namespace ::rtl;

//----------------------------------------------------------------------------

// helper class for sorting by tabindex
class TabSortHelper
{
public:
    ::rtl::OUString name;
    sal_Int16       tabindex;

    bool operator<(const TabSortHelper& rComp) const { return tabindex < rComp.tabindex ? true : false; }
};

//----------------------------------------------------------------------------

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
    // DIRTY HACK!!!
    // I need the window borders from MT
    //
    // window borders in pixel
    sal_Int32 nLeftBorder_px = 3;
    sal_Int32 nRightBorder_px = 3;
    sal_Int32 nTopBorder_px = 22;
    sal_Int32 nBottomBorder_px = 3;

    if ( !ISA(DlgEdForm) )
    {
        // get property sets
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xPSet(GetUnoControlModel(), ::com::sun::star::uno::UNO_QUERY);
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xPSetForm(GetDlgEdForm()->GetUnoControlModel(), ::com::sun::star::uno::UNO_QUERY);

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
            aPos.Width() += aFormPos.Width() + nLeftBorder_px;
            aPos.Height() += aFormPos.Height() + nTopBorder_px;

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
    else if ( ISA(DlgEdForm) )
    {
        // get property set
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xPSetForm(GetUnoControlModel(), ::com::sun::star::uno::UNO_QUERY);

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
            aFormSize.Width() += nLeftBorder_px + nRightBorder_px;
            aFormSize.Height() += nTopBorder_px + nBottomBorder_px;

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

    /* old method
    //
    // get property set
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xPSet(GetUnoControlModel(), ::com::sun::star::uno::UNO_QUERY);
    if (xPSet.is())
    {
        // get position and size from properties
        sal_Int32 nX, nY, nWidth, nHeight;
        xPSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PositionX" ) ) ) >>= nX;
        xPSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PositionY" ) ) ) >>= nY;
        xPSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Width" ) ) ) >>= nWidth;
        xPSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Height" ) ) ) >>= nHeight;

        Size aPos( nX, nY );
        Size aSize( nWidth, nHeight );

        // convert logic units
        OutputDevice* pDevice = Application::GetDefaultDevice();
        DBG_ASSERT( pDevice, "Missing Default Device!" );
        if( pDevice )
        {
            aPos = pDevice->LogicToPixel( aPos, MapMode(MAP_APPFONT) );
            aPos = pDevice->PixelToLogic( aPos, MapMode(MAP_100TH_MM) );
            aSize = pDevice->LogicToPixel( aSize, MapMode(MAP_APPFONT) );
            aSize = pDevice->PixelToLogic( aSize, MapMode(MAP_100TH_MM) );
        }

        Point aPoint;
        aPoint.X() = aPos.Width();
        aPoint.Y() = aPos.Height();

        if ( !ISA(DlgEdForm) )
        {
            Point aFormPos = (GetDlgEdForm()->GetSnapRect()).TopLeft();
            aPoint += aFormPos;
        }

        // set rectangle position and size
        SetSnapRect( Rectangle( aPoint, aSize ) );
    }
    */
}

//----------------------------------------------------------------------------

void DlgEdObj::SetPropsFromRect()
{
    // DIRTY HACK!!!
    // I need the window borders from MT
    //
    // window borders in pixel
    sal_Int32 nLeftBorder_px = 3;
    sal_Int32 nRightBorder_px = 3;
    sal_Int32 nTopBorder_px = 22;
    sal_Int32 nBottomBorder_px = 3;

    if ( !ISA(DlgEdForm) )
    {
        //EndListening(sal_False);

        // get control property set
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xPSet(GetUnoControlModel(), ::com::sun::star::uno::UNO_QUERY);
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
            aPos.Width() -= aFormPos.Width() + nLeftBorder_px;
            aPos.Height() -= aFormPos.Height() + nTopBorder_px;

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

        //StartListening();
    }
    else if ( ISA(DlgEdForm) )
    {
        //EndListening(sal_False);

        // get control property set
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xPSetForm(GetUnoControlModel(), ::com::sun::star::uno::UNO_QUERY);
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
            aFormSize.Width() -= nLeftBorder_px + nRightBorder_px;
            aFormSize.Height() -= nTopBorder_px + nBottomBorder_px;

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

    /* old method
    //
    EndListening(sal_False);

    // get property set
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xPSet(GetUnoControlModel(), ::com::sun::star::uno::UNO_QUERY);
    if (xPSet.is())
    {
        // get position and size from rectangle
        Rectangle aRect = GetSnapRect();
        Point aPoint = aRect.TopLeft();
        Size aSize = aRect.GetSize();

        if ( !ISA(DlgEdForm) )
        {
            Point aFormPos = (GetDlgEdForm()->GetSnapRect()).TopLeft();
            aPoint -= aFormPos;
        }

        Size aPos;
        aPos.Width() = aPoint.X();
        aPos.Height() = aPoint.Y();

        // convert logic units
        OutputDevice* pDevice = Application::GetDefaultDevice();
        DBG_ASSERT( pDevice, "Missing Default Device!" );
        if( pDevice )
        {
            aPos = pDevice->LogicToPixel( aPos, MapMode(MAP_100TH_MM) );
            aPos = pDevice->PixelToLogic( aPos, MapMode(MAP_APPFONT) );
            aSize = pDevice->LogicToPixel( aSize, MapMode(MAP_100TH_MM) );
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

    StartListening();
    */
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

    // remove the control by the old name and insert the control by the new name in the container
    uno::Reference< container::XNameAccess > xNameAcc((GetDlgEdForm()->GetUnoControlModel()), uno::UNO_QUERY);
    if ( xNameAcc.is() && xNameAcc->hasByName(aOldName) )
    {
        uno::Reference< container::XNameContainer > xCont(xNameAcc, uno::UNO_QUERY );
        if ( xCont.is() )
        {
            uno::Reference< awt::XControlModel > xCtrl(GetUnoControlModel(), uno::UNO_QUERY);
            uno::Any aAny;
            aAny <<= xCtrl;
            xCont->removeByName( aOldName );
            xCont->insertByName( aNewName , aAny );
        }
    }

    // sort the controls by tabindex
    GetDlgEdForm()->SortByTabIndex();
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

void DlgEdObj::SetStep( sal_Int32 nStep )
{
    // set step property
    uno::Reference< beans::XPropertySet > xPSet( GetUnoControlModel(), uno::UNO_QUERY );
    if (xPSet.is())
    {
        uno::Any aStep;
        aStep <<= nStep;
        xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Step" ) ), aStep );
    }
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

void SAL_CALL DlgEdObj::TabIndexChange( const  ::com::sun::star::beans::PropertyChangeEvent& evt ) throw( ::com::sun::star::uno::RuntimeException)
{
    // stop listening with all childs
    ::std::vector<DlgEdObj*> aChildList = GetDlgEdForm()->GetChilds();
    ::std::vector<DlgEdObj*>::iterator aIter;
    for ( aIter = aChildList.begin() ; aIter != aChildList.end() ; aIter++ )
    {
        (*aIter)->EndListening(sal_False);
    }

    Reference< ::com::sun::star::container::XNameAccess > xNameAcc( GetDlgEdForm()->GetUnoControlModel() , UNO_QUERY );
    if ( xNameAcc.is() )
    {
        // get sequence of control names
        Sequence< ::rtl::OUString > aNames = xNameAcc->getElementNames();
        const ::rtl::OUString* pNames = aNames.getConstArray();
        sal_Int32 nCtrls = aNames.getLength();
        ::std::vector<::rtl::OUString> aNameList(nCtrls);

        // fill helper list
        for ( sal_Int16 i = 0; i < nCtrls; i++ )
        {
            aNameList[i] = pNames[i];
        }

        // check tabindex
        sal_Int16 nOldTabIndex;
        evt.OldValue >>= nOldTabIndex;
        sal_Int16 nNewTabIndex;
        evt.NewValue >>= nNewTabIndex;
        if (nNewTabIndex < 0)
        {
            nNewTabIndex = 0;
        }
        else if (nNewTabIndex > nCtrls - 1)
        {
            nNewTabIndex = nCtrls - 1;
        }

        // reorder helper list
        ::rtl::OUString aCtrlName = aNameList[nOldTabIndex];
        aNameList.erase( aNameList.begin() + nOldTabIndex );
        aNameList.insert( aNameList.begin() + nNewTabIndex , aCtrlName );

        // sort name container by tabindex
        Reference< container::XNameContainer > xCont( xNameAcc, UNO_QUERY );
        for ( i = 0; i < nCtrls; i++ )
        {
            // get control model
            ::rtl::OUString aName = aNameList[i];
            Any aCtrl = xNameAcc->getByName( aName );

            // set new tabindex
            Reference< ::com::sun::star::beans::XPropertySet > xPSet;
               aCtrl >>= xPSet;
            if (xPSet.is())
            {
                Any aTabIndex;
                aTabIndex <<= (sal_Int16) i;
                xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "TabIndex" ) ), aTabIndex );
            }

            // sort name container
            xCont->removeByName( aName );
            xCont->insertByName( aName , aCtrl );
        }
    }

    // start listening with all childs
    for ( aIter = aChildList.begin() ; aIter != aChildList.end() ; aIter++ )
    {
        (*aIter)->StartListening();
    }
}

//----------------------------------------------------------------------------

::rtl::OUString DlgEdObj::GetServiceName() const
{
    ::rtl::OUString aServiceName;
    Reference< lang::XServiceInfo > xServiceInfo( GetUnoControlModel() , UNO_QUERY );

    if (xServiceInfo.is())
    {
        Sequence< ::rtl::OUString > aServiceNames( xServiceInfo->getSupportedServiceNames() );
        DBG_ASSERT( aServiceNames.getLength() == 1 , "DlgEdObj: aServiceNames.getLength() != 1" );
        aServiceName = aServiceNames.getArray()[ 0 ];
    }

    return aServiceName;
}

//----------------------------------------------------------------------------

::rtl::OUString DlgEdObj::GetDefaultName() const
{
    sal_uInt16 nResId = 0;
    ::rtl::OUString aDefaultName;
    ::rtl::OUString aServiceName = GetServiceName();

    if (aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.awt.UnoControlDialogModel") ))
    {
        nResId = RID_STR_CLASS_DIALOG;
    }
    else if (aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.awt.UnoControlButtonModel") ))
    {
        nResId = RID_STR_CLASS_BUTTON;
    }
    else if (aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.awt.UnoControlRadioButtonModel") ))
    {
        nResId = RID_STR_CLASS_RADIOBUTTON;
    }
    else if (aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.awt.UnoControlCheckBoxModel") ))
    {
        nResId = RID_STR_CLASS_CHECKBOX;
    }
    else if (aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.awt.UnoControlListBoxModel") ))
    {
        nResId = RID_STR_CLASS_LISTBOX;
    }
    else if (aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.awt.UnoControlComboBoxModel") ))
    {
        nResId = RID_STR_CLASS_COMBOBOX;
    }
    else if (aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.awt.UnoControlGroupBoxModel") ))
    {
        nResId = RID_STR_CLASS_GROUPBOX;
    }
    else if (aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.awt.UnoControlEditModel") ))
    {
        nResId = RID_STR_CLASS_EDIT;
    }
    else if (aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.awt.UnoControlFixedTextModel") ))
    {
        nResId = RID_STR_CLASS_FIXEDTEXT;
    }
    else if (aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.awt.UnoControlImageControlModel") ))
    {
        nResId = RID_STR_CLASS_IMAGECONTROL;
    }

    if (nResId)
    {
        aDefaultName = ::rtl::OUString( IDEResId(nResId) );
    }

    return aDefaultName;

    /* old version

    String aStr(aServiceName);

    sal_Unicode pDelims[] = { '.' , 0 };
    xub_StrLen nPos = aStr.SearchCharBackward( pDelims );
    if( nPos != STRING_NOTFOUND )
    {
        aStr.Erase( 0, ++nPos );
    }

    aStr.SearchAndReplaceAscii("UnoControl", UniString() );
    aStr.SearchAndReplaceAscii("Model", UniString() );

    return aStr;
    */
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
    return VCSbxInventor;
}

//----------------------------------------------------------------------------

sal_uInt16 DlgEdObj::GetObjIdentifier() const
{
    ::rtl::OUString aServiceName = GetServiceName();

    if (aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.awt.UnoControlDialogModel") ))
    {
        return OBJ_DIALOG;
    }
    else if (aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.awt.UnoControlButtonModel") ))
    {
        return OBJ_DLG_PUSHBUTTON;
    }
    else if (aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.awt.UnoControlRadioButtonModel") ))
    {
        return OBJ_DLG_RADIOBUTTON;
    }
    else if (aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.awt.UnoControlCheckBoxModel") ))
    {
        return OBJ_DLG_CHECKBOX;
    }
    else if (aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.awt.UnoControlListBoxModel") ))
    {
        return OBJ_DLG_LISTBOX;
    }
    else if (aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.awt.UnoControlComboBoxModel") ))
    {
        return OBJ_DLG_COMBOBOX;
    }
    else if (aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.awt.UnoControlGroupBoxModel") ))
    {
        return OBJ_DLG_GROUPBOX;
    }
    else if (aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.awt.UnoControlEditModel") ))
    {
        return OBJ_DLG_EDIT;
    }
    else if (aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.awt.UnoControlFixedTextModel") ))
    {
        return OBJ_DLG_FIXEDTEXT;
    }
    else if (aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.awt.UnoControlImageControlModel") ))
    {
        return OBJ_DLG_PREVIEW;
    }
    else
    {
        return 0;
    }
}

//----------------------------------------------------------------------------

void DlgEdObj::clonedFrom(const DlgEdObj* _pSource) // not working yet
{
}

//----------------------------------------------------------------------------

SdrObject* DlgEdObj::Clone() const  // not working yet
{
    SdrObject* pReturn = SdrUnoObj::Clone();

    DlgEdObj* pFormObject = PTR_CAST(DlgEdObj, pReturn);
    DBG_ASSERT(pFormObject != NULL, "DlgEdObj::Clone : invalid clone !");
    if (pFormObject)
        pFormObject->clonedFrom(this);

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

void DlgEdObj::operator= (const SdrObject& rObj)    // not working yet
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
    SetPropsFromRect();
}

//----------------------------------------------------------------------------

void DlgEdObj::NbcResize(const Point& rRef, const Fraction& xFract, const Fraction& yFract)
{
    SdrUnoObj::NbcResize( rRef, xFract, yFract );
    SetPropsFromRect();
}

//----------------------------------------------------------------------------

FASTBOOL DlgEdObj::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    sal_Bool bResult = SdrUnoObj::EndCreate(rStat, eCmd);

    // stop listening
    EndListening(sal_False);

    // set parent form
    pDlgEdForm = ((DlgEdPage*)GetPage())->GetDlgEdForm();

    // add child to parent form
    pDlgEdForm->AddChild(this);

    // get unique name
    ::rtl::OUString aOUniqueName( GetUniqueName() );

    // set name property
    uno::Reference< beans::XPropertySet > xPSet( GetUnoControlModel(), uno::UNO_QUERY );
    uno::Any aUniqueName;
    aUniqueName <<= aOUniqueName;
    xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Name" ) ), aUniqueName );

    // set labels
    ::rtl::OUString aServiceName = GetServiceName();
    if (aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.awt.UnoControlButtonModel") ) ||
        aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.awt.UnoControlRadioButtonModel") ) ||
        aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.awt.UnoControlCheckBoxModel") ) ||
        aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.awt.UnoControlGroupBoxModel") ) ||
        aServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.awt.UnoControlFixedTextModel") ) )
    {
        xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Label" ) ), aUniqueName );
    }

    // set geometry properties
    SetPropsFromRect();

    // set tabindex
    uno::Reference< container::XNameAccess > xNameAcc( (GetDlgEdForm()->GetUnoControlModel()) , uno::UNO_QUERY );
       Sequence< OUString > aNames = xNameAcc->getElementNames();
    uno::Any aTabIndex;
    aTabIndex <<= (sal_Int16) aNames.getLength();
    xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "TabIndex" ) ), aTabIndex );

    // insert control model in dialog model
    uno::Reference< container::XNameContainer > xC( xNameAcc , uno::UNO_QUERY );
    uno::Reference< awt::XControlModel > xCtrl( xPSet , uno::UNO_QUERY );
    uno::Any aAny;
    aAny <<= xCtrl;
    xC->insertByName( aOUniqueName , aAny );

    // dialog model changed
    GetDlgEdForm()->GetDlgEditor()->SetDialogModelChanged(TRUE);

    // start listening
    StartListening();

    return bResult;
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
            SetRectFromProps();
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

void DlgEdForm::SortByTabIndex()
{
    // stop listening with all childs
    ::std::vector<DlgEdObj*>::iterator aIter;
    for ( aIter = pChilds.begin() ; aIter != pChilds.end() ; aIter++ )
    {
        (*aIter)->EndListening(sal_False);
    }

    Reference< ::com::sun::star::container::XNameAccess > xNameAcc( GetUnoControlModel() , UNO_QUERY );
    if ( xNameAcc.is() )
    {
        // get sequence of control names
        Sequence< ::rtl::OUString > aNames = xNameAcc->getElementNames();
        const ::rtl::OUString* pNames = aNames.getConstArray();
        sal_Int32 nCtrls = aNames.getLength();
        ::std::vector<TabSortHelper> aTabSortList(nCtrls);

        for ( sal_Int16 i = 0; i < nCtrls; i++ )
        {
            // name
            TabSortHelper aTabSortHelper;
            aTabSortHelper.name = pNames[i];

            // tabindex
            Any aCtrl = xNameAcc->getByName( pNames[i] );
            Reference< ::com::sun::star::beans::XPropertySet > xPSet;
               aCtrl >>= xPSet;
            if (xPSet.is())
            {
                xPSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TabIndex" ) ) ) >>= aTabSortHelper.tabindex;
            }

            // insert element into helper list
            aTabSortList[i] = aTabSortHelper;
        }

        // sort helper list by tabindex
        ::std::sort( aTabSortList.begin() , aTabSortList.end() );

        // sort name container by tabindex
        Reference< container::XNameContainer > xCont( xNameAcc, UNO_QUERY );

        for ( i = 0; i < nCtrls; i++ )
        {
            // get control model
            ::rtl::OUString aName = aTabSortList[i].name;
            Any aCtrl = xNameAcc->getByName( aName );

            // set new tabindex
            Reference< ::com::sun::star::beans::XPropertySet > xPSet;
               aCtrl >>= xPSet;
            if (xPSet.is())
            {
                Any aTabIndex;
                aTabIndex <<= (sal_Int16) i;
                xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "TabIndex" ) ), aTabIndex );
            }

            // sort name container
            xCont->removeByName( aName );
            xCont->insertByName( aName , aCtrl );
        }
    }

    // start listening with all childs
    for ( aIter = pChilds.begin() ; aIter != pChilds.end() ; aIter++ )
    {
        (*aIter)->StartListening();
    }
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


