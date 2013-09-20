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
#ifndef _SWDTFLVR_HXX
#define _SWDTFLVR_HXX

#include <sfx2/objsh.hxx>

#include <svtools/transfer.hxx>
#include <vcl/graph.hxx>
#include <sfx2/lnkbase.hxx>
#include <com/sun/star/embed/XEmbeddedObject.hpp>

class Graphic;
class ImageMap;
class INetBookmark;
class INetImage;
class SwDoc;
class SwDocFac;
class SwTextBlocks;
class SwWrtShell;
class SvxClipboardFmtItem;
class ViewShell;
class SwView_Impl;

typedef sal_uInt16 TransferBufferType;
namespace nsTransferBufferType
{
    const sal_uInt16 TRNSFR_NONE            = 0x0000;
    const sal_uInt16 TRNSFR_DOCUMENT        = 0x0001;
    const sal_uInt16 TRNSFR_DOCUMENT_WORD   = 0x0002;
    const sal_uInt16 TRNSFR_GRAPHIC         = 0x0004;
    const sal_uInt16 TRNSFR_TABELLE         = 0x0008;
    const sal_uInt16 TRNSFR_DDELINK         = 0x0010;
    const sal_uInt16 TRNSFR_OLE             = 0x0020;
    const sal_uInt16 TRNSFR_INETFLD         = 0x0040;
    const sal_uInt16 TRNSFR_DRAWING         = 0x0081;   // drawing is internal too!
}


class SW_DLLPUBLIC SwTransferable : public TransferableHelper
{
    friend class SwView_Impl;
    SfxObjectShellLock             aDocShellRef;
    TransferableDataHelper          aOleData;
    TransferableObjectDescriptor    aObjDesc;
    ::sfx2::SvBaseLinkRef            refDdeLink;

    SwWrtShell      *pWrtShell;
    /* #96392# Added pCreatorView to distinguish SwFrameShell from
       SwWrtShell. */
    const ViewShell       *pCreatorView;
    SwDocFac        *pClpDocFac;
    Graphic         *pClpGraphic, *pClpBitmap, *pOrigGrf;
    INetBookmark    *pBkmk;     // URL and description!
    ImageMap        *pImageMap;
    INetImage       *pTargetURL;

    TransferBufferType eBufferType;

    sal_Bool bOldIdle   :1; //D&D Idle flag from the viewsettings
    sal_Bool bCleanUp   :1; //D&D cleanup after Drop (not by internal Drop)

    // helper methods for the copy
    css::uno::Reference < css::embed::XEmbeddedObject > FindOLEObj( sal_Int64& nAspect ) const;
    const Graphic* FindOLEReplacementGraphic() const;
    void DeleteSelection();

    // helper methods for the paste
    static SwTransferable* GetSwTransferable( const TransferableDataHelper& rData );
    static void SetSelInShell( SwWrtShell& , sal_Bool , const Point* );
    static sal_Bool _CheckForURLOrLNKFile( TransferableDataHelper& rData,
                                OUString& rFileName, OUString* pTitle = 0 );
    static int _TestAllowedFormat( const TransferableDataHelper& rData,
                                        sal_uLong nFormat, sal_uInt16 nDestination );

    static int _PasteFileContent( TransferableDataHelper&,
                                    SwWrtShell& rSh, sal_uLong nFmt, sal_Bool bMsg );
    static int _PasteOLE( TransferableDataHelper& rData, SwWrtShell& rSh,
                            sal_uLong nFmt, sal_uInt8 nActionFlags, sal_Bool bMsg );
    static int _PasteTargetURL( TransferableDataHelper& rData, SwWrtShell& rSh,
                        sal_uInt16 nAction, const Point* pPt, sal_Bool bInsertGRF );

    static int _PasteDDE( TransferableDataHelper& rData, SwWrtShell& rWrtShell,
                            sal_Bool bReReadGrf, sal_Bool bMsg );

    static int _PasteSdrFormat(  TransferableDataHelper& rData,
                                    SwWrtShell& rSh, sal_uInt16 nAction,
                                    const Point* pPt, sal_uInt8 nActionFlags, bool bNeedToSelectBeforePaste);

    static int _PasteGrf( TransferableDataHelper& rData, SwWrtShell& rSh,
                                sal_uLong nFmt, sal_uInt16 nAction, const Point* pPt,
                                sal_uInt8 nActionFlags, sal_Int8 nDropAction, bool bNeedToSelectBeforePaste);

    static int _PasteImageMap( TransferableDataHelper& rData,
                                    SwWrtShell& rSh );

