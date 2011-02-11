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

#ifndef _SOT_EXCHANGE_HXX
#define _SOT_EXCHANGE_HXX

#ifndef __SGI_STL_LIST
#include <list>
#endif
#ifndef __SGI_STL_VECTOR
#include <vector>
#endif
#include <tools/string.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/datatransfer/DataFlavor.hpp>
#ifndef _COM_SUN_STAR_DATATRANSFER_DND_DNDCONSTANTS_HDL_
#include <com/sun/star/datatransfer/dnd/DNDConstants.hdl>
#endif
#include "sot/sotdllapi.h"

class SotDataObject;

namespace com { namespace sun { namespace star { namespace datatransfer {
    class XTransferable;
} } } }

// ---------------------
// - SotFormatStringId -
// ---------------------

typedef sal_uLong SotFormatStringId;

// ----------------
// - DataFlavorEx -
// ----------------

struct DataFlavorEx : public ::com::sun::star::datatransfer::DataFlavor
{
    SotFormatStringId mnSotId;
};

typedef ::std::vector< ::com::sun::star::datatransfer::DataFlavor > DataFlavorVector;
typedef ::std::vector< DataFlavorEx >                               _DataFlavorExVector;

// JP 23.03.2001 - this struct is only for "hide" the STD of the vetor,
// because our makefile filter all this symbols and so nowbody can use
// these struct in any interfacses.
struct DataFlavorExVector : public _DataFlavorExVector
{
};

typedef ::std::list< ::com::sun::star::datatransfer::DataFlavor >   DataFlavorList;
typedef ::std::list< DataFlavorEx >                                 DataFlavorExList;

SOT_DLLPUBLIC sal_Bool IsFormatSupported( const DataFlavorExVector& rDataFlavorExVector,
                            sal_uLong nId );

// -------------------------
// - Vordefinierte Formate -
// -------------------------

// Die Reihenfolge und die Werte d�rfen nicht ge�ndert werden,
// da die Implementation sich darauf verl��t.
// Standard-Formate fuer die es auch Copy/Paste-Methoden gibt
#define FORMAT_STRING           1
#define FORMAT_BITMAP           2
#define FORMAT_GDIMETAFILE      3
#define FORMAT_PRIVATE          4
#define FORMAT_FILE             5
#define FORMAT_FILE_LIST        6

// Weitere Standardformate (diese gehen nur ueber CopyData/PasteData)
#define FORMAT_RTF              10

// Source-Options
#define EXCHG_SOURCE_MOVEABLE       ((sal_uInt16)0x0001)
#define EXCHG_SOURCE_COPYABLE       ((sal_uInt16)0x0002)
#define EXCHG_SOURCE_LINKABLE       ((sal_uInt16)0x0004)
#define EXCHG_SOURCE_PRINTABLE      ((sal_uInt16)0x0008)
#define EXCHG_SOURCE_DISCARDABLE    ((sal_uInt16)0x0010)
#define EXCHG_SOURCE_ALL            ((sal_uInt16)0x001F)
#define EXCHG_SOURCE_DEF_COPYABLE   ((sal_uInt16)0x0020)

