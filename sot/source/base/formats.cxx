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

#include <tools/solar.h>

#include <sot/exchange.hxx>
#include <sot/formats.hxx>
#include <sot/filelist.hxx>
#include <sysformats.hxx>
#include <comphelper/classids.hxx>

#include <tools/globname.hxx>
#include <com/sun/star/datatransfer/DataFlavor.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>

using namespace::com::sun::star::uno;
using namespace::com::sun::star::datatransfer;

struct SotAction_Impl
{
    SotClipboardFormatId nFormatId;          // Clipboard Id
    sal_uInt16           nAction;            // Action Id
    sal_uInt8            nContextCheckId;    // additional check of content in clipboard
};


// define a context check Id for every formatid
#define FILEGRPDSC_ONLY_URL     1

/*
    Fuer jedes Ziel existiert in der Tabelle genau ein SotDestinationEntry_Impl.
    Dieser Eintrag enthaelt u.a. fuer jedes vom Ziel auswertbare Format eine
    Default-Action. Die Default-Aktionen verweisen fuer jedes Format auf
    die auszuwertende Tabelle, d.h. sie enthalten nur EXCHG_IN_ACTION_MOVE,
    EXCHG_IN_ACTION_COPY oder EXCHG_IN_ACTION_LINK. Entsprechend dieser Aktion
    ist dann aMoveActions, aCopyActions oder aLinkActions auszuwerten.
    Die Aktionen sind nach Prioritaet sortiert, d.h. je "wichtiger" das
    Format ist, desto eher erscheint es in der Liste.
*/

struct SotDestinationEntry_Impl
{
    SotExchangeDest         nDestination;
    const SotAction_Impl*   aDefaultActions;
    const SotAction_Impl*   aMoveActions;
    const SotAction_Impl*   aCopyActions;
    const SotAction_Impl*   aLinkActions;
};


/*
    Ueber diese Tabelle erfolgt die Zuordnung von Destination, vorhandenen
    Datenformaten sowie gewuenschter Aktion zu einer Aktion und dem in
    ihr zu benutzenden Datenformat. Die Tabelle ist nach den Exchange-Zielen
    (EXCHG_DEST_*) sortiert. Innerhalb des Zieleintrages befinden sich genau
    vier Tabellen fuer Default-, Move-, Copy- und Linkaktionen. Ueber
    die Default-Tabelle erfolgt das Mapping zwischen Default-Aktion
     (DropEvent::IsDefaultAction()) und daraus resultierender wirklicher
    Aktion. Diese Tabelle enthaelt deshalb nur die Aktionen
     EXCHG_IN_ACTION_COPY, EXCHG_IN_ACTION_MOVE und EXCHG_IN_ACTION_LINK,
    die auf die spezielle Tabelle verweisen. Die uebrigen Tabellen
    koennen beliebige Aktionen enthalten. Jede Tabelle ist nach der
    Format-Prioritaet sortiert. Eintrag Null hat die hoechste Prioritaet.
*/

#define EXCHG_EMPYT_ARRAY                                                 \
static SotAction_Impl const aEmptyArr[] =                         \
    {                                                                     \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                  \
    };

#define EXCHG_DEST_DOC_OLEOBJ_ARRAY                                         \
static SotAction_Impl const aEXCHG_DEST_DOC_OLEOBJ_Def[] =        \
    {                                                                       \
        { SotClipboardFormatId::INET_IMAGE, EXCHG_IN_ACTION_LINK, 0 },              \
        { SotClipboardFormatId::NETSCAPE_IMAGE, EXCHG_IN_ACTION_LINK, 0 },          \
        { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_LINK, 0 },                     \
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_LINK, 0 },                          \
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_IN_ACTION_LINK, 0 },       \
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_IN_ACTION_LINK, FILEGRPDSC_ONLY_URL },       \
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_IN_ACTION_LINK, 0 },       \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_DOC_OLEOBJ_Move[] =       \
    {                                                                       \
        { SotClipboardFormatId::SVIM, EXCHG_OUT_ACTION_INSERT_IMAGEMAP, 0 },        \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_DOC_OLEOBJ_Link[] =       \
    {                                                                       \
        { SotClipboardFormatId::INET_IMAGE, EXCHG_OUT_ACTION_INSERT_INTERACTIVE, 0 },       \
        { SotClipboardFormatId::NETSCAPE_IMAGE, EXCHG_OUT_ACTION_INSERT_INTERACTIVE, 0 },   \
        { SotClipboardFormatId::FILE_LIST, EXCHG_OUT_ACTION_INSERT_INTERACTIVE, 0 },      \
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_INTERACTIVE, 0 },           \
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_INTERACTIVE, 0 },\
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_INTERACTIVE, FILEGRPDSC_ONLY_URL },\
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_INTERACTIVE, 0 },\
        { SotClipboardFormatId::SVIM, EXCHG_OUT_ACTION_INSERT_IMAGEMAP, 0 },        \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };

#define EXCHG_DEST_CHARTDOC_OLEOBJ_ARRAY                                    \
static SotAction_Impl const aEXCHG_DEST_CHARTDOC_OLEOBJ_Def[] =   \
    {                                                                       \
        { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_LINK, 0 },                     \
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_LINK, 0 },                          \
        { SotClipboardFormatId::INET_IMAGE, EXCHG_IN_ACTION_LINK, 0 },              \
        { SotClipboardFormatId::NETSCAPE_IMAGE, EXCHG_IN_ACTION_LINK, 0 },          \
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_IN_ACTION_LINK, 0 },       \
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_IN_ACTION_LINK, FILEGRPDSC_ONLY_URL },       \
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_IN_ACTION_LINK, 0 },       \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_CHARTDOC_OLEOBJ_Move[] =  \
    {                                                                       \
        { SotClipboardFormatId::SVIM, EXCHG_OUT_ACTION_INSERT_IMAGEMAP, 0 },        \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_CHARTDOC_OLEOBJ_Link[] =  \
    {                                                                       \
        { SotClipboardFormatId::FILE_LIST, EXCHG_OUT_ACTION_INSERT_INTERACTIVE, 0 },      \
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_INTERACTIVE, 0 },           \
        { SotClipboardFormatId::INET_IMAGE, EXCHG_OUT_ACTION_INSERT_INTERACTIVE, 0 },              \
        { SotClipboardFormatId::NETSCAPE_IMAGE, EXCHG_OUT_ACTION_INSERT_INTERACTIVE, 0 },          \
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_INTERACTIVE, 0 },\
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_INTERACTIVE, FILEGRPDSC_ONLY_URL },\
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_INTERACTIVE, 0 },\
        { SotClipboardFormatId::SVIM, EXCHG_OUT_ACTION_INSERT_IMAGEMAP, 0 },        \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };

#define EXCHG_DEST_DOC_TEXTFRAME_ARRAY                                      \
static SotAction_Impl const aEXCHG_DEST_DOC_TEXTFRAME_Def[] =     \
    {                                                                       \
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_IN_ACTION_COPY, 0 },                   \
        { SotClipboardFormatId::DRAWING, EXCHG_IN_ACTION_COPY, 0 },                 \
        { SotClipboardFormatId::PNG, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::BITMAP, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::SVXB, EXCHG_IN_ACTION_COPY, 0 },                    \
        { SotClipboardFormatId::HTML, EXCHG_IN_ACTION_COPY, 0 },                    \
        { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_IN_ACTION_COPY, 0 },         \
        { SotClipboardFormatId::HTML_SIMPLE, EXCHG_IN_ACTION_COPY, 0 },             \
        { SotClipboardFormatId::STRING, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::SONLK, EXCHG_IN_ACTION_COPY, 0 },                   \
        { SotClipboardFormatId::INET_IMAGE, EXCHG_IN_ACTION_COPY, 0 },               \
        { SotClipboardFormatId::NETSCAPE_IMAGE, EXCHG_IN_ACTION_COPY, 0 },           \
        { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_COPY, 0 },                     \
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_COPY, 0 },                          \
        { SotClipboardFormatId::SOLK, EXCHG_IN_ACTION_COPY, 0 },       \
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_IN_ACTION_COPY, 0 },       \
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_IN_ACTION_COPY, FILEGRPDSC_ONLY_URL },       \
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_IN_ACTION_COPY, 0 },       \
        { SotClipboardFormatId::SD_OLE, EXCHG_IN_ACTION_MOVE, 0 },                  \
        { SotClipboardFormatId::EMBED_SOURCE, EXCHG_IN_ACTION_MOVE, 0 },             \
        { SotClipboardFormatId::EMBEDDED_OBJ, EXCHG_IN_ACTION_MOVE, 0 },             \
        { SotClipboardFormatId::EMBED_SOURCE_OLE, EXCHG_IN_ACTION_MOVE, 0 },         \
        { SotClipboardFormatId::EMBEDDED_OBJ_OLE, EXCHG_IN_ACTION_MOVE, 0 },         \
        { SotClipboardFormatId::LINK, EXCHG_IN_ACTION_MOVE, 0 },                     \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_DOC_TEXTFRAME_Move[] =    \
    {                                                                       \
        { SotClipboardFormatId::SONLK, EXCHG_IN_ACTION_MOVE, 0 },                   \
        { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_MOVE, 0 },\
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_INSERT_DRAWOBJ| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SD_OLE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::EMBED_SOURCE, EXCHG_OUT_ACTION_INSERT_OLE, 0 },     \
        { SotClipboardFormatId::EMBEDDED_OBJ, EXCHG_OUT_ACTION_INSERT_OLE, 0 },     \
        { SotClipboardFormatId::HTML, EXCHG_OUT_ACTION_INSERT_HTML| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_OUT_ACTION_INSERT_HTML| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::HTML_SIMPLE, EXCHG_OUT_ACTION_INSERT_HTML| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::RTF, EXCHG_IN_ACTION_COPY| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::NETSCAPE_IMAGE, EXCHG_IN_ACTION_COPY | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },          \
        { SotClipboardFormatId::STRING, EXCHG_OUT_ACTION_INSERT_STRING| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_INSERT_BITMAP| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_INSERT_BITMAP| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::EMBED_SOURCE_OLE, EXCHG_OUT_ACTION_INSERT_OLE, 0 }, \
        { SotClipboardFormatId::EMBEDDED_OBJ_OLE, EXCHG_OUT_ACTION_INSERT_OLE, 0 }, \
        { SotClipboardFormatId::LINK, EXCHG_OUT_ACTION_INSERT_DDE, 0 },             \
        { SotClipboardFormatId::SVIM, EXCHG_OUT_ACTION_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },        \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_DOC_TEXTFRAME_Copy[] =    \
    {                                                                       \
        { SotClipboardFormatId::SBA_DATAEXCHANGE, EXCHG_IN_ACTION_COPY, 0 },        \
        { SotClipboardFormatId::SBA_CTRLDATAEXCHANGE, EXCHG_IN_ACTION_COPY, 0 },   \
        { SotClipboardFormatId::SBA_FIELDDATAEXCHANGE, EXCHG_IN_ACTION_COPY, 0 },   \
        { SotClipboardFormatId::XFORMS, EXCHG_IN_ACTION_COPY, 0 },   \
        { SotClipboardFormatId::SONLK, EXCHG_IN_ACTION_COPY | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },                   \
        { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_COPY, 0 },\
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_INSERT_DRAWOBJ| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SD_OLE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::EMBED_SOURCE, EXCHG_OUT_ACTION_INSERT_OLE, 0 },     \
        { SotClipboardFormatId::EMBEDDED_OBJ, EXCHG_OUT_ACTION_INSERT_OLE, 0 },     \
        { SotClipboardFormatId::HTML, EXCHG_OUT_ACTION_INSERT_HTML| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_OUT_ACTION_INSERT_HTML| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::HTML_SIMPLE, EXCHG_OUT_ACTION_INSERT_HTML| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::RTF, EXCHG_IN_ACTION_COPY| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::NETSCAPE_IMAGE, EXCHG_IN_ACTION_COPY | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },          \
        { SotClipboardFormatId::STRING, EXCHG_OUT_ACTION_INSERT_STRING| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, FILEGRPDSC_ONLY_URL },\
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::EMBED_SOURCE_OLE, EXCHG_OUT_ACTION_INSERT_OLE, 0 }, \
        { SotClipboardFormatId::EMBEDDED_OBJ_OLE, EXCHG_OUT_ACTION_INSERT_OLE, 0 }, \
        { SotClipboardFormatId::LINK, EXCHG_OUT_ACTION_INSERT_DDE, 0 },             \
        { SotClipboardFormatId::SVIM, EXCHG_OUT_ACTION_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },        \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_DOC_TEXTFRAME_Link[] =    \
    {                                                                       \
        { SotClipboardFormatId::SONLK, EXCHG_IN_ACTION_LINK, 0 },                   \
        { SotClipboardFormatId::SBA_DATAEXCHANGE, EXCHG_IN_ACTION_LINK, 0 },        \
        { SotClipboardFormatId::SBA_CTRLDATAEXCHANGE, EXCHG_IN_ACTION_LINK, 0 },   \
        { SotClipboardFormatId::SBA_FIELDDATAEXCHANGE, EXCHG_IN_ACTION_LINK, 0 },   \
        { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, FILEGRPDSC_ONLY_URL },\
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_LINK | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_LINK | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::LINK, EXCHG_OUT_ACTION_INSERT_DDE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },             \
        { SotClipboardFormatId::EMBED_SOURCE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },     \
        { SotClipboardFormatId::EMBEDDED_OBJ, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },     \
        { SotClipboardFormatId::EMBED_SOURCE_OLE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 }, \
        { SotClipboardFormatId::EMBEDDED_OBJ_OLE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 }, \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };

#define EXCHG_DEST_DOC_TEXTFRAME_WEB_ARRAY                                      \
static SotAction_Impl const aEXCHG_DEST_DOC_TEXTFRAME_WEB_Def[] =     \
    {                                                                       \
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_IN_ACTION_COPY, 0 },                   \
        { SotClipboardFormatId::PNG, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::BITMAP, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::SVXB, EXCHG_IN_ACTION_COPY, 0 },                    \
        { SotClipboardFormatId::HTML, EXCHG_IN_ACTION_COPY, 0 },                    \
        { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_IN_ACTION_COPY, 0 },         \
        { SotClipboardFormatId::HTML_SIMPLE, EXCHG_IN_ACTION_COPY, 0 },             \
        { SotClipboardFormatId::STRING, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::SONLK, EXCHG_IN_ACTION_COPY, 0 },                   \
        { SotClipboardFormatId::INET_IMAGE, EXCHG_IN_ACTION_COPY, 0 },               \
        { SotClipboardFormatId::NETSCAPE_IMAGE, EXCHG_IN_ACTION_COPY, 0 },           \
        { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_COPY, 0 },                     \
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_COPY, 0 },                          \
        { SotClipboardFormatId::SOLK, EXCHG_IN_ACTION_COPY, 0 },       \
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_IN_ACTION_COPY, 0 },       \
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_IN_ACTION_COPY, FILEGRPDSC_ONLY_URL },       \
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_IN_ACTION_COPY, 0 },       \
        { SotClipboardFormatId::LINK, EXCHG_IN_ACTION_MOVE, 0 },                     \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_DOC_TEXTFRAME_WEB_Move[] =    \
    {                                                                       \
        { SotClipboardFormatId::SONLK, EXCHG_IN_ACTION_MOVE, 0 },                   \
        { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_MOVE, 0 },\
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::HTML, EXCHG_OUT_ACTION_INSERT_HTML| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_OUT_ACTION_INSERT_HTML| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::HTML_SIMPLE, EXCHG_OUT_ACTION_INSERT_HTML| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::RTF, EXCHG_IN_ACTION_COPY| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::NETSCAPE_IMAGE, EXCHG_IN_ACTION_COPY | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },          \
        { SotClipboardFormatId::STRING, EXCHG_OUT_ACTION_INSERT_STRING| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_INSERT_BITMAP| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_INSERT_BITMAP| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::LINK, EXCHG_OUT_ACTION_INSERT_DDE, 0 },             \
        { SotClipboardFormatId::SVIM, EXCHG_OUT_ACTION_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },        \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_DOC_TEXTFRAME_WEB_Copy[] =    \
    {                                                                       \
        { SotClipboardFormatId::SBA_DATAEXCHANGE, EXCHG_IN_ACTION_COPY, 0 },        \
        { SotClipboardFormatId::SBA_CTRLDATAEXCHANGE, EXCHG_IN_ACTION_COPY, 0 },   \
        { SotClipboardFormatId::SBA_FIELDDATAEXCHANGE, EXCHG_IN_ACTION_COPY, 0 },   \
        { SotClipboardFormatId::XFORMS, EXCHG_IN_ACTION_COPY, 0 },   \
        { SotClipboardFormatId::SONLK, EXCHG_IN_ACTION_COPY | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },                   \
        { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_COPY, 0 },\
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::HTML, EXCHG_OUT_ACTION_INSERT_HTML| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_OUT_ACTION_INSERT_HTML| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::HTML_SIMPLE, EXCHG_OUT_ACTION_INSERT_HTML| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::RTF, EXCHG_IN_ACTION_COPY| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::NETSCAPE_IMAGE, EXCHG_IN_ACTION_COPY | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },          \
        { SotClipboardFormatId::STRING, EXCHG_OUT_ACTION_INSERT_STRING| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, FILEGRPDSC_ONLY_URL },\
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::LINK, EXCHG_OUT_ACTION_INSERT_DDE, 0 },             \
        { SotClipboardFormatId::SVIM, EXCHG_OUT_ACTION_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },        \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_DOC_TEXTFRAME_WEB_Link[] =    \
    {                                                                       \
        { SotClipboardFormatId::SONLK, EXCHG_IN_ACTION_LINK, 0 },                   \
        { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, FILEGRPDSC_ONLY_URL },\
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_LINK | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_LINK | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::LINK, EXCHG_OUT_ACTION_INSERT_DDE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },             \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };

