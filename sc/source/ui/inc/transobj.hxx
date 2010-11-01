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

#ifndef SC_TRANSOBJ_HXX
#define SC_TRANSOBJ_HXX

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
//REMOVE        SvEmbeddedObjectRef             aDocShellRef;
//REMOVE        SvEmbeddedObjectRef             aDrawPersistRef;
    SfxObjectShellRef               aDocShellRef;
    SfxObjectShellRef               aDrawPersistRef;
    com::sun::star::uno::Reference<com::sun::star::sheet::XSheetCellRanges> xDragSourceRanges;
    SCCOL                           nDragHandleX;
    SCROW                           nDragHandleY;
    SCTAB                           nVisibleTab;
    USHORT                          nDragSourceFlags;
    BOOL                            bDragWasInternal;
    BOOL                            bUsedForLink;
    bool                            bHasFiltered;       // if has filtered rows

    void        InitDocShell();
    static void StripRefs( ScDocument* pDoc, SCCOL nStartX, SCROW nStartY,
                            SCCOL nEndX, SCROW nEndY,
                            ScDocument* pDestDoc=0,
                            SCCOL nSubX=0, SCROW nSubY=0 );
    static void PaintToDev( OutputDevice* pDev, ScDocument* pDoc, double nPrintFactor,
                            const ScRange& rBlock, BOOL bMetaFile );
    static void GetAreaSize( ScDocument* pDoc, SCTAB nTab1, SCTAB nTab2, SCROW& nRow, SCCOL& nCol );

public:
            ScTransferObj( ScDocument* pClipDoc, const TransferableObjectDescriptor& rDesc );
    virtual ~ScTransferObj();

    virtual void        AddSupportedFormats();
    virtual sal_Bool    GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    virtual sal_Bool    WriteObject( SotStorageStreamRef& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId,
                                        const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    virtual void        ObjectReleased();
    virtual void        DragFinished( sal_Int8 nDropAction );

    ScDocument*         GetDocument()           { return pDoc; }        // owned by ScTransferObj
    const ScRange&      GetRange() const        { return aBlock; }
    SCROW               GetNonFilteredRows() const { return nNonFiltered; }
    SCCOL               GetDragHandleX() const  { return nDragHandleX; }
    SCROW               GetDragHandleY() const  { return nDragHandleY; }
    SCTAB               GetVisibleTab() const   { return nVisibleTab; }
    USHORT              GetDragSourceFlags() const  { return nDragSourceFlags; }
    bool                HasFilteredRows() const { return bHasFiltered; }
    ScDocShell*         GetSourceDocShell();
    ScDocument*         GetSourceDocument();
    ScMarkData          GetSourceMarkData();

    void                SetDrawPersist( const SfxObjectShellRef& rRef );
    void                SetDragHandlePos( SCCOL nX, SCROW nY );
    void                SetVisibleTab( SCTAB nNew );
    void                SetDragSource( ScDocShell* pSourceShell, const ScMarkData& rMark );
    void                SetDragSourceFlags( USHORT nFlags );
    void                SetDragWasInternal();

    static SC_DLLPUBLIC ScTransferObj* GetOwnClipboard( Window* pUIWin );

    static SfxObjectShell*  SetDrawClipDoc( BOOL bAnyOle );     // update ScGlobal::pDrawClipDocShellRef
    virtual sal_Int64 SAL_CALL getSomething( const com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw( com::sun::star::uno::RuntimeException );
    static const com::sun::star::uno::Sequence< sal_Int8 >& getUnoTunnelId();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
