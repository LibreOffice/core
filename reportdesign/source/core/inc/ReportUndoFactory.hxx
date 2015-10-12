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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_REPORTUNDOFACTORY_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_REPORTUNDOFACTORY_HXX

#include <svx/svdundo.hxx>
#include <memory>
#include "ModuleHelper.hxx"

namespace rptui
{
    class OReportUndoFactory : public SdrUndoFactory
    {
        OModuleClient       m_aModuleClient;
        ::std::unique_ptr<SdrUndoFactory> m_pUndoFactory;

        OReportUndoFactory(const OReportUndoFactory&) = delete;
        OReportUndoFactory& operator=(const OReportUndoFactory&) = delete;
    public:
        OReportUndoFactory();
        virtual ~OReportUndoFactory();

           // shapes
        virtual SdrUndoAction* CreateUndoMoveObject( SdrObject& rObject ) SAL_OVERRIDE;
        virtual SdrUndoAction* CreateUndoMoveObject( SdrObject& rObject, const Size& rDist ) SAL_OVERRIDE;
        virtual SdrUndoAction* CreateUndoGeoObject( SdrObject& rObject ) SAL_OVERRIDE;
        virtual SdrUndoAction* CreateUndoAttrObject( SdrObject& rObject, bool bStyleSheet1 = false, bool bSaveText = false ) SAL_OVERRIDE;
        virtual SdrUndoAction* CreateUndoRemoveObject( SdrObject& rObject, bool bOrdNumDirect = false) SAL_OVERRIDE;
        virtual SdrUndoAction* CreateUndoInsertObject( SdrObject& rObject, bool bOrdNumDirect = false) SAL_OVERRIDE;
        virtual SdrUndoAction* CreateUndoDeleteObject( SdrObject& rObject, bool bOrdNumDirect = false) SAL_OVERRIDE;
        virtual SdrUndoAction* CreateUndoNewObject( SdrObject& rObject, bool bOrdNumDirect = false) SAL_OVERRIDE;
        virtual SdrUndoAction* CreateUndoCopyObject( SdrObject& rObject, bool bOrdNumDirect = false) SAL_OVERRIDE;

        virtual SdrUndoAction* CreateUndoObjectOrdNum( SdrObject& rObject, sal_uInt32 nOldOrdNum1, sal_uInt32 nNewOrdNum1) SAL_OVERRIDE;

        virtual SdrUndoAction* CreateUndoReplaceObject( SdrObject& rOldObject, SdrObject& rNewObject, bool bOrdNumDirect = false ) SAL_OVERRIDE;
        virtual SdrUndoAction* CreateUndoObjectLayerChange( SdrObject& rObject, SdrLayerID aOldLayer, SdrLayerID aNewLayer ) SAL_OVERRIDE;
        virtual SdrUndoAction* CreateUndoObjectSetText( SdrObject& rNewObj, sal_Int32 nText ) SAL_OVERRIDE;

        // layer
        virtual SdrUndoAction* CreateUndoNewLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel) SAL_OVERRIDE;
        virtual SdrUndoAction* CreateUndoDeleteLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel) SAL_OVERRIDE;
        virtual SdrUndoAction* CreateUndoMoveLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel, sal_uInt16 nNeuPos1) SAL_OVERRIDE;

        // page
        virtual SdrUndoAction*  CreateUndoDeletePage(SdrPage& rPage) SAL_OVERRIDE;
        virtual SdrUndoAction* CreateUndoNewPage(SdrPage& rPage) SAL_OVERRIDE;
        virtual SdrUndoAction* CreateUndoCopyPage(SdrPage& rPage) SAL_OVERRIDE;
        virtual SdrUndoAction* CreateUndoSetPageNum(SdrPage& rNewPg, sal_uInt16 nOldPageNum1, sal_uInt16 nNewPageNum1) SAL_OVERRIDE;

        // master page
        virtual SdrUndoAction* CreateUndoPageRemoveMasterPage(SdrPage& rChangedPage) SAL_OVERRIDE;
        virtual SdrUndoAction* CreateUndoPageChangeMasterPage(SdrPage& rChangedPage) SAL_OVERRIDE;

    };

} // rptui


#endif // INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_REPORTUNDOFACTORY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