// Aktionen
#define EXCHG_ACTION_MASK                       ((sal_uInt16)0x00FF)
#define EXCHG_INOUT_ACTION_NONE                 ((sal_uInt16)com::sun::star::datatransfer::dnd::DNDConstants::ACTION_NONE)
#define EXCHG_IN_ACTION_DEFAULT                 EXCHG_INOUT_ACTION_NONE
#define EXCHG_IN_ACTION_MOVE                    ((sal_uInt16)com::sun::star::datatransfer::dnd::DNDConstants::ACTION_MOVE)
#define EXCHG_IN_ACTION_COPY                    ((sal_uInt16)com::sun::star::datatransfer::dnd::DNDConstants::ACTION_COPY)
#define EXCHG_IN_ACTION_LINK                    ((sal_uInt16)com::sun::star::datatransfer::dnd::DNDConstants::ACTION_LINK)
#define EXCHG_INOUT_ACTION_PRINT                ((sal_uInt16)8)
#define EXCHG_INOUT_ACTION_DISCARD              ((sal_uInt16)16)
#define EXCHG_OUT_ACTION_INSERT_OBJ             ((sal_uInt16)17)
#define EXCHG_OUT_ACTION_INSERT_BOOKMARK        ((sal_uInt16)18)
#define EXCHG_OUT_ACTION_INSERT_FILELINK        ((sal_uInt16)19)
#define EXCHG_OUT_ACTION_INSERT_FILE            ((sal_uInt16)20)
#define EXCHG_OUT_ACTION_INSERT_FILELIST        ((sal_uInt16)21)
#define EXCHG_OUT_ACTION_INSERT_IMAGEMAP        ((sal_uInt16)22)
#define EXCHG_OUT_ACTION_INSERT_OLE             ((sal_uInt16)23)
#define EXCHG_OUT_ACTION_INSERT_INTERACTIVE     ((sal_uInt16)24)
#define EXCHG_OUT_ACTION_INSERT_URLBUTTON       ((sal_uInt16)25)
#define EXCHG_OUT_ACTION_INSERT_CHAOSOBJ        ((sal_uInt16)26) // OBSOLET ab 500.b  ?
#define EXCHG_OUT_ACTION_REPLACE_OBJ            ((sal_uInt16)27)
#define EXCHG_OUT_ACTION_REPLACE_LINK           ((sal_uInt16)28)
#define EXCHG_OUT_ACTION_REPLACE_IMAGEMAP       ((sal_uInt16)29)
#define EXCHG_OUT_ACTION_GET_ATTRIBUTES         ((sal_uInt16)30)
#define EXCHG_OUT_ACTION_UPLOAD                 ((sal_uInt16)31)  // OBSOLET ab 500.b ?
#define EXCHG_OUT_ACTION_MOVE_FILE              ((sal_uInt16)32)
#define EXCHG_OUT_ACTION_MOVE_FILELIST          ((sal_uInt16)33)
#define EXCHG_OUT_ACTION_UPDATE_RANGE           ((sal_uInt16)34)
#define EXCHG_OUT_ACTION_INSERT_PRIVATE         ((sal_uInt16)35)
#define EXCHG_OUT_ACTION_INSERT_HTML            ((sal_uInt16)36)
#define EXCHG_OUT_ACTION_MOVE_PRIVATE           ((sal_uInt16)37)
#define EXCHG_OUT_ACTION_INSERT_STRING          ((sal_uInt16)38)
#define EXCHG_OUT_ACTION_INSERT_DRAWOBJ         ((sal_uInt16)39)
#define EXCHG_OUT_ACTION_INSERT_SVXB            ((sal_uInt16)40)
#define EXCHG_OUT_ACTION_INSERT_GDIMETAFILE     ((sal_uInt16)41)
#define EXCHG_OUT_ACTION_INSERT_BITMAP          ((sal_uInt16)42)
#define EXCHG_OUT_ACTION_INSERT_DDE             ((sal_uInt16)43)
#define EXCHG_OUT_ACTION_INSERT_HYPERLINK       ((sal_uInt16)44)
#define EXCHG_OUT_ACTION_REPLACE_DRAWOBJ        ((sal_uInt16)45)
#define EXCHG_OUT_ACTION_REPLACE_SVXB           ((sal_uInt16)46)
#define EXCHG_OUT_ACTION_REPLACE_GDIMETAFILE    ((sal_uInt16)47)
#define EXCHG_OUT_ACTION_REPLACE_BITMAP         ((sal_uInt16)48)
#define EXCHG_OUT_ACTION_REPLACE_GRAPH          ((sal_uInt16)49)
#define EXCHG_OUT_ACTION_INSERT_GRAPH           ((sal_uInt16)50)
#define EXCHG_OUT_ACTION_INSERT_MSGATTACH       ((sal_uInt16)51)  // obsolet ab 500.b ?
#define EXCHG_OUT_ACTION_COPY_CHAOSOBJ          ((sal_uInt16)52)
#define EXCHG_OUT_ACTION_MOVE_CHAOSOBJ          ((sal_uInt16)53)
#define EXCHG_OUT_ACTION_COPY_MSGATTACH         ((sal_uInt16)54)
#define EXCHG_OUT_ACTION_COPY_BOOKMARK          ((sal_uInt16)55)
#define EXCHG_OUT_ACTION_COPY_FILE              ((sal_uInt16)56)

#define EXCHG_OUT_ACTION_FLAG_CREATE_THEME      ((sal_uInt16)0x0100)
#define EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE      ((sal_uInt16)0x0200)
#define EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP   ((sal_uInt16)0x0400)
#define EXCHG_OUT_ACTION_FLAG_REPLACE_IMAGEMAP  ((sal_uInt16)0x0800)
#define EXCHG_OUT_ACTION_FLAG_FILL              ((sal_uInt16)0x1000)
#define EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL  ((sal_uInt16)0x2000)

