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

#ifndef INCLUDED_SOT_EXCHANGE_HXX
#define INCLUDED_SOT_EXCHANGE_HXX

#include <list>
#include <vector>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/datatransfer/DataFlavor.hpp>
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#include <sot/sotdllapi.h>
#include <sot/formats.hxx>
#include <tools/solar.h>

class SotDataObject;

namespace com { namespace sun { namespace star { namespace datatransfer {
    class XTransferable;
} } } }


struct DataFlavorEx : public css::datatransfer::DataFlavor
{
    SotClipboardFormatId mnSotId;
};

// JP 23.03.2001 - this struct is only for "hide" the STD of the vetor,
// because our makefile filter all this symbols and so nobody can use
// these struct in any interfacses.
struct DataFlavorExVector : public ::std::vector< DataFlavorEx >
{
};


SOT_DLLPUBLIC bool IsFormatSupported( const DataFlavorExVector& rDataFlavorExVector,
                                      SotClipboardFormatId nId );

// actions
#define EXCHG_ACTION_MASK                       ((sal_uInt16)0x00FF)
#define EXCHG_INOUT_ACTION_NONE                 ((sal_uInt16)css::datatransfer::dnd::DNDConstants::ACTION_NONE)
#define EXCHG_IN_ACTION_DEFAULT                 EXCHG_INOUT_ACTION_NONE
#define EXCHG_IN_ACTION_MOVE                    ((sal_uInt16)css::datatransfer::dnd::DNDConstants::ACTION_MOVE)
#define EXCHG_IN_ACTION_COPY                    ((sal_uInt16)css::datatransfer::dnd::DNDConstants::ACTION_COPY)
#define EXCHG_IN_ACTION_LINK                    ((sal_uInt16)css::datatransfer::dnd::DNDConstants::ACTION_LINK)
#define EXCHG_OUT_ACTION_INSERT_OBJ             ((sal_uInt16)17)
#define EXCHG_OUT_ACTION_INSERT_BOOKMARK        ((sal_uInt16)18)
#define EXCHG_OUT_ACTION_INSERT_FILE            ((sal_uInt16)20)
#define EXCHG_OUT_ACTION_INSERT_IMAGEMAP        ((sal_uInt16)22)
#define EXCHG_OUT_ACTION_INSERT_OLE             ((sal_uInt16)23)
#define EXCHG_OUT_ACTION_INSERT_INTERACTIVE     ((sal_uInt16)24)
#define EXCHG_OUT_ACTION_REPLACE_IMAGEMAP       ((sal_uInt16)29)
#define EXCHG_OUT_ACTION_GET_ATTRIBUTES         ((sal_uInt16)30)
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

#define EXCHG_OUT_ACTION_FLAG_CREATE_THEME      ((sal_uInt16)0x0100)
#define EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE      ((sal_uInt16)0x0200)
#define EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP   ((sal_uInt16)0x0400)
#define EXCHG_OUT_ACTION_FLAG_REPLACE_IMAGEMAP  ((sal_uInt16)0x0800)
#define EXCHG_OUT_ACTION_FLAG_FILL              ((sal_uInt16)0x1000)
#define EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL  ((sal_uInt16)0x2000)

// destinations
enum class SotExchangeDest
{
    NONE                     =  0,
    DOC_OLEOBJ               =  1,
    CHARTDOC_OLEOBJ          =  2,
    DOC_TEXTFRAME            =  3,
    DOC_GRAPHOBJ             =  4,
    DOC_LNKD_GRAPHOBJ        =  5,
    DOC_GRAPH_W_IMAP         =  6,
    DOC_LNKD_GRAPH_W_IMAP    =  7,
    DOC_IMAPREGION           =  8,
    DOC_DRAWOBJ              =  9,
    DOC_URLBUTTON            = 10,
    DOC_URLFIELD             = 11,
    DOC_GROUPOBJ             = 12,
    SWDOC_FREE_AREA          = 13,
    SCDOC_FREE_AREA          = 14,
    SDDOC_FREE_AREA          = 15,
    DOC_TEXTFRAME_WEB        = 16,
    SWDOC_FREE_AREA_WEB      = 17,
};

class SvGlobalName;

class SOT_DLLPUBLIC SotExchange
{
public:
    static SotClipboardFormatId    RegisterFormat( const css::datatransfer::DataFlavor& rFlavor );
    static SotClipboardFormatId    RegisterFormatName( const OUString& rName );
    static SotClipboardFormatId    RegisterFormatMimeType( const OUString& rMimeType );

    static SotClipboardFormatId    GetFormat( const css::datatransfer::DataFlavor& rFlavor );
    static OUString     GetFormatName( SotClipboardFormatId nFormat );
    static bool         GetFormatDataFlavor( SotClipboardFormatId nFormat, css::datatransfer::DataFlavor& rFlavor );
    static OUString     GetFormatMimeType( SotClipboardFormatId nFormat );
    static bool         IsInternal( const SvGlobalName& );
    static SotClipboardFormatId   GetFormatIdFromMimeType( const OUString& rMimeType );

    // same for XTransferable interface
    static sal_uInt16   GetExchangeAction(
        // XTransferable
        const DataFlavorExVector& rDataFlavorExVector,
        // destination of an action
        SotExchangeDest nDestination,
        // action for a source
        sal_uInt16 nSourceOptions,
        // user action (EXCHG_IN_*, EXCHG_INOUT_*)
        sal_uInt16 nUserAction,
        // In:- Out: format to use
        SotClipboardFormatId& rFormat,
        // In:- Out: default action (EXCHG_IN_*, EXCHG_INOUT_*)
        sal_uInt16& rDefaultAction,
        // In:- optional - check only for this specific format
        SotClipboardFormatId nOnlyTestFormat = SotClipboardFormatId::NONE,
        // In:- optional - check the contents of Xtransferable
        const css::uno::Reference< css::datatransfer::XTransferable >* pxTransferable = NULL );

    static sal_uInt16 IsChart( const SvGlobalName& rName );
    static sal_uInt16 IsMath( const SvGlobalName& rName );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
