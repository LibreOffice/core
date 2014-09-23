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

#ifndef INCLUDED_SC_SOURCE_UI_INC_DRWTRANS_HXX
#define INCLUDED_SC_SOURCE_UI_INC_DRWTRANS_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <svtools/transfer.hxx>

#include <sfx2/objsh.hxx>
#include "global.hxx"
#include "charthelper.hxx"
#include "rangelst.hxx"

class SdrModel;
class ScDocShell;
class INetBookmark;
class SdrObject;
class SdrView;
class ScDrawView;
class SdrOle2Obj;

class ScDrawTransferObj : public TransferableHelper
{
private:
    SdrModel*                       pModel;
    TransferableDataHelper          aOleData;
    TransferableObjectDescriptor    aObjDesc;
    SfxObjectShellRef               aDocShellRef;
    SfxObjectShellRef               aDrawPersistRef;

                                    // extracted from model in ctor:
    Size                            aSrcSize;
    INetBookmark*                   pBookmark;
    bool                            bGraphic;
    bool                            bGrIsBit;
    bool                            bOleObj;
                                    // source information for drag&drop:
                                    // (view is needed to handle drawing obejcts)
    SdrView*                        pDragSourceView;
    sal_uInt16                      nDragSourceFlags;
    bool                            bDragWasInternal;

    sal_uInt32                      nSourceDocID;

    ScRangeListVector               m_aProtectedChartRangesVector;

    OUString maShellID;

    void                InitDocShell();
    SdrOle2Obj* GetSingleObject();

    void CreateOLEData();

public:
            ScDrawTransferObj( SdrModel* pClipModel, ScDocShell* pContainerShell,
                                const TransferableObjectDescriptor& rDesc );
    virtual ~ScDrawTransferObj();

    virtual void        AddSupportedFormats() SAL_OVERRIDE;
    virtual bool GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc ) SAL_OVERRIDE;
    virtual bool        WriteObject( SotStorageStreamRef& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId,
                                        const ::com::sun::star::datatransfer::DataFlavor& rFlavor ) SAL_OVERRIDE;
    virtual void        ObjectReleased() SAL_OVERRIDE;
    virtual void        DragFinished( sal_Int8 nDropAction ) SAL_OVERRIDE;

    SdrModel*           GetModel()  { return pModel; }

    void                SetDrawPersist( const SfxObjectShellRef& rRef );
    void                SetDragSource( ScDrawView* pView );
    void                SetDragSourceObj( SdrObject* pObj, SCTAB nTab );
    void                SetDragSourceFlags( sal_uInt16 nFlags );
    void                SetDragWasInternal();

    OUString GetShellID() const;

    SdrView*            GetDragSourceView()             { return pDragSourceView; }
    sal_uInt16              GetDragSourceFlags() const      { return nDragSourceFlags; }

    void                SetSourceDocID( sal_uInt32 nVal )
                            { nSourceDocID = nVal; }
    sal_uInt32          GetSourceDocID() const      { return nSourceDocID; }

    static ScDrawTransferObj* GetOwnClipboard( vcl::Window* pUIWin );
    virtual sal_Int64 SAL_CALL getSomething( const com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    static const com::sun::star::uno::Sequence< sal_Int8 >& getUnoTunnelId();

    const ScRangeListVector& GetProtectedChartRangesVector() const { return m_aProtectedChartRangesVector; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