// Ziele
#define EXCHG_DEST_DOC_OLEOBJ               1
#define EXCHG_DEST_CHARTDOC_OLEOBJ          2
#define EXCHG_DEST_DOC_TEXTFRAME            3
#define EXCHG_DEST_DOC_GRAPHOBJ             4
#define EXCHG_DEST_DOC_LNKD_GRAPHOBJ        5
#define EXCHG_DEST_DOC_GRAPH_W_IMAP         6
#define EXCHG_DEST_DOC_LNKD_GRAPH_W_IMAP    7
#define EXCHG_DEST_DOC_IMAPREGION           8
#define EXCHG_DEST_DOC_DRAWOBJ              9
#define EXCHG_DEST_DOC_URLBUTTON           10
#define EXCHG_DEST_DOC_URLFIELD            11
#define EXCHG_DEST_DOC_GROUPOBJ            12
#define EXCHG_DEST_SWDOC_FREE_AREA         13
#define EXCHG_DEST_SCDOC_FREE_AREA         14
#define EXCHG_DEST_SDDOC_FREE_AREA         15
#define EXCHG_DEST_DOC_TEXTFRAME_WEB       16
#define EXCHG_DEST_SWDOC_FREE_AREA_WEB     17

// ------------
// - Exchange -
// ------------
class SvGlobalName;
class SOT_DLLPUBLIC SotExchange
{
public:
    static sal_uLong    RegisterFormat( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    static sal_uLong    RegisterFormatName( const String& rName );
    static sal_uLong    RegisterFormatMimeType( const String& rMimeType );

    static sal_uLong    GetFormat( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    static String   GetFormatName( sal_uLong nFormat );
    static sal_Bool GetFormatDataFlavor( sal_uLong nFormat, ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    static String   GetFormatMimeType( sal_uLong nFormat );
    static sal_Bool     IsInternal( const SvGlobalName& );
    static sal_uLong    GetFormatIdFromMimeType( const String& rMimeType );

    // bestimme die SotFormatStringId von dem registrierten Format
    //JP 12.11.98: diese 3 Methoden sind ab sofort ueberfluessig, da
    //              die ClipboardIds statisch sind und aequivalent zur
    //              SotFormatStringId ist!
    static SotFormatStringId GetFormatStringId( sal_uLong nFormat )
        { return nFormat; }
    static SotFormatStringId GetFormatStringId( const String& rName )
        { return SotExchange::RegisterFormatMimeType( rName ); }
    static sal_uLong RegisterSotFormatName( SotFormatStringId nId )
        { return nId; }

    // same for XTransferable interface
    static sal_uInt16   GetExchangeAction(
        // XTransferable
        const DataFlavorExVector& rDataFlavorExVector,
        // Ziel der Aktion (EXCHG_DEST_*)
        sal_uInt16 nDestination,
        // Aktionen, die Quelle unterstuetzt (EXCHG_SOURCE_...)
        sal_uInt16 nSourceOptions,
        // vom Anwender gewaehlte Aktion (EXCHG_IN_*, EXCHG_INOUT_*)
        sal_uInt16 nUserAction,
        // In:- Out: Zu benutzendes Format
        sal_uLong& rFormat,
        // In:- Out: Default-Action (EXCHG_IN_*, EXCHG_INOUT_*)
        sal_uInt16& rDefaultAction,
        // In:- optional - check only for this specific format
        sal_uLong nOnlyTestFormat = 0,
        // In:- optional - check the contents of Xtransferable
        const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >* pxTransferable = NULL );

    // same for XTransferable interface
    static sal_uInt16   GetExchangeAction(
        // XTransferable
        const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& rxTransferable,
        // Ziel der Aktion (EXCHG_DEST_*)
        sal_uInt16 nDestination,
        // Aktionen, die Quelle unterstuetzt (EXCHG_SOURCE_...)
        sal_uInt16 nSourceOptions,
        // vom Anwender gewaehlte Aktion (EXCHG_IN_*, EXCHG_INOUT_*)
        sal_uInt16 nUserAction,
        // In:- Out: Zu benutzendes Format
        sal_uLong& rFormat,
        // In:- Out: Default-Action (EXCHG_IN_*, EXCHG_INOUT_*)
        sal_uInt16& rDefaultAction,
        // In:- optional - check only for this specific format
        sal_uLong nOnlyTestFormat = 0 );

    static sal_uInt16 IsChart( const SvGlobalName& rName );
    static sal_uInt16 IsMath( const SvGlobalName& rName );
};

#endif // _EXCHANGE_HXX
