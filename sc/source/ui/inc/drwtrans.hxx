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
#include <vcl/transfer.hxx>

#include <sfx2/objsh.hxx>
#include <charthelper.hxx>

class SdrModel;
class ScDocShell;
class INetBookmark;
class SdrObject;
class SdrView;
class ScDrawView;
class SdrOle2Obj;
enum class ScDragSrc;

class ScDrawTransferObj : public TransferDataContainer
{
private:
    std::unique_ptr<SdrModel>       m_pModel;
    TransferableDataHelper          m_aOleData;
    TransferableObjectDescriptor    m_aObjDesc;
    SfxObjectShellRef               m_aDocShellRef;
    SfxObjectShellRef               m_aDrawPersistRef;

                                    // extracted from model in ctor:
    Size                            m_aSrcSize;
    std::unique_ptr<INetBookmark>   m_pBookmark;
    bool                            m_bGraphic;
    bool                            m_bGrIsBit;
    bool                            m_bOleObj;
                                    // source information for drag&drop:
                                    // (view is needed to handle drawing objects)
    std::unique_ptr<SdrView>        m_pDragSourceView;
    ScDragSrc                       m_nDragSourceFlags;
    bool                            m_bDragWasInternal;

    ScRangeListVector               m_aProtectedChartRangesVector;

    OUString maShellID;

    void                InitDocShell();
    SdrOle2Obj* GetSingleObject();

    void CreateOLEData();

public:
            ScDrawTransferObj( std::unique_ptr<SdrModel> pClipModel, ScDocShell* pContainerShell,
                                const TransferableObjectDescriptor& rDesc );
    virtual ~ScDrawTransferObj() override;

    virtual void        AddSupportedFormats() override;
    virtual bool GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc ) override;
    virtual bool        WriteObject( tools::SvRef<SotStorageStream>& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId,
                                        const css::datatransfer::DataFlavor& rFlavor ) override;
    virtual void        DragFinished( sal_Int8 nDropAction ) override;

    SdrModel*           GetModel() const { return m_pModel.get(); }

    void                SetDrawPersist( const SfxObjectShellRef& rRef );
    void                SetDragSource( const ScDrawView* pView );
    void                SetDragSourceObj( SdrObject& rObj, SCTAB nTab );
    void                SetDragSourceFlags( ScDragSrc nFlags );
    void                SetDragWasInternal();

    const OUString& GetShellID() const;

    SdrView*            GetDragSourceView()             { return m_pDragSourceView.get(); }
    ScDragSrc           GetDragSourceFlags() const      { return m_nDragSourceFlags; }

    static ScDrawTransferObj* GetOwnClipboard(const css::uno::Reference<css::datatransfer::XTransferable2>&);

    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& rId ) override;
    static const css::uno::Sequence< sal_Int8 >& getUnoTunnelId();

    const ScRangeListVector& GetProtectedChartRangesVector() const { return m_aProtectedChartRangesVector; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