#define EXCHG_DEST_DOC_GRAPHOBJ_ARRAY                                       \
static SotAction_Impl const aEXCHG_DEST_DOC_GRAPHOBJ_Def[] =      \
    {                                                                       \
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_IN_ACTION_COPY, 0 },                   \
        { SotClipboardFormatId::DRAWING, EXCHG_IN_ACTION_COPY, 0 },                 \
        { SotClipboardFormatId::PNG, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::BITMAP, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::SVXB, EXCHG_IN_ACTION_COPY, 0 },                    \
        { SotClipboardFormatId::HTML, EXCHG_IN_ACTION_COPY, 0 },                    \
        { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_IN_ACTION_COPY, 0 },         \
        { SotClipboardFormatId::HTML_SIMPLE, EXCHG_IN_ACTION_COPY, 0 },             \
        { SotClipboardFormatId::STRING, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_COPY, 0 },                          \
        { SotClipboardFormatId::SOLK, EXCHG_IN_ACTION_COPY, 0 },       \
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_IN_ACTION_COPY, 0 },       \
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_IN_ACTION_COPY, FILEGRPDSC_ONLY_URL },       \
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_IN_ACTION_COPY, 0 },       \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_DOC_GRAPHOBJ_Move[] =     \
    {                                                                       \
        { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_REPLACE_DRAWOBJ | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_REPLACE_SVXB| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_REPLACE_GDIMETAFILE| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_REPLACE_BITMAP| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_REPLACE_BITMAP| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_REPLACE_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_REPLACE_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_REPLACE_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, FILEGRPDSC_ONLY_URL },\
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_REPLACE_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_REPLACE_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_DOC_GRAPHOBJ_Copy[] =     \
    {                                                                       \
        { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_INSERT_DRAWOBJ | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_INSERT_BITMAP| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_INSERT_BITMAP| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_INSERT_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, FILEGRPDSC_ONLY_URL },\
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_DOC_GRAPHOBJ_Link[] =     \
    {                                                                       \
        { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_GET_ATTRIBUTES| EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_GET_ATTRIBUTES| EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, FILEGRPDSC_ONLY_URL },\
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };

#define EXCHG_DEST_DOC_LNKD_GRAPHOBJ_ARRAY                                  \
static SotAction_Impl const aEXCHG_DEST_DOC_LNKD_GRAPHOBJ_Def[] = \
    {                                                                       \
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_IN_ACTION_COPY, 0 },                   \
        { SotClipboardFormatId::DRAWING, EXCHG_IN_ACTION_COPY, 0 },                 \
        { SotClipboardFormatId::PNG, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::BITMAP, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::SVXB, EXCHG_IN_ACTION_COPY, 0 },                    \
        { SotClipboardFormatId::HTML, EXCHG_IN_ACTION_COPY, 0 },                    \
        { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_IN_ACTION_COPY, 0 },         \
        { SotClipboardFormatId::HTML_SIMPLE, EXCHG_IN_ACTION_COPY, 0 },             \
        { SotClipboardFormatId::STRING, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::SOLK, EXCHG_IN_ACTION_COPY, 0 },       \
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_IN_ACTION_COPY, 0 },       \
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_IN_ACTION_COPY, FILEGRPDSC_ONLY_URL },       \
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_IN_ACTION_COPY, 0 },       \
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_COPY, 0 },                          \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_DOC_LNKD_GRAPHOBJ_Move[] =\
    {                                                                       \
        { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_REPLACE_DRAWOBJ | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_REPLACE_SVXB| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_REPLACE_GDIMETAFILE| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_REPLACE_BITMAP| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_REPLACE_BITMAP| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_REPLACE_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_REPLACE_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_REPLACE_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, FILEGRPDSC_ONLY_URL },\
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_REPLACE_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_REPLACE_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_DOC_LNKD_GRAPHOBJ_Copy[] =\
    {                                                                       \
        { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_INSERT_DRAWOBJ | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_INSERT_BITMAP| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_INSERT_BITMAP| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_INSERT_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, FILEGRPDSC_ONLY_URL },\
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_DOC_LNKD_GRAPHOBJ_Link[] =\
    {                                                                       \
        { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_GET_ATTRIBUTES| EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_GET_ATTRIBUTES| EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, FILEGRPDSC_ONLY_URL },\
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };

#define EXCHG_DEST_DOC_GRAPH_W_IMAP_ARRAY                                   \
static SotAction_Impl const aEXCHG_DEST_DOC_GRAPH_W_IMAP_Def[] =  \
    {                                                                       \
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_IN_ACTION_COPY, 0 },                   \
        { SotClipboardFormatId::DRAWING, EXCHG_IN_ACTION_COPY, 0 },                 \
        { SotClipboardFormatId::PNG, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::BITMAP, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::SVXB, EXCHG_IN_ACTION_COPY, 0 },                    \
        { SotClipboardFormatId::HTML, EXCHG_IN_ACTION_COPY, 0 },                    \
        { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_IN_ACTION_COPY, 0 },         \
        { SotClipboardFormatId::HTML_SIMPLE, EXCHG_IN_ACTION_COPY, 0 },             \
        { SotClipboardFormatId::STRING, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_COPY, 0 },                          \
        { SotClipboardFormatId::SOLK, EXCHG_IN_ACTION_COPY, 0 },       \
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_IN_ACTION_COPY, 0 },       \
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_IN_ACTION_COPY, FILEGRPDSC_ONLY_URL },       \
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_IN_ACTION_COPY, 0 },       \
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_COPY, 0 },                          \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_DOC_GRAPH_W_IMAP_Move[] = \
    {                                                                       \
        { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_REPLACE_DRAWOBJ | EXCHG_OUT_ACTION_FLAG_REPLACE_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_REPLACE_SVXB| EXCHG_OUT_ACTION_FLAG_REPLACE_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_REPLACE_GDIMETAFILE| EXCHG_OUT_ACTION_FLAG_REPLACE_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_REPLACE_BITMAP| EXCHG_OUT_ACTION_FLAG_REPLACE_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_REPLACE_BITMAP| EXCHG_OUT_ACTION_FLAG_REPLACE_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_REPLACE_GRAPH | EXCHG_OUT_ACTION_FLAG_REPLACE_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_REPLACE_GRAPH | EXCHG_OUT_ACTION_FLAG_REPLACE_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_REPLACE_GRAPH | EXCHG_OUT_ACTION_FLAG_REPLACE_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, FILEGRPDSC_ONLY_URL },\
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_REPLACE_GRAPH | EXCHG_OUT_ACTION_FLAG_REPLACE_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_REPLACE_GRAPH | EXCHG_OUT_ACTION_FLAG_REPLACE_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_DOC_GRAPH_W_IMAP_Copy[] =  \
    {                                                                       \
        { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_INSERT_DRAWOBJ | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_INSERT_BITMAP| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_INSERT_BITMAP| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_INSERT_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, FILEGRPDSC_ONLY_URL },\
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_DOC_GRAPH_W_IMAP_Link[] = \
    {                                                                       \
        { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_GET_ATTRIBUTES| EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_GET_ATTRIBUTES| EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, FILEGRPDSC_ONLY_URL },\
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },               \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };

#define EXCHG_DEST_DOC_LNKD_GRAPH_W_IMAP_ARRAY                              \
static SotAction_Impl const aEXCHG_DEST_DOC_LNKD_GRAPH_W_IMAP_Def[] =\
    {                                                                       \
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_IN_ACTION_COPY, 0 },                   \
        { SotClipboardFormatId::DRAWING, EXCHG_IN_ACTION_COPY, 0 },                 \
        { SotClipboardFormatId::PNG, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::BITMAP, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::SVXB, EXCHG_IN_ACTION_COPY, 0 },                    \
        { SotClipboardFormatId::HTML, EXCHG_IN_ACTION_COPY, 0 },                    \
        { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_IN_ACTION_COPY, 0 },         \
        { SotClipboardFormatId::HTML_SIMPLE, EXCHG_IN_ACTION_COPY, 0 },             \
        { SotClipboardFormatId::STRING, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_COPY, 0 },                          \
        { SotClipboardFormatId::SOLK, EXCHG_IN_ACTION_COPY, 0 },       \
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_IN_ACTION_COPY, 0 },       \
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_IN_ACTION_COPY, FILEGRPDSC_ONLY_URL },       \
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_IN_ACTION_COPY, 0 },       \
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_COPY, 0 },                          \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_DOC_LNKD_GRAPH_W_IMAP_Move[] =\
    {                                                                       \
        { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_REPLACE_DRAWOBJ | EXCHG_OUT_ACTION_FLAG_REPLACE_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_REPLACE_SVXB| EXCHG_OUT_ACTION_FLAG_REPLACE_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_REPLACE_GDIMETAFILE| EXCHG_OUT_ACTION_FLAG_REPLACE_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_REPLACE_BITMAP| EXCHG_OUT_ACTION_FLAG_REPLACE_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_REPLACE_BITMAP| EXCHG_OUT_ACTION_FLAG_REPLACE_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_REPLACE_GRAPH | EXCHG_OUT_ACTION_FLAG_REPLACE_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_REPLACE_GRAPH | EXCHG_OUT_ACTION_FLAG_REPLACE_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_REPLACE_GRAPH | EXCHG_OUT_ACTION_FLAG_REPLACE_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, FILEGRPDSC_ONLY_URL },\
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_REPLACE_GRAPH | EXCHG_OUT_ACTION_FLAG_REPLACE_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_REPLACE_GRAPH | EXCHG_OUT_ACTION_FLAG_REPLACE_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_DOC_LNKD_GRAPH_W_IMAP_Copy[] =\
    {                                                                       \
        { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_INSERT_DRAWOBJ | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_INSERT_BITMAP| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_INSERT_BITMAP| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_INSERT_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, FILEGRPDSC_ONLY_URL },\
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                           \
    };                                                                       \
static SotAction_Impl const aEXCHG_DEST_DOC_LNKD_GRAPH_W_IMAP_Link[] =\
    {                                                                        \
        { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_GET_ATTRIBUTES| EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_GET_ATTRIBUTES| EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, FILEGRPDSC_ONLY_URL },\
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },               \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };


#define EXCHG_DEST_DOC_IMAPREGION_ARRAY                                     \
static SotAction_Impl const aEXCHG_DEST_DOC_IMAPREGION_Def[] =    \
    {                                                                       \
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_IN_ACTION_COPY, 0 },       \
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_IN_ACTION_COPY, FILEGRPDSC_ONLY_URL },       \
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_IN_ACTION_COPY, 0 },       \
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_COPY, 0 },                          \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_DOC_IMAPREGION_Copy[] =   \
    {                                                                       \
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, FILEGRPDSC_ONLY_URL },\
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },                  \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };


