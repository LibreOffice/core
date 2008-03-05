/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RptObject.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:59:50 $
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
#include "precompiled_reportdesign.hxx"
#ifndef _REPORT_RPTUIOBJ_HXX
#include "RptObject.hxx"
#endif
#include <vector>
#include <algorithm>

#ifndef REPORTDESIGN_SHARED_CORESTRINGS_HRC
#include "corestrings.hrc"
#endif
#ifndef _REPORT_RPTUIDEF_HXX
#include <RptDef.hxx>
#endif
#ifndef _SVX_UNOSHAPE_HXX
#include <svx/unoshape.hxx>
#endif
#ifndef REPORT_RPTMODEL_HXX
#include "RptModel.hxx"
#endif
#ifndef _REPORT_RPTUILIST_HXX
#include "RptObjectListener.hxx"
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif
#ifndef _TOOLKIT_HELPER_CONVERT_HXX_
#include <toolkit/helper/convert.hxx>
#endif
#ifndef _REPORT_RPTUIPAGE_HXX
#include "RptPage.hxx"
#endif
#ifndef REPORTDESIGN_SHARED_CORESTRINGS_HRC
#include "corestrings.hrc"
#endif
#ifndef DBAUI_SINGLEDOCCONTROLLER_HXX
#include <dbaccess/singledoccontroller.hxx>
#endif
#ifndef _RPTUI_MODULE_HELPER_RPT_HXX_
#include "ModuleHelper.hxx"
#endif

#ifndef _REPORT_DLGRESID_HRC
#include <RptResId.hrc>
#endif

#ifndef _SVX_XFLCLIT_HXX
#include <svx/xflclit.hxx>
#endif
#ifndef _SVX_XLNCLIT_HXX
#include <svx/xlnclit.hxx>
#endif
#ifndef _SVX_XLNDSIT_HXX
#include <svx/xlndsit.hxx>
#endif
#ifndef _SVX_XLINEIT0_HXX
#include <svx/xlineit0.hxx>
#endif
#ifndef _SDERITM_HXX
#include <svx/sderitm.hxx>
#endif
#ifndef _SVX_XLNWTIT_HXX
#include <svx/xlnwtit.hxx>
#endif
#ifndef _SVX_XLNTRIT_HXX
#include <svx/xlntrit.hxx>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLE_HPP_
#include <com/sun/star/style/XStyle.hpp>
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
#ifndef _COMPHELPER_GENERICPROPERTYSET_HXX_
#include <comphelper/genericpropertyset.hxx>
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
#ifndef _COM_SUN_STAR_REPORT_XSHAPE_HPP_
#include <com/sun/star/report/XShape.hpp>
#endif
#include <com/sun/star/report/XFixedLine.hpp>
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef RPTUI_PROPERTYSETFORWARD_HXX
#include "PropertyForward.hxx"
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef RPTUI_UNDOACTIONS_HXX
#include "UndoActions.hxx"
#endif
#include "UndoEnv.hxx"
#include <algorithm>
#include <functional>

