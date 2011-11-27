/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#include "ReportUndoFactory.hxx"
#include "RptObject.hxx"
#include "UndoActions.hxx"
#include "RptResId.hrc"

namespace rptui
{
    using namespace ::com::sun::star;
// -----------------------------------------------------------------------------
SdrUndoAction* lcl_createUndo(SdrObject& rObject,Action _eAction,sal_uInt16 _nCommentId)
{
    OObjectBase* pObj = dynamic_cast<OObjectBase*>(&rObject);
    if ( !pObj )
        return NULL;
    uno::Reference< report::XReportComponent> xReportComponent = pObj->getReportComponent();
    uno::Reference< report::XSection> xSection = pObj->getSection();
    uno::Reference< report::XGroup> xGroup = xSection->getGroup();
    SdrUndoAction* pUndo = NULL;
    if ( xGroup.is() )
        pUndo = new OUndoGroupSectionAction(*rObject.GetModel(),_eAction,OGroupHelper::getMemberFunction(xSection),xGroup,xReportComponent,_nCommentId);
    else
        pUndo = new OUndoReportSectionAction(*rObject.GetModel(),_eAction,OReportHelper::getMemberFunction(xSection),xSection->getReportDefinition(),xReportComponent,_nCommentId);
    return pUndo;
}
// -----------------------------------------------------------------------------
DBG_NAME( rpt_OReportUndoFactory )
// -----------------------------------------------------------------------------
OReportUndoFactory::OReportUndoFactory() : m_pUndoFactory(new SdrUndoFactory)
{
    DBG_CTOR( rpt_OReportUndoFactory,NULL);
}
// -----------------------------------------------------------------------------
OReportUndoFactory::~OReportUndoFactory()
{
    DBG_DTOR( rpt_OReportUndoFactory,NULL);
}
///////////////////////////////////////////////////////////////////////
// shapes
SdrUndoAction* OReportUndoFactory::CreateUndoMoveObject( SdrObject& rObject )
{
    return m_pUndoFactory->CreateUndoMoveObject( rObject );
}

SdrUndoAction* OReportUndoFactory::CreateUndoMoveObject( SdrObject& rObject, const Size& rDist )
{
    return m_pUndoFactory->CreateUndoMoveObject( rObject, rDist );
}

SdrUndoAction* OReportUndoFactory::CreateUndoGeoObject( SdrObject& rObject )
{
    return m_pUndoFactory->CreateUndoGeoObject( rObject );
}

SdrUndoAction* OReportUndoFactory::CreateUndoAttrObject( SdrObject& rObject, bool bStyleSheet1, bool bSaveText )
{
    return m_pUndoFactory->CreateUndoAttrObject( rObject, bStyleSheet1 ? sal_True : sal_False, bSaveText ? sal_True : sal_False );
}

SdrUndoAction* OReportUndoFactory::CreateUndoRemoveObject( SdrObject& rObject, bool bOrdNumDirect )
{
    return m_pUndoFactory->CreateUndoRemoveObject( rObject, bOrdNumDirect ? sal_True : sal_False );
}

SdrUndoAction* OReportUndoFactory::CreateUndoInsertObject( SdrObject& rObject, bool /*bOrdNumDirect*/ )
{
    return lcl_createUndo(rObject,rptui::Inserted,RID_STR_UNDO_INSERT_CONTROL);
}

SdrUndoAction* OReportUndoFactory::CreateUndoDeleteObject( SdrObject& rObject, bool /*bOrdNumDirect*/ )
{
    return lcl_createUndo(rObject,rptui::Removed,RID_STR_UNDO_DELETE_CONTROL);
}

SdrUndoAction* OReportUndoFactory::CreateUndoNewObject( SdrObject& rObject, bool /*bOrdNumDirect*/ )
{
    return lcl_createUndo(rObject,rptui::Inserted,RID_STR_UNDO_INSERT_CONTROL);
}

SdrUndoAction* OReportUndoFactory::CreateUndoCopyObject( SdrObject& rObject, bool bOrdNumDirect )
{
    return m_pUndoFactory->CreateUndoCopyObject( rObject, bOrdNumDirect ? sal_True : sal_False );
}

SdrUndoAction* OReportUndoFactory::CreateUndoObjectOrdNum( SdrObject& rObject, sal_uInt32 nOldOrdNum1, sal_uInt32 nNewOrdNum1)
{
    return m_pUndoFactory->CreateUndoObjectOrdNum( rObject, nOldOrdNum1, nNewOrdNum1 );
}

SdrUndoAction* OReportUndoFactory::CreateUndoReplaceObject( SdrObject& rOldObject, SdrObject& rNewObject, bool bOrdNumDirect )
{
    return m_pUndoFactory->CreateUndoReplaceObject( rOldObject, rNewObject, bOrdNumDirect ? sal_True : sal_False );
}

SdrUndoAction* OReportUndoFactory::CreateUndoObjectLayerChange( SdrObject& rObject, SdrLayerID aOldLayer, SdrLayerID aNewLayer )
{
    return m_pUndoFactory->CreateUndoObjectLayerChange( rObject, aOldLayer, aNewLayer );
}

SdrUndoAction* OReportUndoFactory::CreateUndoObjectSetText( SdrObject& rNewObj, sal_Int32 nText )
{
    return m_pUndoFactory->CreateUndoObjectSetText( rNewObj, nText );
}

// layer
SdrUndoAction* OReportUndoFactory::CreateUndoNewLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel)
{
    return m_pUndoFactory->CreateUndoNewLayer( nLayerNum, rNewLayerAdmin, rNewModel );
}

SdrUndoAction* OReportUndoFactory::CreateUndoDeleteLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel)
{
    return m_pUndoFactory->CreateUndoDeleteLayer( nLayerNum, rNewLayerAdmin, rNewModel );
}

SdrUndoAction* OReportUndoFactory::CreateUndoMoveLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel, sal_uInt16 nNeuPos1)
{
    return m_pUndoFactory->CreateUndoMoveLayer( nLayerNum, rNewLayerAdmin, rNewModel, nNeuPos1 );
}

// page
SdrUndoAction*  OReportUndoFactory::CreateUndoDeletePage(SdrPage& rPage)
{
    return m_pUndoFactory->CreateUndoDeletePage( rPage );
}

SdrUndoAction* OReportUndoFactory::CreateUndoNewPage(SdrPage& rPage)
{
    return m_pUndoFactory->CreateUndoNewPage( rPage );
}

SdrUndoAction* OReportUndoFactory::CreateUndoCopyPage(SdrPage& rPage)
{
    return m_pUndoFactory->CreateUndoCopyPage( rPage );
}

SdrUndoAction* OReportUndoFactory::CreateUndoSetPageNum(SdrPage& rNewPg, sal_uInt16 nOldPageNum1, sal_uInt16 nNewPageNum1)
{
    return m_pUndoFactory->CreateUndoSetPageNum( rNewPg, nOldPageNum1, nNewPageNum1 );
}
    // master page
SdrUndoAction* OReportUndoFactory::CreateUndoPageRemoveMasterPage(SdrPage& rChangedPage)
{
    return m_pUndoFactory->CreateUndoPageRemoveMasterPage( rChangedPage );
}

SdrUndoAction* OReportUndoFactory::CreateUndoPageChangeMasterPage(SdrPage& rChangedPage)
{
    return m_pUndoFactory->CreateUndoPageChangeMasterPage(rChangedPage);
}

//==================================================================
}   //rptui
//==================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
