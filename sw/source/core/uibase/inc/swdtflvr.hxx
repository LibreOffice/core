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
#ifndef INCLUDED_SW_SOURCE_CORE_UIBASE_INC_SWDTFLVR_HXX
#define INCLUDED_SW_SOURCE_CORE_UIBASE_INC_SWDTFLVR_HXX

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
class SwViewShell;
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
    const SwViewShell       *pCreatorView;
    SwDocFac        *pClpDocFac;
    Graphic         *pClpGraphic, *pClpBitmap, *pOrigGrf;
    INetBookmark    *pBkmk;     // URL and description!
    ImageMap        *pImageMap;
    INetImage       *pTargetURL;

    TransferBufferType eBufferType;

    bool bOldIdle   :1; //D&D Idle flag from the viewsettings
    bool bCleanUp   :1; //D&D cleanup after Drop (not by internal Drop)

    // helper methods for the copy
    css::uno::Reference < css::embed::XEmbeddedObject > FindOLEObj( sal_Int64& nAspect ) const;
    const Graphic* FindOLEReplacementGraphic() const;
    void DeleteSelection();

    // helper methods for the paste
    static SwTransferable* GetSwTransferable( const TransferableDataHelper& rData );
    static void SetSelInShell( SwWrtShell& , bool , const Point* );
    static bool _CheckForURLOrLNKFile( TransferableDataHelper& rData,
                                OUString& rFileName, OUString* pTitle = 0 );
    static bool _TestAllowedFormat( const TransferableDataHelper& rData,
                                        sal_uLong nFormat, sal_uInt16 nDestination );

    static bool _PasteFileContent( TransferableDataHelper&,
                                    SwWrtShell& rSh, sal_uLong nFmt, bool bMsg );
    static bool _PasteOLE( TransferableDataHelper& rData, SwWrtShell& rSh,
                            sal_uLong nFmt, sal_uInt8 nActionFlags, bool bMsg );
    static bool _PasteTargetURL( TransferableDataHelper& rData, SwWrtShell& rSh,
                        sal_uInt16 nAction, const Point* pPt, bool bInsertGRF );

    static bool _PasteDDE( TransferableDataHelper& rData, SwWrtShell& rWrtShell,
                            bool bReReadGrf, bool bMsg );

    static bool _PasteSdrFormat(  TransferableDataHelper& rData,
                                    SwWrtShell& rSh, sal_uInt16 nAction,
                                    const Point* pPt, sal_uInt8 nActionFlags, bool bNeedToSelectBeforePaste);

    static bool _PasteGrf( TransferableDataHelper& rData, SwWrtShell& rSh,
                                sal_uLong nFmt, sal_uInt16 nAction, const Point* pPt,
                                sal_uInt8 nActionFlags, sal_Int8 nDropAction, bool bNeedToSelectBeforePaste);

    static bool _PasteImageMap( TransferableDataHelper& rData,
                                    SwWrtShell& rSh );

    static bool _PasteAsHyperlink( TransferableDataHelper& rData,
                                        SwWrtShell& rSh, sal_uLong nFmt );

    static bool _PasteFileName( TransferableDataHelper& rData,
                            SwWrtShell& rSh, sal_uLong nFmt, sal_uInt16 nAction,
                            const Point* pPt, sal_uInt8 nActionFlags, bool bMsg, bool * graphicInserted );

    static bool _PasteDBData( TransferableDataHelper& rData, SwWrtShell& rSh,
                            sal_uLong nFmt, bool bLink, const Point* pDragPt,
                            bool bMsg );

    static bool _PasteFileList( TransferableDataHelper& rData,
                                SwWrtShell& rSh, bool bLink,
                                const Point* pPt, bool bMsg );

    bool PrivateDrop( SwWrtShell& rSh, const Point& rDragPt, bool bMove,
                        bool bIsXSelection );
    bool PrivatePaste( SwWrtShell& rShell );

    void SetDataForDragAndDrop( const Point& rSttPos );

                                    // not available
                                    SwTransferable();
                                    SwTransferable( const SwTransferable& );
    SwTransferable&                 operator=( const SwTransferable& );

protected:
    virtual void        AddSupportedFormats() SAL_OVERRIDE;
    virtual bool GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc ) SAL_OVERRIDE;
    virtual bool        WriteObject( SotStorageStreamRef& rxOStm,
                                        void* pUserObject,
                                        sal_uInt32 nUserObjectId,
                                        const css::datatransfer::DataFlavor& rFlavor ) SAL_OVERRIDE;
    virtual void        DragFinished( sal_Int8 nDropAction ) SAL_OVERRIDE;
    virtual void        ObjectReleased() SAL_OVERRIDE;

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
    int  Copy( bool bIsCut = false );
    int  PrepareForCopy( bool bIsCut = false );
    int  CalculateAndCopy();                // special for Calculator
    int  CopyGlossary( SwTextBlocks& rGlossary, const OUString& rStr );

    // remove the DDE-Link format promise
    void RemoveDDELinkFormat( const Window& rWin );

    // paste - methods and helper methods for the paste
    static bool IsPaste( const SwWrtShell&, const TransferableDataHelper& );
    static bool Paste( SwWrtShell&, TransferableDataHelper& );
    static bool PasteData( TransferableDataHelper& rData,
                          SwWrtShell& rSh, sal_uInt16 nAction, sal_uLong nFormat,
                          sal_uInt16 nDestination, bool bIsPasteFmt,
                          bool bIsDefault,
                          const Point* pDDPos = 0, sal_Int8 nDropAction = 0,
                          bool bPasteSelection = false );

    static bool IsPasteSpecial( const SwWrtShell& rWrtShell,
                                const TransferableDataHelper& );
    static bool PasteUnformatted( SwWrtShell& rSh, TransferableDataHelper& );
    static bool PasteSpecial( SwWrtShell& rSh, TransferableDataHelper&, sal_uLong& rFormatUsed );
    static bool PasteFormat( SwWrtShell& rSh, TransferableDataHelper& rData,
                             sal_uLong nFormat );

    static void FillClipFmtItem( const SwWrtShell& rSh,
                                const TransferableDataHelper& rData,
                                SvxClipboardFmtItem & rToFill );

    // Interfaces for Drag & Drop
    void StartDrag( Window* pWin, const Point& rPos );

    SwWrtShell* GetShell()              { return pWrtShell; }
    void SetCleanUp( bool bFlag )       { bCleanUp = bFlag; }

    // Interfaces for Selection
    /* #96392# Added pCreator to distinguish SwFrameShell from SwWrtShell. */
    static void CreateSelection( SwWrtShell & rSh,
                                 const SwViewShell * pCreator = NULL );
    static void ClearSelection( SwWrtShell& rSh,
                                const SwViewShell * pCreator = NULL );

    // the related SwView is being closed and the SwTransferable is invalid now
    void    Invalidate() {pWrtShell = 0;}
    static const css::uno::Sequence< sal_Int8 >& getUnoTunnelId();

    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& rId ) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