namespace rptui
{

using namespace ::com::sun::star;
using namespace uno;
using namespace beans;
using namespace reportdesign;
using namespace container;
using namespace script;
//----------------------------------------------------------------------------
sal_uInt16 OObjectBase::getObjectType(const uno::Reference< report::XReportComponent>& _xComponent)
{
    uno::Reference< lang::XServiceInfo > xServiceInfo( _xComponent , uno::UNO_QUERY );
    OSL_ENSURE(xServiceInfo.is(),"Who deletes the XServiceInfo interface!");
    if ( xServiceInfo.is() )
    {
        if ( xServiceInfo->supportsService( SERVICE_FIXEDTEXT ))
            return OBJ_DLG_FIXEDTEXT;
        if ( xServiceInfo->supportsService( SERVICE_FIXEDLINE ))
        {
            uno::Reference< report::XFixedLine> xFixedLine(_xComponent,uno::UNO_QUERY);
            return xFixedLine->getOrientation() ? OBJ_DLG_HFIXEDLINE : OBJ_DLG_VFIXEDLINE;
        }
        if ( xServiceInfo->supportsService( SERVICE_IMAGECONTROL))
            return OBJ_DLG_IMAGECONTROL;
        if ( xServiceInfo->supportsService( SERVICE_FORMATTEDFIELD ))
            return OBJ_DLG_FORMATTEDFIELD;
        if ( xServiceInfo->supportsService( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.OLE2Shape")) ) )
            return OBJ_OLE2;
        if ( xServiceInfo->supportsService( SERVICE_SHAPE ))
            return OBJ_CUSTOMSHAPE;
        if ( xServiceInfo->supportsService( SERVICE_REPORTDEFINITION ) )
            return OBJ_DLG_SUBREPORT;
        return OBJ_OLE2;
    }
    return 0;
}
// -----------------------------------------------------------------------------
SdrObject* OObjectBase::createObject(const uno::Reference< report::XReportComponent>& _xComponent)
{
    SdrObject* pNewObj = NULL;
    sal_uInt16 nType = OObjectBase::getObjectType(_xComponent);
    switch( nType )
    {
        case OBJ_DLG_FIXEDTEXT:
            pNewObj = new OUnoObject( _xComponent
                                    ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.form.component.FixedText"))
                                    ,OBJ_DLG_FIXEDTEXT);
            break;
        case OBJ_DLG_IMAGECONTROL:
            pNewObj = new OUnoObject(_xComponent
                                    ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.form.component.DatabaseImageControl"))
                                    ,OBJ_DLG_IMAGECONTROL);
            break;
        case OBJ_DLG_FORMATTEDFIELD:
            pNewObj = new OUnoObject( _xComponent
                                    ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.form.component.FormattedField"))
                                    ,OBJ_DLG_FORMATTEDFIELD);
            break;
        case OBJ_DLG_HFIXEDLINE:
        case OBJ_DLG_VFIXEDLINE:
            pNewObj = new OUnoObject( _xComponent
                                    ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlFixedLineModel"))
                                    ,nType);
            break;
        case OBJ_CUSTOMSHAPE:
            pNewObj = OCustomShape::Create( _xComponent );
            break;
        case OBJ_DLG_SUBREPORT:
        case OBJ_OLE2:
            pNewObj = OOle2Obj::Create( _xComponent );
            break;
        default:
            OSL_ENSURE(0,"Unknown object id");
            break;
    }

    ensureSdrObjectOwnership( _xComponent );

    return pNewObj;
}
// -----------------------------------------------------------------------------
const TPropertyNamePair& getPropertyNameMap(sal_uInt16 _nObjectId)
{
    switch(_nObjectId)
    {
        case OBJ_DLG_IMAGECONTROL:
            {
                static TPropertyNamePair s_aNameMap;
                if ( s_aNameMap.empty() )
                {
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_CONTROLBACKGROUND,PROPERTY_BACKGROUNDCOLOR));
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_CONTROLBORDER,PROPERTY_BORDER));
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_CONTROLBORDERCOLOR,PROPERTY_BORDERCOLOR));
                    //s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_PARAADJUST,PROPERTY_ALIGN));
                }
                return s_aNameMap;
            }

        case OBJ_DLG_FIXEDTEXT:
        case OBJ_DLG_FORMATTEDFIELD:
            {
                static TPropertyNamePair s_aNameMap;
                if ( s_aNameMap.empty() )
                {
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_CHARCOLOR,PROPERTY_TEXTCOLOR));
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_CONTROLBACKGROUND,PROPERTY_BACKGROUNDCOLOR));
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_CHARUNDERLINECOLOR,PROPERTY_TEXTLINECOLOR));
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_CHARRELIEF,PROPERTY_FONTRELIEF));
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_CHARFONTHEIGHT,PROPERTY_FONTHEIGHT));
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_CHARSTRIKEOUT,PROPERTY_FONTSTRIKEOUT));
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_CONTROLTEXTEMPHASISMARK,PROPERTY_FONTEMPHASISMARK));
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_CONTROLBORDER,PROPERTY_BORDER));
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_CONTROLBORDERCOLOR,PROPERTY_BORDERCOLOR));
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_PARAADJUST,PROPERTY_ALIGN));
                }
                return s_aNameMap;
            }

        case OBJ_CUSTOMSHAPE:
            {
                static TPropertyNamePair s_aNameMap;
                if ( s_aNameMap.empty() )
                {
                    s_aNameMap.insert(TPropertyNamePair::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FillColor")),PROPERTY_CONTROLBACKGROUND));
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_PARAADJUST,PROPERTY_ALIGN));
                }
                return s_aNameMap;
            }

        default:
            break;
    }
    static TPropertyNamePair s_aEmptyNameMap;
    return s_aEmptyNameMap;
}
// -----------------------------------------------------------------------------

