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

#ifndef INCLUDED_SC_SOURCE_UI_INC_TRANSOBJ_HXX
#define INCLUDED_SC_SOURCE_UI_INC_TRANSOBJ_HXX

#include <vcl/transfer.hxx>
#include <address.hxx>
#include <document.hxx>
#include <sfx2/objsh.hxx>


class ScDocShell;
class ScMarkData;
enum class ScDragSrc;

namespace com::sun::star {
    namespace sheet {
        class XSheetCellRanges;
    }
}

class SAL_DLLPUBLIC_RTTI ScTransferObj : public TransferDataContainer
{
private:
    ScDocumentUniquePtr             m_pDoc;
    ScRange                         m_aBlock;
    SCROW                           m_nNonFiltered;       // non-filtered rows
    TransferableObjectDescriptor    m_aObjDesc;
    SfxObjectShellRef               m_aDocShellRef;
    SfxObjectShellRef               m_aDrawPersistRef;
    css::uno::Reference<css::sheet::XSheetCellRanges> m_xDragSourceRanges;
    SCCOL                           m_nDragHandleX;
    SCROW                           m_nDragHandleY;
    SCCOL                           m_nSourceCursorX;
    SCROW                           m_nSourceCursorY;
    SCTAB                           m_nVisibleTab;
    ScDragSrc                       m_nDragSourceFlags;
    bool                            m_bDragWasInternal;
    bool                            m_bUsedForLink;
    bool                            m_bHasFiltered;       // if has filtered rows
    bool                            m_bUseInApi;          // to recognize clipboard content copied from API

    // #i123405# added parameter to allow size calculation without limitation
    // to PageSize, e.g. used for Metafile creation for clipboard.
    void        InitDocShell(bool bLimitToPageSize);
    static void StripRefs( ScDocument& rDoc, SCCOL nStartX, SCROW nStartY,
                            SCCOL nEndX, SCROW nEndY,
                            ScDocument& rDestDoc );
    static void PaintToDev( OutputDevice* pDev, ScDocument& rDoc, double nPrintFactor,
                            const ScRange& rBlock );
    static void GetAreaSize( const ScDocument& rDoc, SCTAB nTab1, SCTAB nTab2, SCROW& nRow, SCCOL& nCol );

public:
            ScTransferObj( ScDocumentUniquePtr pClipDoc, const TransferableObjectDescriptor& rDesc );
    virtual ~ScTransferObj() override;

    virtual void        AddSupportedFormats() override;
    virtual bool GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc ) override;
    virtual bool        WriteObject( tools::SvRef<SotTempStream>& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId,
                                        const css::datatransfer::DataFlavor& rFlavor ) override;
    virtual void        DragFinished( sal_Int8 nDropAction ) override;
    virtual sal_Bool SAL_CALL isComplex() override;

    ScDocument*         GetDocument() const     { return m_pDoc.get(); }        // owned by ScTransferObj
    const ScRange&      GetRange() const        { return m_aBlock; }
    SCROW               GetNonFilteredRows() const { return m_nNonFiltered; }
    SCCOL               GetDragHandleX() const  { return m_nDragHandleX; }
    SCROW               GetDragHandleY() const  { return m_nDragHandleY; }
    bool                WasSourceCursorInSelection() const;
    SCCOL               GetSourceCursorX() const  { return m_nSourceCursorX; }
    SCROW               GetSourceCursorY() const  { return m_nSourceCursorY; }
    SCTAB               GetVisibleTab() const   { return m_nVisibleTab; }
    ScDragSrc           GetDragSourceFlags() const  { return m_nDragSourceFlags; }
    bool                HasFilteredRows() const { return m_bHasFiltered; }
    bool                GetUseInApi() const     { return m_bUseInApi; }
    ScDocShell*         GetSourceDocShell();
    ScDocument*         GetSourceDocument();
    ScMarkData          GetSourceMarkData() const;

    void                SetDrawPersist( const SfxObjectShellRef& rRef );
    void                SetDragHandlePos( SCCOL nX, SCROW nY );
    void                SetSourceCursorPos( SCCOL nX, SCROW nY );
    void                SetVisibleTab( SCTAB nNew );
    void                SetDragSource( ScDocShell* pSourceShell, const ScMarkData& rMark );
    void                SetDragSourceFlags( ScDragSrc nFlags );
    void                SetDragWasInternal();
    SC_DLLPUBLIC void   SetUseInApi( bool bSet );

    static  SC_DLLPUBLIC ScTransferObj* GetOwnClipboard(const css::uno::Reference<css::datatransfer::XTransferable2>&);

    static SfxObjectShell*  SetDrawClipDoc( bool bAnyOle );     // update ScGlobal::xDrawClipDocShellRef
    virtual sal_Int64 SAL_CALL getSomething( const com::sun::star::uno::Sequence< sal_Int8 >& rId ) override;
    static const com::sun::star::uno::Sequence< sal_Int8 >& getUnoTunnelId();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
