/*************************************************************************
 *
 *  $RCSfile: dlgedobj.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2001-02-27 14:50:51 $
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

#ifndef _BASCTL_DLGEDOBJ_HXX
#include "dlgedobj.hxx"
#endif

#ifndef _SVX_FMPROP_HXX
//#include "fmprop.hxx"
#endif

#ifndef _TOOLS_RESMGR_HXX
#include <tools/resmgr.hxx>
#endif

#ifndef _SVX_FMPROP_HRC
//#include "fmprop.hrc"
#endif

#ifndef _SVDIO_HXX //autogen
#include <svx/svdio.hxx>
#endif

#ifndef _SVX_FMTOOLS_HXX
//#include "fmtools.hxx"
#endif

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

#ifndef _SVX_FMRESIDS_HRC
//#include "fmresids.hrc"
#endif

#ifndef _SVX_FMGLOB_HXX
#include <svx/fmglob.hxx>
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

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#include "vcsbxdef.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::rtl;

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

DlgEdObj::DlgEdObj(DlgEdForm* pForm)
          :SdrUnoObj(String(), sal_False)
          ,bIsListening(sal_False)
          ,pDlgEdForm(pForm)
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

DlgEdObj::DlgEdObj(const ::rtl::OUString& rModelName, DlgEdForm* pForm)
          :SdrUnoObj(rModelName, sal_False)
          ,bIsListening(sal_False)
          ,pDlgEdForm(pForm)
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

DlgEdObj::DlgEdObj(const ::rtl::OUString& rModelName,
                   const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxSFac,
                   DlgEdForm* pForm)
          :SdrUnoObj(rModelName, rxSFac, sal_False)
          ,bIsListening(sal_False)
          ,pDlgEdForm(pForm)
{
    DBG_CTOR(DlgEdObj, NULL);
}

//----------------------------------------------------------------------------

DlgEdObj::~DlgEdObj()
{
    DBG_DTOR(DlgEdObj, NULL);

    EndPropertyListening();
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
}

//----------------------------------------------------------------------------

void DlgEdObj::SetPropsFromRect()
{
    EndPropertyListening(sal_False);

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

    StartPropertyListening();
}

//----------------------------------------------------------------------------

sal_uInt32 DlgEdObj::GetObjInventor()   const
{
    /*
    if( GetModel() && ((FmFormModel*)GetModel())->IsStreamingOldVersion() )
        return SdrInventor;
    return FmFormInventor;
    */
    return VCSbxInventor;
}

//----------------------------------------------------------------------------

sal_uInt16 DlgEdObj::GetObjIdentifier() const
{
    /*
    if( GetModel() && ((FmFormModel*)GetModel())->IsStreamingOldVersion() )
        return OBJ_RECT;
    return OBJ_FM_CONTROL;
    */
    return OBJ_FM_CONTROL;  // change this
}

//----------------------------------------------------------------------------

void DlgEdObj::clonedFrom(const DlgEdObj* _pSource)
{
}

//----------------------------------------------------------------------------

SdrObject* DlgEdObj::Clone() const
{
    SdrObject* pReturn = SdrUnoObj::Clone();

    DlgEdObj* pFormObject = PTR_CAST(DlgEdObj, pReturn);
    DBG_ASSERT(pFormObject != NULL, "DlgEdObj::Clone : invalid clone !");
    if (pFormObject)
        pFormObject->clonedFrom(this);

    return pReturn;
}

//----------------------------------------------------------------------------

SdrObject* DlgEdObj::Clone(SdrPage* _pPage, SdrModel* _pModel) const
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

void DlgEdObj::WriteData(SvStream& rOut) const
{
    /*
    FmFormModel* pModel = (FmFormModel*)GetModel();
    if( pModel && pModel->IsStreamingOldVersion() )
    {
        SdrLayerID nOld = GetLayer();
        ((FmFormObj*)this)->NbcSetLayer( pModel->GetControlExportLayerId( *this ) );
        SdrUnoObj::WriteData( rOut );
        ((FmFormObj*)this)->NbcSetLayer( nOld );
        return;
    }
    */
    SdrUnoObj::WriteData(rOut);
}

//----------------------------------------------------------------------------

void DlgEdObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
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

    // set properties
    SetPropsFromRect();

    // insert control model in dialog model
    uno::Reference< container::XNameContainer > xC((GetDlgEdForm()->GetUnoControlModel()), uno::UNO_QUERY);
    uno::Reference< awt::XControlModel > xCtrl(GetUnoControlModel(), uno::UNO_QUERY);
    uno::Any aAny;
    aAny <<= xCtrl;
    xC->insertByName( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("newcontrol" ) ), aAny );    // name handling missing!!!

    return bResult;
}

//----------------------------------------------------------------------------

IMPL_LINK(DlgEdObj, OnCreate, void*, EMPTYTAG)
{
    /*
    nEvent = 0;
    if (pTempView)
        pTempView->ObjectCreated(this);
    */
    return 0;
}

//----------------------------------------------------------------------------

void SAL_CALL DlgEdObj::_propertyChange( const  ::com::sun::star::beans::PropertyChangeEvent& evt ) throw( ::com::sun::star::uno::RuntimeException)
{
    if (isListening())
    {
        // set rectangle, if geometry information in the model changed
        if ( evt.PropertyName == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Width")) ||
             evt.PropertyName == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Height")) ||
             evt.PropertyName == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PositionX")) ||
             evt.PropertyName == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PositionY")) )
        {
            SetRectFromProps();
        }
    }
}

//----------------------------------------------------------------------------

void DlgEdObj::StartPropertyListening()
{
    DBG_ASSERT(!isListening(), "DlgEdObj::StartPropertyListening: already listening!");

    if (!isListening())
    {
        Reference< XPropertySet > xControlModel(GetUnoControlModel(), UNO_QUERY);

        if (!m_xListener.is() && xControlModel.is())
        {
            // create listener
            m_xListener = static_cast< ::com::sun::star::beans::XPropertyChangeListener*>( new DlgEdListenerImpl( (DlgEdObj*)this ) );

            // register listener to properties
            xControlModel->addPropertyChangeListener(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Width")), m_xListener);
            xControlModel->addPropertyChangeListener(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Height")), m_xListener);
            xControlModel->addPropertyChangeListener(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PositionX")), m_xListener);
            xControlModel->addPropertyChangeListener(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PositionY")), m_xListener);
        }

        bIsListening = sal_True;
    }
}

//----------------------------------------------------------------------------

void DlgEdObj::EndPropertyListening(sal_Bool bRemoveListener)
{
    DBG_ASSERT(isListening(), "DlgEdObj::EndPropertyListening: not listening currently!");

    if (isListening())
    {
        bIsListening = sal_False;

        if (bRemoveListener)
        {
            Reference< XPropertySet > xControlModel(GetUnoControlModel(), UNO_QUERY);
            if (m_xListener.is() && xControlModel.is())
            {
                // remove listener
                xControlModel->removePropertyChangeListener(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Width")), m_xListener);
                xControlModel->removePropertyChangeListener(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Height")), m_xListener);
                xControlModel->removePropertyChangeListener(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PositionX")), m_xListener);
                xControlModel->removePropertyChangeListener(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PositionY")), m_xListener);
            }

            m_xListener.clear();
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

    SetPropsFromRect();

    return bResult;
}

//----------------------------------------------------------------------------