DBG_NAME( rpt_OObjectBase )
OObjectBase::OObjectBase(const uno::Reference< report::XReportComponent>& _xComponent)
:m_bIsListening(sal_False)
{
    DBG_CTOR( rpt_OObjectBase,NULL);
    m_xReportComponent = _xComponent;
}
//----------------------------------------------------------------------------
OObjectBase::OObjectBase(const ::rtl::OUString& _sComponentName)
:m_sComponentName(_sComponentName)
,m_bIsListening(sal_False)
{
    DBG_CTOR( rpt_OObjectBase,NULL);
}
//----------------------------------------------------------------------------
OObjectBase::~OObjectBase()
{
    DBG_DTOR( rpt_OObjectBase,NULL);
    m_xMediator.reset();
    if ( isListening() )
        EndListening();
    m_xReportComponent.clear();
}
// -----------------------------------------------------------------------------
uno::Reference< report::XSection> OObjectBase::getSection() const
{
    uno::Reference< report::XSection> xSection;
    OReportPage* pPage = dynamic_cast<OReportPage*>(GetImplPage());
    if ( pPage )
        xSection = pPage->getSection();
    return xSection;
}
// -----------------------------------------------------------------------------
uno::Reference< report::XReportComponent> OObjectBase::getReportComponent() const
{
    return m_xReportComponent;
}
// -----------------------------------------------------------------------------
uno::Reference< beans::XPropertySet> OObjectBase::getAwtComponent()
{
    return uno::Reference< beans::XPropertySet>();
}
// -----------------------------------------------------------------------------
sal_Bool OObjectBase::IsInside(const Rectangle& _rRect,const Point& rPnt,USHORT nTol) const
{
    sal_Bool bRet = sal_False;
    Rectangle aRect( _rRect );
    if ( !aRect.IsEmpty() )
    {
        aRect.Left() -= nTol;
        aRect.Top() -= nTol;
        aRect.Right() = ( aRect.Right() == RECT_EMPTY ? _rRect.Left() + nTol : aRect.Right() + nTol );
        aRect.Bottom() = ( aRect.Bottom() == RECT_EMPTY ? _rRect.Top() + nTol : aRect.Bottom() + nTol );

        bRet = aRect.IsInside( rPnt );
    }
    return bRet;
}
//----------------------------------------------------------------------------
void OObjectBase::StartListening()
{
    DBG_CHKTHIS( rpt_OObjectBase,NULL);
    OSL_ENSURE(!isListening(), "OUnoObject::StartListening: already listening!");

    if ( !isListening() && m_xReportComponent.is() )
    {
        m_bIsListening = sal_True;

        if ( !m_xPropertyChangeListener.is() )
        {
            m_xPropertyChangeListener = new OObjectListener( this );
            // register listener to all properties
            m_xReportComponent->addPropertyChangeListener( ::rtl::OUString() , m_xPropertyChangeListener );
        }
    }
}
//----------------------------------------------------------------------------
void OObjectBase::EndListening(sal_Bool /*bRemoveListener*/)
{
    DBG_CHKTHIS( rpt_OObjectBase,NULL);
    OSL_ENSURE(!m_xReportComponent.is() || isListening(), "OUnoObject::EndListening: not listening currently!");

    if ( isListening() && m_xReportComponent.is() )
    {
        m_bIsListening = sal_False;

        // XPropertyChangeListener
        if ( m_xPropertyChangeListener.is() )
        {
            // remove listener
            try
            {
                m_xReportComponent->removePropertyChangeListener( ::rtl::OUString() , m_xPropertyChangeListener );
            }
            catch(uno::Exception)
            {
                OSL_ENSURE(0,"OObjectBase::EndListening: Exception caught!");
            }
        }
        m_xPropertyChangeListener.clear();
    }
}
//----------------------------------------------------------------------------
void OObjectBase::SetPropsFromRect(const Rectangle& _rRect)
{
    DBG_CHKTHIS( rpt_OObjectBase,NULL);
    // set properties
    OReportPage* pPage = dynamic_cast<OReportPage*>(GetImplPage());
    if ( pPage && !_rRect.IsEmpty() )
    {
        uno::Reference<report::XSection> xSection = pPage->getSection();
        if ( xSection.is() && (static_cast<sal_uInt32>(_rRect.getHeight() + _rRect.Top()) > xSection->getHeight()) )
            xSection->setHeight(_rRect.getHeight() + _rRect.Top());

        // TODO
        //pModel->GetRefDevice()->Invalidate(INVALIDATE_CHILDREN);
    }
}
//----------------------------------------------------------------------------
void OObjectBase::PositionAndSizeChange( const beans::PropertyChangeEvent& evt )
{
    DBG_CHKTHIS( rpt_OObjectBase,NULL);
    OReportPage* pPage = dynamic_cast<OReportPage*>(GetImplPage());
    OSL_ENSURE( pPage, "OUnoObject::PositionAndSizeChange: no page!" );
    if ( pPage )
    {
        sal_Int32 nPageX = 0;
        sal_Int32 nPageY = 0;
        Size aPageSize = pPage->GetSize();
        sal_Int32 nPageWidth = aPageSize.Width();
        sal_Int32 nPageHeight = aPageSize.Height();
        if ( m_xReportComponent.is() )
        {
            sal_Int32 nX = m_xReportComponent->getPositionX();
            sal_Int32 nY = m_xReportComponent->getPositionY();
            sal_Int32 nWidth = m_xReportComponent->getWidth();
            sal_Int32 nHeight = m_xReportComponent->getHeight();

            sal_Int32 nValue = 0;
            evt.NewValue >>= nValue;
            sal_Int32 nNewValue = nValue;

            if ( evt.PropertyName == PROPERTY_POSITIONX )
            {
                if ( nNewValue + nWidth > nPageX + nPageWidth )
                    nNewValue = nPageX + nPageWidth - nWidth;
                if ( nNewValue < nPageX )
                    nNewValue = nPageX;
            }
            else if ( evt.PropertyName == PROPERTY_POSITIONY )
            {
                if ( nNewValue + nHeight > nPageY + nPageHeight )
                    nNewValue = nPageY + nPageHeight - nHeight;
                if ( nNewValue < nPageY )
                    nNewValue = nPageY;
            }
            else if ( evt.PropertyName == PROPERTY_WIDTH )
            {
                if ( nX + nNewValue > nPageX + nPageWidth )
                    nNewValue = nPageX + nPageWidth - nX;
                if ( nNewValue < 1 )
                    nNewValue = 1;
            }
            else if ( evt.PropertyName == PROPERTY_HEIGHT )
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
                m_xReportComponent->setPropertyValue( evt.PropertyName, aNewValue );
                StartListening();
            }
        }
    }
}
//----------------------------------------------------------------------------
void OObjectBase::_propertyChange( const  beans::PropertyChangeEvent& /*evt*/ ) throw( uno::RuntimeException)
{
    DBG_CHKTHIS( rpt_OObjectBase,NULL);
}
//----------------------------------------------------------------------------
void OObjectBase::SetObjectItemHelper(const SfxPoolItem& /*rItem*/)
{
    // do nothing
}

