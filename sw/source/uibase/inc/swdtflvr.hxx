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

#include <svtools/transfer.hxx>
#include <vcl/graph.hxx>
#include <sfx2/lnkbase.hxx>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <o3tl/typed_flags_set.hxx>
#include <svx/swframetypes.hxx>
#include <memory>

class Graphic;
class ImageMap;
class INetBookmark;
class INetImage;
class SwDoc;
class SwDocFac;
class SwTextBlocks;
class SwWrtShell;
class SvxClipboardFormatItem;
class SwFrameShell;
class SwView_Impl;
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

class SW_DLLPUBLIC SwTransferable : public TransferableHelper
{
    friend class SwView_Impl;
    SfxObjectShellLock              m_aDocShellRef;
    TransferableObjectDescriptor    m_aObjDesc;
    tools::SvRef<sfx2::SvBaseLink>  m_xDdeLink;

    SwWrtShell      *m_pWrtShell;
    /* #96392# Added pCreatorView to distinguish SwFrameShell from
       SwWrtShell. */
    const SwFrameShell *m_pCreatorView;
    std::unique_ptr<SwDocFac>       m_pClpDocFac;
    std::unique_ptr<Graphic>        m_pClpGraphic;
    std::unique_ptr<Graphic>        m_pClpBitmap;
    Graphic                         *m_pOrigGraphic;
    std::unique_ptr<INetBookmark>   m_pBookmark;     // URL and description!
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
    static bool CheckForURLOrLNKFile( TransferableDataHelper& rData,
                                OUString& rFileName, OUString* pTitle = nullptr );
    static bool TestAllowedFormat( const TransferableDataHelper& rData,
                                        SotClipboardFormatId nFormat, SotExchangeDest nDestination );

    static bool PasteFileContent( TransferableDataHelper&,
                                    SwWrtShell& rSh, SotClipboardFormatId nFormat, bool bMsg );
    static bool PasteOLE( TransferableDataHelper& rData, SwWrtShell& rSh,
                            SotClipboardFormatId nFormat, SotExchangeActionFlags nActionFlags, bool bMsg );
    static bool PasteTargetURL( TransferableDataHelper& rData, SwWrtShell& rSh,
                        SwPasteSdr nAction, const Point* pPt, bool bInsertGRF );

    static bool PasteDDE( TransferableDataHelper& rData, SwWrtShell& rWrtShell,
                            bool bReReadGrf, bool bMsg );

    static bool PasteSdrFormat(  TransferableDataHelper& rData,
                                    SwWrtShell& rSh, SwPasteSdr nAction,
                                    const Point* pPt, SotExchangeActionFlags nActionFlags, bool bNeedToSelectBeforePaste);

    static bool PasteGrf( TransferableDataHelper& rData, SwWrtShell& rSh,
                                SotClipboardFormatId nFormat, SwPasteSdr nAction, const Point* pPt,
                                SotExchangeActionFlags nActionFlags, sal_Int8 nDropAction, bool bNeedToSelectBeforePaste, RndStdIds nAnchorType = RndStdIds::FLY_AT_PARA );

    static bool PasteImageMap( TransferableDataHelper& rData,
                                    SwWrtShell& rSh );

    static bool PasteAsHyperlink( TransferableDataHelper& rData,
                                        SwWrtShell& rSh, SotClipboardFormatId nFormat );

    static bool PasteFileName( TransferableDataHelper& rData,
                            SwWrtShell& rSh, SotClipboardFormatId nFormat, SwPasteSdr nAction,
                            const Point* pPt, SotExchangeActionFlags nActionFlags, bool * graphicInserted );

    static bool PasteDBData( TransferableDataHelper& rData, SwWrtShell& rSh,
                            SotClipboardFormatId nFormat, bool bLink, const Point* pDragPt,
                            bool bMsg );

    static bool PasteFileList( TransferableDataHelper& rData,
                                SwWrtShell& rSh, bool bLink,
                                const Point* pPt, bool bMsg );