#define EXCHG_DEST_DOC_DRAWOBJ_ARRAY                                        \
static SotAction_Impl const aEXCHG_DEST_DOC_DRAWOBJ_Def[] =       \
    {                                                                       \
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_IN_ACTION_COPY, 0 },                   \
        { SotClipboardFormatId::DRAWING, EXCHG_IN_ACTION_COPY, 0 },                 \
        { SotClipboardFormatId::PNG, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::BITMAP, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::SVXB, EXCHG_IN_ACTION_COPY, 0 },                    \
        { SotClipboardFormatId::HTML, EXCHG_IN_ACTION_COPY, 0 },                    \
        { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_IN_ACTION_COPY, 0 },         \
        { SotClipboardFormatId::HTML_SIMPLE, EXCHG_IN_ACTION_COPY, 0 },             \
        { SotClipboardFormatId::STRING, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_COPY, 0 },                          \
        { SotClipboardFormatId::SOLK, EXCHG_IN_ACTION_COPY, 0 },       \
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_IN_ACTION_COPY, 0 },       \
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_IN_ACTION_COPY, FILEGRPDSC_ONLY_URL },       \
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_IN_ACTION_COPY, 0 },       \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_DOC_DRAWOBJ_Copy[] =      \
    {                                                                       \
        { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_REPLACE_DRAWOBJ | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_REPLACE_SVXB| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_REPLACE_GDIMETAFILE| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_REPLACE_BITMAP| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_REPLACE_BITMAP| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_REPLACE_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_REPLACE_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_REPLACE_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, FILEGRPDSC_ONLY_URL },\
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_REPLACE_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_REPLACE_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_DOC_DRAWOBJ_Move[] =      \
    {                                                                       \
        { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_INSERT_DRAWOBJ | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_INSERT_BITMAP| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_INSERT_BITMAP| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_INSERT_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, FILEGRPDSC_ONLY_URL },\
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_DOC_DRAWOBJ_Link[] =      \
    {                                                                       \
        { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_GET_ATTRIBUTES| EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_GET_ATTRIBUTES| EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, FILEGRPDSC_ONLY_URL },\
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };


#define EXCHG_DEST_DOC_URLBUTTON_ARRAY                                      \
static SotAction_Impl const aEXCHG_DEST_DOC_URLBUTTON_Def[] =     \
    {                                                                       \
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_IN_ACTION_COPY, 0 },                   \
        { SotClipboardFormatId::DRAWING, EXCHG_IN_ACTION_COPY, 0 },                 \
        { SotClipboardFormatId::PNG, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::BITMAP, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::SVXB, EXCHG_IN_ACTION_COPY, 0 },                    \
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_COPY, 0 },                          \
        { SotClipboardFormatId::SOLK, EXCHG_IN_ACTION_COPY, 0 },       \
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_IN_ACTION_COPY, 0 },       \
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_IN_ACTION_COPY, FILEGRPDSC_ONLY_URL },       \
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_IN_ACTION_COPY, 0 },       \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_DOC_URLBUTTON_Move[] =    \
    {                                                                       \
        { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_REPLACE_DRAWOBJ | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_REPLACE_SVXB | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_REPLACE_GDIMETAFILE | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_DOC_URLBUTTON_Copy[] =    \
    {                                                                       \
        { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_INSERT_DRAWOBJ | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },      \
        { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },            \
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },    \
        { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_INSERT_BITMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },              \
        { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_INSERT_BITMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },              \
        { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, FILEGRPDSC_ONLY_URL },\
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },                  \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };


#define EXCHG_DEST_DOC_URLFIELD_ARRAY                                       \
static SotAction_Impl const aEXCHG_DEST_DOC_URLFIELD_Def[] =      \
    {                                                                       \
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_COPY, 0 },                          \
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_IN_ACTION_COPY, 0 },       \
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_IN_ACTION_COPY, FILEGRPDSC_ONLY_URL },       \
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_IN_ACTION_COPY, 0 },       \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_DOC_URLFIELD_Copy[] =     \
    {                                                                       \
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, FILEGRPDSC_ONLY_URL },\
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },                  \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_DOC_URLFIELD_Link[] =     \
    {                                                                       \
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_HYPERLINK, 0 },        \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };

#define EXCHG_DEST_DOC_GROUPOBJ_ARRAY                                       \
static SotAction_Impl const aEXCHG_DEST_DOC_GROUPOBJ_Def[] =      \
    {                                                                       \
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_IN_ACTION_COPY, 0 },                   \
        { SotClipboardFormatId::DRAWING, EXCHG_IN_ACTION_COPY, 0 },                 \
        { SotClipboardFormatId::PNG, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::BITMAP, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::SVXB, EXCHG_IN_ACTION_COPY, 0 },                    \
        { SotClipboardFormatId::HTML, EXCHG_IN_ACTION_COPY, 0 },                    \
        { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_IN_ACTION_COPY, 0 },         \
        { SotClipboardFormatId::HTML_SIMPLE, EXCHG_IN_ACTION_COPY, 0 },             \
        { SotClipboardFormatId::STRING, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_COPY, 0 },                          \
        { SotClipboardFormatId::SOLK, EXCHG_IN_ACTION_COPY, 0 },       \
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_IN_ACTION_COPY, 0 },       \
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_IN_ACTION_COPY, FILEGRPDSC_ONLY_URL },       \
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_IN_ACTION_COPY, 0 },       \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_DOC_GROUPOBJ_Move[] =     \
    {                                                                       \
        { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_REPLACE_DRAWOBJ | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_REPLACE_SVXB| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_REPLACE_GDIMETAFILE| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_REPLACE_BITMAP| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_REPLACE_BITMAP| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_REPLACE_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_REPLACE_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_REPLACE_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, FILEGRPDSC_ONLY_URL },\
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_REPLACE_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_REPLACE_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_KEEP_POSSIZE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_DOC_GROUPOBJ_Copy[] =     \
    {                                                                       \
        { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_INSERT_DRAWOBJ | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_INSERT_BITMAP| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_INSERT_BITMAP| EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_INSERT_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, FILEGRPDSC_ONLY_URL },\
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_GRAPH | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_DOC_GROUPOBJ_Link[] =     \
    {                                                                       \
        { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_GET_ATTRIBUTES| EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_GET_ATTRIBUTES| EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, FILEGRPDSC_ONLY_URL },\
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_GET_ATTRIBUTES | EXCHG_OUT_ACTION_FLAG_FILL | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };


#define EXCHG_DEST_SWDOC_FREE_AREA_ARRAY                                    \
static SotAction_Impl const aEXCHG_DEST_SWDOC_FREE_AREA_Def[] =   \
    {                                                                       \
        { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_COPY, 0 },                     \
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_COPY, 0 },                          \
        { SotClipboardFormatId::HTML, EXCHG_IN_ACTION_COPY, 0 },                    \
        { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_IN_ACTION_COPY, 0 },         \
        { SotClipboardFormatId::HTML_SIMPLE, EXCHG_IN_ACTION_COPY, 0 },             \
        { SotClipboardFormatId::RTF, EXCHG_IN_ACTION_COPY, 0 },                           \
        { SotClipboardFormatId::SVIM, EXCHG_IN_ACTION_COPY, 0 },                    \
        { SotClipboardFormatId::NETSCAPE_IMAGE, EXCHG_IN_ACTION_COPY, 0 },           \
        { SotClipboardFormatId::STRING, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_IN_ACTION_COPY, 0 },       \
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_IN_ACTION_COPY, FILEGRPDSC_ONLY_URL },       \
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_IN_ACTION_COPY, 0 },       \
        { SotClipboardFormatId::DRAWING, EXCHG_IN_ACTION_COPY, 0 },                 \
        { SotClipboardFormatId::SVXB, EXCHG_IN_ACTION_COPY, 0 },                    \
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_IN_ACTION_COPY, 0 },                   \
        { SotClipboardFormatId::PNG, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::BITMAP, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::SONLK, EXCHG_IN_ACTION_COPY, 0 },                   \
        { SotClipboardFormatId::SOLK, EXCHG_IN_ACTION_COPY, 0 },                   \
        { SotClipboardFormatId::SD_OLE, EXCHG_IN_ACTION_MOVE, 0 },                  \
        { SotClipboardFormatId::EMBED_SOURCE, EXCHG_IN_ACTION_MOVE, 0 },             \
        { SotClipboardFormatId::EMBEDDED_OBJ, EXCHG_IN_ACTION_MOVE, 0 },             \
        { SotClipboardFormatId::EMBED_SOURCE_OLE, EXCHG_IN_ACTION_MOVE, 0 },         \
        { SotClipboardFormatId::EMBEDDED_OBJ_OLE, EXCHG_IN_ACTION_MOVE, 0 },         \
        { SotClipboardFormatId::LINK, EXCHG_IN_ACTION_MOVE, 0 },                     \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_SWDOC_FREE_AREA_Move[] =  \
    {                                                                       \
        { SotClipboardFormatId::SONLK, EXCHG_IN_ACTION_MOVE, 0 },                   \
        { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_MOVE, 0 },\
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_INSERT_DRAWOBJ | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SD_OLE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::EMBED_SOURCE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },     \
        { SotClipboardFormatId::EMBEDDED_OBJ, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },     \
        { SotClipboardFormatId::HTML, EXCHG_OUT_ACTION_INSERT_HTML | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_OUT_ACTION_INSERT_HTML | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::HTML_SIMPLE, EXCHG_OUT_ACTION_INSERT_HTML | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::RTF, EXCHG_IN_ACTION_COPY | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },                           \
        { SotClipboardFormatId::STRING, EXCHG_OUT_ACTION_INSERT_STRING, 0 },              \
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_INSERT_BITMAP | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_INSERT_BITMAP | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::EMBED_SOURCE_OLE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 }, \
        { SotClipboardFormatId::EMBEDDED_OBJ_OLE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 }, \
        { SotClipboardFormatId::LINK, EXCHG_OUT_ACTION_INSERT_DDE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },             \
        { SotClipboardFormatId::SVIM, EXCHG_OUT_ACTION_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },        \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_SWDOC_FREE_AREA_Copy[] =  \
    {                                                                       \
        { SotClipboardFormatId::SBA_DATAEXCHANGE, EXCHG_IN_ACTION_COPY, 0 },        \
        { SotClipboardFormatId::SBA_CTRLDATAEXCHANGE, EXCHG_IN_ACTION_COPY, 0 },   \
        { SotClipboardFormatId::SBA_FIELDDATAEXCHANGE, EXCHG_IN_ACTION_COPY, 0 },   \
        { SotClipboardFormatId::XFORMS, EXCHG_IN_ACTION_COPY, 0 },   \
        { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_COPY, 0 },\
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SONLK, EXCHG_IN_ACTION_COPY | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },                   \
        { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },                   \
        { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_INSERT_DRAWOBJ | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SD_OLE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::EMBED_SOURCE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },     \
        { SotClipboardFormatId::EMBEDDED_OBJ, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },     \
        { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_INSERT_BITMAP | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_INSERT_BITMAP | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::HTML, EXCHG_OUT_ACTION_INSERT_HTML | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_OUT_ACTION_INSERT_HTML | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::HTML_SIMPLE, EXCHG_OUT_ACTION_INSERT_HTML | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::RTF, EXCHG_IN_ACTION_COPY | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },                           \
        { SotClipboardFormatId::NETSCAPE_IMAGE, EXCHG_IN_ACTION_COPY | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },          \
        { SotClipboardFormatId::STRING, EXCHG_OUT_ACTION_INSERT_STRING | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },              \
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, FILEGRPDSC_ONLY_URL },\
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::EMBED_SOURCE_OLE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 }, \
        { SotClipboardFormatId::EMBEDDED_OBJ_OLE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 }, \
        { SotClipboardFormatId::LINK, EXCHG_OUT_ACTION_INSERT_DDE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },             \
        { SotClipboardFormatId::SVIM, EXCHG_OUT_ACTION_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },        \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_SWDOC_FREE_AREA_Link[] =  \
    {                                                                       \
        { SotClipboardFormatId::SONLK, EXCHG_IN_ACTION_LINK, 0 },                   \
        { SotClipboardFormatId::SBA_DATAEXCHANGE, EXCHG_IN_ACTION_LINK, 0 },        \
        { SotClipboardFormatId::SBA_CTRLDATAEXCHANGE, EXCHG_IN_ACTION_LINK, 0 },   \
        { SotClipboardFormatId::SBA_FIELDDATAEXCHANGE, EXCHG_IN_ACTION_LINK, 0 },   \
        { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_LINK, 0 },        \
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_LINK | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },             \
        { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, FILEGRPDSC_ONLY_URL },\
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::LINK, EXCHG_OUT_ACTION_INSERT_DDE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },             \
        { SotClipboardFormatId::EMBED_SOURCE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },     \
        { SotClipboardFormatId::EMBEDDED_OBJ, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },     \
        { SotClipboardFormatId::EMBED_SOURCE_OLE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 }, \
        { SotClipboardFormatId::EMBEDDED_OBJ_OLE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 }, \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };

#define EXCHG_DEST_SWDOC_FREE_AREA_WEB_ARRAY                                    \
static SotAction_Impl const aEXCHG_DEST_SWDOC_FREE_AREA_WEB_Def[] =   \
    {                                                                       \
        { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_COPY, 0 },                     \
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_COPY, 0 },                          \
        { SotClipboardFormatId::SOLK, EXCHG_IN_ACTION_COPY, 0 },       \
        { SotClipboardFormatId::HTML, EXCHG_IN_ACTION_COPY, 0 },                    \
        { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_IN_ACTION_COPY, 0 },         \
        { SotClipboardFormatId::HTML_SIMPLE, EXCHG_IN_ACTION_COPY, 0 },             \
        { SotClipboardFormatId::RTF, EXCHG_IN_ACTION_COPY, 0 },                           \
        { SotClipboardFormatId::SVIM, EXCHG_IN_ACTION_COPY, 0 },                    \
        { SotClipboardFormatId::NETSCAPE_IMAGE, EXCHG_IN_ACTION_COPY, 0 },           \
        { SotClipboardFormatId::STRING, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_IN_ACTION_COPY, 0 },       \
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_IN_ACTION_COPY, FILEGRPDSC_ONLY_URL },       \
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_IN_ACTION_COPY, 0 },       \
        { SotClipboardFormatId::SVXB, EXCHG_IN_ACTION_COPY, 0 },                    \
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_IN_ACTION_COPY, 0 },                   \
        { SotClipboardFormatId::PNG, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::BITMAP, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::SONLK, EXCHG_IN_ACTION_COPY, 0 },                   \
        { SotClipboardFormatId::LINK, EXCHG_IN_ACTION_MOVE, 0 },                     \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_SWDOC_FREE_AREA_WEB_Move[] =  \
    {                                                                       \
        { SotClipboardFormatId::SONLK, EXCHG_IN_ACTION_MOVE, 0 },                   \
        { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_MOVE, 0 },\
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::HTML, EXCHG_OUT_ACTION_INSERT_HTML | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_OUT_ACTION_INSERT_HTML | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::HTML_SIMPLE, EXCHG_OUT_ACTION_INSERT_HTML | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::RTF, EXCHG_IN_ACTION_COPY | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },                           \
        { SotClipboardFormatId::STRING, EXCHG_OUT_ACTION_INSERT_STRING, 0 },              \
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_INSERT_BITMAP | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_INSERT_BITMAP | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::LINK, EXCHG_OUT_ACTION_INSERT_DDE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },             \
        { SotClipboardFormatId::SVIM, EXCHG_OUT_ACTION_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },        \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_SWDOC_FREE_AREA_WEB_Copy[] =  \
    {                                                                       \
        { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_COPY, 0 },\
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SONLK, EXCHG_IN_ACTION_COPY | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },                   \
        { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::HTML, EXCHG_OUT_ACTION_INSERT_HTML | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_OUT_ACTION_INSERT_HTML | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::HTML_SIMPLE, EXCHG_OUT_ACTION_INSERT_HTML | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::RTF, EXCHG_IN_ACTION_COPY | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },                           \
        { SotClipboardFormatId::NETSCAPE_IMAGE, EXCHG_IN_ACTION_COPY | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },          \
        { SotClipboardFormatId::STRING, EXCHG_OUT_ACTION_INSERT_STRING | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },              \
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, FILEGRPDSC_ONLY_URL },\
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_INSERT_BITMAP | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_INSERT_BITMAP | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::LINK, EXCHG_OUT_ACTION_INSERT_DDE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },             \
        { SotClipboardFormatId::SVIM, EXCHG_OUT_ACTION_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },        \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_SWDOC_FREE_AREA_WEB_Link[] =  \
    {                                                                       \
        { SotClipboardFormatId::SONLK, EXCHG_IN_ACTION_LINK, 0 },                   \
        { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_LINK, 0 },        \
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_LINK | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },             \
        { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, FILEGRPDSC_ONLY_URL },\
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::LINK, EXCHG_OUT_ACTION_INSERT_DDE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },             \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };


#define EXCHG_DEST_SCDOC_FREE_AREA_ARRAY                                    \
static SotAction_Impl const aEXCHG_DEST_SCDOC_FREE_AREA_Def[] =   \
    {                                                                       \
        { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_COPY, 0 },                     \
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_COPY, 0 },                          \
        { SotClipboardFormatId::HTML, EXCHG_IN_ACTION_COPY, 0 },                    \
        { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_IN_ACTION_COPY, 0 },         \
        { SotClipboardFormatId::HTML_SIMPLE, EXCHG_IN_ACTION_COPY, 0 },             \
        { SotClipboardFormatId::SVIM, EXCHG_IN_ACTION_COPY, 0 },                    \
        { SotClipboardFormatId::STRING, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_IN_ACTION_COPY, 0 },       \
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_IN_ACTION_COPY, FILEGRPDSC_ONLY_URL },       \
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_IN_ACTION_COPY, 0 },       \
        { SotClipboardFormatId::DRAWING, EXCHG_IN_ACTION_COPY, 0 },                 \
        { SotClipboardFormatId::SVXB, EXCHG_IN_ACTION_COPY, 0 },                    \
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_IN_ACTION_COPY, 0 },                   \
        { SotClipboardFormatId::PNG, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::BITMAP, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::SD_OLE, EXCHG_IN_ACTION_MOVE, 0 },                  \
        { SotClipboardFormatId::EMBED_SOURCE, EXCHG_IN_ACTION_MOVE, 0 },             \
        { SotClipboardFormatId::EMBEDDED_OBJ, EXCHG_IN_ACTION_MOVE, 0 },             \
        { SotClipboardFormatId::EMBED_SOURCE_OLE, EXCHG_IN_ACTION_MOVE, 0 },         \
        { SotClipboardFormatId::EMBEDDED_OBJ_OLE, EXCHG_IN_ACTION_MOVE, 0 },         \
        { SotClipboardFormatId::LINK, EXCHG_IN_ACTION_MOVE, 0 },                     \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_SCDOC_FREE_AREA_Move[] =  \
    {                                                                       \
        { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_MOVE, 0 },\
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_INSERT_DRAWOBJ | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SD_OLE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::EMBED_SOURCE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },     \
        { SotClipboardFormatId::EMBEDDED_OBJ, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },     \
        { SotClipboardFormatId::BIFF_5, EXCHG_IN_ACTION_MOVE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },                  \
        { SotClipboardFormatId::BIFF__5,EXCHG_IN_ACTION_MOVE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },                  \
        { SotClipboardFormatId::HTML, EXCHG_OUT_ACTION_INSERT_HTML | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_OUT_ACTION_INSERT_HTML | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::HTML_SIMPLE, EXCHG_OUT_ACTION_INSERT_HTML | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::STRING, EXCHG_OUT_ACTION_INSERT_STRING | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },              \
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_INSERT_BITMAP | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_INSERT_BITMAP | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::EMBED_SOURCE_OLE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 }, \
        { SotClipboardFormatId::EMBEDDED_OBJ_OLE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 }, \
        { SotClipboardFormatId::LINK, EXCHG_OUT_ACTION_INSERT_DDE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },             \
        { SotClipboardFormatId::SVIM, EXCHG_OUT_ACTION_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },        \
        { SotClipboardFormatId::BIFF_8, EXCHG_IN_ACTION_MOVE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },                  \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_SCDOC_FREE_AREA_Copy[] =  \
    {                                                                       \
        { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_COPY, 0 },\
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_INSERT_DRAWOBJ | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SD_OLE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::EMBED_SOURCE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },     \
        { SotClipboardFormatId::EMBEDDED_OBJ, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },     \
        { SotClipboardFormatId::BIFF_5, EXCHG_IN_ACTION_COPY | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },                  \
        { SotClipboardFormatId::BIFF__5,EXCHG_IN_ACTION_COPY | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },                  \
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_INSERT_BITMAP | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_INSERT_BITMAP | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::HTML, EXCHG_OUT_ACTION_INSERT_HTML | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_OUT_ACTION_INSERT_HTML | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::HTML_SIMPLE, EXCHG_OUT_ACTION_INSERT_HTML | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::STRING, EXCHG_OUT_ACTION_INSERT_STRING | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },              \
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, FILEGRPDSC_ONLY_URL },\
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::EMBED_SOURCE_OLE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 }, \
        { SotClipboardFormatId::EMBEDDED_OBJ_OLE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 }, \
        { SotClipboardFormatId::LINK, EXCHG_OUT_ACTION_INSERT_DDE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },             \
        { SotClipboardFormatId::SVIM, EXCHG_OUT_ACTION_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },        \
        { SotClipboardFormatId::BIFF_8, EXCHG_IN_ACTION_COPY | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },                  \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_SCDOC_FREE_AREA_Link[] =  \
    {                                                                       \
        { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_LINK, 0 },        \
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_LINK | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },             \
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, FILEGRPDSC_ONLY_URL },\
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::LINK, EXCHG_OUT_ACTION_INSERT_DDE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },             \
        { SotClipboardFormatId::EMBED_SOURCE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },     \
        { SotClipboardFormatId::EMBEDDED_OBJ, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },     \
        { SotClipboardFormatId::EMBED_SOURCE_OLE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 }, \
        { SotClipboardFormatId::EMBEDDED_OBJ_OLE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 }, \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };


#define EXCHG_DEST_SDDOC_FREE_AREA_ARRAY                                    \
static SotAction_Impl const aEXCHG_DEST_SDDOC_FREE_AREA_Def[] =   \
    {                                                                       \
        { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_COPY, 0 },                     \
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_COPY, 0 },                          \
        { SotClipboardFormatId::HTML, EXCHG_IN_ACTION_COPY, 0 },                    \
        { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_IN_ACTION_COPY, 0 },         \
        { SotClipboardFormatId::HTML_SIMPLE, EXCHG_IN_ACTION_COPY, 0 },             \
        { SotClipboardFormatId::SVIM, EXCHG_IN_ACTION_COPY, 0 },                    \
        { SotClipboardFormatId::STRING, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_IN_ACTION_COPY, 0 },       \
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_IN_ACTION_COPY, FILEGRPDSC_ONLY_URL },       \
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_IN_ACTION_COPY, 0 },       \
        { SotClipboardFormatId::DRAWING, EXCHG_IN_ACTION_COPY, 0 },                 \
        { SotClipboardFormatId::SVXB, EXCHG_IN_ACTION_COPY, 0 },                    \
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_IN_ACTION_COPY, 0 },                   \
        { SotClipboardFormatId::PNG, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::BITMAP, EXCHG_IN_ACTION_COPY, 0 },                        \
        { SotClipboardFormatId::SD_OLE, EXCHG_IN_ACTION_MOVE, 0 },                  \
        { SotClipboardFormatId::EMBED_SOURCE, EXCHG_IN_ACTION_MOVE, 0 },             \
        { SotClipboardFormatId::EMBEDDED_OBJ, EXCHG_IN_ACTION_MOVE, 0 },             \
        { SotClipboardFormatId::EMBED_SOURCE_OLE, EXCHG_IN_ACTION_MOVE, 0 },         \
        { SotClipboardFormatId::EMBEDDED_OBJ_OLE, EXCHG_IN_ACTION_MOVE, 0 },         \
        { SotClipboardFormatId::LINK, EXCHG_IN_ACTION_MOVE, 0 },                    \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_SDDOC_FREE_AREA_Move[] =  \
    {                                                                       \
        { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_MOVE, 0 },\
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_INSERT_DRAWOBJ | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SD_OLE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::EMBED_SOURCE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },     \
        { SotClipboardFormatId::EMBEDDED_OBJ, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },     \
        { SotClipboardFormatId::HTML, EXCHG_OUT_ACTION_INSERT_HTML | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_OUT_ACTION_INSERT_HTML | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::HTML_SIMPLE, EXCHG_OUT_ACTION_INSERT_HTML | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::STRING, EXCHG_OUT_ACTION_INSERT_STRING | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },              \
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_INSERT_BITMAP | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_INSERT_BITMAP | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::EMBED_SOURCE_OLE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 }, \
        { SotClipboardFormatId::EMBEDDED_OBJ_OLE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 }, \
        { SotClipboardFormatId::LINK, EXCHG_OUT_ACTION_INSERT_DDE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },             \
        { SotClipboardFormatId::SVIM, EXCHG_OUT_ACTION_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },        \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_SDDOC_FREE_AREA_Copy[] =  \
    {                                                                       \
        { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_COPY, 0 },\
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_INSERT_DRAWOBJ | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::SD_OLE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::EMBED_SOURCE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },     \
        { SotClipboardFormatId::EMBEDDED_OBJ, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },     \
        { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_INSERT_BITMAP | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_INSERT_BITMAP | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::HTML, EXCHG_OUT_ACTION_INSERT_HTML | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_OUT_ACTION_INSERT_HTML | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::HTML_SIMPLE, EXCHG_OUT_ACTION_INSERT_HTML | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::STRING, EXCHG_OUT_ACTION_INSERT_STRING | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },              \
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, FILEGRPDSC_ONLY_URL },\
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::EMBED_SOURCE_OLE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 }, \
        { SotClipboardFormatId::EMBEDDED_OBJ_OLE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 }, \
        { SotClipboardFormatId::LINK, EXCHG_OUT_ACTION_INSERT_DDE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },             \
        { SotClipboardFormatId::SVIM, EXCHG_OUT_ACTION_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },        \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };                                                                      \
static SotAction_Impl const aEXCHG_DEST_SDDOC_FREE_AREA_Link[] =  \
    {                                                                       \
        { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_LINK, 0 },        \
        { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_LINK | EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },             \
        { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, FILEGRPDSC_ONLY_URL },\
        { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },\
        { SotClipboardFormatId::LINK, EXCHG_OUT_ACTION_INSERT_DDE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },             \
        { SotClipboardFormatId::EMBED_SOURCE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },     \
        { SotClipboardFormatId::EMBEDDED_OBJ, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 },     \
        { SotClipboardFormatId::EMBED_SOURCE_OLE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 }, \
        { SotClipboardFormatId::EMBEDDED_OBJ_OLE, EXCHG_OUT_ACTION_INSERT_OLE | EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL, 0 }, \
        { static_cast<SotClipboardFormatId>(0xffff), 0, 0 }                                                          \
    };



#define IMPL_DATA_ARRAY_1                                                 \
EXCHG_EMPYT_ARRAY                                                         \
EXCHG_DEST_DOC_OLEOBJ_ARRAY                                               \
EXCHG_DEST_CHARTDOC_OLEOBJ_ARRAY                                          \
EXCHG_DEST_DOC_TEXTFRAME_ARRAY                                            \
EXCHG_DEST_DOC_GRAPHOBJ_ARRAY                                             \
EXCHG_DEST_DOC_LNKD_GRAPHOBJ_ARRAY                                        \
EXCHG_DEST_DOC_TEXTFRAME_WEB_ARRAY

#define IMPL_DATA_ARRAY_2                                                 \
EXCHG_DEST_DOC_GRAPH_W_IMAP_ARRAY                                         \
EXCHG_DEST_DOC_LNKD_GRAPH_W_IMAP_ARRAY                                    \
EXCHG_DEST_DOC_IMAPREGION_ARRAY                                           \
EXCHG_DEST_DOC_DRAWOBJ_ARRAY                                              \
EXCHG_DEST_DOC_URLBUTTON_ARRAY                                            \
EXCHG_DEST_DOC_URLFIELD_ARRAY                                             \
EXCHG_DEST_DOC_GROUPOBJ_ARRAY                                             \
EXCHG_DEST_SWDOC_FREE_AREA_ARRAY                                          \
EXCHG_DEST_SCDOC_FREE_AREA_ARRAY                                          \
EXCHG_DEST_SDDOC_FREE_AREA_ARRAY                                          \
EXCHG_DEST_SWDOC_FREE_AREA_WEB_ARRAY                                      \

