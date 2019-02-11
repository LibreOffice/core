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
#include <RptPage.hxx>
#include <RptModel.hxx>
#include <Section.hxx>
#include <RptObject.hxx>
#include <svx/unoapi.hxx>
#include <svx/unoshape.hxx>
#include <ReportDrawPage.hxx>

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

SdrPage* OReportPage::CloneSdrPage(SdrModel& rTargetModel) const
{
    OReportModel& rOReportModel(static_cast< OReportModel& >(rTargetModel));
    OReportPage* pClonedOReportPage(
        new OReportPage(
            rOReportModel,
            m_xSection));
    pClonedOReportPage->SdrPage::lateInit(*this);
    return pClonedOReportPage;
}


sal_uLong OReportPage::getIndexOf(const uno::Reference< report::XReportComponent >& _xObject)
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
    return static_cast<sal_uLong>(i);
}

void OReportPage::removeSdrObject(const uno::Reference< report::XReportComponent >& _xObject)
{
    sal_uLong nPos = getIndexOf(_xObject);
    if ( nPos < GetObjCount() )
    {
        OObjectBase* pBase = dynamic_cast<OObjectBase*>(GetObj(nPos));
        OSL_ENSURE(pBase,"Why is this not a OObjectBase?");
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
    reportdesign::OSection* pSection = reportdesign::OSection::getImplementation(m_xSection);
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
    sal_uLong nPos = getIndexOf(_xObject);
    if ( nPos < GetObjCount() )
        return; // Object already in list

    SvxShape* pShape = SvxShape::getImplementation( _xObject );
    OObjectBase* pObject = pShape ? dynamic_cast< OObjectBase* >( pShape->GetSdrObject() ) : nullptr;
    OSL_ENSURE( pObject, "OReportPage::insertObject: no implementation object found for the given shape/component!" );
    if ( pObject )
        pObject->StartListening();
}


uno::Reference< uno::XInterface > OReportPage::createUnoPage()
{
    return static_cast<cppu::OWeakObject*>( new reportdesign::OReportDrawPage(this,m_xSection) );
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
    reportdesign::OSection* pSection = reportdesign::OSection::getImplementation(m_xSection);
    uno::Reference< drawing::XShape> xShape(pObj->getUnoShape(),uno::UNO_QUERY);
    pSection->notifyElementAdded(xShape);

    // now that the shape is inserted into its structures, we can allow the OObjectBase
    // to release the reference to it
    OObjectBase* pObjectBase = dynamic_cast< OObjectBase* >( pObj );
    OSL_ENSURE( pObjectBase, "OReportPage::NbcInsertObject: what is being inserted here?" );
    if ( pObjectBase )
        pObjectBase->releaseUnoShape();
}

} // rptui


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
