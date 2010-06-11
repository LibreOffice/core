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
    const sal_uInt16 TRNSFR_DRAWING         = 0x0081;   //Drawing ist auch intern!
}

#define DATA_FLAVOR     ::com::sun::star::datatransfer::DataFlavor

class SwTransferable : public TransferableHelper
{
    friend class SwView_Impl;
    SfxObjectShellRef             aDocShellRef;
    TransferableDataHelper          aOleData;
    TransferableObjectDescriptor    aObjDesc;
    ::sfx2::SvBaseLinkRef            refDdeLink;

    SwWrtShell      *pWrtShell;
    /* #96392# Added pCreatorView to distinguish SwFrameShell from
       SwWrtShell. */
    const ViewShell       *pCreatorView;
    SwDocFac        *pClpDocFac;
    Graphic         *pClpGraphic, *pClpBitmap, *pOrigGrf;
    INetBookmark    *pBkmk;     // URL und Beschreibung!
    ImageMap        *pImageMap;
    INetImage       *pTargetURL;

    TransferBufferType eBufferType;

    BOOL bOldIdle   :1; //D&D Idle flag from the viewsettings
    BOOL bCleanUp   :1; //D&D cleanup after Drop (not by internal Drop)

    // helper methods for the copy
    com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject > FindOLEObj( sal_Int64& nAspect ) const;
    Graphic* FindOLEReplacementGraphic() const;
    void DeleteSelection();

    // helper methods for the paste
    static void SetSelInShell( SwWrtShell& , BOOL , const Point* );
    static BOOL _CheckForURLOrLNKFile( TransferableDataHelper& rData,
                                String& rFileName, String* pTitle = 0 );
    static int _TestAllowedFormat( const TransferableDataHelper& rData,
                                        ULONG nFormat, USHORT nDestination );

    static int _PasteFileContent( TransferableDataHelper&,
                                    SwWrtShell& rSh, ULONG nFmt, BOOL bMsg );
    static int _PasteOLE( TransferableDataHelper& rData, SwWrtShell& rSh,
                            ULONG nFmt, BYTE nActionFlags, BOOL bMsg );
    static int _PasteTargetURL( TransferableDataHelper& rData, SwWrtShell& rSh,
                        USHORT nAction, const Point* pPt, BOOL bInsertGRF );

    static int _PasteDDE( TransferableDataHelper& rData, SwWrtShell& rWrtShell,
                            BOOL bReReadGrf, BOOL bMsg );

    static int _PasteSdrFormat(  TransferableDataHelper& rData,
                                    SwWrtShell& rSh, USHORT nAction,
                                    const Point* pPt, BYTE nActionFlags );

    static int _PasteGrf( TransferableDataHelper& rData, SwWrtShell& rSh,
                                ULONG nFmt, USHORT nAction, const Point* pPt,
                                BYTE nActionFlags, BOOL bMsg );

    static int _PasteImageMap( TransferableDataHelper& rData,
                                    SwWrtShell& rSh );

    static int _PasteAsHyperlink( TransferableDataHelper& rData,
                                        SwWrtShell& rSh, ULONG nFmt );

    static int _PasteFileName( TransferableDataHelper& rData,
                            SwWrtShell& rSh, ULONG nFmt, USHORT nAction,
                            const Point* pPt, BYTE nActionFlags, BOOL bMsg );

    static int _PasteDBData( TransferableDataHelper& rData, SwWrtShell& rSh,
                            ULONG nFmt, BOOL bLink, const Point* pDragPt,
                            BOOL bMsg );

    static int _PasteFileList( TransferableDataHelper& rData,
                                SwWrtShell& rSh, BOOL bLink,
                                const Point* pPt, BOOL bMsg );

    int PrivateDrop( SwWrtShell& rSh, const Point& rDragPt, BOOL bMove,
                        BOOL bIsXSelection );
    int PrivatePaste( SwWrtShell& rShell );

    void SetDataForDragAndDrop( const Point& rSttPos );

                                    // not available
                                    SwTransferable();
                                    SwTransferable( const SwTransferable& );
    SwTransferable&                 operator=( const SwTransferable& );

protected:
    virtual void        AddSupportedFormats();
    virtual sal_Bool    GetData( const DATA_FLAVOR& rFlavor );
    virtual sal_Bool    WriteObject( SotStorageStreamRef& rxOStm,
                                        void* pUserObject,
                                        sal_uInt32 nUserObjectId,
                                        const DATA_FLAVOR& rFlavor );
    virtual void        DragFinished( sal_Int8 nDropAction );
    virtual void        ObjectReleased();

    using TransferableHelper::StartDrag;

public:
    SwTransferable( SwWrtShell& );
    virtual ~SwTransferable();

    static USHORT GetSotDestination( const SwWrtShell& rSh, const Point* = 0 );

    // set properties on the document, like PageMargin, VisArea.
    // And set real Size
    static void InitOle( SfxObjectShell* pDoc, SwDoc& rDoc );

    // copy - methods and helper methods for the copy
    int  Cut();
    int  Copy( BOOL bIsCut = FALSE );
    int  PrepareForCopy( BOOL bIsCut = FALSE );
    int  CalculateAndCopy();                // special for Calculator
    int  CopyGlossary( SwTextBlocks& rGlossary, const String& rStr );

    // remove the DDE-Link format promise
    void RemoveDDELinkFormat( const Window& rWin );

    // paste - methods and helper methods for the paste
    static BOOL IsPaste( const SwWrtShell&, const TransferableDataHelper& );
    static int Paste( SwWrtShell&, TransferableDataHelper& );
    static int PasteData( TransferableDataHelper& rData,
                          SwWrtShell& rSh, USHORT nAction, ULONG nFormat,
                          USHORT nDestination, BOOL bIsPasteFmt,
                          sal_Bool bIsDefault,
                          const Point* pDDPos = 0, sal_Int8 nDropAction = 0,
                          BOOL bPasteSelection = FALSE );

    static BOOL IsPasteSpecial( const SwWrtShell& rWrtShell,
                                const TransferableDataHelper& );
    static int PasteUnformatted( SwWrtShell& rSh, TransferableDataHelper& );
    static int PasteSpecial( SwWrtShell& rSh, TransferableDataHelper&, ULONG& rFormatUsed );
    static int PasteFormat( SwWrtShell& rSh, TransferableDataHelper& rData,
                             ULONG nFormat );

    static void FillClipFmtItem( const SwWrtShell& rSh,
                                const TransferableDataHelper& rData,
                                SvxClipboardFmtItem & rToFill );

    // Interfaces for Drag & Drop
    void StartDrag( Window* pWin, const Point& rPos );

    SwWrtShell* GetShell()              { return pWrtShell; }
    void SetCleanUp( BOOL bFlag )       { bCleanUp = bFlag; }

    // Interfaces for Selection
    /* #96392# Added pCreator to distinguish SwFrameShell from SwWrtShell. */
    static void CreateSelection( SwWrtShell & rSh,
                                 const ViewShell * pCreator = NULL );
    static void ClearSelection( SwWrtShell& rSh,
                                const ViewShell * pCreator = NULL );

    // the related SwView is being closed and the SwTransferable is invalid now
    void    Invalidate() {pWrtShell = 0;}
    static const ::com::sun::star::uno::Sequence< sal_Int8 >& getUnoTunnelId();

    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw( ::com::sun::star::uno::RuntimeException );
};


#endif