    static int _PasteAsHyperlink( TransferableDataHelper& rData,
                                        SwWrtShell& rSh, sal_uLong nFmt );

    static int _PasteFileName( TransferableDataHelper& rData,
                            SwWrtShell& rSh, sal_uLong nFmt, sal_uInt16 nAction,
                            const Point* pPt, sal_uInt8 nActionFlags, sal_Bool bMsg );

    static int _PasteDBData( TransferableDataHelper& rData, SwWrtShell& rSh,
                            sal_uLong nFmt, sal_Bool bLink, const Point* pDragPt,
                            sal_Bool bMsg );

    static int _PasteFileList( TransferableDataHelper& rData,
                                SwWrtShell& rSh, sal_Bool bLink,
                                const Point* pPt, sal_Bool bMsg );

    int PrivateDrop( SwWrtShell& rSh, const Point& rDragPt, sal_Bool bMove,
                        sal_Bool bIsXSelection );
    int PrivatePaste( SwWrtShell& rShell );

    void SetDataForDragAndDrop( const Point& rSttPos );

                                    // not available
                                    SwTransferable();
                                    SwTransferable( const SwTransferable& );
    SwTransferable&                 operator=( const SwTransferable& );

protected:
    virtual void        AddSupportedFormats();
    virtual sal_Bool    GetData( const css::datatransfer::DataFlavor& rFlavor );
    virtual sal_Bool    WriteObject( SotStorageStreamRef& rxOStm,
                                        void* pUserObject,
                                        sal_uInt32 nUserObjectId,
                                        const css::datatransfer::DataFlavor& rFlavor );
    virtual void        DragFinished( sal_Int8 nDropAction );
    virtual void        ObjectReleased();

    using TransferableHelper::StartDrag;

public:
    SwTransferable( SwWrtShell& );
    virtual ~SwTransferable();

    static sal_uInt16 GetSotDestination( const SwWrtShell& rSh, const Point* = 0 );

    // set properties on the document, like PageMargin, VisArea.
    // And set real Size
    static void InitOle( SfxObjectShell* pDoc, SwDoc& rDoc );

    // copy - methods and helper methods for the copy
    int  Cut();
    int  Copy( sal_Bool bIsCut = sal_False );
    int  PrepareForCopy( sal_Bool bIsCut = sal_False );
    int  CalculateAndCopy();                // special for Calculator
    int  CopyGlossary( SwTextBlocks& rGlossary, const String& rStr );

    // remove the DDE-Link format promise
    void RemoveDDELinkFormat( const Window& rWin );

    // paste - methods and helper methods for the paste
    static sal_Bool IsPaste( const SwWrtShell&, const TransferableDataHelper& );
    static int Paste( SwWrtShell&, TransferableDataHelper& );
    static int PasteData( TransferableDataHelper& rData,
                          SwWrtShell& rSh, sal_uInt16 nAction, sal_uLong nFormat,
                          sal_uInt16 nDestination, sal_Bool bIsPasteFmt,
                          sal_Bool bIsDefault,
                          const Point* pDDPos = 0, sal_Int8 nDropAction = 0,
                          sal_Bool bPasteSelection = sal_False );

    static sal_Bool IsPasteSpecial( const SwWrtShell& rWrtShell,
                                const TransferableDataHelper& );
    static int PasteUnformatted( SwWrtShell& rSh, TransferableDataHelper& );
    static int PasteSpecial( SwWrtShell& rSh, TransferableDataHelper&, sal_uLong& rFormatUsed );
    static int PasteFormat( SwWrtShell& rSh, TransferableDataHelper& rData,
                             sal_uLong nFormat );

    static void FillClipFmtItem( const SwWrtShell& rSh,
                                const TransferableDataHelper& rData,
                                SvxClipboardFmtItem & rToFill );

    // Interfaces for Drag & Drop
    void StartDrag( Window* pWin, const Point& rPos );

    SwWrtShell* GetShell()              { return pWrtShell; }
    void SetCleanUp( sal_Bool bFlag )       { bCleanUp = bFlag; }

    // Interfaces for Selection
    /* #96392# Added pCreator to distinguish SwFrameShell from SwWrtShell. */
    static void CreateSelection( SwWrtShell & rSh,
                                 const ViewShell * pCreator = NULL );
    static void ClearSelection( SwWrtShell& rSh,
                                const ViewShell * pCreator = NULL );

    // the related SwView is being closed and the SwTransferable is invalid now
    void    Invalidate() {pWrtShell = 0;}
    static const css::uno::Sequence< sal_Int8 >& getUnoTunnelId();

    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& rId ) throw( css::uno::RuntimeException );
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