    bool PrivateDrop( SwWrtShell& rSh, const Point& rDragPt, bool bMove,
                        bool bIsXSelection );
    bool PrivatePaste( SwWrtShell& rShell );

    void SetDataForDragAndDrop( const Point& rSttPos );

                                    SwTransferable( const SwTransferable& ) = delete;
    SwTransferable&                 operator=( const SwTransferable& ) = delete;

protected:
    virtual void        AddSupportedFormats() override;
    virtual bool GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc ) override;
    virtual bool        WriteObject( tools::SvRef<SotStorageStream>& rxOStm,
                                        void* pUserObject,
                                        sal_uInt32 nUserObjectId,
                                        const css::datatransfer::DataFlavor& rFlavor ) override;
    virtual void        DragFinished( sal_Int8 nDropAction ) override;
    virtual void        ObjectReleased() override;

    using TransferableHelper::StartDrag;

public:
    SwTransferable( SwWrtShell& );
    virtual ~SwTransferable() override;

    static SotExchangeDest GetSotDestination( const SwWrtShell& rSh );

    // set properties on the document, like PageMargin, VisArea.
    // And set real Size
    static void InitOle( SfxObjectShell* pDoc );

    // copy - methods and helper methods for the copy
    int  Cut();
    int  Copy( bool bIsCut = false );
    int  PrepareForCopy( bool bIsCut = false );
    void  CalculateAndCopy();                // special for Calculator
    bool  CopyGlossary( SwTextBlocks& rGlossary, const OUString& rStr );

    // remove the DDE-Link format promise
    void RemoveDDELinkFormat( const vcl::Window& rWin );

    // paste - methods and helper methods for the paste
    static bool IsPaste( const SwWrtShell&, const TransferableDataHelper& );
    static bool Paste( SwWrtShell&, TransferableDataHelper&, RndStdIds nAnchorType = RndStdIds::FLY_AT_PARA );
    static bool PasteData( TransferableDataHelper& rData,
                          SwWrtShell& rSh, sal_uInt8 nAction, SotExchangeActionFlags nActionFlags,
                          SotClipboardFormatId nFormat,
                          SotExchangeDest nDestination, bool bIsPasteFormat,
                          bool bIsDefault,
                          const Point* pDDPos = nullptr, sal_Int8 nDropAction = 0,
                          bool bPasteSelection = false, RndStdIds nAnchorType = RndStdIds::FLY_AT_PARA );

    static bool IsPasteSpecial( const SwWrtShell& rWrtShell,
                                const TransferableDataHelper& );
    static bool PasteUnformatted( SwWrtShell& rSh, TransferableDataHelper& );
    static bool PasteSpecial( SwWrtShell& rSh, TransferableDataHelper&, SotClipboardFormatId& rFormatUsed );
    static bool PasteFormat( SwWrtShell& rSh, TransferableDataHelper& rData,
                             SotClipboardFormatId nFormat );

    static void FillClipFormatItem( const SwWrtShell& rSh,
                                const TransferableDataHelper& rData,
                                SvxClipboardFormatItem & rToFill );

    // Interfaces for Drag & Drop
    void StartDrag( vcl::Window* pWin, const Point& rPos );

    SwWrtShell* GetShell()              { return m_pWrtShell; }
    void SetCleanUp( bool bFlag )       { m_bCleanUp = bFlag; }

    // Interfaces for Selection
    /* #96392# Added pCreator to distinguish SwFrameShell from SwWrtShell. */
    static void CreateSelection( SwWrtShell & rSh,
                                 const SwFrameShell * pCreator = nullptr );
    static void ClearSelection( SwWrtShell& rSh,
                                const SwFrameShell * pCreator = nullptr );

    // the related SwView is being closed and the SwTransferable is invalid now
    void    Invalidate() {m_pWrtShell = nullptr;}
    static const css::uno::Sequence< sal_Int8 >& getUnoTunnelId();

    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& rId ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