//----------------------------------------------------------------------------
void OObjectBase::_elementInserted(const container::ContainerEvent& /*Event*/) throw(uno::RuntimeException)
{
    DBG_CHKTHIS( rpt_OObjectBase,NULL);
}

//----------------------------------------------------------------------------
void OObjectBase::_elementReplaced(const container::ContainerEvent& /*Event*/) throw(uno::RuntimeException)
{
    DBG_CHKTHIS( rpt_OObjectBase,NULL);
}

//----------------------------------------------------------------------------
void OObjectBase::_elementRemoved(const container::ContainerEvent& /*Event*/) throw(uno::RuntimeException)
{
    DBG_CHKTHIS( rpt_OObjectBase,NULL);
}

//----------------------------------------------------------------------------
sal_Bool OObjectBase::supportsService( const ::rtl::OUString& _sServiceName ) const
{
    DBG_CHKTHIS( rpt_OObjectBase,NULL);
    sal_Bool bSupports = sal_False;

    Reference< lang::XServiceInfo > xServiceInfo( m_xReportComponent , UNO_QUERY );
        // TODO: cache xServiceInfo as member?
    if ( xServiceInfo.is() )
        bSupports = xServiceInfo->supportsService( _sServiceName );

    return bSupports;
}

//----------------------------------------------------------------------------
void OObjectBase::ensureSdrObjectOwnership( const uno::Reference< uno::XInterface >& _rxShape )
{
    // UNDO in the report designer is implemented at the level of the XShapes, not
    // at the level of SdrObjects. That is, if an object is removed from the report
    // design, then this happens by removing the XShape from the UNO DrawPage, and
    // putting this XShape (resp. the ReportComponent which wraps it) into an UNDO
    // action.
    // Unfortunately, the SvxDrawPage implementation usually deletes SdrObjects
    // which are removed from it, which is deadly for us. To prevent this,
    // we give the XShape implementation the ownership of the SdrObject, which
    // ensures the SvxDrawPage won't delete it.
    SvxShape* pShape = SvxShape::getImplementation( _rxShape );
    OSL_ENSURE( pShape, "OObjectBase::ensureSdrObjectOwnership: can't access the SvxShape!" );
    if ( pShape )
    {
        OSL_ENSURE( !pShape->HasSdrObjectOwnership(), "OObjectBase::ensureSdrObjectOwnership: called twice?" );
        pShape->TakeSdrObjectOwnership();
    }
}

//----------------------------------------------------------------------------
uno::Reference< uno::XInterface > OObjectBase::getUnoShapeOf( SdrObject& _rSdrObject )
{
    uno::Reference< uno::XInterface > xShape( _rSdrObject.getWeakUnoShape() );
    if ( xShape.is() )
        return xShape;

    xShape = _rSdrObject.SdrObject::getUnoShape();
    if ( !xShape.is() )
        return xShape;

    ensureSdrObjectOwnership( xShape );

    m_xKeepShapeAlive = xShape;
    return xShape;
}

//----------------------------------------------------------------------------
TYPEINIT1(OCustomShape, SdrObjCustomShape);
DBG_NAME( rpt_OCustomShape );
OCustomShape::OCustomShape(const uno::Reference< report::XReportComponent>& _xComponent
                           )
          :SdrObjCustomShape()
          ,OObjectBase(_xComponent)
{
    DBG_CTOR( rpt_OCustomShape, NULL);
    // start listening
    mxUnoShape = uno::Reference< uno::XInterface >(_xComponent,uno::UNO_QUERY);
    m_bIsListening = sal_True;
}
//----------------------------------------------------------------------------
OCustomShape::OCustomShape(const ::rtl::OUString& _sComponentName)
          :SdrObjCustomShape()
          ,OObjectBase(_sComponentName)
{
    DBG_CTOR( rpt_OCustomShape, NULL);
    m_bIsListening = sal_True;
}

