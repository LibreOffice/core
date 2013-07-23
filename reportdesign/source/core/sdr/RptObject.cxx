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
#include "RptObject.hxx"
#include <vector>
#include <algorithm>

#include <RptDef.hxx>
#include <svx/unoshape.hxx>
#include "RptModel.hxx"
#include "RptObjectListener.hxx"
#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/helper/convert.hxx>
#include "RptPage.hxx"
#include "corestrings.hrc"
#include <dbaccess/dbsubcomponentcontroller.hxx>
#include "ModuleHelper.hxx"

#include <RptResId.hrc>
#include <svx/xflclit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/sderitm.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlntrit.hxx>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/awt/XTabControllerModel.hpp>
#include <com/sun/star/awt/XUnoControlContainer.hpp>
#include <com/sun/star/awt/XVclContainerPeer.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/TextAlign.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/embed/XComponentSupplier.hpp>
#include <com/sun/star/script/XScriptEventsSupplier.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/report/XShape.hpp>
#include <com/sun/star/report/XFixedLine.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/chart2/data/DatabaseDataProvider.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/report/XFormattedField.hpp>
#include <comphelper/genericpropertyset.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/property.hxx>
#include <tools/diagnose_ex.h>
#include "PropertyForward.hxx"
#include <connectivity/dbtools.hxx>
#include <connectivity/dbconversion.hxx>
#include "UndoActions.hxx"
#include "UndoEnv.hxx"
#include <functional>