#define IMPL_DATA_ARRAY_3                                                 \
static SotDestinationEntry_Impl const aDestinationArray[] =     \
{                                                                         \
    { SotExchangeDest::DOC_OLEOBJ,                                              \
        aEXCHG_DEST_DOC_OLEOBJ_Def,                                       \
        aEXCHG_DEST_DOC_OLEOBJ_Move,                                      \
        aEmptyArr,                                                        \
        aEXCHG_DEST_DOC_OLEOBJ_Link                                       \
    },                                                                    \
    { SotExchangeDest::CHARTDOC_OLEOBJ,                                         \
        aEXCHG_DEST_CHARTDOC_OLEOBJ_Def,                                  \
        aEXCHG_DEST_CHARTDOC_OLEOBJ_Move,                                 \
        aEmptyArr,                                                        \
        aEXCHG_DEST_CHARTDOC_OLEOBJ_Link                                  \
    },                                                                    \
    { SotExchangeDest::DOC_TEXTFRAME,                                           \
        aEXCHG_DEST_DOC_TEXTFRAME_Def,                                    \
        aEXCHG_DEST_DOC_TEXTFRAME_Move,                                   \
        aEXCHG_DEST_DOC_TEXTFRAME_Copy,                                   \
        aEXCHG_DEST_DOC_TEXTFRAME_Link                                    \
    },                                                                    \
    { SotExchangeDest::DOC_GRAPHOBJ,                                            \
        aEXCHG_DEST_DOC_GRAPHOBJ_Def,                                     \
        aEXCHG_DEST_DOC_GRAPHOBJ_Move,                                    \
        aEXCHG_DEST_DOC_GRAPHOBJ_Copy,                                    \
        aEXCHG_DEST_DOC_GRAPHOBJ_Link                                     \
    },                                                                    \
    { SotExchangeDest::DOC_LNKD_GRAPHOBJ,                                       \
        aEXCHG_DEST_DOC_LNKD_GRAPHOBJ_Def,                                \
        aEXCHG_DEST_DOC_LNKD_GRAPHOBJ_Move,                               \
        aEXCHG_DEST_DOC_LNKD_GRAPHOBJ_Copy,                               \
        aEXCHG_DEST_DOC_LNKD_GRAPHOBJ_Link                                \
    },                                                                    \
    { SotExchangeDest::DOC_GRAPH_W_IMAP,                                        \
        aEXCHG_DEST_DOC_GRAPH_W_IMAP_Def,                                 \
        aEXCHG_DEST_DOC_GRAPH_W_IMAP_Move,                                \
        aEXCHG_DEST_DOC_GRAPH_W_IMAP_Copy,                                \
        aEXCHG_DEST_DOC_GRAPH_W_IMAP_Link                                 \
    },                                                                    \
    { SotExchangeDest::DOC_LNKD_GRAPH_W_IMAP,                                   \
        aEXCHG_DEST_DOC_LNKD_GRAPH_W_IMAP_Def,                            \
        aEXCHG_DEST_DOC_LNKD_GRAPH_W_IMAP_Move,                           \
        aEXCHG_DEST_DOC_LNKD_GRAPH_W_IMAP_Copy,                           \
        aEXCHG_DEST_DOC_LNKD_GRAPH_W_IMAP_Link                            \
    },                                                                    \
    { SotExchangeDest::DOC_IMAPREGION,                                          \
        aEXCHG_DEST_DOC_IMAPREGION_Def,                                   \
        aEXCHG_DEST_DOC_IMAPREGION_Copy,                                  \
        aEmptyArr,                                                        \
        aEmptyArr                                                         \
    },                                                                    \
    { SotExchangeDest::DOC_DRAWOBJ,                                             \
        aEXCHG_DEST_DOC_DRAWOBJ_Def,                                      \
        aEXCHG_DEST_DOC_DRAWOBJ_Copy,                                     \
        aEXCHG_DEST_DOC_DRAWOBJ_Move,                                     \
        aEXCHG_DEST_DOC_DRAWOBJ_Link                                      \
    },                                                                    \
    { SotExchangeDest::DOC_URLBUTTON,                                           \
        aEXCHG_DEST_DOC_URLBUTTON_Def,                                    \
        aEXCHG_DEST_DOC_URLBUTTON_Move,                                   \
        aEXCHG_DEST_DOC_URLBUTTON_Copy,                                   \
        aEmptyArr                                                         \
    },                                                                    \
    { SotExchangeDest::DOC_URLFIELD,                                            \
        aEXCHG_DEST_DOC_URLFIELD_Def,                                     \
        aEmptyArr,                                                        \
        aEXCHG_DEST_DOC_URLFIELD_Copy,                                    \
        aEXCHG_DEST_DOC_URLFIELD_Link                                     \
    },                                                                    \
    { SotExchangeDest::DOC_GROUPOBJ,                                            \
        aEXCHG_DEST_DOC_GROUPOBJ_Def,                                     \
        aEXCHG_DEST_DOC_GROUPOBJ_Move,                                    \
        aEXCHG_DEST_DOC_GROUPOBJ_Copy,                                    \
        aEXCHG_DEST_DOC_GROUPOBJ_Link                                     \
    },                                                                    \
    { SotExchangeDest::SWDOC_FREE_AREA,                                         \
        aEXCHG_DEST_SWDOC_FREE_AREA_Def,                                  \
        aEXCHG_DEST_SWDOC_FREE_AREA_Move,                                 \
        aEXCHG_DEST_SWDOC_FREE_AREA_Copy,                                 \
        aEXCHG_DEST_SWDOC_FREE_AREA_Link                                  \
    },                                                                    \
    { SotExchangeDest::SCDOC_FREE_AREA,                                         \
        aEXCHG_DEST_SCDOC_FREE_AREA_Def,                                  \
        aEXCHG_DEST_SCDOC_FREE_AREA_Move,                                 \
        aEXCHG_DEST_SCDOC_FREE_AREA_Copy,                                 \
        aEXCHG_DEST_SCDOC_FREE_AREA_Link                                  \
    },                                                                    \
    { SotExchangeDest::SDDOC_FREE_AREA,                                         \
        aEXCHG_DEST_SDDOC_FREE_AREA_Def,                                  \
        aEXCHG_DEST_SDDOC_FREE_AREA_Move,                                 \
        aEXCHG_DEST_SDDOC_FREE_AREA_Copy,                                 \
        aEXCHG_DEST_SDDOC_FREE_AREA_Link                                  \
    },                                                                    \
    { SotExchangeDest::DOC_TEXTFRAME_WEB,                                       \
        aEXCHG_DEST_DOC_TEXTFRAME_WEB_Def,                                \
        aEXCHG_DEST_DOC_TEXTFRAME_WEB_Move,                               \
        aEXCHG_DEST_DOC_TEXTFRAME_WEB_Copy,                               \
        aEXCHG_DEST_DOC_TEXTFRAME_WEB_Link                                \
    },                                                                    \
    { SotExchangeDest::SWDOC_FREE_AREA_WEB,                                     \
        aEXCHG_DEST_SWDOC_FREE_AREA_WEB_Def,                              \
        aEXCHG_DEST_SWDOC_FREE_AREA_WEB_Move,                             \
        aEXCHG_DEST_SWDOC_FREE_AREA_WEB_Copy,                             \
        aEXCHG_DEST_SWDOC_FREE_AREA_WEB_Link                              \
    },                                                                    \
    {                                                                     \
        static_cast<SotExchangeDest>(0xffff), nullptr, nullptr, nullptr, nullptr                        \
    }                                                                     \
};





// - new style GetExchange methods -


bool IsFormatSupported( const DataFlavorExVector& rDataFlavorExVector, SotClipboardFormatId nId )
{
    DataFlavorExVector::iterator    aIter( ( (DataFlavorExVector&) rDataFlavorExVector ).begin() );
    DataFlavorExVector::iterator    aEnd( ( (DataFlavorExVector&) rDataFlavorExVector ).end() );
    bool bRet = false;

    while( aIter != aEnd )
    {
        if( nId == (*aIter++).mnSotId )
        {
            bRet = true;
            aIter = aEnd;
        }
    }

    return bRet;
}



static bool CheckTransferableContext_Impl( const Reference< XTransferable >* pxTransferable, const SotAction_Impl& rEntry )
{
    DataFlavor  aFlavor;
    bool bRet = true;

    try
    {
        if( pxTransferable && (*pxTransferable).is() &&
            SotExchange::GetFormatDataFlavor( SotClipboardFormatId::FILEGRPDESCRIPTOR, aFlavor ) &&
            (*pxTransferable)->isDataFlavorSupported( aFlavor ) )
        {
#ifdef WNT
            switch( rEntry.nContextCheckId )
            {
                case FILEGRPDSC_ONLY_URL:
                {
                    bRet = false;

                    if( SotExchange::GetFormatDataFlavor( SotClipboardFormatId::FILECONTENT, aFlavor ) &&
                        (*pxTransferable)->isDataFlavorSupported( aFlavor ) &&
                        SotExchange::GetFormatDataFlavor( rEntry.nFormatId, aFlavor ) &&
                        (*pxTransferable)->isDataFlavorSupported( aFlavor ) )
                    {
                        Any aAny( (*pxTransferable)->getTransferData( aFlavor ) );

                        if( aAny.hasValue() )
                        {
                            Sequence< sal_Int8 > aSeq; aAny >>= aSeq;

                            if( aSeq.getLength() )
                            {
                                FILEGROUPDESCRIPTOR* pFDesc = (FILEGROUPDESCRIPTOR*) aSeq.getConstArray();

                                if( pFDesc->cItems )
                                {
                                    OString sDesc( pFDesc->fgd[ 0 ].cFileName );
                                    bRet = 4 < sDesc.getLength() && sDesc.copy(sDesc.getLength()-4).equalsIgnoreAsciiCase(".URL");
                                }
                            }
                        }
                    }
                }
                break;
            }
#else
            (void) rEntry; // avoid warnings
#endif
        }
    }
    catch( const UnsupportedFlavorException& )
    {
    }
    catch( const RuntimeException& )
    {
    }


    return bRet;
}



