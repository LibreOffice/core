/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#include "precompiled_reportdesign.hxx"
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
#include <svx/xlnwtit.hxx>
#include <svx/xlntrit.hxx>
#include <svx/svdlegacy.hxx>

#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/awt/XTabControllerModel.hpp>
#include <com/sun/star/awt/XUnoControlContainer.hpp>
#include <com/sun/star/awt/XVclContainerPeer.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/TextAlign.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
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
SdrObject* OObjectBase::createObject(SdrModel* pTargetModel, const uno::Reference< report::XReportComponent>& _xComponent)
{
    SdrObject* pNewObj = NULL;
    sal_uInt16 nType = OObjectBase::getObjectType(_xComponent);
    switch( nType )
    {
        case OBJ_DLG_FIXEDTEXT:
            {
                OUnoObject* pUnoObj = new OUnoObject(
                    *pTargetModel,
                    _xComponent
                    ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.form.component.FixedText"))
                    ,OBJ_DLG_FIXEDTEXT);
                SetUnoShapeAtSdrObjectFromSvxShape(*pUnoObj, _xComponent);
                pNewObj = pUnoObj;

                uno::Reference<beans::XPropertySet> xControlModel(pUnoObj->GetUnoControlModel(),uno::UNO_QUERY);
                if ( xControlModel.is() )
                    xControlModel->setPropertyValue( PROPERTY_MULTILINE,uno::makeAny(sal_True));
            }
            break;
        case OBJ_DLG_IMAGECONTROL:
            pNewObj = new OUnoObject(
                *pTargetModel,
                _xComponent
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.form.component.DatabaseImageControl"))
                ,OBJ_DLG_IMAGECONTROL);
            SetUnoShapeAtSdrObjectFromSvxShape(*pNewObj, _xComponent);
            break;
        case OBJ_DLG_FORMATTEDFIELD:
            pNewObj = new OUnoObject(
                *pTargetModel,
                _xComponent
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.form.component.FormattedField"))
                ,OBJ_DLG_FORMATTEDFIELD);
            SetUnoShapeAtSdrObjectFromSvxShape(*pNewObj, _xComponent);
            break;
        case OBJ_DLG_HFIXEDLINE:
        case OBJ_DLG_VFIXEDLINE:
            pNewObj = new OUnoObject(
                *pTargetModel,
                _xComponent
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlFixedLineModel"))
                ,nType);
            SetUnoShapeAtSdrObjectFromSvxShape(*pNewObj, _xComponent);
            break;
        case OBJ_CUSTOMSHAPE:
            pNewObj = OCustomShape::Create(
                *pTargetModel,
                _xComponent );
            try
            {
                sal_Bool bOpaque = sal_False;
                _xComponent->getPropertyValue(PROPERTY_OPAQUE) >>= bOpaque;
                pNewObj->SetLayer(bOpaque ? RPT_LAYER_FRONT : RPT_LAYER_BACK);
            }
            catch(const uno::Exception&)
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            break;
        case OBJ_DLG_SUBREPORT:
        case OBJ_OLE2:
            pNewObj = OOle2Obj::Create(
                *pTargetModel,
                _xComponent,
                nType );
            break;
        default:
            OSL_ENSURE(0,"Unknown object id");
            break;
    }

    ensureSdrObjectOwnership( _xComponent );

    return pNewObj;
}
// -----------------------------------------------------------------------------
namespace
{
    class ParaAdjust : public AnyConverter
    {
    public:
        virtual ::com::sun::star::uno::Any operator() (const ::rtl::OUString& _sPropertyName,const ::com::sun::star::uno::Any& lhs) const
        {
            uno::Any aRet;
            if ( _sPropertyName.equalsAscii(PROPERTY_PARAADJUST) )
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
                        OSL_ENSURE(0,"Illegal text alignment value!");
                        break;
                } // switch(nTextAlign)
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
                        OSL_ENSURE(0,"Illegal text alignment value!");
                        break;
                } // switch(eParagraphAdjust)
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
                    //s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_PARAADJUST,PROPERTY_ALIGN));
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
                    //s_aNameMap.insert(TPropertyNamePair::value_type(PROPERTY_PARAADJUST,TPropertyConverter(PROPERTY_ALIGN,aNoConverter)));
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
                    s_aNameMap.insert(TPropertyNamePair::value_type(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FillColor")),TPropertyConverter(PROPERTY_CONTROLBACKGROUND,aNoConverter)));
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

    m_bIsListening = sal_False;
    if ( isListening() && m_xReportComponent.is() )
    {
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
        //pModel->GetReferenceDevice()->Invalidate(INVALIDATE_CHILDREN);
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
DBG_NAME( rpt_OCustomShape );
OCustomShape::OCustomShape(SdrModel& rSdrModel, const uno::Reference< report::XReportComponent>& _xComponent)
    :SdrObjCustomShape(rSdrModel)
    ,OObjectBase(_xComponent)
{
    DBG_CTOR( rpt_OCustomShape, NULL);
    m_bIsListening = sal_True;
}
//----------------------------------------------------------------------------
OCustomShape::OCustomShape(SdrModel& rSdrModel, const ::rtl::OUString& _sComponentName)
    :SdrObjCustomShape(rSdrModel)
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

void OCustomShape::copyDataFromSdrObject(const SdrObject& rSource)
{
    if(this != &rSource)
    {
        const OCustomShape* pSource = dynamic_cast< const OCustomShape* >(&rSource);

        if(pSource)
        {
            // call parent
            SdrObjCustomShape::copyDataFromSdrObject(rSource);

            // no local data to copy
        }
        else
        {
            OSL_ENSURE(false, "copyDataFromSdrObject with ObjectType of Source different from Target (!)");
        }
    }
}

SdrObject* OCustomShape::CloneSdrObject(SdrModel* pTargetModel) const
{
    OCustomShape* pClone = new OCustomShape(
        pTargetModel ? *pTargetModel : getSdrModelFromSdrObject(),
        getReportComponent());
    OSL_ENSURE(pClone, "CloneSdrObject error (!)");
    pClone->copyDataFromSdrObject(*this);

    return pClone;
}

OCustomShape* OCustomShape::Create(SdrModel& rSdrModel, const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent>& _xComponent)
{
    OCustomShape* pNew = new OCustomShape( rSdrModel, _xComponent );
    OSL_ENSURE(pNew, "Create error (!)");
    SetUnoShapeAtSdrObjectFromSvxShape(*pNew, _xComponent);

    return pNew;
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
    return getSdrPageFromSdrObject();
}
//----------------------------------------------------------------------------
void OCustomShape::SetSnapRectImpl(const Rectangle& _rRect)
{
    sdr::legacy::SetSnapRect(*this, _rRect );
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
void OCustomShape::setSdrObjectTransformation(const basegfx::B2DHomMatrix& rTransformation)
{
    // call parent
    SdrObjCustomShape::setSdrObjectTransformation(rTransformation);

    if ( m_bIsListening )
    {
        m_bIsListening = sal_False;

        if ( m_xReportComponent.is() )
        {
            OReportModel& rRptModel = static_cast< OReportModel& >(getSdrModelFromSdrObject());
            OXUndoEnvironment::OUndoEnvLock aLock(rRptModel.GetUndoEnv());
            const basegfx::B2DPoint& rTopLeft = getSdrObjectTranslate();
            m_xReportComponent->setPositionX(int(rTopLeft.getX()));
            m_xReportComponent->setPositionY(int(rTopLeft.getY()));
        }

        // set geometry properties
        SetPropsFromRect(sdr::legacy::GetSnapRect(*this));

        m_bIsListening = sal_True;
    }
}

//----------------------------------------------------------------------------
bool OCustomShape::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    bool bResult(SdrObjCustomShape::EndCreate(rStat, eCmd));
    if ( bResult )
    {
        OReportModel& rRptModel = static_cast< OReportModel& >(getSdrModelFromSdrObject());
        OXUndoEnvironment::OUndoEnvLock aLock(rRptModel.GetUndoEnv());
            if ( !m_xReportComponent.is() )
                m_xReportComponent.set(getUnoShape(),uno::UNO_QUERY);
        SetPropsFromRect(sdr::legacy::GetSnapRect(*this));
    }

    return bResult;
}

//----------------------------------------------------------------------------
void OCustomShape::SetObjectItemHelper(const SfxPoolItem& rItem)
{
    SetObjectItem(rItem);
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
        OReportModel& rRptModel = static_cast< OReportModel& >(getSdrModelFromSdrObject());
        OXUndoEnvironment::OUndoEnvLock aLock(rRptModel.GetUndoEnv());
        m_xReportComponent.set(xShape,uno::UNO_QUERY);
    }
    return xShape;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
DBG_NAME( rpt_OUnoObject );
OUnoObject::OUnoObject(SdrModel& rSdrModel
    ,const ::rtl::OUString& _sComponentName
                       ,const ::rtl::OUString& rModelName
                       ,sal_uInt16   _nObjectType)
:   SdrUnoObj(rSdrModel, rModelName, sal_True)
          ,OObjectBase(_sComponentName)
          ,m_nObjectType(_nObjectType)
{
    DBG_CTOR( rpt_OUnoObject, NULL);
    if ( rModelName.getLength() )
        impl_initializeModel_nothrow();
}
//----------------------------------------------------------------------------
OUnoObject::OUnoObject(SdrModel& rSdrModel
    ,const uno::Reference< report::XReportComponent>& _xComponent
                       ,const ::rtl::OUString& rModelName
                       ,sal_uInt16   _nObjectType)
:   SdrUnoObj(rSdrModel, rModelName, sal_True)
          ,OObjectBase(_xComponent)
          ,m_nObjectType(_nObjectType)
{
    DBG_CTOR( rpt_OUnoObject, NULL);

    if ( rModelName.getLength() )
        impl_initializeModel_nothrow();

    if ( rModelName.getLength() )
        impl_initializeModel_nothrow();
}
//----------------------------------------------------------------------------
OUnoObject::~OUnoObject()
{
    DBG_DTOR( rpt_OUnoObject, NULL);
}

void OUnoObject::copyDataFromSdrObject(const SdrObject& rSource)
{
    if(this != &rSource)
    {
        const OUnoObject* pSource = dynamic_cast< const OUnoObject* >(&rSource);

        if(pSource)
        {
            // call parent
            SdrUnoObj::copyDataFromSdrObject(rSource);

            // no own local data to copy (? had no own operator= at least)
            Reference<XPropertySet> xSource(const_cast< OUnoObject* >(pSource)->getUnoShape(),uno::UNO_QUERY);
            Reference<XPropertySet> xDest(getUnoShape(),uno::UNO_QUERY);

            if ( xSource.is() && xDest.is() )
            {
                comphelper::copyProperties(xSource.get(),xDest.get());
            }
        }
        else
        {
            OSL_ENSURE(false, "copyDataFromSdrObject with ObjectType of Source different from Target (!)");
        }
    }
}

SdrObject* OUnoObject::CloneSdrObject(SdrModel* pTargetModel) const
{
    OUnoObject* pClone = new OUnoObject(
        pTargetModel ? *pTargetModel : getSdrModelFromSdrObject(),
        getReportComponent(),
        String(),
        GetObjIdentifier());
    SetUnoShapeAtSdrObjectFromSvxShape(*pClone, getReportComponent());
    OSL_ENSURE(pClone, "CloneSdrObject error (!)");
    pClone->copyDataFromSdrObject(*this);

    return pClone;
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
            const ::rtl::OUString sTreatAsNumberProperty = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TreatAsNumber" ) );
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

    OReportModel& rReportModel = static_cast< OReportModel& >(getSdrModelFromSdrObject());
    OXUndoEnvironment::OUndoEnvLock aLock( rReportModel.GetUndoEnv() );
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
    return getSdrPageFromSdrObject();
}
//----------------------------------------------------------------------------
void OUnoObject::SetSnapRectImpl(const Rectangle& _rRect)
{
    DBG_CHKTHIS( rpt_OUnoObject,NULL);
    sdr::legacy::SetSnapRect(*this, _rRect );
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
void OUnoObject::setSdrObjectTransformation(const basegfx::B2DHomMatrix& rTransformation)
{
    DBG_CHKTHIS( rpt_OUnoObject,NULL);

    // call parent
    SdrUnoObj::setSdrObjectTransformation(rTransformation);

    if ( m_bIsListening )
    {
        // stop listening
        OObjectBase::EndListening(sal_False);

        if ( m_xReportComponent.is() )
        {
            OReportModel& rRptModel = static_cast< OReportModel& >(getSdrModelFromSdrObject());
            const bool bUndoMode(rRptModel.GetUndoEnv().IsUndoMode());
            OXUndoEnvironment::OUndoEnvLock aLock(rRptModel.GetUndoEnv());
            const basegfx::B2DPoint& rNewTopLeft(getSdrObjectTranslate());
            basegfx::B2DPoint aCorrectedTopLeft(rNewTopLeft);

            if(rNewTopLeft.getY() < 0.0 && !bUndoMode)
        {
                aCorrectedTopLeft.setY(0.0);
}

            if(aCorrectedTopLeft != rNewTopLeft)
{
                basegfx::B2DHomMatrix aCorrected(rTransformation);

                aCorrected.translate(aCorrectedTopLeft - rNewTopLeft);
                rRptModel.AddUndo(rRptModel.GetSdrUndoFactory().CreateUndoGeoObject(*this));
                SdrUnoObj::setSdrObjectTransformation(aCorrected);
            }

            m_xReportComponent->setPositionX(int(aCorrectedTopLeft.getX()));
            m_xReportComponent->setPositionY(int(aCorrectedTopLeft.getY()));
}

    // set geometry properties
        SetPropsFromRect(sdr::legacy::GetLogicRect(*this));

    // start listening
    OObjectBase::StartListening();
}
}

//----------------------------------------------------------------------------

bool OUnoObject::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    DBG_CHKTHIS( rpt_OUnoObject,NULL);
    bool bResult(SdrUnoObj::EndCreate(rStat, eCmd));
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
        SetPropsFromRect(sdr::legacy::GetLogicRect(*this));
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
//----------------------------------------------------------------------------
// OOle2Obj
//----------------------------------------------------------------------------
DBG_NAME( rpt_OOle2Obj );
OOle2Obj::OOle2Obj(
    SdrModel& rSdrModel,
    const uno::Reference< report::XReportComponent>& _xComponent,
    sal_uInt16 _nType)
:   SdrOle2Obj(rSdrModel)
          ,OObjectBase(_xComponent)
          ,m_nType(_nType)
          ,m_bOnlyOnce(true)
{
    DBG_CTOR( rpt_OOle2Obj, NULL);
    m_bIsListening = sal_True;
}
//----------------------------------------------------------------------------
OOle2Obj::OOle2Obj(
    SdrModel& rSdrModel,
    ::rtl::OUString _sComponentName,
    sal_uInt16 _nType)
:   SdrOle2Obj(rSdrModel)
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
    return getSdrPageFromSdrObject();
}
//----------------------------------------------------------------------------
void OOle2Obj::SetSnapRectImpl(const Rectangle& _rRect)
{
    DBG_CHKTHIS( rpt_OOle2Obj,NULL);
    sdr::legacy::SetSnapRect(*this, _rRect );
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
void OOle2Obj::setSdrObjectTransformation(const basegfx::B2DHomMatrix& rTransformation)
{
    DBG_CHKTHIS( rpt_OOle2Obj,NULL);

    // call parent
    SdrOle2Obj::setSdrObjectTransformation(rTransformation);

    if ( m_bIsListening )
    {
        // stop listening
        OObjectBase::EndListening(sal_False);

        if ( m_xReportComponent.is() )
        {
            OReportModel& rRptModel = static_cast< OReportModel& >(getSdrModelFromSdrObject());
            const bool bUndoMode(rRptModel.GetUndoEnv().IsUndoMode());
            OXUndoEnvironment::OUndoEnvLock aLock(rRptModel.GetUndoEnv());
            const basegfx::B2DPoint& rNewTopLeft(getSdrObjectTranslate());
            basegfx::B2DPoint aCorrectedTopLeft(rNewTopLeft);

            if(rNewTopLeft.getY() < 0.0 && !bUndoMode)
        {
                aCorrectedTopLeft.setY(0.0);
}

            if(aCorrectedTopLeft != rNewTopLeft)
{
                basegfx::B2DHomMatrix aCorrected(rTransformation);

                aCorrected.translate(aCorrectedTopLeft - rNewTopLeft);
                rRptModel.AddUndo(rRptModel.GetSdrUndoFactory().CreateUndoGeoObject(*this));
                SdrOle2Obj::setSdrObjectTransformation(aCorrected);
            }

            m_xReportComponent->setPositionX(int(aCorrectedTopLeft.getX()));
            m_xReportComponent->setPositionY(int(aCorrectedTopLeft.getY()));
}

    // set geometry properties
        SetPropsFromRect(sdr::legacy::GetLogicRect(*this));

    // start listening
    OObjectBase::StartListening();
}
}

//----------------------------------------------------------------------------

bool OOle2Obj::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    DBG_CHKTHIS( rpt_OOle2Obj,NULL);
    bool bResult(SdrOle2Obj::EndCreate(rStat, eCmd));
    if ( bResult )
    {
        OReportModel& rRptModel = static_cast< OReportModel& >(getSdrModelFromSdrObject());
        OXUndoEnvironment::OUndoEnvLock aLock(rRptModel.GetUndoEnv());
            if ( !m_xReportComponent.is() )
                m_xReportComponent.set(getUnoShape(),uno::UNO_QUERY);
        // set geometry properties
        SetPropsFromRect(sdr::legacy::GetLogicRect(*this));
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
        OReportModel& rRptModel = static_cast< OReportModel& >(getSdrModelFromSdrObject());
        OXUndoEnvironment::OUndoEnvLock aLock(rRptModel.GetUndoEnv());
        m_xReportComponent.set(xShape,uno::UNO_QUERY);
    }
    return xShape;
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
    } // if( xCompSupp.is())
    return xSource;
}
// -----------------------------------------------------------------------------
void OOle2Obj::copyDataFromSdrObject(const SdrObject& rSource)
{
    if(this != &rSource)
    {
        const OOle2Obj* pSource = dynamic_cast< const OOle2Obj* >(&rSource);

        if(pSource)
        {
            // call parent
            SdrOle2Obj::copyDataFromSdrObject(rSource);

            // no own local data to copy (? had no own operator= at least)
            OReportModel& rRptModel = static_cast< OReportModel& >(pSource->getSdrModelFromSdrObject());
            svt::EmbeddedObjectRef::TryRunningState( GetObjRef() );
            impl_createDataProvider_nothrow(rRptModel.getReportDefinition().get());

            uno::Reference< chart2::data::XDatabaseDataProvider > xSource( lcl_getDataProvider(pSource->GetObjRef()) );
            uno::Reference< chart2::data::XDatabaseDataProvider > xDest( lcl_getDataProvider(GetObjRef()) );

            if ( xSource.is() && xDest.is() )
            {
                comphelper::copyProperties(xSource.get(),xDest.get());
            }

            initializeChart(rRptModel.getReportDefinition().get());
        }
        else
        {
            OSL_ENSURE(false, "copyDataFromSdrObject with ObjectType of Source different from Target (!)");
        }
    }
}
// -----------------------------------------------------------------------------
SdrObject* OOle2Obj::CloneSdrObject(SdrModel* pTargetModel) const
{
    OOle2Obj* pClone = new OOle2Obj(
        pTargetModel ? *pTargetModel : getSdrModelFromSdrObject(),
        getReportComponent(),
        GetObjIdentifier());
    OSL_ENSURE(pClone, "CloneSdrObject error (!)");
    SetUnoShapeAtSdrObjectFromSvxShape(*pClone, getReportComponent());
    pClone->copyDataFromSdrObject(*this);

    return pClone;
}
// -----------------------------------------------------------------------------
OOle2Obj* OOle2Obj::Create(
    SdrModel& rSdrModel,
    const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent >& _xComponent,
    sal_uInt16 _nType)
{
    OOle2Obj* pNew = new OOle2Obj( rSdrModel, _xComponent, _nType );
    OSL_ENSURE(pNew, "Create error (!)");
    SetUnoShapeAtSdrObjectFromSvxShape(*pNew, _xComponent);

    return pNew;
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
            uno::Reference< chart2::data::XDatabaseDataProvider > xDataProvider( xFac->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.chart2.data.DataProvider"))),uno::UNO_QUERY);
            xReceiver->attachDataProvider( xDataProvider.get() );
        } // if( xReceiver.is() )
    }
    catch(uno::Exception)
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
        OReportModel& rRptModel = static_cast< OReportModel& >(getSdrModelFromSdrObject());
        rRptModel.GetUndoEnv().AddElement(lcl_getDataProvider(xObj));

        uno::Reference< embed::XComponentSupplier > xCompSupp( xObj, uno::UNO_QUERY );
        if( xCompSupp.is() )
        {
            uno::Reference< beans::XPropertySet > xChartProps( xCompSupp->getComponent(), uno::UNO_QUERY );
            if ( xChartProps.is() )
                xChartProps->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NullDate")),uno::makeAny(util::DateTime(0,0,0,0,1,1,1900)));
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

        OReportModel& rRptModel = static_cast< OReportModel& >(getSdrModelFromSdrObject());
        rRptModel.GetUndoEnv().AddElement(lcl_getDataProvider(xObj));

        ::comphelper::NamedValueCollection aArgs;
        aArgs.put( "CellRangeRepresentation", uno::makeAny( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "all" ) ) ) );
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