namespace rptui
{

using namespace ::com::sun::star;
using namespace uno;
using namespace beans;
using namespace reportdesign;
using namespace container;
using namespace script;
using namespace report;
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
        if ( xServiceInfo->supportsService("com.sun.star.drawing.OLE2Shape") )
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
            {
                OUnoObject* pUnoObj = new OUnoObject( _xComponent
                                    ,OUString("com.sun.star.form.component.FixedText")
                                    ,OBJ_DLG_FIXEDTEXT);
                pNewObj = pUnoObj;

                uno::Reference<beans::XPropertySet> xControlModel(pUnoObj->GetUnoControlModel(),uno::UNO_QUERY);
                if ( xControlModel.is() )
                    xControlModel->setPropertyValue( PROPERTY_MULTILINE,uno::makeAny(sal_True));
            }
            break;
        case OBJ_DLG_IMAGECONTROL:
            pNewObj = new OUnoObject(_xComponent
                                    ,OUString("com.sun.star.form.component.DatabaseImageControl")
                                    ,OBJ_DLG_IMAGECONTROL);
            break;
        case OBJ_DLG_FORMATTEDFIELD:
            pNewObj = new OUnoObject( _xComponent
                                    ,OUString("com.sun.star.form.component.FormattedField")
                                    ,OBJ_DLG_FORMATTEDFIELD);
            break;
        case OBJ_DLG_HFIXEDLINE:
        case OBJ_DLG_VFIXEDLINE:
            pNewObj = new OUnoObject( _xComponent
                                    ,OUString("com.sun.star.awt.UnoControlFixedLineModel")
                                    ,nType);
            break;
        case OBJ_CUSTOMSHAPE:
            pNewObj = OCustomShape::Create( _xComponent );
            try
            {
                sal_Bool bOpaque = sal_False;
                _xComponent->getPropertyValue(PROPERTY_OPAQUE) >>= bOpaque;
                pNewObj->NbcSetLayer(bOpaque ? RPT_LAYER_FRONT : RPT_LAYER_BACK);
            }
            catch(const uno::Exception&)
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            break;
        case OBJ_DLG_SUBREPORT:
        case OBJ_OLE2:
            pNewObj = OOle2Obj::Create( _xComponent,nType );
            break;
        default:
            OSL_FAIL("Unknown object id");
            break;
    }

    if ( pNewObj )
        pNewObj->SetDoNotInsertIntoPageAutomatically( true );

    ensureSdrObjectOwnership( _xComponent );

    return pNewObj;
}
// -----------------------------------------------------------------------------
namespace
{
    class ParaAdjust : public AnyConverter
    {
    public:
        virtual ::com::sun::star::uno::Any operator() (const OUString& _sPropertyName,const ::com::sun::star::uno::Any& lhs) const
        {
            uno::Any aRet;
            if (_sPropertyName == PROPERTY_PARAADJUST)
            {
                sal_Int16 nTextAlign = 0;
                lhs >>= nTextAlign;
                switch(nTextAlign)
                {
                    case awt::TextAlign::LEFT:
                        nTextAlign = style::ParagraphAdjust_LEFT;
                        break;
                    case awt::TextAlign::CENTER:
                        nTextAlign = style::ParagraphAdjust_CENTER;
                        break;
                    case awt::TextAlign::RIGHT:
                        nTextAlign = style::ParagraphAdjust_RIGHT;
                        break;
                    default:
                        OSL_FAIL("Illegal text alignment value!");
                        break;
                }
                aRet <<= (style::ParagraphAdjust)nTextAlign;
            }
            else
            {
                sal_Int16 nTextAlign = 0;
                sal_Int16 eParagraphAdjust = 0;
                lhs >>= eParagraphAdjust;
                switch(eParagraphAdjust)
                {
                    case style::ParagraphAdjust_LEFT:
                    case style::ParagraphAdjust_BLOCK:
                        nTextAlign = awt::TextAlign::LEFT;
                        break;
                    case style::ParagraphAdjust_CENTER:
                        nTextAlign = awt::TextAlign::CENTER;
                        break;
                    case style::ParagraphAdjust_RIGHT:
                        nTextAlign = awt::TextAlign::RIGHT;
                        break;
                    default:
                        OSL_FAIL("Illegal text alignment value!");
                        break;
                }
                aRet <<= nTextAlign;
            }
            return aRet;
        }
    };
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
                    ::boost::shared_ptr<AnyConverter> aNoConverter(new AnyConverter());
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_CONTROLBACKGROUND,TPropertyConverter(PROPERTY_BACKGROUNDCOLOR,aNoConverter)));
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_CONTROLBORDER,TPropertyConverter(PROPERTY_BORDER,aNoConverter)));
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_CONTROLBORDERCOLOR,TPropertyConverter(PROPERTY_BORDERCOLOR,aNoConverter)));
                }
                return s_aNameMap;
            }

        case OBJ_DLG_FIXEDTEXT:
            {
                static TPropertyNamePair s_aNameMap;
                if ( s_aNameMap.empty() )
                {
                    ::boost::shared_ptr<AnyConverter> aNoConverter(new AnyConverter());
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_CHARCOLOR,TPropertyConverter(PROPERTY_TEXTCOLOR,aNoConverter)));
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_CONTROLBACKGROUND,TPropertyConverter(PROPERTY_BACKGROUNDCOLOR,aNoConverter)));
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_CHARUNDERLINECOLOR,TPropertyConverter(PROPERTY_TEXTLINECOLOR,aNoConverter)));
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_CHARRELIEF,TPropertyConverter(PROPERTY_FONTRELIEF,aNoConverter)));
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_CHARFONTHEIGHT,TPropertyConverter(PROPERTY_FONTHEIGHT,aNoConverter)));
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_CHARSTRIKEOUT,TPropertyConverter(PROPERTY_FONTSTRIKEOUT,aNoConverter)));
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_CONTROLTEXTEMPHASISMARK,TPropertyConverter(PROPERTY_FONTEMPHASISMARK,aNoConverter)));
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_CONTROLBORDER,TPropertyConverter(PROPERTY_BORDER,aNoConverter)));
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_CONTROLBORDERCOLOR,TPropertyConverter(PROPERTY_BORDERCOLOR,aNoConverter)));

                    ::boost::shared_ptr<AnyConverter> aParaAdjust(new ParaAdjust());
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_PARAADJUST,TPropertyConverter(PROPERTY_ALIGN,aParaAdjust)));
                }
                return s_aNameMap;
            }
        case OBJ_DLG_FORMATTEDFIELD:
            {
                static TPropertyNamePair s_aNameMap;
                if ( s_aNameMap.empty() )
                {
                    ::boost::shared_ptr<AnyConverter> aNoConverter(new AnyConverter());
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_CHARCOLOR,TPropertyConverter(PROPERTY_TEXTCOLOR,aNoConverter)));
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_CONTROLBACKGROUND,TPropertyConverter(PROPERTY_BACKGROUNDCOLOR,aNoConverter)));
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_CHARUNDERLINECOLOR,TPropertyConverter(PROPERTY_TEXTLINECOLOR,aNoConverter)));
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_CHARRELIEF,TPropertyConverter(PROPERTY_FONTRELIEF,aNoConverter)));
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_CHARFONTHEIGHT,TPropertyConverter(PROPERTY_FONTHEIGHT,aNoConverter)));
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_CHARSTRIKEOUT,TPropertyConverter(PROPERTY_FONTSTRIKEOUT,aNoConverter)));
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_CONTROLTEXTEMPHASISMARK,TPropertyConverter(PROPERTY_FONTEMPHASISMARK,aNoConverter)));
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_CONTROLBORDER,TPropertyConverter(PROPERTY_BORDER,aNoConverter)));
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_CONTROLBORDERCOLOR,TPropertyConverter(PROPERTY_BORDERCOLOR,aNoConverter)));
                    ::boost::shared_ptr<AnyConverter> aParaAdjust(new ParaAdjust());
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_PARAADJUST,TPropertyConverter(PROPERTY_ALIGN,aParaAdjust)));
                }
                return s_aNameMap;
            }

        case OBJ_CUSTOMSHAPE:
            {
                static TPropertyNamePair s_aNameMap;
                if ( s_aNameMap.empty() )
                {
                    ::boost::shared_ptr<AnyConverter> aNoConverter(new AnyConverter());
                    s_aNameMap.insert(TPropertyNamePair::value_type(OUString("FillColor"),TPropertyConverter(PROPERTY_CONTROLBACKGROUND,aNoConverter)));
                    s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_PARAADJUST,TPropertyConverter(PROPERTY_ALIGN,aNoConverter)));
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
OObjectBase::OObjectBase(const OUString& _sComponentName)
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
            m_xReportComponent->addPropertyChangeListener( OUString() , m_xPropertyChangeListener );
        }
    }
}
//----------------------------------------------------------------------------
void OObjectBase::EndListening(sal_Bool /*bRemoveListener*/)
{
    DBG_CHKTHIS( rpt_OObjectBase,NULL);
    OSL_ENSURE(!m_xReportComponent.is() || isListening(), "OUnoObject::EndListening: not listening currently!");

    m_bIsListening = sal_False;
    if ( isListening() && m_xReportComponent.is() )
    {
        // XPropertyChangeListener
        if ( m_xPropertyChangeListener.is() )
        {
            // remove listener
            try
            {
                m_xReportComponent->removePropertyChangeListener( OUString() , m_xPropertyChangeListener );
            }
            catch(const uno::Exception &)
            {
                OSL_FAIL("OObjectBase::EndListening: Exception caught!");
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
        assert(_rRect.getHeight() >= 0);
        const sal_uInt32 newHeight( ::std::max(0l, _rRect.getHeight()+_rRect.Top()) );
        if ( xSection.is() && ( newHeight > xSection->getHeight() ) )
            xSection->setHeight( newHeight );

        // TODO
        //pModel->GetRefDevice()->Invalidate(INVALIDATE_CHILDREN);
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
sal_Bool OObjectBase::supportsService( const OUString& _sServiceName ) const
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
    impl_setUnoShape( uno::Reference< uno::XInterface >(_xComponent,uno::UNO_QUERY) );
    m_bIsListening = sal_True;
}
//----------------------------------------------------------------------------
OCustomShape::OCustomShape(const OUString& _sComponentName)
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
}
// -----------------------------------------------------------------------------
sal_uInt16 OCustomShape::GetObjIdentifier() const
{
    return sal_uInt16(OBJ_CUSTOMSHAPE);
}
//----------------------------------------------------------------------------
sal_uInt32 OCustomShape::GetObjInventor() const
{
    return ReportInventor;
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
    OSL_FAIL("Who called me!");
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
bool OCustomShape::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    bool bResult = SdrObjCustomShape::EndCreate(rStat, eCmd);
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
    uno::Reference< uno::XInterface> xShape = OObjectBase::getUnoShapeOf( *this );
    if ( !m_xReportComponent.is() )
    {
        OReportModel* pRptModel = static_cast<OReportModel*>(GetModel());
        OXUndoEnvironment::OUndoEnvLock aLock(pRptModel->GetUndoEnv());
        m_xReportComponent.set(xShape,uno::UNO_QUERY);
    }
    return xShape;
}

void OCustomShape::impl_setUnoShape( const uno::Reference< uno::XInterface >& rxUnoShape )
{
    SdrObjCustomShape::impl_setUnoShape( rxUnoShape );
    releaseUnoShape();
    m_xReportComponent.clear();
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
TYPEINIT1(OUnoObject, SdrUnoObj);
DBG_NAME( rpt_OUnoObject );
//----------------------------------------------------------------------------
OUnoObject::OUnoObject(const OUString& _sComponentName
                       ,const OUString& rModelName
                       ,sal_uInt16   _nObjectType)
          :SdrUnoObj(rModelName, sal_True)
          ,OObjectBase(_sComponentName)
          ,m_nObjectType(_nObjectType)
{
    DBG_CTOR( rpt_OUnoObject, NULL);
    if ( !rModelName.isEmpty() )
        impl_initializeModel_nothrow();
}
//----------------------------------------------------------------------------
OUnoObject::OUnoObject(const uno::Reference< report::XReportComponent>& _xComponent
                       ,const OUString& rModelName
                       ,sal_uInt16   _nObjectType)
          :SdrUnoObj(rModelName, sal_True)
          ,OObjectBase(_xComponent)
          ,m_nObjectType(_nObjectType)
{
    DBG_CTOR( rpt_OUnoObject, NULL);
    impl_setUnoShape( uno::Reference< uno::XInterface >( _xComponent, uno::UNO_QUERY ) );

    if ( !rModelName.isEmpty() )
        impl_initializeModel_nothrow();

}
//----------------------------------------------------------------------------
OUnoObject::~OUnoObject()
{
    DBG_DTOR( rpt_OUnoObject, NULL);
}
// -----------------------------------------------------------------------------
void OUnoObject::impl_initializeModel_nothrow()
{
    try
    {
        Reference< XFormattedField > xFormatted( m_xReportComponent, UNO_QUERY );
        if ( xFormatted.is() )
        {
            const Reference< XPropertySet > xModelProps( GetUnoControlModel(), UNO_QUERY_THROW );
            const OUString sTreatAsNumberProperty = OUString( "TreatAsNumber" );
            xModelProps->setPropertyValue( sTreatAsNumberProperty, makeAny( sal_False ) );
            xModelProps->setPropertyValue( PROPERTY_VERTICALALIGN,m_xReportComponent->getPropertyValue(PROPERTY_VERTICALALIGN));
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}
// -----------------------------------------------------------------------------
void OUnoObject::impl_setReportComponent_nothrow()
{
    if ( m_xReportComponent.is() )
        return;

    OReportModel* pReportModel = static_cast<OReportModel*>(GetModel());
    OSL_ENSURE( pReportModel, "OUnoObject::impl_setReportComponent_nothrow: no report model!" );
    if ( !pReportModel )
        return;

    OXUndoEnvironment::OUndoEnvLock aLock( pReportModel->GetUndoEnv() );
    m_xReportComponent.set(getUnoShape(),uno::UNO_QUERY);

    impl_initializeModel_nothrow();
}
// -----------------------------------------------------------------------------
sal_uInt16 OUnoObject::GetObjIdentifier() const
{
    return sal_uInt16(m_nObjectType);
}
//----------------------------------------------------------------------------
sal_uInt32 OUnoObject::GetObjInventor() const
{
    return ReportInventor;
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
    OSL_FAIL("Who called me!");
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

        bool bPositionFixed = false;
        Size aUndoSize(0,0);
        bool bUndoMode = false;
        if ( m_xReportComponent.is() )
        {
            OReportModel* pRptModel = static_cast<OReportModel*>(GetModel());
            if (pRptModel->GetUndoEnv().IsUndoMode())
            {
                // if we are locked from outside, then we must not handle wrong moves, we are in UNDO mode
                bUndoMode = true;
            }
            OXUndoEnvironment::OUndoEnvLock aLock(pRptModel->GetUndoEnv());

            // LLA: why there exists getPositionX and getPositionY and NOT getPosition() which return a Point?
            int nNewX = m_xReportComponent->getPositionX() + rSize.A();
            m_xReportComponent->setPositionX(nNewX);
            int nNewY = m_xReportComponent->getPositionY() + rSize.B();
            if (nNewY < 0 && !bUndoMode)
            {
                aUndoSize.B() = abs(nNewY);
                bPositionFixed = true;
                nNewY = 0;
            }
            m_xReportComponent->setPositionY(nNewY);
        }
        if (bPositionFixed)
        {
            GetModel()->AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoMoveObject(*this, aUndoSize));
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

bool OUnoObject::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    DBG_CHKTHIS( rpt_OUnoObject,NULL);
    bool bResult = SdrUnoObj::EndCreate(rStat, eCmd);
    if ( bResult )
    {
        impl_setReportComponent_nothrow();
        // set labels
        if ( m_xReportComponent.is() )
        {
            try
            {
                if ( supportsService( SERVICE_FIXEDTEXT ) )
                {
                    m_xReportComponent->setPropertyValue( PROPERTY_LABEL, uno::makeAny(GetDefaultName(this)) );
                }
            }
            catch(const uno::Exception&)
            {
                DBG_UNHANDLED_EXCEPTION();
            }

            impl_initializeModel_nothrow();
        }
        // set geometry properties
        SetPropsFromRect(GetLogicRect());
    }

    return bResult;
}
//----------------------------------------------------------------------------
OUString OUnoObject::GetDefaultName(const OUnoObject* _pObj)
{
    sal_uInt16 nResId = 0;
    OUString aDefaultName = OUString("HERE WE HAVE TO INSERT OUR NAME!");
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
        aDefaultName = OUString( String(ModuleRes(nResId)) );

    return aDefaultName;
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
                OUString aOldName;
                evt.OldValue >>= aOldName;

                // get new name
                OUString aNewName;
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
        impl_setReportComponent_nothrow();

        Reference<XPropertySet> xControlModel(GetUnoControlModel(),uno::UNO_QUERY);
        if ( !m_xMediator.is() && m_xReportComponent.is() && xControlModel.is() )
            m_xMediator = TMediator::createFromQuery(new OPropertyMediator(m_xReportComponent.get(),xControlModel,getPropertyNameMap(GetObjIdentifier()),_bReverse));
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

void OUnoObject::impl_setUnoShape( const uno::Reference< uno::XInterface >& rxUnoShape )
{
    SdrUnoObj::impl_setUnoShape( rxUnoShape );
    releaseUnoShape();
}

OUnoObject& OUnoObject::operator=(const OUnoObject& rObj)
{
    if( this == &rObj )
        return *this;
    SdrUnoObj::operator=(rObj);

    Reference<XPropertySet> xSource(const_cast<OUnoObject&>(rObj).getUnoShape(), uno::UNO_QUERY);
    Reference<XPropertySet> xDest(getUnoShape(), uno::UNO_QUERY);
    if ( xSource.is() && xDest.is() )
        comphelper::copyProperties(xSource.get(), xDest.get());

    return *this;
}

OUnoObject* OUnoObject::Clone() const
{
    return CloneHelper< OUnoObject >();
}
//----------------------------------------------------------------------------
// OOle2Obj
//----------------------------------------------------------------------------
TYPEINIT1(OOle2Obj, SdrOle2Obj);
DBG_NAME( rpt_OOle2Obj );
OOle2Obj::OOle2Obj(const uno::Reference< report::XReportComponent>& _xComponent,sal_uInt16 _nType)
          :SdrOle2Obj()
          ,OObjectBase(_xComponent)
          ,m_nType(_nType)
          ,m_bOnlyOnce(true)
{
    DBG_CTOR( rpt_OOle2Obj, NULL);

    impl_setUnoShape( uno::Reference< uno::XInterface >( _xComponent, uno::UNO_QUERY ) );
    m_bIsListening = sal_True;
}
//----------------------------------------------------------------------------
OOle2Obj::OOle2Obj(const OUString& _sComponentName,sal_uInt16 _nType)
          :SdrOle2Obj()
          ,OObjectBase(_sComponentName)
          ,m_nType(_nType)
          ,m_bOnlyOnce(true)
{
    DBG_CTOR( rpt_OOle2Obj, NULL);
    m_bIsListening = sal_True;
}
//----------------------------------------------------------------------------
OOle2Obj::~OOle2Obj()
{
    DBG_DTOR( rpt_OOle2Obj, NULL);
}
// -----------------------------------------------------------------------------
sal_uInt16 OOle2Obj::GetObjIdentifier() const
{
    return m_nType;
}
//----------------------------------------------------------------------------
sal_uInt32 OOle2Obj::GetObjInventor() const
{
    return ReportInventor;
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
    OSL_FAIL("Who called me!");
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

        bool bPositionFixed = false;
        Size aUndoSize(0,0);
        bool bUndoMode = false;
        if ( m_xReportComponent.is() )
        {
            OReportModel* pRptModel = static_cast<OReportModel*>(GetModel());
            if (pRptModel->GetUndoEnv().IsUndoMode())
            {
                // if we are locked from outside, then we must not handle wrong moves, we are in UNDO mode
                bUndoMode = true;
            }
            OXUndoEnvironment::OUndoEnvLock aLock(pRptModel->GetUndoEnv());

            // LLA: why there exists getPositionX and getPositionY and NOT getPosition() which return a Point?
            int nNewX = m_xReportComponent->getPositionX() + rSize.A();
            // can this hinder us to set components outside the area?
            // if (nNewX < 0)
            // {
            //     nNewX = 0;
            // }
            m_xReportComponent->setPositionX(nNewX);
            int nNewY = m_xReportComponent->getPositionY() + rSize.B();
            if (nNewY < 0 && !bUndoMode)
            {
                aUndoSize.B() = abs(nNewY);
                bPositionFixed = true;
                nNewY = 0;
            }
            m_xReportComponent->setPositionY(nNewY);
        }
        if (bPositionFixed)
        {
            GetModel()->AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoMoveObject(*this, aUndoSize));
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

bool OOle2Obj::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    DBG_CHKTHIS( rpt_OOle2Obj,NULL);
    bool bResult = SdrOle2Obj::EndCreate(rStat, eCmd);
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

void OOle2Obj::impl_setUnoShape( const uno::Reference< uno::XInterface >& rxUnoShape )
{
    SdrOle2Obj::impl_setUnoShape( rxUnoShape );
    releaseUnoShape();
    m_xReportComponent.clear();
}

// -----------------------------------------------------------------------------
uno::Reference< chart2::data::XDatabaseDataProvider > lcl_getDataProvider(const uno::Reference < embed::XEmbeddedObject >& _xObj)
{
    uno::Reference< chart2::data::XDatabaseDataProvider > xSource;
    uno::Reference< embed::XComponentSupplier > xCompSupp(_xObj,uno::UNO_QUERY);
    if( xCompSupp.is())
    {
        uno::Reference< chart2::XChartDocument> xChartDoc( xCompSupp->getComponent(), uno::UNO_QUERY );
        if ( xChartDoc.is() )
        {
            xSource.set(xChartDoc->getDataProvider(),uno::UNO_QUERY);
        }
    }
    return xSource;
}

OOle2Obj& OOle2Obj::operator=(const OOle2Obj& rObj)
{
    if( this == &rObj )
        return *this;
    SdrOle2Obj::operator=(rObj);

    OReportModel* pRptModel = static_cast<OReportModel*>(rObj.GetModel());
    svt::EmbeddedObjectRef::TryRunningState( GetObjRef() );
    impl_createDataProvider_nothrow(pRptModel->getReportDefinition().get());

    uno::Reference< chart2::data::XDatabaseDataProvider > xSource( lcl_getDataProvider(rObj.GetObjRef()) );
    uno::Reference< chart2::data::XDatabaseDataProvider > xDest( lcl_getDataProvider(GetObjRef()) );
    if ( xSource.is() && xDest.is() )
        comphelper::copyProperties(xSource.get(),xDest.get());

    initializeChart(pRptModel->getReportDefinition().get());

    return *this;
}

// -----------------------------------------------------------------------------
// Clone() soll eine komplette Kopie des Objektes erzeugen.
OOle2Obj* OOle2Obj::Clone() const
{
    return CloneHelper< OOle2Obj >();
}
// -----------------------------------------------------------------------------
void OOle2Obj::impl_createDataProvider_nothrow(const uno::Reference< frame::XModel>& _xModel)
{
    try
    {
        uno::Reference < embed::XEmbeddedObject > xObj = GetObjRef();
        uno::Reference< chart2::data::XDataReceiver > xReceiver;
        uno::Reference< embed::XComponentSupplier > xCompSupp( xObj, uno::UNO_QUERY );
        if( xCompSupp.is())
            xReceiver.set( xCompSupp->getComponent(), uno::UNO_QUERY );
        OSL_ASSERT( xReceiver.is());
        if( xReceiver.is() )
        {
            uno::Reference< lang::XMultiServiceFactory> xFac(_xModel,uno::UNO_QUERY);
            uno::Reference< chart2::data::XDatabaseDataProvider > xDataProvider( xFac->createInstance("com.sun.star.chart2.data.DataProvider"),uno::UNO_QUERY);
            xReceiver->attachDataProvider( xDataProvider.get() );
        }
    }
    catch(const uno::Exception &)
    {
    }
}
// -----------------------------------------------------------------------------
void OOle2Obj::initializeOle()
{
    if ( m_bOnlyOnce )
    {
        m_bOnlyOnce = false;
        uno::Reference < embed::XEmbeddedObject > xObj = GetObjRef();
        OReportModel* pRptModel = static_cast<OReportModel*>(GetModel());
        pRptModel->GetUndoEnv().AddElement(lcl_getDataProvider(xObj));

        uno::Reference< embed::XComponentSupplier > xCompSupp( xObj, uno::UNO_QUERY );
        if( xCompSupp.is() )
        {
            uno::Reference< beans::XPropertySet > xChartProps( xCompSupp->getComponent(), uno::UNO_QUERY );
            if ( xChartProps.is() )
                xChartProps->setPropertyValue("NullDate",
                    uno::makeAny(util::DateTime(0,0,0,0,30,12,1899,false)));
        }
    }
}
// -----------------------------------------------------------------------------
void OOle2Obj::initializeChart( const uno::Reference< frame::XModel>& _xModel)
{
    uno::Reference < embed::XEmbeddedObject > xObj = GetObjRef();
    uno::Reference< chart2::data::XDataReceiver > xReceiver;
    uno::Reference< embed::XComponentSupplier > xCompSupp( xObj, uno::UNO_QUERY );
    if( xCompSupp.is())
        xReceiver.set( xCompSupp->getComponent(), uno::UNO_QUERY );
    OSL_ASSERT( xReceiver.is());
    if( xReceiver.is() )
    {
        // lock the model to suppress any internal updates
        uno::Reference< frame::XModel > xChartModel( xReceiver, uno::UNO_QUERY );
        if( xChartModel.is() )
            xChartModel->lockControllers();

        if ( !lcl_getDataProvider(xObj).is() )
            impl_createDataProvider_nothrow(_xModel);

        OReportModel* pRptModel = static_cast<OReportModel*>(GetModel());
        pRptModel->GetUndoEnv().AddElement(lcl_getDataProvider(xObj));

        ::comphelper::NamedValueCollection aArgs;
        aArgs.put( "CellRangeRepresentation", uno::makeAny( OUString( "all" ) ) );
        aArgs.put( "HasCategories", uno::makeAny( sal_True ) );
        aArgs.put( "FirstCellAsLabel", uno::makeAny( sal_True ) );
        aArgs.put( "DataRowSource", uno::makeAny( chart::ChartDataRowSource_COLUMNS ) );
        xReceiver->setArguments( aArgs.getPropertyValues() );

        if( xChartModel.is() )
            xChartModel->unlockControllers();
    }
}
// -----------------------------------------------------------------------------
uno::Reference< style::XStyle> getUsedStyle(const uno::Reference< report::XReportDefinition>& _xReport)
{
    uno::Reference<container::XNameAccess> xStyles = _xReport->getStyleFamilies();
    uno::Reference<container::XNameAccess> xPageStyles(xStyles->getByName("PageStyles"),uno::UNO_QUERY);

    uno::Reference< style::XStyle> xReturn;
    uno::Sequence< OUString> aSeq = xPageStyles->getElementNames();
    const OUString* pIter = aSeq.getConstArray();
    const OUString* pEnd   = pIter + aSeq.getLength();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