//----------------------------------------------------------------------------
OCustomShape::~OCustomShape()
{
    DBG_DTOR( rpt_OCustomShape, NULL);
    //mxUnoShape = uno::WeakReference< uno::XInterface >();
}
//----------------------------------------------------------------------------
SdrPage* OCustomShape::GetImplPage() const
{
    return GetPage();
}
//----------------------------------------------------------------------------
void OCustomShape::SetSnapRectImpl(const Rectangle& _rRect)
{
    SetSnapRect( _rRect );
}
//----------------------------------------------------------------------------
sal_Int32 OCustomShape::GetStep() const
{
    // get step property
    sal_Int32 nStep = 0;
    OSL_ENSURE(0,"Who called me!");
    return nStep;
}
//----------------------------------------------------------------------------
void OCustomShape::NbcMove( const Size& rSize )
{
    if ( m_bIsListening )
    {
        m_bIsListening = sal_False;

        if ( m_xReportComponent.is() )
        {
            OReportModel* pRptModel = static_cast<OReportModel*>(GetModel());
            OXUndoEnvironment::OUndoEnvLock aLock(pRptModel->GetUndoEnv());
            m_xReportComponent->setPositionX(m_xReportComponent->getPositionX() + rSize.A());
            m_xReportComponent->setPositionY(m_xReportComponent->getPositionY() + rSize.B());
        }

        // set geometry properties
        SetPropsFromRect(GetSnapRect());

        m_bIsListening = sal_True;
    }
    else
        SdrObjCustomShape::NbcMove( rSize );
}
//----------------------------------------------------------------------------
void OCustomShape::NbcResize(const Point& rRef, const Fraction& xFract, const Fraction& yFract)
{
    SdrObjCustomShape::NbcResize( rRef, xFract, yFract );

    SetPropsFromRect(GetSnapRect());
}
//----------------------------------------------------------------------------
void OCustomShape::NbcSetLogicRect(const Rectangle& rRect)
{
    SdrObjCustomShape::NbcSetLogicRect(rRect);
    SetPropsFromRect(rRect);
}
//----------------------------------------------------------------------------

FASTBOOL OCustomShape::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    FASTBOOL bResult = SdrObjCustomShape::EndCreate(rStat, eCmd);
    if ( bResult )
    {
        OReportModel* pRptModel = static_cast<OReportModel*>(GetModel());
        if ( pRptModel )
        {
            OXUndoEnvironment::OUndoEnvLock aLock(pRptModel->GetUndoEnv());
            if ( !m_xReportComponent.is() )
                m_xReportComponent.set(getUnoShape(),uno::UNO_QUERY);
        }
        SetPropsFromRect(GetSnapRect());
    }

    return bResult;
}
//----------------------------------------------------------------------------
SdrObject* OCustomShape::CheckHit( const Point& rPnt, USHORT nTol,const SetOfByte* pSet ) const
{
    // #109994# fixed here, because the drawing layer doesn't handle objects
    // with a width or height of 0 in a proper way
    if ( IsInside(aOutRect,rPnt,nTol) )
        return const_cast<OCustomShape*>(this);

    return SdrObjCustomShape::CheckHit( rPnt, nTol, pSet );
}

//----------------------------------------------------------------------------
void OCustomShape::SetObjectItemHelper(const SfxPoolItem& rItem)
{
    SetObjectItem(rItem);
    // TODO
    //getSectionWindow()->getView()->AdjustMarkHdl();
}

// -----------------------------------------------------------------------------
uno::Reference< beans::XPropertySet> OCustomShape::getAwtComponent()
{
    return uno::Reference< beans::XPropertySet>(m_xReportComponent,uno::UNO_QUERY);
}

