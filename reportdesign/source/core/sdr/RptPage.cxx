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
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <comphelper/embeddedobjectcontainer.hxx>
#include <comphelper/mimeconfighelper.hxx>
#include <editeng/outlobj.hxx>
#include <tools/diagnose_ex.h>
#include <RptPage.hxx>
#include <RptModel.hxx>
#include <Section.hxx>
#include <RptObject.hxx>
#include <svx/unoshape.hxx>
#include <strings.hxx>

namespace rptui
{
using namespace ::com::sun::star;

OReportPage::OReportPage(
    OReportModel& _rModel,
    const uno::Reference< report::XSection >& _xSection)
:   SdrPage(_rModel, false/*bMasterPage*/)
    ,rModel(_rModel)
    ,m_xSection(_xSection)
    ,m_bSpecialInsertMode(false)
{
}

OReportPage::~OReportPage()
{
}

rtl::Reference<SdrPage> OReportPage::CloneSdrPage(SdrModel& rTargetModel) const
{
    OReportModel& rOReportModel(static_cast< OReportModel& >(rTargetModel));
    rtl::Reference<OReportPage> pClonedOReportPage(
        new OReportPage(
            rOReportModel,
            m_xSection));
    pClonedOReportPage->SdrPage::lateInit(*this);
    return pClonedOReportPage;
}


size_t OReportPage::getIndexOf(const uno::Reference< report::XReportComponent >& _xObject)
{
    const size_t nCount = GetObjCount();
    size_t i = 0;
    for (; i < nCount; ++i)
    {
        OObjectBase* pObj = dynamic_cast<OObjectBase*>(GetObj(i));
        OSL_ENSURE(pObj,"Invalid object found!");
        if ( pObj && pObj->getReportComponent() == _xObject )
        {
            break;
        }
    }
    return i;
}

void OReportPage::removeSdrObject(const uno::Reference< report::XReportComponent >& _xObject)
{
    size_t nPos = getIndexOf(_xObject);
    if ( nPos < GetObjCount() )
    {
        OObjectBase* pBase = dynamic_cast<OObjectBase*>(GetObj(nPos));
        OSL_ENSURE(pBase,"Why is this not an OObjectBase?");
        if ( pBase )
            pBase->EndListening();
        RemoveObject(nPos);
    }
}

SdrObject* OReportPage::RemoveObject(size_t nObjNum)
{
    SdrObject* pObj = SdrPage::RemoveObject(nObjNum);
    if (getSpecialMode())
    {
        return pObj;
    }

    // this code is evil, but what else shall I do
    reportdesign::OSection* pSection = comphelper::getUnoTunnelImplementation<reportdesign::OSection>(m_xSection);
    uno::Reference< drawing::XShape> xShape(pObj->getUnoShape(),uno::UNO_QUERY);
    pSection->notifyElementRemoved(xShape);
    if (dynamic_cast< const OUnoObject *>( pObj ) !=  nullptr)
    {
        OUnoObject& rUnoObj = dynamic_cast<OUnoObject&>(*pObj);
        uno::Reference< container::XChild> xChild(rUnoObj.GetUnoControlModel(),uno::UNO_QUERY);
        if ( xChild.is() )
            xChild->setParent(nullptr);
    }
    return pObj;
}

void OReportPage::insertObject(const uno::Reference< report::XReportComponent >& _xObject)
{
    OSL_ENSURE(_xObject.is(),"Object is not valid to create a SdrObject!");
    if ( !_xObject.is() )
        return;
    size_t nPos = getIndexOf(_xObject);
    if ( nPos < GetObjCount() )
        return; // Object already in list

    OObjectBase* pObject = dynamic_cast< OObjectBase* >(SdrObject::getSdrObjectFromXShape( _xObject ));
    OSL_ENSURE( pObject, "OReportPage::insertObject: no implementation object found for the given shape/component!" );
    if ( pObject )
        pObject->StartListening();
}


void OReportPage::removeTempObject(SdrObject const *_pToRemoveObj)
{
    if (_pToRemoveObj)
    {
        for (size_t i=0; i<GetObjCount(); ++i)
        {
            SdrObject *aObj = GetObj(i);
            if (aObj && aObj == _pToRemoveObj)
            {
                (void) RemoveObject(i);
                break;
            }
        }
    }
}

void OReportPage::resetSpecialMode()
{
    const bool bChanged = rModel.IsChanged();

    for (const auto& pTemporaryObject : m_aTemporaryObjectList)
    {
         removeTempObject(pTemporaryObject);
    }
    m_aTemporaryObjectList.clear();
    rModel.SetChanged(bChanged);

    m_bSpecialInsertMode = false;
}

void OReportPage::NbcInsertObject(SdrObject* pObj, size_t nPos)
{
    SdrPage::NbcInsertObject(pObj, nPos);

    OUnoObject* pUnoObj = dynamic_cast< OUnoObject* >( pObj );
    if (getSpecialMode())
    {
        m_aTemporaryObjectList.push_back(pObj);
        return;
    }

    if ( pUnoObj )
    {
        pUnoObj->CreateMediator();
        uno::Reference< container::XChild> xChild(pUnoObj->GetUnoControlModel(),uno::UNO_QUERY);
        if ( xChild.is() && !xChild->getParent().is() )
            xChild->setParent(m_xSection);
    }

    // this code is evil, but what else shall I do
    reportdesign::OSection* pSection = comphelper::getUnoTunnelImplementation<reportdesign::OSection>(m_xSection);
    uno::Reference< drawing::XShape> xShape(pObj->getUnoShape(),uno::UNO_QUERY);
    pSection->notifyElementAdded(xShape);

    // now that the shape is inserted into its structures, we can allow the OObjectBase
    // to release the reference to it
    OObjectBase* pObjectBase = dynamic_cast< OObjectBase* >( pObj );
    OSL_ENSURE( pObjectBase, "OReportPage::NbcInsertObject: what is being inserted here?" );
    if ( pObjectBase )
        pObjectBase->releaseUnoShape();
}

SdrObject* OReportPage::CreateSdrObject_(const uno::Reference< drawing::XShape > & xDescr)
{
    uno::Reference< report::XReportComponent> xReportComponent(xDescr,uno::UNO_QUERY);
    if ( xReportComponent.is() )
    {
        return OObjectBase::createObject(
            getSdrModelFromSdrPage(),
            xReportComponent);
    }

    return SdrPage::CreateSdrObject_( xDescr );
}

uno::Reference< drawing::XShape >  OReportPage::CreateShape( SdrObject *pObj ) const
{
    OObjectBase* pBaseObj = dynamic_cast<OObjectBase*>(pObj);
    if ( !pBaseObj )
        return SdrPage::CreateShape( pObj );

    uno::Reference< report::XSection> xSection = m_xSection;
    uno::Reference< lang::XMultiServiceFactory> xFactory;
    if ( xSection.is() )
        xFactory.set(xSection->getReportDefinition(),uno::UNO_QUERY);
    uno::Reference< drawing::XShape > xRet;
    uno::Reference< drawing::XShape > xShape;
    if ( xFactory.is() )
    {
        bool bChangeOrientation = false;
        const OUString& sServiceName = pBaseObj->getServiceName();
        OSL_ENSURE(!sServiceName.isEmpty(),"No Service Name given!");

        if (dynamic_cast< const OUnoObject* >(pObj) != nullptr)
        {
            OUnoObject& rUnoObj = dynamic_cast<OUnoObject&>(*pObj);
            if (rUnoObj.GetObjIdentifier() == OBJ_RD_FIXEDTEXT)
            {
                uno::Reference<beans::XPropertySet> xControlModel(rUnoObj.GetUnoControlModel(),uno::UNO_QUERY);
                if ( xControlModel.is() )
                    xControlModel->setPropertyValue( PROPERTY_MULTILINE,uno::makeAny(true));
            }
            else
                bChangeOrientation = rUnoObj.GetObjIdentifier() == OBJ_RD_HFIXEDLINE;
            SvxShapeControl* pShape = new SvxShapeControl( pObj );
            xShape = static_cast<SvxShape_UnoImplHelper *>(pShape);
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
                xObj = pObj->getSdrModelFromSdrObject().GetPersist()->getEmbeddedObjectContainer().CreateEmbeddedObject(
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
            xShape.set( SdrPage::CreateShape( pObj ) );

        try
        {
            OReportModel& rRptModel(static_cast< OReportModel& >(pObj->getSdrModelFromSdrObject()));
            xRet.set( rRptModel.createShape(sServiceName,xShape,bChangeOrientation ? 0 : 1), uno::UNO_QUERY_THROW );
        }
        catch( const uno::Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("reportdesign");
        }
    }

    return xRet;
}

} // rptui


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
