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
#ifndef REPORT_UNDOFACTORY_HXX_INCLUDED
#define REPORT_UNDOFACTORY_HXX_INCLUDED

#include <svx/svdundo.hxx>
#include <memory>
#include "ModuleHelper.hxx"

namespace rptui
{
    class OReportUndoFactory : public SdrUndoFactory
    {
        OModuleClient       m_aModuleClient;
        ::std::auto_ptr<SdrUndoFactory> m_pUndoFactory;

        OReportUndoFactory(const OReportUndoFactory&);
        OReportUndoFactory& operator=(const OReportUndoFactory&);
    public:
        OReportUndoFactory();
        virtual ~OReportUndoFactory();

           // shapes
        virtual SdrUndoAction* CreateUndoMoveObject( SdrObject& rObject );
        virtual SdrUndoAction* CreateUndoMoveObject( SdrObject& rObject, const Size& rDist );
        virtual SdrUndoAction* CreateUndoGeoObject( SdrObject& rObject );
        virtual SdrUndoAction* CreateUndoAttrObject( SdrObject& rObject, bool bStyleSheet1 = false, bool bSaveText = false );
        virtual SdrUndoAction* CreateUndoRemoveObject( SdrObject& rObject, bool bOrdNumDirect = false);
        virtual SdrUndoAction* CreateUndoInsertObject( SdrObject& rObject, bool bOrdNumDirect = false);
        virtual SdrUndoAction* CreateUndoDeleteObject( SdrObject& rObject, bool bOrdNumDirect = false);
        virtual SdrUndoAction* CreateUndoNewObject( SdrObject& rObject, bool bOrdNumDirect = false);
        virtual SdrUndoAction* CreateUndoCopyObject( SdrObject& rObject, bool bOrdNumDirect = false);

        virtual SdrUndoAction* CreateUndoObjectOrdNum( SdrObject& rObject, sal_uInt32 nOldOrdNum1, sal_uInt32 nNewOrdNum1);

        virtual SdrUndoAction* CreateUndoReplaceObject( SdrObject& rOldObject, SdrObject& rNewObject, bool bOrdNumDirect = false );
        virtual SdrUndoAction* CreateUndoObjectLayerChange( SdrObject& rObject, SdrLayerID aOldLayer, SdrLayerID aNewLayer );
        virtual SdrUndoAction* CreateUndoObjectSetText( SdrObject& rNewObj, sal_Int32 nText );

        // layer
        virtual SdrUndoAction* CreateUndoNewLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel);
        virtual SdrUndoAction* CreateUndoDeleteLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel);
        virtual SdrUndoAction* CreateUndoMoveLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel, sal_uInt16 nNeuPos1);

        // page
        virtual SdrUndoAction*  CreateUndoDeletePage(SdrPage& rPage);
        virtual SdrUndoAction* CreateUndoNewPage(SdrPage& rPage);
        virtual SdrUndoAction* CreateUndoCopyPage(SdrPage& rPage);
        virtual SdrUndoAction* CreateUndoSetPageNum(SdrPage& rNewPg, sal_uInt16 nOldPageNum1, sal_uInt16 nNewPageNum1);

        // master page
        virtual SdrUndoAction* CreateUndoPageRemoveMasterPage(SdrPage& rChangedPage);
        virtual SdrUndoAction* CreateUndoPageChangeMasterPage(SdrPage& rChangedPage);

    };
//==============================================================================
} // rptui
//==============================================================================

#endif // REPORT_UNDOFACTORY_HXX_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