//----------------------------------------------------------------------------
uno::Reference< uno::XInterface > OCustomShape::getUnoShape()
{
    return OObjectBase::getUnoShapeOf( *this );
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
TYPEINIT1(OUnoObject, SdrUnoObj);
DBG_NAME( rpt_OUnoObject );
OUnoObject::OUnoObject(const uno::Reference< report::XReportComponent>& _xComponent
                       ,const uno::Reference< awt::XControlModel >& _xControlModel
                       ,sal_uInt16   _nObjectType)
          :SdrUnoObj(String(), sal_False)
          ,OObjectBase(_xComponent)
          ,m_nObjectType(_nObjectType)
{
    DBG_CTOR( rpt_OUnoObject, NULL);
    SetUnoControlModel( _xControlModel );
    mxUnoShape = uno::Reference< uno::XInterface >(_xComponent,uno::UNO_QUERY);
    //CreateMediator();
}
//----------------------------------------------------------------------------
OUnoObject::OUnoObject(const ::rtl::OUString& _sComponentName
                       ,const ::rtl::OUString& rModelName
                       ,sal_uInt16   _nObjectType)
          :SdrUnoObj(rModelName, sal_True)
          ,OObjectBase(_sComponentName)
          ,m_nObjectType(_nObjectType)
{
    DBG_CTOR( rpt_OUnoObject, NULL);
}
//----------------------------------------------------------------------------
OUnoObject::OUnoObject(const uno::Reference< report::XReportComponent>& _xComponent
                       ,const ::rtl::OUString& rModelName
                       ,sal_uInt16   _nObjectType)
          :SdrUnoObj(rModelName, sal_True)
          ,OObjectBase(_xComponent)
          ,m_nObjectType(_nObjectType)
{
    DBG_CTOR( rpt_OUnoObject, NULL);
    mxUnoShape = uno::Reference< uno::XInterface >(_xComponent,uno::UNO_QUERY);
}
//----------------------------------------------------------------------------
OUnoObject::~OUnoObject()
{
    DBG_DTOR( rpt_OUnoObject, NULL);
    //mxUnoShape = uno::WeakReference< uno::XInterface >();
}
//----------------------------------------------------------------------------
SdrPage* OUnoObject::GetImplPage() const
{
    DBG_CHKTHIS( rpt_OUnoObject,NULL);
    return GetPage();
}
//----------------------------------------------------------------------------
void OUnoObject::SetSnapRectImpl(const Rectangle& _rRect)
{
    DBG_CHKTHIS( rpt_OUnoObject,NULL);
    SetSnapRect( _rRect );
}
//----------------------------------------------------------------------------
sal_Int32 OUnoObject::GetStep() const
{
    DBG_CHKTHIS( rpt_OUnoObject,NULL);
    // get step property
    sal_Int32 nStep = 0;
    OSL_ENSURE(0,"Who called me!");
    return nStep;
}

//----------------------------------------------------------------------------
void OUnoObject::NbcMove( const Size& rSize )
{
    DBG_CHKTHIS( rpt_OUnoObject,NULL);

    if ( m_bIsListening )
    {
        // stop listening
        OObjectBase::EndListening(sal_False);

        if ( m_xReportComponent.is() )
        {
            OReportModel* pRptModel = static_cast<OReportModel*>(GetModel());
            OXUndoEnvironment::OUndoEnvLock aLock(pRptModel->GetUndoEnv());
            m_xReportComponent->setPositionX(m_xReportComponent->getPositionX() + rSize.A());
            m_xReportComponent->setPositionY(m_xReportComponent->getPositionY() + rSize.B());
        }

        // set geometry properties
        SetPropsFromRect(GetLogicRect());

        // start listening
        OObjectBase::StartListening();
    }
    else
        SdrUnoObj::NbcMove( rSize );
}

//----------------------------------------------------------------------------

void OUnoObject::NbcResize(const Point& rRef, const Fraction& xFract, const Fraction& yFract)
{
    DBG_CHKTHIS( rpt_OUnoObject,NULL);
    SdrUnoObj::NbcResize( rRef, xFract, yFract );

    // stop listening
    OObjectBase::EndListening(sal_False);

    // set geometry properties
    SetPropsFromRect(GetLogicRect());

    // start listening
    OObjectBase::StartListening();
}
//----------------------------------------------------------------------------
void OUnoObject::NbcSetLogicRect(const Rectangle& rRect)
{
    SdrUnoObj::NbcSetLogicRect(rRect);
    // stop listening
    OObjectBase::EndListening(sal_False);

    // set geometry properties
    SetPropsFromRect(rRect);

    // start listening
    OObjectBase::StartListening();
}
//----------------------------------------------------------------------------

FASTBOOL OUnoObject::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    DBG_CHKTHIS( rpt_OUnoObject,NULL);
    FASTBOOL bResult = SdrUnoObj::EndCreate(rStat, eCmd);
    if ( bResult )
    {
        OReportModel* pRptModel = static_cast<OReportModel*>(GetModel());
        if ( pRptModel )
        {
            OXUndoEnvironment::OUndoEnvLock aLock(pRptModel->GetUndoEnv());
            if ( !m_xReportComponent.is() )
                m_xReportComponent.set(getUnoShape(),uno::UNO_QUERY);
            // set labels
            if ( m_xReportComponent.is() )
            {
                try
                {
                    if ( supportsService( SERVICE_FIXEDTEXT ) )
                        m_xReportComponent->setPropertyValue( PROPERTY_LABEL, uno::makeAny(GetDefaultName(this)) );
                }
                catch(const uno::Exception&)
                {
                    OSL_ENSURE(0,"OUnoObject::EndCreate: Exception caught!");
                }
            }
        }
        // set geometry properties
        SetPropsFromRect(GetLogicRect());
    }

    return bResult;
}
//----------------------------------------------------------------------------
::rtl::OUString OUnoObject::GetDefaultName(const OUnoObject* _pObj)
{
    sal_uInt16 nResId = 0;
    ::rtl::OUString aDefaultName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HERE WE HAVE TO INSERT OUR NAME!"));
    if ( _pObj->supportsService( SERVICE_FIXEDTEXT ) )
    {
        nResId = RID_STR_CLASS_FIXEDTEXT;
    }
    else if ( _pObj->supportsService( SERVICE_FIXEDLINE ) )
    {
        nResId = RID_STR_CLASS_FIXEDLINE;
    }
    else if ( _pObj->supportsService( SERVICE_IMAGECONTROL ) )
    {
        nResId = RID_STR_CLASS_IMAGECONTROL;
    }
    else if ( _pObj->supportsService( SERVICE_FORMATTEDFIELD ) )
    {
        nResId = RID_STR_CLASS_FORMATTEDFIELD;
    }

    if (nResId)
        aDefaultName = ::rtl::OUString( String(ModuleRes(nResId)) );

    return aDefaultName;
}
//----------------------------------------------------------------------------
SdrObject* OUnoObject::CheckHit( const Point& rPnt, USHORT nTol,const SetOfByte* pSet ) const
{
    DBG_CHKTHIS( rpt_OUnoObject,NULL);
    if ( IsInside(aOutRect,rPnt,nTol) )
        return const_cast<OUnoObject*>(this);

    return SdrUnoObj::CheckHit( rPnt, nTol, pSet );
}
// -----------------------------------------------------------------------------
void OUnoObject::_propertyChange( const  beans::PropertyChangeEvent& evt ) throw( uno::RuntimeException)
{
    DBG_CHKTHIS( rpt_OUnoObject,NULL);
    OObjectBase::_propertyChange(evt);
    if (isListening())
    {
        if ( evt.PropertyName == PROPERTY_CHARCOLOR )
        {
            Reference<XPropertySet> xControlModel(GetUnoControlModel(),uno::UNO_QUERY);
            if ( xControlModel.is() )
            {
                OObjectBase::EndListening(sal_False);
                try
                {
                    xControlModel->setPropertyValue(PROPERTY_TEXTCOLOR,evt.NewValue);
                }
                catch(uno::Exception&)
                {
                }
                OObjectBase::StartListening();
            }
        }
        else if ( evt.PropertyName == PROPERTY_NAME )
        {
            Reference<XPropertySet> xControlModel(GetUnoControlModel(),uno::UNO_QUERY);
            if ( xControlModel.is() && xControlModel->getPropertySetInfo()->hasPropertyByName(PROPERTY_NAME) )
            {
                // get old name
                ::rtl::OUString aOldName;
                evt.OldValue >>= aOldName;

                // get new name
                ::rtl::OUString aNewName;
                evt.NewValue >>= aNewName;

                if ( !aNewName.equals(aOldName) )
                {
                    // set old name property
                    OObjectBase::EndListening(sal_False);
                    if ( m_xMediator.is() )
                        m_xMediator.get()->stopListening();
                    try
                    {
                        xControlModel->setPropertyValue( PROPERTY_NAME, evt.NewValue );
                    }
                    catch(uno::Exception&)
                    {
                    }
                    if ( m_xMediator.is() )
                        m_xMediator.get()->startListening();
                    OObjectBase::StartListening();
                }
            }
        }
    }
}
// -----------------------------------------------------------------------------
void OUnoObject::CreateMediator(sal_Bool _bReverse)
{
    if ( !m_xMediator.is() )
    {
        if ( !m_xReportComponent.is() )
            m_xReportComponent.set(getUnoShape(),uno::UNO_QUERY);

        Reference<XPropertySet> xControlModel(GetUnoControlModel(),uno::UNO_QUERY);
        if ( !m_xMediator.is() && m_xReportComponent.is() && xControlModel.is() )
            m_xMediator = TMediator::createFromQuery(new OPropertyMediator(m_xReportComponent.get(),xControlModel,getPropertyNameMap(getObjectId()),_bReverse));
        OObjectBase::StartListening();
    }
}
// -----------------------------------------------------------------------------
uno::Reference< beans::XPropertySet> OUnoObject::getAwtComponent()
{
    return Reference<XPropertySet>(GetUnoControlModel(),uno::UNO_QUERY);
}

