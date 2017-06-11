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
#include "ReportDrawPage.hxx"
#include "RptObject.hxx"
#include "RptModel.hxx"
#include "RptDef.hxx"
#include "strings.hxx"
#include <comphelper/mimeconfighelper.hxx>
#include <comphelper/classids.hxx>
#include <comphelper/embeddedobjectcontainer.hxx>
#include <comphelper/documentconstants.hxx>

#include <svx/svdmodel.hxx>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/embed/Aspects.hpp>

#include <tools/diagnose_ex.h>
#include <svx/unoshape.hxx>

namespace reportdesign
{
    using namespace ::com::sun::star;
    using namespace rptui;

OReportDrawPage::OReportDrawPage(SdrPage* _pPage
                                 ,const uno::Reference< report::XSection >& _xSection)
: SvxDrawPage(_pPage)
,m_xSection(_xSection)
{
}

SdrObject* OReportDrawPage::CreateSdrObject_(const uno::Reference< drawing::XShape > & xDescr)
{
    uno::Reference< report::XReportComponent> xReportComponent(xDescr,uno::UNO_QUERY);
    if ( xReportComponent.is() )
        return OObjectBase::createObject(xReportComponent);
    return SvxDrawPage::CreateSdrObject_( xDescr );
}

uno::Reference< drawing::XShape >  OReportDrawPage::CreateShape( SdrObject *pObj ) const
{
    OObjectBase* pBaseObj = dynamic_cast<OObjectBase*>(pObj);
    if ( !pBaseObj )
        return SvxDrawPage::CreateShape( pObj );

    uno::Reference< report::XSection> xSection = m_xSection;
    uno::Reference< lang::XMultiServiceFactory> xFactory;
    if ( xSection.is() )
        xFactory.set(xSection->getReportDefinition(),uno::UNO_QUERY);
    uno::Reference< drawing::XShape > xRet;
    uno::Reference< drawing::XShape > xShape;
    if ( xFactory.is() )
    {
        bool bChangeOrientation = false;
        OUString sServiceName = pBaseObj->getServiceName();
        OSL_ENSURE(!sServiceName.isEmpty(),"No Service Name given!");

        if (dynamic_cast< const OUnoObject* >(pObj) != nullptr)
        {
            OUnoObject& rUnoObj = dynamic_cast<OUnoObject&>(*pObj);
            if (rUnoObj.GetObjIdentifier() == OBJ_DLG_FIXEDTEXT)
            {
                uno::Reference<beans::XPropertySet> xControlModel(rUnoObj.GetUnoControlModel(),uno::UNO_QUERY);
                if ( xControlModel.is() )
                    xControlModel->setPropertyValue( PROPERTY_MULTILINE,uno::makeAny(true));
            }
            else
                bChangeOrientation = rUnoObj.GetObjIdentifier() == OBJ_DLG_HFIXEDLINE;
            SvxShapeControl* pShape = new SvxShapeControl( pObj );
            xShape.set(static_cast<cppu::OWeakObject*>(static_cast<SvxShape_UnoImplHelper *>(pShape)),uno::UNO_QUERY);
            pShape->setShapeKind(pObj->GetObjIdentifier());
        }
        else if (dynamic_cast< const OCustomShape* >(pObj) != nullptr)
        {
            SvxCustomShape* pShape = new SvxCustomShape( pObj );
            uno::Reference < drawing::XEnhancedCustomShapeDefaulter > xShape2 = pShape;
            xShape.set(xShape2,uno::UNO_QUERY);
            pShape->setShapeKind(pObj->GetObjIdentifier());
        }
        else if (dynamic_cast< const SdrOle2Obj* >(pObj) != nullptr)
        {
            SdrOle2Obj& rOle2Obj = dynamic_cast<SdrOle2Obj&>(*pObj);
            if (!rOle2Obj.GetObjRef().is())
            {
                sal_Int64 nAspect = embed::Aspects::MSOLE_CONTENT;
                uno::Reference < embed::XEmbeddedObject > xObj;
                OUString sName;
                xObj = pObj->GetModel()->GetPersist()->getEmbeddedObjectContainer().CreateEmbeddedObject(
                    ::comphelper::MimeConfigurationHelper::GetSequenceClassIDRepresentation(
                    "80243D39-6741-46C5-926E-069164FF87BB"), sName );
                OSL_ENSURE(xObj.is(),"Embedded Object could not be created!");

                /**************************************************
                * The empty OLE object gets a new IPObj
                **************************************************/
                pObj->SetEmptyPresObj(false);
                rOle2Obj.SetOutlinerParaObject(nullptr);
                rOle2Obj.SetObjRef(xObj);
                rOle2Obj.SetPersistName(sName);
                rOle2Obj.SetName(sName);
                rOle2Obj.SetAspect(nAspect);
                tools::Rectangle aRect = rOle2Obj.GetLogicRect();

                Size aTmp = aRect.GetSize();
                awt::Size aSz( aTmp.Width(), aTmp.Height() );
                xObj->setVisualAreaSize( nAspect, aSz );
            }
            SvxOle2Shape* pShape = new SvxOle2Shape( pObj );
            xShape.set(*pShape,uno::UNO_QUERY);
            pShape->setShapeKind(pObj->GetObjIdentifier());
        }

        if ( !xShape.is() )
            xShape.set( SvxDrawPage::CreateShape( pObj ) );

        try
        {
            OReportModel* pRptModel = static_cast<OReportModel*>(pObj->GetModel());
            xRet.set( pRptModel->createShape(sServiceName,xShape,bChangeOrientation ? 0 : 1), uno::UNO_QUERY_THROW );
        }
        catch( const uno::Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    return xRet;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
