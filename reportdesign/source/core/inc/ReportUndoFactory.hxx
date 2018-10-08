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

namespace rptui
{
    class OReportUndoFactory : public SdrUndoFactory
    {
        ::std::unique_ptr<SdrUndoFactory> m_pUndoFactory;

        OReportUndoFactory(const OReportUndoFactory&) = delete;
        OReportUndoFactory& operator=(const OReportUndoFactory&) = delete;
    public:
        OReportUndoFactory();
        virtual ~OReportUndoFactory() override;

           // shapes
        virtual std::unique_ptr<SdrUndoAction> CreateUndoMoveObject( SdrObject& rObject, const Size& rDist ) override;
        virtual std::unique_ptr<SdrUndoAction> CreateUndoGeoObject( SdrObject& rObject ) override;
        virtual std::unique_ptr<SdrUndoAction> CreateUndoAttrObject( SdrObject& rObject, bool bStyleSheet1 = false, bool bSaveText = false ) override;
        virtual std::unique_ptr<SdrUndoAction> CreateUndoRemoveObject( SdrObject& rObject, bool bOrdNumDirect = false) override;
        virtual std::unique_ptr<SdrUndoAction> CreateUndoInsertObject( SdrObject& rObject, bool bOrdNumDirect = false) override;
        virtual std::unique_ptr<SdrUndoAction> CreateUndoDeleteObject( SdrObject& rObject, bool bOrdNumDirect = false) override;
        virtual std::unique_ptr<SdrUndoAction> CreateUndoNewObject( SdrObject& rObject, bool bOrdNumDirect = false) override;
        virtual std::unique_ptr<SdrUndoAction> CreateUndoCopyObject( SdrObject& rObject, bool bOrdNumDirect = false) override;

        virtual std::unique_ptr<SdrUndoAction> CreateUndoObjectOrdNum( SdrObject& rObject, sal_uInt32 nOldOrdNum1, sal_uInt32 nNewOrdNum1) override;

        virtual std::unique_ptr<SdrUndoAction> CreateUndoReplaceObject( SdrObject& rOldObject, SdrObject& rNewObject, bool bOrdNumDirect = false ) override;
        virtual std::unique_ptr<SdrUndoAction> CreateUndoObjectLayerChange( SdrObject& rObject, SdrLayerID aOldLayer, SdrLayerID aNewLayer ) override;
        virtual std::unique_ptr<SdrUndoAction> CreateUndoObjectSetText( SdrObject& rNewObj, sal_Int32 nText ) override;

        // layer
        virtual std::unique_ptr<SdrUndoAction> CreateUndoNewLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel) override;
        virtual std::unique_ptr<SdrUndoAction> CreateUndoDeleteLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel) override;

        // page
        virtual std::unique_ptr<SdrUndoAction> CreateUndoDeletePage(SdrPage& rPage) override;
        virtual std::unique_ptr<SdrUndoAction> CreateUndoNewPage(SdrPage& rPage) override;
        virtual std::unique_ptr<SdrUndoAction> CreateUndoCopyPage(SdrPage& rPage) override;
        virtual std::unique_ptr<SdrUndoAction> CreateUndoSetPageNum(SdrPage& rNewPg, sal_uInt16 nOldPageNum1, sal_uInt16 nNewPageNum1) override;

        // master page
        virtual std::unique_ptr<SdrUndoAction> CreateUndoPageRemoveMasterPage(SdrPage& rChangedPage) override;
        virtual std::unique_ptr<SdrUndoAction> CreateUndoPageChangeMasterPage(SdrPage& rChangedPage) override;

    };

} // rptui


#endif // INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_REPORTUNDOFACTORY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