// -----------------------------------------------------------------------------
uno::Reference< uno::XInterface > OUnoObject::getUnoShape()
{
    return OObjectBase::getUnoShapeOf( *this );
}
//----------------------------------------------------------------------------
// OOle2Obj
//----------------------------------------------------------------------------
TYPEINIT1(OOle2Obj, SdrOle2Obj);
DBG_NAME( rpt_OOle2Obj );
OOle2Obj::OOle2Obj(const uno::Reference< report::XReportComponent>& _xComponent)
          :SdrOle2Obj()
          ,OObjectBase(_xComponent)
{
    DBG_CTOR( rpt_OOle2Obj, NULL);

    // start listening
    mxUnoShape = uno::Reference< uno::XInterface >(_xComponent,uno::UNO_QUERY);
    m_bIsListening = sal_True;
    //uno::Reference< embed::XEmbeddedObject > xEmbed(_xComponent,uno::UNO_QUERY);
    //OSL_ENSURE(xEmbed.is(),"This is no embedded object!");

    //SetObjRef(xEmbed);
}
//----------------------------------------------------------------------------
OOle2Obj::OOle2Obj(const ::rtl::OUString& _sComponentName)
          :SdrOle2Obj()
          ,OObjectBase(_sComponentName)
{
    DBG_CTOR( rpt_OOle2Obj, NULL);
}
// -----------------------------------------------------------------------------
OOle2Obj::OOle2Obj(const ::rtl::OUString& _sComponentName,const svt::EmbeddedObjectRef& rNewObjRef, const String& rNewObjName, const Rectangle& rNewRect, FASTBOOL bFrame_)
          :SdrOle2Obj(rNewObjRef,rNewObjName,rNewRect,bFrame_)
          ,OObjectBase(_sComponentName)
{
}
//----------------------------------------------------------------------------
OOle2Obj::~OOle2Obj()
{
    DBG_DTOR( rpt_OOle2Obj, NULL);
}
//----------------------------------------------------------------------------
SdrPage* OOle2Obj::GetImplPage() const
{
    DBG_CHKTHIS( rpt_OOle2Obj,NULL);
    return GetPage();
}
//----------------------------------------------------------------------------
void OOle2Obj::SetSnapRectImpl(const Rectangle& _rRect)
{
    DBG_CHKTHIS( rpt_OOle2Obj,NULL);
    SetSnapRect( _rRect );
}
//----------------------------------------------------------------------------
sal_Int32 OOle2Obj::GetStep() const
{
    DBG_CHKTHIS( rpt_OOle2Obj,NULL);
    // get step property
    sal_Int32 nStep = 0;
    OSL_ENSURE(0,"Who called me!");
    return nStep;
}

