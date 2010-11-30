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

#ifndef _SOT_EXCHANGE_HXX
#define _SOT_EXCHANGE_HXX

#include <list>
#include <vector>
#include <tools/string.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/datatransfer/DataFlavor.hpp>
#include <com/sun/star/datatransfer/dnd/DNDConstants.hdl>
#include "sot/sotdllapi.h"

class SotDataObject;

namespace com { namespace sun { namespace star { namespace datatransfer {
    class XTransferable;
} } } }

// ---------------------
// - SotFormatStringId -
// ---------------------

typedef ULONG SotFormatStringId;

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
                            ULONG nId );

// -------------------------
// - Vordefinierte Formate -
// -------------------------

// Die Reihenfolge und die Werte dürfen nicht geändert werden,
// da die Implementation sich darauf verläßt.
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
#define EXCHG_SOURCE_MOVEABLE       ((USHORT)0x0001)
#define EXCHG_SOURCE_COPYABLE       ((USHORT)0x0002)
#define EXCHG_SOURCE_LINKABLE       ((USHORT)0x0004)
#define EXCHG_SOURCE_PRINTABLE      ((USHORT)0x0008)
#define EXCHG_SOURCE_DISCARDABLE    ((USHORT)0x0010)
#define EXCHG_SOURCE_ALL            ((USHORT)0x001F)
#define EXCHG_SOURCE_DEF_COPYABLE   ((USHORT)0x0020)

// Aktionen
#define EXCHG_ACTION_MASK                       ((USHORT)0x00FF)
#define EXCHG_INOUT_ACTION_NONE                 ((USHORT)com::sun::star::datatransfer::dnd::DNDConstants::ACTION_NONE)
#define EXCHG_IN_ACTION_DEFAULT                 EXCHG_INOUT_ACTION_NONE
#define EXCHG_IN_ACTION_MOVE                    ((USHORT)com::sun::star::datatransfer::dnd::DNDConstants::ACTION_MOVE)
#define EXCHG_IN_ACTION_COPY                    ((USHORT)com::sun::star::datatransfer::dnd::DNDConstants::ACTION_COPY)
#define EXCHG_IN_ACTION_LINK                    ((USHORT)com::sun::star::datatransfer::dnd::DNDConstants::ACTION_LINK)
#define EXCHG_INOUT_ACTION_PRINT                ((USHORT)8)
#define EXCHG_INOUT_ACTION_DISCARD              ((USHORT)16)
#define EXCHG_OUT_ACTION_INSERT_OBJ             ((USHORT)17)
#define EXCHG_OUT_ACTION_INSERT_BOOKMARK        ((USHORT)18)
#define EXCHG_OUT_ACTION_INSERT_FILELINK        ((USHORT)19)
#define EXCHG_OUT_ACTION_INSERT_FILE            ((USHORT)20)
#define EXCHG_OUT_ACTION_INSERT_FILELIST        ((USHORT)21)
#define EXCHG_OUT_ACTION_INSERT_IMAGEMAP        ((USHORT)22)
#define EXCHG_OUT_ACTION_INSERT_OLE             ((USHORT)23)
#define EXCHG_OUT_ACTION_INSERT_INTERACTIVE     ((USHORT)24)
#define EXCHG_OUT_ACTION_INSERT_URLBUTTON       ((USHORT)25)
#define EXCHG_OUT_ACTION_INSERT_CHAOSOBJ        ((USHORT)26) // OBSOLET ab 500.b  ?
#define EXCHG_OUT_ACTION_REPLACE_OBJ            ((USHORT)27)
#define EXCHG_OUT_ACTION_REPLACE_LINK           ((USHORT)28)
#define EXCHG_OUT_ACTION_REPLACE_IMAGEMAP       ((USHORT)29)
#define EXCHG_OUT_ACTION_GET_ATTRIBUTES         ((USHORT)30)
#define EXCHG_OUT_ACTION_UPLOAD                 ((USHORT)31)  // OBSOLET ab 500.b ?
#define EXCHG_OUT_ACTION_MOVE_FILE              ((USHORT)32)
#define EXCHG_OUT_ACTION_MOVE_FILELIST          ((USHORT)33)
#define EXCHG_OUT_ACTION_UPDATE_RANGE           ((USHORT)34)
#define EXCHG_OUT_ACTION_INSERT_PRIVATE         ((USHORT)35)
#define EXCHG_OUT_ACTION_INSERT_HTML            ((USHORT)36)
#define EXCHG_OUT_ACTION_MOVE_PRIVATE           ((USHORT)37)
#define EXCHG_OUT_ACTION_INSERT_STRING          ((USHORT)38)
#define EXCHG_OUT_ACTION_INSERT_DRAWOBJ         ((USHORT)39)
#define EXCHG_OUT_ACTION_INSERT_SVXB            ((USHORT)40)
#define EXCHG_OUT_ACTION_INSERT_GDIMETAFILE     ((USHORT)41)
#define EXCHG_OUT_ACTION_INSERT_BITMAP          ((USHORT)42)
#define EXCHG_OUT_ACTION_INSERT_DDE             ((USHORT)43)
#define EXCHG_OUT_ACTION_INSERT_HYPERLINK       ((USHORT)44)
#define EXCHG_OUT_ACTION_REPLACE_DRAWOBJ        ((USHORT)45)
#define EXCHG_OUT_ACTION_REPLACE_SVXB           ((USHORT)46)
#define EXCHG_OUT_ACTION_REPLACE_GDIMETAFILE    ((USHORT)47)
#define EXCHG_OUT_ACTION_REPLACE_BITMAP         ((USHORT)48)
#define EXCHG_OUT_ACTION_REPLACE_GRAPH          ((USHORT)49)
#define EXCHG_OUT_ACTION_INSERT_GRAPH           ((USHORT)50)
#define EXCHG_OUT_ACTION_INSERT_MSGATTACH       ((USHORT)51)  // obsolet ab 500.b ?
#define EXCHG_OUT_ACTION_COPY_CHAOSOBJ          ((USHORT)52)
#define EXCHG_OUT_ACTION_MOVE_CHAOSOBJ          ((USHORT)53)
#define EXCHG_OUT_ACTION_COPY_MSGATTACH         ((USHORT)54)
#define EXCHG_OUT_ACTION_COPY_BOOKMARK          ((USHORT)55)
#define EXCHG_OUT_ACTION_COPY_FILE              ((USHORT)56)

