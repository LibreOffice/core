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

#include <svtools/transfer.hxx>
#include "global.hxx"
#include "address.hxx"

class ScDocShell;
class ScMarkData;
class SfxObjectShell;

namespace com { namespace sun { namespace star {
    namespace sheet {
        class XSheetCellRanges;
    }
}}}

#include <sfx2/objsh.hxx>

class ScTransferObj : public TransferableHelper
{
private:
    ScDocument*                     pDoc;
    ScRange                         aBlock;
    SCROW                           nNonFiltered;       // non-filtered rows
    TransferableDataHelper          aOleData;
    TransferableObjectDescriptor    aObjDesc;
    SfxObjectShellRef               aDocShellRef;
    SfxObjectShellRef               aDrawPersistRef;
    css::uno::Reference<css::sheet::XSheetCellRanges> xDragSourceRanges;
    SCCOL                           nDragHandleX;
    SCROW                           nDragHandleY;
    SCTAB                           nVisibleTab;
    sal_uInt16                      nDragSourceFlags;
    bool                            bDragWasInternal;
    bool                            bUsedForLink;
    bool                            bHasFiltered;       // if has filtered rows
    bool                            bUseInApi;          // to recognize clipboard content copied from API

    // #i123405# added parameter to allow size calculation without limitation
    // to PageSize, e.g. used for Metafile creation for clipboard.
    void        InitDocShell(bool bLimitToPageSize);
    static void StripRefs( ScDocument* pDoc, SCCOL nStartX, SCROW nStartY,
                            SCCOL nEndX, SCROW nEndY,
                            ScDocument* pDestDoc=nullptr,
                            SCCOL nSubX=0, SCROW nSubY=0 );
    static void PaintToDev( OutputDevice* pDev, ScDocument* pDoc, double nPrintFactor,
                            const ScRange& rBlock, bool bMetaFile );
    static void GetAreaSize( ScDocument* pDoc, SCTAB nTab1, SCTAB nTab2, SCROW& nRow, SCCOL& nCol );

public:
            ScTransferObj( ScDocument* pClipDoc, const TransferableObjectDescriptor& rDesc );
    virtual ~ScTransferObj();

    virtual void        AddSupportedFormats() override;
    virtual bool GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc ) override;
    virtual bool        WriteObject( tools::SvRef<SotStorageStream>& rxOStm, void* pUserObject, SotClipboardFormatId nUserObjectId,
                                        const css::datatransfer::DataFlavor& rFlavor ) override;
    virtual void        ObjectReleased() override;
    virtual void        DragFinished( sal_Int8 nDropAction ) override;

    ScDocument*         GetDocument()           { return pDoc; }        // owned by ScTransferObj
    const ScRange&      GetRange() const        { return aBlock; }
    SCROW               GetNonFilteredRows() const { return nNonFiltered; }
    SCCOL               GetDragHandleX() const  { return nDragHandleX; }
    SCROW               GetDragHandleY() const  { return nDragHandleY; }
    SCTAB               GetVisibleTab() const   { return nVisibleTab; }
    sal_uInt16              GetDragSourceFlags() const  { return nDragSourceFlags; }
    bool                HasFilteredRows() const { return bHasFiltered; }
    bool                GetUseInApi() const     { return bUseInApi; }
    ScDocShell*         GetSourceDocShell();
    ScDocument*         GetSourceDocument();
    ScMarkData          GetSourceMarkData();

    void                SetDrawPersist( const SfxObjectShellRef& rRef );
    void                SetDragHandlePos( SCCOL nX, SCROW nY );
    void                SetVisibleTab( SCTAB nNew );
    void                SetDragSource( ScDocShell* pSourceShell, const ScMarkData& rMark );
    void                SetDragSourceFlags( sal_uInt16 nFlags );
    void                SetDragWasInternal();
    SC_DLLPUBLIC void   SetUseInApi( bool bSet );

    static SC_DLLPUBLIC ScTransferObj* GetOwnClipboard( vcl::Window* pUIWin );

    static SfxObjectShell*  SetDrawClipDoc( bool bAnyOle );     // update ScGlobal::xDrawClipDocShellRef
    virtual sal_Int64 SAL_CALL getSomething( const com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw( com::sun::star::uno::RuntimeException, std::exception ) override;
    static const com::sun::star::uno::Sequence< sal_Int8 >& getUnoTunnelId();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
