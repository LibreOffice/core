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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_SWDTFLVR_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_SWDTFLVR_HXX

#include <sfx2/objsh.hxx>

#include <vcl/transfer.hxx>
#include <vcl/graph.hxx>
#include <vcl/vclptr.hxx>
#include <sfx2/lnkbase.hxx>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <o3tl/deleter.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <svx/swframetypes.hxx>
#include <svl/urlbmk.hxx>
#include <memory>
#include <optional>

#include <swdllapi.h>

class Graphic;
class ImageMap;
class INetImage;
class SfxAbstractPasteDialog;
class SwDoc;
class SwDocFac;
class SwPaM;
class SwTextBlocks;
class SwWrtShell;
class SvxClipboardFormatItem;
class SwFrameShell;
class SwView_Impl;
class SwPasteContext;
enum class SwPasteSdr;

enum class TransferBufferType : sal_uInt16
{
        NONE          = 0x0000,
        Document      = 0x0001,
        DocumentWord  = 0x0002,
        Graphic       = 0x0004,
        Table         = 0x0008,
        Ole           = 0x0020,
        InetField     = 0x0040,
        Drawing       = 0x0081,   // drawing is internal too!
};
namespace o3tl {
    template<> struct typed_flags<TransferBufferType> : is_typed_flags<TransferBufferType, 0x00ef> {};
}
// paste table into a table
enum class PasteTableType
{
        PASTE_DEFAULT, // paste table by overwriting table cells
        PASTE_ROW,     // paste table as rows above
        PASTE_COLUMN,  // paste table as columns before
        PASTE_TABLE    // paste table as nested table
};

class SwTransferDdeLink;

class SAL_DLLPUBLIC_RTTI SwTransferable final : public TransferableHelper
{
    friend class SwView_Impl;
    SfxObjectShellLock              m_aDocShellRef;
    TransferableObjectDescriptor    m_aObjDesc;
    tools::SvRef<SwTransferDdeLink>  m_xDdeLink;

    SwWrtShell      *m_pWrtShell;
    /* #96392# Added pCreatorView to distinguish SwFrameShell from
       SwWrtShell. */
    const SwFrameShell *m_pCreatorView;
    std::unique_ptr<SwDocFac, o3tl::default_delete<SwDocFac>> m_pClpDocFac;
    std::optional<Graphic>        m_oClpGraphic;
    std::optional<Graphic>        m_oClpBitmap;
    Graphic                         *m_pOrigGraphic;
    std::optional<INetBookmark>     m_oBookmark;     // URL and description!
    std::unique_ptr<ImageMap>       m_pImageMap;
    std::unique_ptr<INetImage>      m_pTargetURL;

    TransferBufferType m_eBufferType;

    bool m_bOldIdle   :1; //D&D Idle flag from the viewsettings
    bool m_bCleanUp   :1; //D&D cleanup after Drop (not by internal Drop)

    // helper methods for the copy
    css::uno::Reference < css::embed::XEmbeddedObject > FindOLEObj( sal_Int64& nAspect ) const;
    const Graphic* FindOLEReplacementGraphic() const;
    void DeleteSelection();

    // helper methods for the paste
    static SwTransferable* GetSwTransferable( const TransferableDataHelper& rData );
    static void SetSelInShell( SwWrtShell& , bool , const Point* );
    static bool CheckForURLOrLNKFile( const TransferableDataHelper& rData,
                                OUString& rFileName, OUString* pTitle = nullptr );
    static bool TestAllowedFormat( const TransferableDataHelper& rData,
                                        SotClipboardFormatId nFormat, SotExchangeDest nDestination );

    static bool PasteFileContent( const TransferableDataHelper&,
                                    SwWrtShell& rSh, SotClipboardFormatId nFormat, bool bMsg, bool bIgnoreComments = false );
    static bool PasteOLE( const TransferableDataHelper& rData, SwWrtShell& rSh,
                            SotClipboardFormatId nFormat, SotExchangeActionFlags nActionFlags, bool bMsg );
    static bool PasteTargetURL( const TransferableDataHelper& rData, SwWrtShell& rSh,
                        SwPasteSdr nAction, const Point* pPt, bool bInsertGRF );

    static bool PasteDDE( const TransferableDataHelper& rData, SwWrtShell& rWrtShell,
                            bool bReReadGrf, bool bMsg );

    static bool PasteSdrFormat(  const TransferableDataHelper& rData,
                                    SwWrtShell& rSh, SwPasteSdr nAction,
                                    const Point* pPt, SotExchangeActionFlags nActionFlags, bool bNeedToSelectBeforePaste);

    static bool PasteGrf( const TransferableDataHelper& rData, SwWrtShell& rSh,
                                SotClipboardFormatId nFormat, SwPasteSdr nAction, const Point* pPt,
                                SotExchangeActionFlags nActionFlags, sal_Int8 nDropAction, bool bNeedToSelectBeforePaste, RndStdIds nAnchorType = RndStdIds::FLY_AT_PARA );

    static bool PasteImageMap( const TransferableDataHelper& rData,
                                    SwWrtShell& rSh );

    static bool PasteAsHyperlink( const TransferableDataHelper& rData,
                                        SwWrtShell& rSh, SotClipboardFormatId nFormat );

    static bool PasteFileName( const TransferableDataHelper& rData,
                            SwWrtShell& rSh, SotClipboardFormatId nFormat, SwPasteSdr nAction,
                            const Point* pPt, SotExchangeActionFlags nActionFlags, bool * graphicInserted );