//----------------------------------------------------------------------------
void OOle2Obj::NbcMove( const Size& rSize )
{
    DBG_CHKTHIS( rpt_OOle2Obj,NULL);

    if ( m_bIsListening )
    {
        // stop listening
        OObjectBase::EndListening(sal_False);

        if ( m_xReportComponent.is() )
        {
            OReportModel* pRptModel = static_cast<OReportModel*>(GetModel());
            OXUndoEnvironment::OUndoEnvLock aLock(pRptModel->GetUndoEnv());
            m_xReportComponent->setPositionX(m_xReportComponent->getPositionX() + rSize.A());
            m_xReportComponent->setPositionY(m_xReportComponent->getPositionY() + rSize.B());
        }

        // set geometry properties
        SetPropsFromRect(GetLogicRect());

        // start listening
        OObjectBase::StartListening();
    }
    else
        SdrOle2Obj::NbcMove( rSize );
}

//----------------------------------------------------------------------------

void OOle2Obj::NbcResize(const Point& rRef, const Fraction& xFract, const Fraction& yFract)
{
    DBG_CHKTHIS( rpt_OOle2Obj,NULL);
    SdrOle2Obj::NbcResize( rRef, xFract, yFract );

    // stop listening
    OObjectBase::EndListening(sal_False);

    // set geometry properties
    SetPropsFromRect(GetLogicRect());

    // start listening
    OObjectBase::StartListening();
}
//----------------------------------------------------------------------------
void OOle2Obj::NbcSetLogicRect(const Rectangle& rRect)
{
    SdrOle2Obj::NbcSetLogicRect(rRect);
    // stop listening
    OObjectBase::EndListening(sal_False);

    // set geometry properties
    SetPropsFromRect(rRect);

    // start listening
    OObjectBase::StartListening();
}
//----------------------------------------------------------------------------

FASTBOOL OOle2Obj::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    DBG_CHKTHIS( rpt_OOle2Obj,NULL);
    FASTBOOL bResult = SdrOle2Obj::EndCreate(rStat, eCmd);
    if ( bResult )
    {
        OReportModel* pRptModel = static_cast<OReportModel*>(GetModel());
        if ( pRptModel )
        {
            OXUndoEnvironment::OUndoEnvLock aLock(pRptModel->GetUndoEnv());
            if ( !m_xReportComponent.is() )
                m_xReportComponent.set(getUnoShape(),uno::UNO_QUERY);
        }
        // set geometry properties
        SetPropsFromRect(GetLogicRect());
    }

    return bResult;
}
//----------------------------------------------------------------------------
SdrObject* OOle2Obj::CheckHit( const Point& rPnt, USHORT nTol,const SetOfByte* pSet ) const
{
    DBG_CHKTHIS( rpt_OOle2Obj,NULL);
    if ( IsInside(aOutRect,rPnt,nTol) )
        return const_cast<OOle2Obj*>(this);

    return SdrOle2Obj::CheckHit( rPnt, nTol, pSet );
}
// -----------------------------------------------------------------------------
uno::Reference< beans::XPropertySet> OOle2Obj::getAwtComponent()
{
    return uno::Reference< beans::XPropertySet>(m_xReportComponent,uno::UNO_QUERY);
}

// -----------------------------------------------------------------------------
uno::Reference< uno::XInterface > OOle2Obj::getUnoShape()
{
    uno::Reference< uno::XInterface> xShape = OObjectBase::getUnoShapeOf( *this );
    if ( !m_xReportComponent.is() )
    {
        OReportModel* pRptModel = static_cast<OReportModel*>(GetModel());
        OXUndoEnvironment::OUndoEnvLock aLock(pRptModel->GetUndoEnv());
        m_xReportComponent.set(xShape,uno::UNO_QUERY);
    }
    return xShape;
}
// -----------------------------------------------------------------------------
uno::Reference< style::XStyle> getUsedStyle(const uno::Reference< report::XReportDefinition>& _xReport)
{
    uno::Reference<container::XNameAccess> xStyles = _xReport->getStyleFamilies();
    uno::Reference<container::XNameAccess> xPageStyles(xStyles->getByName(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PageStyles"))),uno::UNO_QUERY);

    uno::Reference< style::XStyle> xReturn;
    uno::Sequence< ::rtl::OUString> aSeq = xPageStyles->getElementNames();
    const ::rtl::OUString* pIter = aSeq.getConstArray();
    const ::rtl::OUString* pEnd   = pIter + aSeq.getLength();
    for(;pIter != pEnd && !xReturn.is() ;++pIter)
    {
        uno::Reference< style::XStyle> xStyle(xPageStyles->getByName(*pIter),uno::UNO_QUERY);
        if ( xStyle->isInUse() )
            xReturn = xStyle;
    }
    return xReturn;
}
//----------------------------------------------------------------------------
//============================================================================
} // rptui
//============================================================================