#define EXCHG_OUT_ACTION_FLAG_CREATE_THEME      ((USHORT)0x0100)
#define EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE      ((USHORT)0x0200)
#define EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP   ((USHORT)0x0400)
#define EXCHG_OUT_ACTION_FLAG_REPLACE_IMAGEMAP  ((USHORT)0x0800)
#define EXCHG_OUT_ACTION_FLAG_FILL              ((USHORT)0x1000)
#define EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL  ((USHORT)0x2000)

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
    static ULONG    RegisterFormat( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    static ULONG    RegisterFormatName( const String& rName );
    static ULONG    RegisterFormatMimeType( const String& rMimeType );

    static ULONG    GetFormat( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    static String   GetFormatName( ULONG nFormat );
    static sal_Bool GetFormatDataFlavor( ULONG nFormat, ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    static String   GetFormatMimeType( ULONG nFormat );
    static BOOL     IsInternal( const SvGlobalName& );
    static ULONG    GetFormatIdFromMimeType( const String& rMimeType );

    // bestimme die SotFormatStringId von dem registrierten Format
    //JP 12.11.98: diese 3 Methoden sind ab sofort ueberfluessig, da
    //              die ClipboardIds statisch sind und aequivalent zur
    //              SotFormatStringId ist!
    static SotFormatStringId GetFormatStringId( ULONG nFormat )
        { return nFormat; }
    static SotFormatStringId GetFormatStringId( const String& rName )
        { return SotExchange::RegisterFormatMimeType( rName ); }
    static ULONG RegisterSotFormatName( SotFormatStringId nId )
        { return nId; }

    // same for XTransferable interface
    static USHORT   GetExchangeAction(
        // XTransferable
        const DataFlavorExVector& rDataFlavorExVector,
        // Ziel der Aktion (EXCHG_DEST_*)
        USHORT nDestination,
        // Aktionen, die Quelle unterstuetzt (EXCHG_SOURCE_...)
        USHORT nSourceOptions,
        // vom Anwender gewaehlte Aktion (EXCHG_IN_*, EXCHG_INOUT_*)
        USHORT nUserAction,
        // In:- Out: Zu benutzendes Format
        ULONG& rFormat,
        // In:- Out: Default-Action (EXCHG_IN_*, EXCHG_INOUT_*)
        USHORT& rDefaultAction,
        // In:- optional - check only for this specific format
        ULONG nOnlyTestFormat = 0,
        // In:- optional - check the contents of Xtransferable
        const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >* pxTransferable = NULL );

    // same for XTransferable interface
    static USHORT   GetExchangeAction(
        // XTransferable
        const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& rxTransferable,
        // Ziel der Aktion (EXCHG_DEST_*)
        USHORT nDestination,
        // Aktionen, die Quelle unterstuetzt (EXCHG_SOURCE_...)
        USHORT nSourceOptions,
        // vom Anwender gewaehlte Aktion (EXCHG_IN_*, EXCHG_INOUT_*)
        USHORT nUserAction,
        // In:- Out: Zu benutzendes Format
        ULONG& rFormat,
        // In:- Out: Default-Action (EXCHG_IN_*, EXCHG_INOUT_*)
        USHORT& rDefaultAction,
        // In:- optional - check only for this specific format
        ULONG nOnlyTestFormat = 0 );

    static USHORT IsChart( const SvGlobalName& rName );
    static USHORT IsMath( const SvGlobalName& rName );
};

#endif // _EXCHANGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
