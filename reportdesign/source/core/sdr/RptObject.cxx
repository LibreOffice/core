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
#include <RptObject.hxx>
#include <algorithm>

#include <RptDef.hxx>
#include <svx/unoshape.hxx>
#include <RptModel.hxx>
#include <RptObjectListener.hxx>
#include <RptPage.hxx>

#include <strings.hxx>
#include <svtools/embedhlp.hxx>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/awt/TextAlign.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/XComponentSupplier.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/report/XFixedLine.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/chart2/data/XDatabaseDataProvider.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/report/XFormattedField.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/property.hxx>
#include <svx/svdundo.hxx>
#include <tools/diagnose_ex.h>
#include <PropertyForward.hxx>
#include <UndoEnv.hxx>

namespace rptui
{

using namespace ::com::sun::star;
using namespace uno;
using namespace beans;
using namespace reportdesign;
using namespace container;
using namespace report;

SdrObjKind OObjectBase::getObjectType(const uno::Reference< report::XReportComponent>& _xComponent)
{
    uno::Reference< lang::XServiceInfo > xServiceInfo( _xComponent , uno::UNO_QUERY );
    OSL_ENSURE(xServiceInfo.is(),"Who deletes the XServiceInfo interface!");
    if ( xServiceInfo.is() )
    {
        if ( xServiceInfo->supportsService( SERVICE_FIXEDTEXT ))
            return OBJ_RD_FIXEDTEXT;
        if ( xServiceInfo->supportsService( SERVICE_FIXEDLINE ))
        {
            uno::Reference< report::XFixedLine> xFixedLine(_xComponent,uno::UNO_QUERY);
            return xFixedLine->getOrientation() ? OBJ_RD_HFIXEDLINE : OBJ_RD_VFIXEDLINE;
        }
        if ( xServiceInfo->supportsService( SERVICE_IMAGECONTROL))
            return OBJ_RD_IMAGECONTROL;
        if ( xServiceInfo->supportsService( SERVICE_FORMATTEDFIELD ))
            return OBJ_RD_FORMATTEDFIELD;
        if ( xServiceInfo->supportsService("com.sun.star.drawing.OLE2Shape") )
            return OBJ_OLE2;
        if ( xServiceInfo->supportsService( SERVICE_SHAPE ))
            return OBJ_CUSTOMSHAPE;
        if ( xServiceInfo->supportsService( SERVICE_REPORTDEFINITION ) )
            return OBJ_RD_SUBREPORT;
        return OBJ_OLE2;
    }
    return OBJ_NONE;
}

SdrObject* OObjectBase::createObject(
    SdrModel& rTargetModel,
    const uno::Reference< report::XReportComponent>& _xComponent)
{
    SdrObject* pNewObj = nullptr;
    SdrObjKind nType = OObjectBase::getObjectType(_xComponent);
    switch( nType )
    {
        case OBJ_RD_FIXEDTEXT:
            {
                OUnoObject* pUnoObj = new OUnoObject(
                    rTargetModel,
                    _xComponent,
                    OUString("com.sun.star.form.component.FixedText"),
                    OBJ_RD_FIXEDTEXT);
                pNewObj = pUnoObj;

                uno::Reference<beans::XPropertySet> xControlModel(pUnoObj->GetUnoControlModel(),uno::UNO_QUERY);
                if ( xControlModel.is() )
                    xControlModel->setPropertyValue( PROPERTY_MULTILINE,uno::makeAny(true));
            }
            break;
        case OBJ_RD_IMAGECONTROL:
            pNewObj = new OUnoObject(
                rTargetModel,
                _xComponent,
                OUString("com.sun.star.form.component.DatabaseImageControl"),
                OBJ_RD_IMAGECONTROL);
            break;
        case OBJ_RD_FORMATTEDFIELD:
            pNewObj = new OUnoObject(
                rTargetModel,
                _xComponent,
                OUString("com.sun.star.form.component.FormattedField"),
                OBJ_RD_FORMATTEDFIELD);
            break;
        case OBJ_RD_HFIXEDLINE:
        case OBJ_RD_VFIXEDLINE:
            pNewObj = new OUnoObject(
                rTargetModel,
                _xComponent,
                OUString("com.sun.star.awt.UnoControlFixedLineModel"),
                nType);
            break;
        case OBJ_CUSTOMSHAPE:
            pNewObj = OCustomShape::Create(
                rTargetModel,
                _xComponent);
            try
            {
                bool bOpaque = false;
                _xComponent->getPropertyValue(PROPERTY_OPAQUE) >>= bOpaque;
                pNewObj->NbcSetLayer(bOpaque ? RPT_LAYER_FRONT : RPT_LAYER_BACK);
            }
            catch(const uno::Exception&)
            {
                DBG_UNHANDLED_EXCEPTION("reportdesign");
            }
            break;
        case OBJ_RD_SUBREPORT:
        case OBJ_OLE2:
            pNewObj = OOle2Obj::Create(
                rTargetModel,
                _xComponent,
                nType);
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

namespace
{
    class ParaAdjust : public AnyConverter
    {
    public:
        virtual css::uno::Any operator() (const OUString& _sPropertyName,const css::uno::Any& lhs) const override
        {
            uno::Any aRet;
            if (_sPropertyName == PROPERTY_PARAADJUST)
            {
                sal_Int16 nTextAlign = 0;
                lhs >>= nTextAlign;
                style::ParagraphAdjust eAdjust;
                switch(nTextAlign)
                {
                    case awt::TextAlign::LEFT:
                        eAdjust = style::ParagraphAdjust_LEFT;
                        break;
                    case awt::TextAlign::CENTER:
                        eAdjust = style::ParagraphAdjust_CENTER;
                        break;
                    case awt::TextAlign::RIGHT:
                        eAdjust = style::ParagraphAdjust_RIGHT;
                        break;
                    default:
                        OSL_FAIL("Illegal text alignment value!");
                        break;
                }
                aRet <<= eAdjust;
            }
            else
            {
                sal_Int16 nTextAlign = 0;
                sal_Int16 eParagraphAdjust = 0;
                lhs >>= eParagraphAdjust;
                switch(static_cast<style::ParagraphAdjust>(eParagraphAdjust))
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

const TPropertyNamePair& getPropertyNameMap(sal_uInt16 _nObjectId)
{
    switch(_nObjectId)
    {
        case OBJ_RD_IMAGECONTROL:
            {
                static TPropertyNamePair s_aNameMap = []()
                {
                    auto aNoConverter = std::make_shared<AnyConverter>();
                    TPropertyNamePair tmp;
                    tmp.emplace(PROPERTY_CONTROLBACKGROUND,TPropertyConverter(PROPERTY_BACKGROUNDCOLOR,aNoConverter));
                    tmp.emplace(PROPERTY_CONTROLBORDER,TPropertyConverter(PROPERTY_BORDER,aNoConverter));
                    tmp.emplace(PROPERTY_CONTROLBORDERCOLOR,TPropertyConverter(PROPERTY_BORDERCOLOR,aNoConverter));
                    return tmp;
                }();
                return s_aNameMap;
            }

        case OBJ_RD_FIXEDTEXT:
            {
                static TPropertyNamePair s_aNameMap = []()
                {
                    auto aNoConverter = std::make_shared<AnyConverter>();
                    TPropertyNamePair tmp;
                    tmp.emplace(PROPERTY_CHARCOLOR,TPropertyConverter(PROPERTY_TEXTCOLOR,aNoConverter));
                    tmp.emplace(PROPERTY_CONTROLBACKGROUND,TPropertyConverter(PROPERTY_BACKGROUNDCOLOR,aNoConverter));
                    tmp.emplace(PROPERTY_CHARUNDERLINECOLOR,TPropertyConverter(PROPERTY_TEXTLINECOLOR,aNoConverter));
                    tmp.emplace(PROPERTY_CHARRELIEF,TPropertyConverter(PROPERTY_FONTRELIEF,aNoConverter));
                    tmp.emplace(PROPERTY_CHARFONTHEIGHT,TPropertyConverter(PROPERTY_FONTHEIGHT,aNoConverter));
                    tmp.emplace(PROPERTY_CHARSTRIKEOUT,TPropertyConverter(PROPERTY_FONTSTRIKEOUT,aNoConverter));
                    tmp.emplace(PROPERTY_CONTROLTEXTEMPHASISMARK,TPropertyConverter(PROPERTY_FONTEMPHASISMARK,aNoConverter));
                    tmp.emplace(PROPERTY_CONTROLBORDER,TPropertyConverter(PROPERTY_BORDER,aNoConverter));
                    tmp.emplace(PROPERTY_CONTROLBORDERCOLOR,TPropertyConverter(PROPERTY_BORDERCOLOR,aNoConverter));

                    auto aParaAdjust = std::make_shared<ParaAdjust>();
                    tmp.emplace(PROPERTY_PARAADJUST,TPropertyConverter(PROPERTY_ALIGN,aParaAdjust));
                    return tmp;
                }();
                return s_aNameMap;
            }
        case OBJ_RD_FORMATTEDFIELD:
            {
                static TPropertyNamePair s_aNameMap = []()
                {
                    auto aNoConverter = std::make_shared<AnyConverter>();
                    TPropertyNamePair tmp;
                    tmp.emplace(PROPERTY_CHARCOLOR,TPropertyConverter(PROPERTY_TEXTCOLOR,aNoConverter));
                    tmp.emplace(PROPERTY_CONTROLBACKGROUND,TPropertyConverter(PROPERTY_BACKGROUNDCOLOR,aNoConverter));
                    tmp.emplace(PROPERTY_CHARUNDERLINECOLOR,TPropertyConverter(PROPERTY_TEXTLINECOLOR,aNoConverter));
                    tmp.emplace(PROPERTY_CHARRELIEF,TPropertyConverter(PROPERTY_FONTRELIEF,aNoConverter));
                    tmp.emplace(PROPERTY_CHARFONTHEIGHT,TPropertyConverter(PROPERTY_FONTHEIGHT,aNoConverter));
                    tmp.emplace(PROPERTY_CHARSTRIKEOUT,TPropertyConverter(PROPERTY_FONTSTRIKEOUT,aNoConverter));
                    tmp.emplace(PROPERTY_CONTROLTEXTEMPHASISMARK,TPropertyConverter(PROPERTY_FONTEMPHASISMARK,aNoConverter));
                    tmp.emplace(PROPERTY_CONTROLBORDER,TPropertyConverter(PROPERTY_BORDER,aNoConverter));
                    tmp.emplace(PROPERTY_CONTROLBORDERCOLOR,TPropertyConverter(PROPERTY_BORDERCOLOR,aNoConverter));
                    auto aParaAdjust = std::make_shared<ParaAdjust>();
                    tmp.emplace(PROPERTY_PARAADJUST,TPropertyConverter(PROPERTY_ALIGN,aParaAdjust));
                    return tmp;
                }();
                return s_aNameMap;
            }

        case OBJ_CUSTOMSHAPE:
            {
                static TPropertyNamePair s_aNameMap = []()
                {
                    auto aNoConverter = std::make_shared<AnyConverter>();
                    TPropertyNamePair tmp;
                    tmp.emplace(OUString("FillColor"),TPropertyConverter(PROPERTY_CONTROLBACKGROUND,aNoConverter));
                    tmp.emplace(PROPERTY_PARAADJUST,TPropertyConverter(PROPERTY_ALIGN,aNoConverter));
                    return tmp;
                }();
                return s_aNameMap;
            }

        default:
            break;
    }
    static TPropertyNamePair s_aEmptyNameMap;
    return s_aEmptyNameMap;
}


OObjectBase::OObjectBase(const uno::Reference< report::XReportComponent>& _xComponent)
:m_bIsListening(false)
{
    m_xReportComponent = _xComponent;
}

OObjectBase::OObjectBase(const OUString& _sComponentName)
:m_sComponentName(_sComponentName)
,m_bIsListening(false)
{
}

OObjectBase::~OObjectBase()
{
    m_xMediator.clear();
    if ( isListening() )
        EndListening();
    m_xReportComponent.clear();
}

uno::Reference< report::XSection> OObjectBase::getSection() const
{
    uno::Reference< report::XSection> xSection;
    OReportPage* pPage = dynamic_cast<OReportPage*>(GetImplPage());
    if ( pPage )
        xSection = pPage->getSection();
    return xSection;
}


uno::Reference< beans::XPropertySet> OObjectBase::getAwtComponent()
{
    return uno::Reference< beans::XPropertySet>();
}

void OObjectBase::StartListening()
{
    OSL_ENSURE(!isListening(), "OUnoObject::StartListening: already listening!");

    if ( !isListening() && m_xReportComponent.is() )
    {
        m_bIsListening = true;

        if ( !m_xPropertyChangeListener.is() )
        {
            m_xPropertyChangeListener = new OObjectListener( this );
            // register listener to all properties
            m_xReportComponent->addPropertyChangeListener( OUString() , m_xPropertyChangeListener );
        }
    }
}

void OObjectBase::EndListening()
{
    OSL_ENSURE(!m_xReportComponent.is() || isListening(), "OUnoObject::EndListening: not listening currently!");

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
                TOOLS_WARN_EXCEPTION( "package", "OObjectBase::EndListening");
            }
        }
        m_xPropertyChangeListener.clear();
    }
    m_bIsListening = false;
}

void OObjectBase::SetPropsFromRect(const tools::Rectangle& _rRect)
{
    // set properties
    OReportPage* pPage = dynamic_cast<OReportPage*>(GetImplPage());
    if ( pPage && !_rRect.IsEmpty() )
    {
        const uno::Reference<report::XSection>& xSection = pPage->getSection();
        assert(_rRect.getHeight() >= 0);
        const sal_uInt32 newHeight( ::std::max(tools::Long(0), _rRect.getHeight()+_rRect.Top()) );
        if ( xSection.is() && ( newHeight > xSection->getHeight() ) )
            xSection->setHeight( newHeight );

        // TODO
        //pModel->GetRefDevice()->Invalidate(InvalidateFlags::Children);
    }
}

void OObjectBase::_propertyChange( const  beans::PropertyChangeEvent& /*evt*/ )
{
}

bool OObjectBase::supportsService( const OUString& _sServiceName ) const
{
    // TODO: cache xServiceInfo as member?
    Reference< lang::XServiceInfo > xServiceInfo( m_xReportComponent , UNO_QUERY );

    if ( xServiceInfo.is() )
        return cppu::supportsService(xServiceInfo.get(), _sServiceName);
    else
        return false;
}


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
    SvxShape* pShape = comphelper::getFromUnoTunnel<SvxShape>( _rxShape );
    OSL_ENSURE( pShape, "OObjectBase::ensureSdrObjectOwnership: can't access the SvxShape!" );
    if ( pShape )
    {
        OSL_ENSURE( !pShape->HasSdrObjectOwnership(), "OObjectBase::ensureSdrObjectOwnership: called twice?" );
        pShape->TakeSdrObjectOwnership();
    }
}


uno::Reference< drawing::XShape > OObjectBase::getUnoShapeOf( SdrObject& _rSdrObject )
{
    uno::Reference< drawing::XShape > xShape( _rSdrObject.getWeakUnoShape() );
    if ( xShape.is() )
        return xShape;

    xShape = _rSdrObject.SdrObject::getUnoShape();
    if ( !xShape.is() )
        return xShape;

    ensureSdrObjectOwnership( xShape );

    m_xKeepShapeAlive = xShape;
    return xShape;
}

OCustomShape::OCustomShape(
    SdrModel& rSdrModel,
    const uno::Reference< report::XReportComponent>& _xComponent)
:   SdrObjCustomShape(rSdrModel)
    ,OObjectBase(_xComponent)
{
    setUnoShape( uno::Reference< drawing::XShape >(_xComponent,uno::UNO_QUERY_THROW) );
    m_bIsListening = true;
}

OCustomShape::OCustomShape(
    SdrModel& rSdrModel,
    const OUString& _sComponentName)
:   SdrObjCustomShape(rSdrModel)
    ,OObjectBase(_sComponentName)
{
    m_bIsListening = true;
}


OCustomShape::~OCustomShape()
{
}

SdrObjKind OCustomShape::GetObjIdentifier() const
{
    return OBJ_CUSTOMSHAPE;
}

SdrInventor OCustomShape::GetObjInventor() const
{
    return SdrInventor::ReportDesign;
}

SdrPage* OCustomShape::GetImplPage() const
{
    return getSdrPageFromSdrObject();
}

void OCustomShape::NbcMove( const Size& rSize )
{
    if ( m_bIsListening )
    {
        m_bIsListening = false;

        if ( m_xReportComponent.is() )
        {
            OReportModel& rRptModel(static_cast< OReportModel& >(getSdrModelFromSdrObject()));
            OXUndoEnvironment::OUndoEnvLock aLock(rRptModel.GetUndoEnv());
            m_xReportComponent->setPositionX(m_xReportComponent->getPositionX() + rSize.Width());
            m_xReportComponent->setPositionY(m_xReportComponent->getPositionY() + rSize.Height());
        }

        // set geometry properties
        SetPropsFromRect(GetSnapRect());

        m_bIsListening = true;
    }
    else
        SdrObjCustomShape::NbcMove( rSize );
}

void OCustomShape::NbcResize(const Point& rRef, const Fraction& xFract, const Fraction& yFract)
{
    SdrObjCustomShape::NbcResize( rRef, xFract, yFract );

    SetPropsFromRect(GetSnapRect());
}

void OCustomShape::NbcSetLogicRect(const tools::Rectangle& rRect)
{
    SdrObjCustomShape::NbcSetLogicRect(rRect);
    SetPropsFromRect(rRect);
}

bool OCustomShape::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    bool bResult = SdrObjCustomShape::EndCreate(rStat, eCmd);
    if ( bResult )
    {
        OReportModel& rRptModel(static_cast< OReportModel& >(getSdrModelFromSdrObject()));
        OXUndoEnvironment::OUndoEnvLock aLock(rRptModel.GetUndoEnv());

        if ( !m_xReportComponent.is() )
            m_xReportComponent.set(getUnoShape(),uno::UNO_QUERY);

        SetPropsFromRect(GetSnapRect());
    }

    return bResult;
}


uno::Reference< beans::XPropertySet> OCustomShape::getAwtComponent()
{
    return m_xReportComponent;
}


uno::Reference< drawing::XShape > OCustomShape::getUnoShape()
{
    uno::Reference<drawing::XShape> xShape = OObjectBase::getUnoShapeOf( *this );
    if ( !m_xReportComponent.is() )
    {
        OReportModel& rRptModel(static_cast< OReportModel& >(getSdrModelFromSdrObject()));
        OXUndoEnvironment::OUndoEnvLock aLock(rRptModel.GetUndoEnv());
        m_xReportComponent.set(xShape,uno::UNO_QUERY);
    }
    return xShape;
}

void OCustomShape::setUnoShape( const uno::Reference< drawing::XShape >& rxUnoShape )
{
    SdrObjCustomShape::setUnoShape( rxUnoShape );
    releaseUnoShape();
    m_xReportComponent.clear();
}

void OCustomShape::setUnoShape( SvxShape& rNewShape )
{
    SdrObjCustomShape::setUnoShape( rNewShape );
    releaseUnoShape();
    m_xReportComponent.clear();
}

OUnoObject::OUnoObject(
    SdrModel& rSdrModel,
    const OUString& _sComponentName,
    const OUString& rModelName,
    SdrObjKind _nObjectType)
:   SdrUnoObj(rSdrModel, rModelName)
    ,OObjectBase(_sComponentName)
    ,m_nObjectType(_nObjectType)
    // tdf#119067
    ,m_bSetDefaultLabel(false)
{
    if ( !rModelName.isEmpty() )
        impl_initializeModel_nothrow();
}

OUnoObject::OUnoObject(
    SdrModel& rSdrModel, OUnoObject const & rSource)
:   SdrUnoObj(rSdrModel, rSource)
    ,OObjectBase(rSource.getServiceName())
    ,m_nObjectType(rSource.m_nObjectType)
    // tdf#119067
    ,m_bSetDefaultLabel(rSource.m_bSetDefaultLabel)
{
    if ( !rSource.getUnoControlModelTypeName().isEmpty() )
        impl_initializeModel_nothrow();
    Reference<XPropertySet> xSource(const_cast<OUnoObject&>(rSource).getUnoShape(), uno::UNO_QUERY);
    Reference<XPropertySet> xDest(getUnoShape(), uno::UNO_QUERY);
    if ( xSource.is() && xDest.is() )
        comphelper::copyProperties(xSource, xDest);
}

OUnoObject::OUnoObject(
    SdrModel& rSdrModel,
    const uno::Reference< report::XReportComponent>& _xComponent,
    const OUString& rModelName,
    SdrObjKind _nObjectType)
:   SdrUnoObj(rSdrModel, rModelName)
    ,OObjectBase(_xComponent)
    ,m_nObjectType(_nObjectType)
    // tdf#119067
    ,m_bSetDefaultLabel(false)
{
    setUnoShape( uno::Reference< drawing::XShape >( _xComponent, uno::UNO_QUERY_THROW ) );

    if ( !rModelName.isEmpty() )
        impl_initializeModel_nothrow();

}

OUnoObject::~OUnoObject()
{
}

void OUnoObject::impl_initializeModel_nothrow()
{
    try
    {
        Reference< XFormattedField > xFormatted( m_xReportComponent, UNO_QUERY );
        if ( xFormatted.is() )
        {
            const Reference< XPropertySet > xModelProps( GetUnoControlModel(), UNO_QUERY_THROW );
            xModelProps->setPropertyValue( "TreatAsNumber", makeAny( false ) );
            xModelProps->setPropertyValue( PROPERTY_VERTICALALIGN,m_xReportComponent->getPropertyValue(PROPERTY_VERTICALALIGN));
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("reportdesign");
    }
}

SdrObjKind OUnoObject::GetObjIdentifier() const
{
    return m_nObjectType;
}

SdrInventor OUnoObject::GetObjInventor() const
{
    return SdrInventor::ReportDesign;
}

SdrPage* OUnoObject::GetImplPage() const
{
    return getSdrPageFromSdrObject();
}

void OUnoObject::NbcMove( const Size& rSize )
{

    if ( m_bIsListening )
    {
        // stop listening
        OObjectBase::EndListening();

        bool bPositionFixed = false;
        Size aUndoSize(0,0);
        if ( m_xReportComponent.is() )
        {
            bool bUndoMode = false;
            OReportModel& rRptModel(static_cast< OReportModel& >(getSdrModelFromSdrObject()));

            if (rRptModel.GetUndoEnv().IsUndoMode())
            {
                // if we are locked from outside, then we must not handle wrong moves, we are in UNDO mode
                bUndoMode = true;
            }
            OXUndoEnvironment::OUndoEnvLock aLock(rRptModel.GetUndoEnv());

            // LLA: why there exists getPositionX and getPositionY and NOT getPosition() which return a Point?
            int nNewX = m_xReportComponent->getPositionX() + rSize.Width();
            m_xReportComponent->setPositionX(nNewX);
            int nNewY = m_xReportComponent->getPositionY() + rSize.Height();
            if (nNewY < 0 && !bUndoMode)
            {
                aUndoSize.setHeight( abs(nNewY) );
                bPositionFixed = true;
                nNewY = 0;
            }
            m_xReportComponent->setPositionY(nNewY);
        }
        if (bPositionFixed)
        {
            getSdrModelFromSdrObject().AddUndo(getSdrModelFromSdrObject().GetSdrUndoFactory().CreateUndoMoveObject(*this, aUndoSize));
        }
        // set geometry properties
        SetPropsFromRect(GetLogicRect());

        // start listening
        OObjectBase::StartListening();
    }
    else
        SdrUnoObj::NbcMove( rSize );
}


void OUnoObject::NbcResize(const Point& rRef, const Fraction& xFract, const Fraction& yFract)
{
    SdrUnoObj::NbcResize( rRef, xFract, yFract );

    // stop listening
    OObjectBase::EndListening();

    // set geometry properties
    SetPropsFromRect(GetLogicRect());

    // start listening
    OObjectBase::StartListening();
}

void OUnoObject::NbcSetLogicRect(const tools::Rectangle& rRect)
{
    SdrUnoObj::NbcSetLogicRect(rRect);
    // stop listening
    OObjectBase::EndListening();

    // set geometry properties
    SetPropsFromRect(rRect);

    // start listening
    OObjectBase::StartListening();
}

bool OUnoObject::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    const bool bResult(SdrUnoObj::EndCreate(rStat, eCmd));

    if(bResult)
    {
        // tdf#118730 remember if this object was created interactively (due to ::EndCreate being called)
        m_bSetDefaultLabel = true;

        // set geometry properties
        SetPropsFromRect(GetLogicRect());
    }

    return bResult;
}

OUString OUnoObject::GetDefaultName(const OUnoObject* _pObj)
{
    OUString aDefaultName = "HERE WE HAVE TO INSERT OUR NAME!";
    if ( _pObj->supportsService( SERVICE_FIXEDTEXT ) )
    {
        aDefaultName = RID_STR_CLASS_FIXEDTEXT;
    }
    else if ( _pObj->supportsService( SERVICE_FIXEDLINE ) )
    {
        aDefaultName = RID_STR_CLASS_FIXEDLINE;
    }
    else if ( _pObj->supportsService( SERVICE_IMAGECONTROL ) )
    {
        aDefaultName = RID_STR_CLASS_IMAGECONTROL;
    }
    else if ( _pObj->supportsService( SERVICE_FORMATTEDFIELD ) )
    {
        aDefaultName = RID_STR_CLASS_FORMATTEDFIELD;
    }

    return aDefaultName;
}

void OUnoObject::_propertyChange( const  beans::PropertyChangeEvent& evt )
{
    OObjectBase::_propertyChange(evt);
    if (!isListening())
        return;

    if ( evt.PropertyName == PROPERTY_CHARCOLOR )
    {
        Reference<XPropertySet> xControlModel(GetUnoControlModel(),uno::UNO_QUERY);
        if ( xControlModel.is() )
        {
            OObjectBase::EndListening();
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

            if ( aNewName != aOldName )
            {
                // set old name property
                OObjectBase::EndListening();
                if ( m_xMediator.is() )
                    m_xMediator->stopListening();
                try
                {
                    xControlModel->setPropertyValue( PROPERTY_NAME, evt.NewValue );
                }
                catch(uno::Exception&)
                {
                }
                if ( m_xMediator.is() )
                    m_xMediator->startListening();
                OObjectBase::StartListening();
            }
        }
    }
}

void OUnoObject::CreateMediator(bool _bReverse)
{
    if ( m_xMediator.is() )
        return;

    // tdf#118730 Directly do things formerly done in
    // OUnoObject::impl_setReportComponent_nothrow here
    if(!m_xReportComponent.is())
    {
        OReportModel& rRptModel(static_cast< OReportModel& >(getSdrModelFromSdrObject()));
        OXUndoEnvironment::OUndoEnvLock aLock( rRptModel.GetUndoEnv() );
        m_xReportComponent.set(getUnoShape(),uno::UNO_QUERY);

        impl_initializeModel_nothrow();
    }

    if(m_xReportComponent.is() && m_bSetDefaultLabel)
    {
        // tdf#118730 Directly do things formerly done in
        // OUnoObject::EndCreate here
        // tdf#119067 ...but *only* if result of interactive
        // creation in Report DesignView
        m_bSetDefaultLabel = false;

        try
        {
            if ( supportsService( SERVICE_FIXEDTEXT ) )
            {
                m_xReportComponent->setPropertyValue(
                    PROPERTY_LABEL,
                    uno::makeAny(GetDefaultName(this)));
            }
        }
        catch(const uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("reportdesign");
        }
    }

    if(!m_xMediator.is() && m_xReportComponent.is())
    {
        Reference<XPropertySet> xControlModel(GetUnoControlModel(),uno::UNO_QUERY);

        if(xControlModel.is())
        {
            m_xMediator = new OPropertyMediator(
                m_xReportComponent,
                xControlModel,
                TPropertyNamePair(getPropertyNameMap(GetObjIdentifier())),
                _bReverse);
        }
    }

    OObjectBase::StartListening();
}

uno::Reference< beans::XPropertySet> OUnoObject::getAwtComponent()
{
    return Reference<XPropertySet>(GetUnoControlModel(),uno::UNO_QUERY);
}


uno::Reference< drawing::XShape > OUnoObject::getUnoShape()
{
    return OObjectBase::getUnoShapeOf( *this );
}

void OUnoObject::setUnoShape( const uno::Reference< drawing::XShape >& rxUnoShape )
{
    SdrUnoObj::setUnoShape( rxUnoShape );
    releaseUnoShape();
}

void OUnoObject::setUnoShape( SvxShape& rNewShape )
{
    SdrUnoObj::setUnoShape( rNewShape );
    releaseUnoShape();
}

OUnoObject* OUnoObject::CloneSdrObject(SdrModel& rTargetModel) const
{
    return new OUnoObject(rTargetModel, *this);
}

// OOle2Obj
OOle2Obj::OOle2Obj(
    SdrModel& rSdrModel,
    const uno::Reference< report::XReportComponent>& _xComponent,
    SdrObjKind _nType)
:   SdrOle2Obj(rSdrModel)
    ,OObjectBase(_xComponent)
    ,m_nType(_nType)
    ,m_bOnlyOnce(true)
{
    setUnoShape( uno::Reference< drawing::XShape >( _xComponent, uno::UNO_QUERY_THROW ) );
    m_bIsListening = true;
}

OOle2Obj::OOle2Obj(
    SdrModel& rSdrModel,
    const OUString& _sComponentName,
    SdrObjKind _nType)
:   SdrOle2Obj(rSdrModel)
    ,OObjectBase(_sComponentName)
    ,m_nType(_nType)
    ,m_bOnlyOnce(true)
{
    m_bIsListening = true;
}

static uno::Reference< chart2::data::XDatabaseDataProvider > lcl_getDataProvider(const uno::Reference < embed::XEmbeddedObject >& _xObj);

OOle2Obj::OOle2Obj(SdrModel& rSdrModel, OOle2Obj const & rSource)
:   SdrOle2Obj(rSdrModel, rSource)
    ,OObjectBase(rSource.getServiceName())
    ,m_nType(rSource.m_nType)
    ,m_bOnlyOnce(rSource.m_bOnlyOnce)
{
    m_bIsListening = true;

    OReportModel& rRptModel(static_cast< OReportModel& >(getSdrModelFromSdrObject()));
    svt::EmbeddedObjectRef::TryRunningState( GetObjRef() );
    impl_createDataProvider_nothrow(rRptModel.getReportDefinition());

    uno::Reference< chart2::data::XDatabaseDataProvider > xSource( lcl_getDataProvider(rSource.GetObjRef()) );
    uno::Reference< chart2::data::XDatabaseDataProvider > xDest( lcl_getDataProvider(GetObjRef()) );
    if ( xSource.is() && xDest.is() )
        comphelper::copyProperties(xSource, xDest);

    initializeChart(rRptModel.getReportDefinition());
}

OOle2Obj::~OOle2Obj()
{
}

SdrObjKind OOle2Obj::GetObjIdentifier() const
{
    return m_nType;
}

SdrInventor OOle2Obj::GetObjInventor() const
{
    return SdrInventor::ReportDesign;
}

SdrPage* OOle2Obj::GetImplPage() const
{
    return getSdrPageFromSdrObject();
}

void OOle2Obj::NbcMove( const Size& rSize )
{

    if ( m_bIsListening )
    {
        // stop listening
        OObjectBase::EndListening();

        bool bPositionFixed = false;
        Size aUndoSize(0,0);
        if ( m_xReportComponent.is() )
        {
            bool bUndoMode = false;
            OReportModel& rRptModel(static_cast< OReportModel& >(getSdrModelFromSdrObject()));

            if (rRptModel.GetUndoEnv().IsUndoMode())
            {
                // if we are locked from outside, then we must not handle wrong moves, we are in UNDO mode
                bUndoMode = true;
            }
            OXUndoEnvironment::OUndoEnvLock aLock(rRptModel.GetUndoEnv());

            // LLA: why there exists getPositionX and getPositionY and NOT getPosition() which return a Point?
            int nNewX = m_xReportComponent->getPositionX() + rSize.Width();
            // can this hinder us to set components outside the area?
            // if (nNewX < 0)
            // {
            //     nNewX = 0;
            // }
            m_xReportComponent->setPositionX(nNewX);
            int nNewY = m_xReportComponent->getPositionY() + rSize.Height();
            if (nNewY < 0 && !bUndoMode)
            {
                aUndoSize.setHeight( abs(nNewY) );
                bPositionFixed = true;
                nNewY = 0;
            }
            m_xReportComponent->setPositionY(nNewY);
        }
        if (bPositionFixed)
        {
            getSdrModelFromSdrObject().AddUndo(getSdrModelFromSdrObject().GetSdrUndoFactory().CreateUndoMoveObject(*this, aUndoSize));
        }
        // set geometry properties
        SetPropsFromRect(GetLogicRect());

        // start listening
        OObjectBase::StartListening();
    }
    else
        SdrOle2Obj::NbcMove( rSize );
}


void OOle2Obj::NbcResize(const Point& rRef, const Fraction& xFract, const Fraction& yFract)
{
    SdrOle2Obj::NbcResize( rRef, xFract, yFract );

    // stop listening
    OObjectBase::EndListening();

    // set geometry properties
    SetPropsFromRect(GetLogicRect());

    // start listening
    OObjectBase::StartListening();
}

void OOle2Obj::NbcSetLogicRect(const tools::Rectangle& rRect)
{
    SdrOle2Obj::NbcSetLogicRect(rRect);
    // stop listening
    OObjectBase::EndListening();

    // set geometry properties
    SetPropsFromRect(rRect);

    // start listening
    OObjectBase::StartListening();
}


bool OOle2Obj::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    bool bResult = SdrOle2Obj::EndCreate(rStat, eCmd);
    if ( bResult )
    {
        OReportModel& rRptModel(static_cast< OReportModel& >(getSdrModelFromSdrObject()));
        OXUndoEnvironment::OUndoEnvLock aLock(rRptModel.GetUndoEnv());

        if ( !m_xReportComponent.is() )
            m_xReportComponent.set(getUnoShape(),uno::UNO_QUERY);

        // set geometry properties
        SetPropsFromRect(GetLogicRect());
    }

    return bResult;
}

uno::Reference< beans::XPropertySet> OOle2Obj::getAwtComponent()
{
    return m_xReportComponent;
}


uno::Reference< drawing::XShape > OOle2Obj::getUnoShape()
{
    uno::Reference< drawing::XShape> xShape = OObjectBase::getUnoShapeOf( *this );
    if ( !m_xReportComponent.is() )
    {
        OReportModel& rRptModel(static_cast< OReportModel& >(getSdrModelFromSdrObject()));
        OXUndoEnvironment::OUndoEnvLock aLock(rRptModel.GetUndoEnv());
        m_xReportComponent.set(xShape,uno::UNO_QUERY);
    }
    return xShape;
}

void OOle2Obj::setUnoShape( const uno::Reference< drawing::XShape >& rxUnoShape )
{
    SdrOle2Obj::setUnoShape( rxUnoShape );
    releaseUnoShape();
    m_xReportComponent.clear();
}

void OOle2Obj::setUnoShape( SvxShape& rNewShape )
{
    SdrOle2Obj::setUnoShape( rNewShape );
    releaseUnoShape();
    m_xReportComponent.clear();
}

static uno::Reference< chart2::data::XDatabaseDataProvider > lcl_getDataProvider(const uno::Reference < embed::XEmbeddedObject >& _xObj)
{
    uno::Reference< chart2::data::XDatabaseDataProvider > xSource;
    uno::Reference< embed::XComponentSupplier > xCompSupp(_xObj);
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

// Clone() should make a complete copy of the object.
OOle2Obj* OOle2Obj::CloneSdrObject(SdrModel& rTargetModel) const
{
    return new OOle2Obj(rTargetModel, *this);
}

void OOle2Obj::impl_createDataProvider_nothrow(const uno::Reference< frame::XModel>& _xModel)
{
    try
    {
        uno::Reference < embed::XEmbeddedObject > xObj = GetObjRef();
        uno::Reference< chart2::data::XDataReceiver > xReceiver;
        uno::Reference< embed::XComponentSupplier > xCompSupp( xObj );
        if( xCompSupp.is())
            xReceiver.set( xCompSupp->getComponent(), uno::UNO_QUERY );
        OSL_ASSERT( xReceiver.is());
        if( xReceiver.is() )
        {
            uno::Reference< lang::XMultiServiceFactory> xFac(_xModel,uno::UNO_QUERY);
            uno::Reference< chart2::data::XDatabaseDataProvider > xDataProvider( xFac->createInstance("com.sun.star.chart2.data.DataProvider"),uno::UNO_QUERY);
            xReceiver->attachDataProvider( xDataProvider );
        }
    }
    catch(const uno::Exception &)
    {
    }
}

void OOle2Obj::initializeOle()
{
    if ( !m_bOnlyOnce )
        return;

    m_bOnlyOnce = false;
    uno::Reference < embed::XEmbeddedObject > xObj = GetObjRef();
    OReportModel& rRptModel(static_cast< OReportModel& >(getSdrModelFromSdrObject()));
    rRptModel.GetUndoEnv().AddElement(lcl_getDataProvider(xObj));

    uno::Reference< embed::XComponentSupplier > xCompSupp( xObj );
    if( xCompSupp.is() )
    {
        uno::Reference< beans::XPropertySet > xChartProps( xCompSupp->getComponent(), uno::UNO_QUERY );
        if ( xChartProps.is() )
            xChartProps->setPropertyValue("NullDate",
                uno::makeAny(util::DateTime(0,0,0,0,30,12,1899,false)));
    }
}

void OOle2Obj::initializeChart( const uno::Reference< frame::XModel>& _xModel)
{
    uno::Reference < embed::XEmbeddedObject > xObj = GetObjRef();
    uno::Reference< chart2::data::XDataReceiver > xReceiver;
    uno::Reference< embed::XComponentSupplier > xCompSupp( xObj );
    if( xCompSupp.is())
        xReceiver.set( xCompSupp->getComponent(), uno::UNO_QUERY );
    OSL_ASSERT( xReceiver.is());
    if( !xReceiver.is() )
        return;

    // lock the model to suppress any internal updates
    uno::Reference< frame::XModel > xChartModel( xReceiver, uno::UNO_QUERY );
    if( xChartModel.is() )
        xChartModel->lockControllers();

    if ( !lcl_getDataProvider(xObj).is() )
        impl_createDataProvider_nothrow(_xModel);

    OReportModel& rRptModel(static_cast< OReportModel& >(getSdrModelFromSdrObject()));
    rRptModel.GetUndoEnv().AddElement(lcl_getDataProvider(xObj));

    ::comphelper::NamedValueCollection aArgs;
    aArgs.put( "CellRangeRepresentation", uno::makeAny( OUString( "all" ) ) );
    aArgs.put( "HasCategories", uno::makeAny( true ) );
    aArgs.put( "FirstCellAsLabel", uno::makeAny( true ) );
    aArgs.put( "DataRowSource", uno::makeAny( chart::ChartDataRowSource_COLUMNS ) );
    xReceiver->setArguments( aArgs.getPropertyValues() );

    if( xChartModel.is() )
        xChartModel->unlockControllers();
}

uno::Reference< style::XStyle> getUsedStyle(const uno::Reference< report::XReportDefinition>& _xReport)
{
    uno::Reference<container::XNameAccess> xStyles = _xReport->getStyleFamilies();
    uno::Reference<container::XNameAccess> xPageStyles(xStyles->getByName("PageStyles"),uno::UNO_QUERY);

    uno::Reference< style::XStyle> xReturn;
    const uno::Sequence< OUString> aSeq = xPageStyles->getElementNames();
    for(const OUString& rName : aSeq)
    {
        uno::Reference< style::XStyle> xStyle(xPageStyles->getByName(rName),uno::UNO_QUERY);
        if ( xStyle->isInUse() )
        {
            xReturn = xStyle;
            break;
        }
    }
    return xReturn;
}


} // rptui


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