static sal_uInt16 GetTransferableAction_Impl(
                            const DataFlavorExVector& rDataFlavorExVector,
                             const SotAction_Impl* pArray,
                            SotClipboardFormatId& rFormat,
                            SotClipboardFormatId nOnlyTestFormat,
                            const Reference< XTransferable >* pxTransferable )
{
    try
    {
        if( rDataFlavorExVector.size() )
        {
            const SotAction_Impl*   pArrayStart = pArray;
            SotClipboardFormatId    nId = pArray->nFormatId;

            while( nId != static_cast<SotClipboardFormatId>(0xffff) )
            {
                rFormat = nId;

                if( ( nOnlyTestFormat == SotClipboardFormatId::NONE || nOnlyTestFormat == nId ) &&
                    IsFormatSupported( rDataFlavorExVector, nId ) &&
                    ( !pArray->nContextCheckId || CheckTransferableContext_Impl( pxTransferable, *pArray ) ) )
                {
                    if( pxTransferable && (*pxTransferable).is() && ( SotClipboardFormatId::FILE_LIST == rFormat ) )
                    {
                        if( IsFormatSupported( rDataFlavorExVector, SotClipboardFormatId::SIMPLE_FILE ) )
                        {
                            DataFlavor aFileListFlavor;
                            SotExchange::GetFormatDataFlavor( SotClipboardFormatId::FILE_LIST, aFileListFlavor );
                            Any aAny( (*pxTransferable)->getTransferData( aFileListFlavor ) );

                            if( aAny.hasValue() )
                            {
                                Sequence< sal_Int8 >    aSeq; aAny >>= aSeq;
                                SvMemoryStream          aMemStm( const_cast<sal_Int8 *>(aSeq.getConstArray()), aSeq.getLength(), StreamMode::READ );
                                FileList                aFileList;

                                ReadFileList( aMemStm, aFileList );

                                if( !aMemStm.GetError() && ( aFileList.Count() == 1 ) )
                                {
                                    const SotAction_Impl* pCur = pArrayStart;

                                    while( pCur->nFormatId != static_cast<SotClipboardFormatId>(0xffff) )
                                    {
                                        if( pCur->nFormatId == SotClipboardFormatId::SIMPLE_FILE )
                                        {
                                            rFormat = SotClipboardFormatId::SIMPLE_FILE;
                                            return pCur->nAction;
                                        }
                                        pCur++;
                                    }
                                }
                            }
                        }
                    }
                    return pArray->nAction;
                }
                pArray++;
                nId = pArray->nFormatId;
            }
        }
    }
    catch( const UnsupportedFlavorException& )
    {
    }
    catch( const RuntimeException& )
    {
    }

    return EXCHG_INOUT_ACTION_NONE;
}



sal_uInt16 SotExchange::GetExchangeAction( const DataFlavorExVector& rDataFlavorExVector,
                                       SotExchangeDest nDestination,
                                       sal_uInt16 nSourceOptions,
                                       sal_uInt16 nUserAction,
                                       SotClipboardFormatId& rFormat,
                                       sal_uInt16& rDefaultAction,
                                       SotClipboardFormatId nOnlyTestFormat,
                                       const Reference< XTransferable >* pxTransferable )
{
    // hier wird jetzt die oben definierte Tabelle "implementiert"
    IMPL_DATA_ARRAY_1;
    IMPL_DATA_ARRAY_2;
    IMPL_DATA_ARRAY_3;

    rFormat = SotClipboardFormatId::STRING;

    //Todo: Binaere Suche einbauen
    const SotDestinationEntry_Impl* pEntry = aDestinationArray;
    while( static_cast<SotExchangeDest>(0xffff) != pEntry->nDestination )
    {
        if( pEntry->nDestination == nDestination )
            break;
        ++pEntry;
    }

    if( static_cast<SotExchangeDest>(0xffff) == pEntry->nDestination )
    {
        return EXCHG_INOUT_ACTION_NONE;
    }

    nUserAction &= EXCHG_ACTION_MASK;
    rFormat = SotClipboardFormatId::NONE;

    /* Behandlung der Default-Action nach folgender Vorgehensweise:

       - Das Ziel wird nach der Default-Action gefragt
       - Unterstuetzt die Quelle diese Aktion so wird sie uebernommen
       - Anderenfalls wird aus den von der Quelle zur Verfuegung gestellten
         Aktionen eine ausgewaehlt, die zu einer moeglichst nicht leeren
          Ergebnisaktion fuehrt. Hierbei wird in dieser Reihenfolge
          vorgegangen: Copy -> Link -> Move
    */
    if( nUserAction == EXCHG_IN_ACTION_DEFAULT )
    {
            nUserAction = GetTransferableAction_Impl(
                rDataFlavorExVector, pEntry->aDefaultActions,
                rFormat, nOnlyTestFormat, pxTransferable );
            // Unterstuetzt die Quelle die Aktion?
            if( !(nUserAction & nSourceOptions ))
            {
                // Nein -> Alle Aktionen der Quelle checken
                rDefaultAction = (EXCHG_IN_ACTION_COPY & nSourceOptions);
                if( rDefaultAction )
                {
                    nUserAction = GetTransferableAction_Impl(
                        rDataFlavorExVector, pEntry->aCopyActions,
                        rFormat, nOnlyTestFormat, pxTransferable );
                    if ( nUserAction )
                        return nUserAction;
                }
                rDefaultAction = (EXCHG_IN_ACTION_LINK & nSourceOptions);
                if( rDefaultAction )
                {
                    nUserAction = GetTransferableAction_Impl(
                        rDataFlavorExVector, pEntry->aLinkActions,
                        rFormat, nOnlyTestFormat, pxTransferable );
                    if ( nUserAction )
                        return nUserAction;
                }
                rDefaultAction = (EXCHG_IN_ACTION_MOVE & nSourceOptions);
                if( rDefaultAction )
                {
                    nUserAction = GetTransferableAction_Impl(
                        rDataFlavorExVector, pEntry->aMoveActions,
                        rFormat, nOnlyTestFormat, pxTransferable );
                    if ( nUserAction )
                        return nUserAction;
                }
                rDefaultAction = 0;
                return 0;
            }
            rDefaultAction = nUserAction;
    }
      else
            rDefaultAction = nUserAction;

    switch( nUserAction )
    {
    case EXCHG_IN_ACTION_MOVE:
        nUserAction = GetTransferableAction_Impl(
                            rDataFlavorExVector, pEntry->aMoveActions,
                            rFormat, nOnlyTestFormat, pxTransferable );
        break;

    case EXCHG_IN_ACTION_COPY:
        nUserAction = GetTransferableAction_Impl(
                            rDataFlavorExVector, pEntry->aCopyActions,
                            rFormat, nOnlyTestFormat, pxTransferable );
        break;

    case EXCHG_IN_ACTION_LINK:
        nUserAction = GetTransferableAction_Impl(
                            rDataFlavorExVector, pEntry->aLinkActions,
                            rFormat, nOnlyTestFormat, pxTransferable );
        break;

    default:
        nUserAction = EXCHG_INOUT_ACTION_NONE;
    }
    return nUserAction;
}



sal_uInt16 SotExchange::IsChart(  const SvGlobalName& rName )
{
    sal_uInt16 nRet=0;
//  if ( rName == SvGlobalName( SO3_SCH_CLASSID_8 ) )
//      nRet = SOFFICE_FILEFORMAT_8;
//  else
    if ( rName == SvGlobalName( SO3_SCH_CLASSID_60 ) )
        nRet = SOFFICE_FILEFORMAT_60;
    else if ( rName == SvGlobalName( SO3_SCH_CLASSID_50 ) )
        nRet = SOFFICE_FILEFORMAT_50;
    else if ( rName == SvGlobalName( SO3_SCH_CLASSID_40 ) )
        nRet = SOFFICE_FILEFORMAT_40;
    else if ( rName == SvGlobalName( SO3_SCH_CLASSID_30 ) )
        nRet = SOFFICE_FILEFORMAT_31;

    return nRet;
}

sal_uInt16 SotExchange::IsMath(  const SvGlobalName& rName )
{
    sal_uInt16 nRet=0;
//  if ( rName == SvGlobalName( SO3_SM_CLASSID_8 ) )
//      nRet = SOFFICE_FILEFORMAT_8;
//  else
    if ( rName == SvGlobalName( SO3_SM_CLASSID_60 ) )
        nRet = SOFFICE_FILEFORMAT_60;
    else if ( rName == SvGlobalName( SO3_SM_CLASSID_50 ) )
        nRet = SOFFICE_FILEFORMAT_50;
    else if ( rName == SvGlobalName( SO3_SM_CLASSID_40 ) )
        nRet = SOFFICE_FILEFORMAT_40;
    else if ( rName == SvGlobalName( SO3_SM_CLASSID_30 ) )
        nRet = SOFFICE_FILEFORMAT_31;

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