    static bool PasteDBData( const TransferableDataHelper& rData, SwWrtShell& rSh,
                            SotClipboardFormatId nFormat, bool bLink, const Point* pDragPt,
                            bool bMsg );

    static bool PasteFileList( const TransferableDataHelper& rData,
                                SwWrtShell& rSh, bool bLink,
                                const Point* pPt, bool bMsg );

    bool PrivatePaste( SwWrtShell& rShell, SwPasteContext* pContext = nullptr, PasteTableType ePasteTable = PasteTableType::PASTE_DEFAULT );

    void SetDataForDragAndDrop( const Point& rSttPos );

                                    SwTransferable( const SwTransferable& ) = delete;
    SwTransferable&                 operator=( const SwTransferable& ) = delete;

    virtual void        AddSupportedFormats() override;
    virtual bool GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc ) override;
    virtual bool        WriteObject( SvStream& rOStm,
                                        void* pUserObject,
                                        sal_uInt32 nUserObjectId,
                                        const css::datatransfer::DataFlavor& rFlavor ) override;
    virtual void        DragFinished( sal_Int8 nDropAction ) override;
    virtual void        ObjectReleased() override;
    virtual sal_Bool SAL_CALL isComplex() override;

    using TransferableHelper::StartDrag;

public:
    SW_DLLPUBLIC SwTransferable( SwWrtShell& );
    virtual ~SwTransferable() override;

    static SotExchangeDest GetSotDestination( const SwWrtShell& rSh );

    // set properties on the document, like PageMargin, VisArea.
    // And set real Size
    static void InitOle( SfxObjectShell* pDoc );

    // copy - methods and helper methods for the copy
    SW_DLLPUBLIC int  Cut();
    SW_DLLPUBLIC int  Copy( bool bIsCut = false );
    int  PrepareForCopy( bool bIsCut = false );
    void PrepareForCopyTextRange(SwPaM & rPaM);
    void  CalculateAndCopy();                // special for Calculator
    bool  CopyGlossary( SwTextBlocks& rGlossary, const OUString& rStr );

    // remove the DDE-Link format promise
    void RemoveDDELinkFormat(vcl::Window& rWin);
    // disconnect to not receive DataChanged listener notifications
    void DisconnectDDE();

    // paste - methods and helper methods for the paste
    static bool IsPaste( const SwWrtShell&, const TransferableDataHelper& );
    SW_DLLPUBLIC static bool Paste( SwWrtShell&, TransferableDataHelper&, RndStdIds nAnchorType = RndStdIds::FLY_AT_PARA,
                          bool bIgnoreComments = false, PasteTableType ePasteTable = PasteTableType::PASTE_DEFAULT );
    static bool PasteData( const TransferableDataHelper& rData,
                          SwWrtShell& rSh, sal_uInt8 nAction, SotExchangeActionFlags nActionFlags,
                          SotClipboardFormatId nFormat,
                          SotExchangeDest nDestination, bool bIsPasteFormat,
                          bool bIsDefault,
                          const Point* pDDPos = nullptr, sal_Int8 nDropAction = 0,
                          bool bPasteSelection = false, RndStdIds nAnchorType = RndStdIds::FLY_AT_PARA,
                          bool bIgnoreComments = false,
                          SwPasteContext* pContext = nullptr,
                          PasteTableType nPaste = PasteTableType::PASTE_DEFAULT );

    static bool IsPasteSpecial( const SwWrtShell& rWrtShell,
                                const TransferableDataHelper& );
    static bool IsPasteOwnFormat( const TransferableDataHelper& );
    static bool PasteUnformatted( SwWrtShell& rSh, TransferableDataHelper& );
    /**
     * @brief PrePasteSpecial Prepares the given dialog without actually running it
     * @param rSh
     * @param rFormatUsed
     */
    static void PrePasteSpecial( const SwWrtShell& rSh, const TransferableDataHelper&, const VclPtr<SfxAbstractPasteDialog>& pDlg );
    SW_DLLPUBLIC static bool PasteFormat( SwWrtShell& rSh, const TransferableDataHelper& rData,
                             SotClipboardFormatId nFormat );

    static void FillClipFormatItem( const SwWrtShell& rSh,
                                const TransferableDataHelper& rData,
                                SvxClipboardFormatItem & rToFill );

    // Interfaces for Drag & Drop
    void StartDrag( vcl::Window* pWin, const Point& rPos );

    SwWrtShell* GetShell()              { return m_pWrtShell; }
    void SetCleanUp( bool bFlag )       { m_bCleanUp = bFlag; }

    // public only for testing
    SW_DLLPUBLIC bool PrivateDrop( SwWrtShell& rSh, const Point& rDragPt, bool bMove,
                        bool bIsXSelection );

    // Interfaces for Selection
    /* #96392# Added pCreator to distinguish SwFrameShell from SwWrtShell. */
    static void CreateSelection( SwWrtShell & rSh,
                                 const SwFrameShell * pCreator = nullptr );
    static void ClearSelection( const SwWrtShell& rSh,
                                const SwFrameShell * pCreator = nullptr );

    // the related SwView is being closed and the SwTransferable is invalid now
    void    Invalidate() {m_pWrtShell = nullptr;}

    SW_DLLPUBLIC static void SelectPasteFormat(TransferableDataHelper& rData, sal_uInt8& nAction,
                                  SotClipboardFormatId& nFormat);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
