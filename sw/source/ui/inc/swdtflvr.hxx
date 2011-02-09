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
    INetBookmark    *pBkmk;     // URL und Beschreibung!
    ImageMap        *pImageMap;
    INetImage       *pTargetURL;

    TransferBufferType eBufferType;

    sal_Bool bOldIdle   :1; //D&D Idle flag from the viewsettings
    sal_Bool bCleanUp   :1; //D&D cleanup after Drop (not by internal Drop)

    // helper methods for the copy
    com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject > FindOLEObj( sal_Int64& nAspect ) const;
    Graphic* FindOLEReplacementGraphic() const;
    void DeleteSelection();

    // helper methods for the paste
    static SwTransferable* GetSwTransferable( const TransferableDataHelper& rData );
    static void SetSelInShell( SwWrtShell& , sal_Bool , const Point* );
    static sal_Bool _CheckForURLOrLNKFile( TransferableDataHelper& rData,
                                String& rFileName, String* pTitle = 0 );
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
                                    const Point* pPt, sal_uInt8 nActionFlags );

    static int _PasteGrf( TransferableDataHelper& rData, SwWrtShell& rSh,
                                sal_uLong nFmt, sal_uInt16 nAction, const Point* pPt,
                                sal_uInt8 nActionFlags, sal_Bool bMsg );

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
    static const ::com::sun::star::uno::Sequence< sal_Int8 >& getUnoTunnelId();

    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw( ::com::sun::star::uno::RuntimeException );
};


#endif
