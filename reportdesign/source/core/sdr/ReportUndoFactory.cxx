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
#include <ReportUndoFactory.hxx>
#include <RptObject.hxx>
#include <UndoActions.hxx>
#include <strings.hrc>

namespace rptui
{
    using namespace ::com::sun::star;

static std::unique_ptr<SdrUndoAction> lcl_createUndo(SdrObject& rObject, Action _eAction, const char* pCommentId)
{
    OObjectBase* pObj = dynamic_cast<OObjectBase*>(&rObject);
    if ( !pObj )
        return nullptr;
    uno::Reference< report::XReportComponent> xReportComponent = pObj->getReportComponent();
    uno::Reference< report::XSection> xSection = pObj->getSection();
    uno::Reference< report::XGroup> xGroup = xSection->getGroup();
    if ( xGroup.is() )
        return std::make_unique<OUndoGroupSectionAction>(rObject.getSdrModelFromSdrObject(),_eAction,OGroupHelper::getMemberFunction(xSection),xGroup,xReportComponent,pCommentId);
    else
        return std::make_unique<OUndoReportSectionAction>(rObject.getSdrModelFromSdrObject(),_eAction,OReportHelper::getMemberFunction(xSection),xSection->getReportDefinition(),xReportComponent,pCommentId);
}


OReportUndoFactory::OReportUndoFactory() : m_pUndoFactory(new SdrUndoFactory)
{
}

OReportUndoFactory::~OReportUndoFactory()
{
}

std::unique_ptr<SdrUndoAction> OReportUndoFactory::CreateUndoMoveObject( SdrObject& rObject, const Size& rDist )
{
    return m_pUndoFactory->CreateUndoMoveObject( rObject, rDist );
}

std::unique_ptr<SdrUndoAction> OReportUndoFactory::CreateUndoGeoObject( SdrObject& rObject )
{
    return m_pUndoFactory->CreateUndoGeoObject( rObject );
}

std::unique_ptr<SdrUndoAction> OReportUndoFactory::CreateUndoAttrObject( SdrObject& rObject, bool bStyleSheet1, bool bSaveText )
{
    return m_pUndoFactory->CreateUndoAttrObject( rObject, bStyleSheet1, bSaveText );
}

std::unique_ptr<SdrUndoAction> OReportUndoFactory::CreateUndoRemoveObject( SdrObject& rObject, bool bOrdNumDirect )
{
    return m_pUndoFactory->CreateUndoRemoveObject( rObject, bOrdNumDirect );
}

std::unique_ptr<SdrUndoAction> OReportUndoFactory::CreateUndoInsertObject( SdrObject& rObject, bool /*bOrdNumDirect*/ )
{
    return lcl_createUndo(rObject,rptui::Inserted,RID_STR_UNDO_INSERT_CONTROL);
}

std::unique_ptr<SdrUndoAction> OReportUndoFactory::CreateUndoDeleteObject( SdrObject& rObject, bool /*bOrdNumDirect*/ )
{
    return lcl_createUndo(rObject,rptui::Removed,RID_STR_UNDO_DELETE_CONTROL);
}

std::unique_ptr<SdrUndoAction> OReportUndoFactory::CreateUndoNewObject( SdrObject& rObject, bool /*bOrdNumDirect*/ )
{
    return lcl_createUndo(rObject,rptui::Inserted,RID_STR_UNDO_INSERT_CONTROL);
}

std::unique_ptr<SdrUndoAction> OReportUndoFactory::CreateUndoCopyObject( SdrObject& rObject, bool bOrdNumDirect )
{
    return m_pUndoFactory->CreateUndoCopyObject( rObject, bOrdNumDirect );
}

std::unique_ptr<SdrUndoAction> OReportUndoFactory::CreateUndoObjectOrdNum( SdrObject& rObject, sal_uInt32 nOldOrdNum1, sal_uInt32 nNewOrdNum1)
{
    return m_pUndoFactory->CreateUndoObjectOrdNum( rObject, nOldOrdNum1, nNewOrdNum1 );
}

std::unique_ptr<SdrUndoAction> OReportUndoFactory::CreateUndoReplaceObject( SdrObject& rOldObject, SdrObject& rNewObject, bool bOrdNumDirect )
{
    return m_pUndoFactory->CreateUndoReplaceObject( rOldObject, rNewObject, bOrdNumDirect );
}

std::unique_ptr<SdrUndoAction> OReportUndoFactory::CreateUndoObjectLayerChange( SdrObject& rObject, SdrLayerID aOldLayer, SdrLayerID aNewLayer )
{
    return m_pUndoFactory->CreateUndoObjectLayerChange( rObject, aOldLayer, aNewLayer );
}

std::unique_ptr<SdrUndoAction> OReportUndoFactory::CreateUndoObjectSetText( SdrObject& rNewObj, sal_Int32 nText )
{
    return m_pUndoFactory->CreateUndoObjectSetText( rNewObj, nText );
}

// layer
std::unique_ptr<SdrUndoAction> OReportUndoFactory::CreateUndoNewLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel)
{
    return m_pUndoFactory->CreateUndoNewLayer( nLayerNum, rNewLayerAdmin, rNewModel );
}

std::unique_ptr<SdrUndoAction> OReportUndoFactory::CreateUndoDeleteLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel)
{
    return m_pUndoFactory->CreateUndoDeleteLayer( nLayerNum, rNewLayerAdmin, rNewModel );
}

// page
std::unique_ptr<SdrUndoAction>  OReportUndoFactory::CreateUndoDeletePage(SdrPage& rPage)
{
    return m_pUndoFactory->CreateUndoDeletePage(rPage);
}

std::unique_ptr<SdrUndoAction> OReportUndoFactory::CreateUndoNewPage(SdrPage& rPage)
{
    return m_pUndoFactory->CreateUndoNewPage( rPage );
}

std::unique_ptr<SdrUndoAction> OReportUndoFactory::CreateUndoCopyPage(SdrPage& rPage)
{
    return m_pUndoFactory->CreateUndoCopyPage( rPage );
}

std::unique_ptr<SdrUndoAction> OReportUndoFactory::CreateUndoSetPageNum(SdrPage& rNewPg, sal_uInt16 nOldPageNum1, sal_uInt16 nNewPageNum1)
{
    return m_pUndoFactory->CreateUndoSetPageNum( rNewPg, nOldPageNum1, nNewPageNum1 );
}
    // master page
std::unique_ptr<SdrUndoAction> OReportUndoFactory::CreateUndoPageRemoveMasterPage(SdrPage& rChangedPage)
{
    return m_pUndoFactory->CreateUndoPageRemoveMasterPage( rChangedPage );
}

std::unique_ptr<SdrUndoAction> OReportUndoFactory::CreateUndoPageChangeMasterPage(SdrPage& rChangedPage)
{
    return m_pUndoFactory->CreateUndoPageChangeMasterPage(rChangedPage);
}


}   //rptui


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
