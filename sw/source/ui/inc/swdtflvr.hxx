/*************************************************************************
 *
 *  $RCSfile: swdtflvr.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2001-03-08 21:22:13 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SWDTFLVR_HXX
#define _SWDTFLVR_HXX

#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif
#ifndef _EMBOBJ_HXX
#include <so3/embobj.hxx>
#endif
#ifndef _SV_GRAPH_HXX
#include <vcl/graph.hxx>
#endif
#ifndef _LNKBASE_HXX
#include <so3/lnkbase.hxx>
#endif


class Graphic;
class ImageMap;
class INetBookmark;
class INetImage;
class SwDoc;
class SwDocFac;
class SwNode;
class SwTextBlocks;
class SwWrtShell;
class Reader;

enum TransferBufferType
{
    TRNSFR_NONE             = 0x0000,
    TRNSFR_DOCUMENT         = 0x0001,
    TRNSFR_DOCUMENT_WORD    = 0x0002,
    TRNSFR_GRAPHIC          = 0x0004,
    TRNSFR_TABELLE          = 0x0008,
    TRNSFR_DDELINK          = 0x0010,
    TRNSFR_OLE              = 0x0020,
    TRNSFR_INETFLD          = 0x0040,
    TRNSFR_DRAWING          = 0x0081    //Drawing ist auch intern!
};

#define DATA_FLAVOR     ::com::sun::star::datatransfer::DataFlavor

class SwTransferable : public TransferableHelper
{
// new:
    SvEmbeddedObjectRef             aDocShellRef;
    TransferableDataHelper          aOleData;
    TransferableObjectDescriptor    aObjDesc;

    SwWrtShell *pWrtShell;

    TransferBufferType eBufferType;
    SwDocFac        *pClpDocFac;
    Graphic         *pClpGraphic, *pClpBitmap, *pOrigGrf;
    ::so3::SvBaseLinkRef    refDdeLink;
    INetBookmark    *pBkmk;     // URL und Beschreibung!
    ImageMap        *pImageMap;
    INetImage       *pTargetURL;

    BOOL bIntern    :1; //D&D innerhalb des SW
    BOOL bTblSel    :1; //D&D Tabellen oder Rahmenselektion

    // helper methods for the copy
    SvEmbeddedObject* FindOLEObj() const;
    void DeleteSelection();

    // helper methods for the paste
    static USHORT GetSotDestination( const SwWrtShell& rSh, const Point* = 0 );
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
                            FASTBOOL bReReadGrf, BOOL bMsg );

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
                            SwWrtShell& rSh, const Point* pPt, BOOL bMsg );

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
    virtual void        ObjectReleased();

public:
    SwTransferable( SwWrtShell& );
    virtual ~SwTransferable();

    //Stellt das Document ein (Seitenraender, VisArea) und defaultet
    //die RealSize.
    static void InitOle( SvEmbeddedObjectRef rRef, SwDoc& rDoc );

    // copy - methods and helper methods for the copy
    int  Cut();
    int  Copy( BOOL bIsCut = FALSE );
    int  CalculateAndCopy();        //Spezialfall fuer Calculator
    int  CopyGlossary( SwTextBlocks& rGlossary, const String& rStr );

    // remove the DDE-Link format promise
    void RemoveDDELinkFormat();

    // paste - methods and helper methods for the paste
    static BOOL IsPaste( const SwWrtShell&, const TransferableDataHelper& );
    static int Paste( SwWrtShell&, TransferableDataHelper& );
    static int PasteData( TransferableDataHelper& rData,
                          SwWrtShell& rSh, USHORT nAction, ULONG nFormat,
                          USHORT nDestination, BOOL bIsPasteFmt );

    static BOOL IsPasteSpecial( const SwWrtShell& rWrtShell,
                                const TransferableDataHelper& );
    static int PasteSpecial( SwWrtShell& rSh, TransferableDataHelper& );
    static int PasteFormat( SwWrtShell& rSh, TransferableDataHelper& rData,
                             ULONG nFormat );

};


#endif
