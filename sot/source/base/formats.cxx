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

#include <sot/exchange.hxx>
#include <sot/formats.hxx>
#include <sot/filelist.hxx>
#include <sysformats.hxx>
#include <comphelper/classids.hxx>
#include <comphelper/fileformat.h>

#include <tools/globname.hxx>
#include <tools/stream.hxx>
#include <com/sun/star/datatransfer/DataFlavor.hpp>
#include <com/sun/star/datatransfer/UnsupportedFlavorException.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::datatransfer;

// define a context check Id for every formatid
#define FILEGRPDSC_ONLY_URL     1

/*
 *  For each target there is exactly one SotDestinationEntry_Impl in the table.
 *  This entry contains, among other things, a default action for each format
 *  that can be evaluated by the target. The default actions refer to the table
 *  to be evaluated for each format, i.e., they contain only EXCHG_IN_ACTION_MOVE,
 *  EXCHG_IN_ACTION_COPY, or EXCHG_IN_ACTION_LINK. Corresponding to this action,
 *  aMoveActions, aCopyActions, or aLinkActions is then evaluated. The actions
 *  are sorted by priority, i.e., the "more important" is the format, the sooner
 *  it appears in the list.
 */

namespace {

struct SotDestinationEntry_Impl
{
    SotExchangeDest         nDestination;
    const SotAction_Impl*   aDefaultActions;
    const SotAction_Impl*   aMoveActions;
    const SotAction_Impl*   aCopyActions;
    const SotAction_Impl*   aLinkActions;
};

/*
 *  Via this table, the destination, existing data formats and the desired action
 *  are assigned to an action and the data format to be used in it. The table is
 *  sorted by the Exchange destinations (EXCHG_DEST_*). Within the goal entry are
 *  exactly four tables for default, move, copy and link actions. The mapping
 *  between default action (DropEvent::IsDefaultAction()) and the resulting real
 *  action is done via the default table. This table therefore contains only the
 *  EXCHG_IN_ACTION_COPY, EXCHG_IN_ACTION_MOVE, and EXCHG_IN_ACTION_LINK actions
 *  that point to the specific table. The other tables can contain any actions.
 *  Each table is sorted by format priority. Entry zero has the highest priority.
 */

SotAction_Impl const aEmptyArr[] =
{
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};

/* --- DOC_OLEOBJ --- */
SotAction_Impl const aEXCHG_DEST_DOC_OLEOBJ_Def[] =
{
    { SotClipboardFormatId::INET_IMAGE, EXCHG_IN_ACTION_LINK },
    { SotClipboardFormatId::NETSCAPE_IMAGE, EXCHG_IN_ACTION_LINK },
    { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_LINK },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_LINK },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_IN_ACTION_LINK },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_IN_ACTION_LINK, SotExchangeActionFlags::NONE, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_IN_ACTION_LINK },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_DOC_OLEOBJ_Move[] =
{
    { SotClipboardFormatId::SVIM, EXCHG_OUT_ACTION_INSERT_IMAGEMAP },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_DOC_OLEOBJ_Link[] =
{
    { SotClipboardFormatId::INET_IMAGE, EXCHG_OUT_ACTION_INSERT_INTERACTIVE },
    { SotClipboardFormatId::NETSCAPE_IMAGE, EXCHG_OUT_ACTION_INSERT_INTERACTIVE },
    { SotClipboardFormatId::FILE_LIST, EXCHG_OUT_ACTION_INSERT_INTERACTIVE },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_INTERACTIVE },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_INTERACTIVE },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_INTERACTIVE, SotExchangeActionFlags::NONE, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_INTERACTIVE },
    { SotClipboardFormatId::SVIM, EXCHG_OUT_ACTION_INSERT_IMAGEMAP },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};

/* --- CHARTDOC_OLEOBJ --- */
SotAction_Impl const aEXCHG_DEST_CHARTDOC_OLEOBJ_Def[] =
{
    { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_LINK },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_LINK },
    { SotClipboardFormatId::INET_IMAGE, EXCHG_IN_ACTION_LINK },
    { SotClipboardFormatId::NETSCAPE_IMAGE, EXCHG_IN_ACTION_LINK },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_IN_ACTION_LINK },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_IN_ACTION_LINK, SotExchangeActionFlags::NONE, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_IN_ACTION_LINK },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_CHARTDOC_OLEOBJ_Move[] =
{
    { SotClipboardFormatId::SVIM, EXCHG_OUT_ACTION_INSERT_IMAGEMAP },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_CHARTDOC_OLEOBJ_Link[] =
{
    { SotClipboardFormatId::FILE_LIST, EXCHG_OUT_ACTION_INSERT_INTERACTIVE },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_INTERACTIVE },
    { SotClipboardFormatId::INET_IMAGE, EXCHG_OUT_ACTION_INSERT_INTERACTIVE },
    { SotClipboardFormatId::NETSCAPE_IMAGE, EXCHG_OUT_ACTION_INSERT_INTERACTIVE },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_INTERACTIVE },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_INTERACTIVE, SotExchangeActionFlags::NONE, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_INTERACTIVE },
    { SotClipboardFormatId::SVIM, EXCHG_OUT_ACTION_INSERT_IMAGEMAP },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};

/* --- DOC_TEXTFRAME --- */
SotAction_Impl const aEXCHG_DEST_DOC_TEXTFRAME_Def[] =
{
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::DRAWING, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::PNG, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::JPEG, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::BITMAP, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SVXB, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::HTML, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::HTML_SIMPLE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::STRING, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SONLK, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::INET_IMAGE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::NETSCAPE_IMAGE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SOLK, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::NONE, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SD_OLE, EXCHG_IN_ACTION_MOVE },
    { SotClipboardFormatId::EMBED_SOURCE, EXCHG_IN_ACTION_MOVE },
    { SotClipboardFormatId::EMBEDDED_OBJ, EXCHG_IN_ACTION_MOVE },
    { SotClipboardFormatId::EMBED_SOURCE_OLE, EXCHG_IN_ACTION_MOVE },
    { SotClipboardFormatId::EMBEDDED_OBJ_OLE, EXCHG_IN_ACTION_MOVE },
    { SotClipboardFormatId::LINK, EXCHG_IN_ACTION_MOVE },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_DOC_TEXTFRAME_Move[] =
{
    { SotClipboardFormatId::SONLK, EXCHG_IN_ACTION_MOVE },
    { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_MOVE },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_INSERT_DRAWOBJ, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SD_OLE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBED_SOURCE, EXCHG_OUT_ACTION_INSERT_OLE },
    { SotClipboardFormatId::EMBEDDED_OBJ, EXCHG_OUT_ACTION_INSERT_OLE },
    { SotClipboardFormatId::HTML, EXCHG_OUT_ACTION_INSERT_HTML, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_OUT_ACTION_INSERT_HTML, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::HTML_SIMPLE, EXCHG_OUT_ACTION_INSERT_HTML, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::RTF, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::RICHTEXT, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::NETSCAPE_IMAGE, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::STRING, EXCHG_OUT_ACTION_INSERT_STRING, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::JPEG, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBED_SOURCE_OLE, EXCHG_OUT_ACTION_INSERT_OLE },
    { SotClipboardFormatId::EMBEDDED_OBJ_OLE, EXCHG_OUT_ACTION_INSERT_OLE },
    { SotClipboardFormatId::LINK, EXCHG_OUT_ACTION_INSERT_DDE },
    { SotClipboardFormatId::SVIM, EXCHG_OUT_ACTION_INSERT_IMAGEMAP, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_DOC_TEXTFRAME_Copy[] =
{
    { SotClipboardFormatId::SBA_DATAEXCHANGE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SBA_CTRLDATAEXCHANGE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SBA_FIELDDATAEXCHANGE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::XFORMS, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SONLK, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_INSERT_DRAWOBJ, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SD_OLE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBED_SOURCE, EXCHG_OUT_ACTION_INSERT_OLE },
    { SotClipboardFormatId::EMBEDDED_OBJ, EXCHG_OUT_ACTION_INSERT_OLE },
    { SotClipboardFormatId::HTML, EXCHG_OUT_ACTION_INSERT_HTML, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_OUT_ACTION_INSERT_HTML, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::HTML_SIMPLE, EXCHG_OUT_ACTION_INSERT_HTML, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::RTF, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::RICHTEXT, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::NETSCAPE_IMAGE, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::STRING, EXCHG_OUT_ACTION_INSERT_STRING, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBED_SOURCE_OLE, EXCHG_OUT_ACTION_INSERT_OLE },
    { SotClipboardFormatId::EMBEDDED_OBJ_OLE, EXCHG_OUT_ACTION_INSERT_OLE },
    { SotClipboardFormatId::LINK, EXCHG_OUT_ACTION_INSERT_DDE },
    { SotClipboardFormatId::SVIM, EXCHG_OUT_ACTION_INSERT_IMAGEMAP,SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_DOC_TEXTFRAME_Link[] =
{
    { SotClipboardFormatId::SONLK, EXCHG_IN_ACTION_LINK },
    { SotClipboardFormatId::SBA_DATAEXCHANGE, EXCHG_IN_ACTION_LINK },
    { SotClipboardFormatId::SBA_CTRLDATAEXCHANGE, EXCHG_IN_ACTION_LINK },
    { SotClipboardFormatId::SBA_FIELDDATAEXCHANGE, EXCHG_IN_ACTION_LINK },
    { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_LINK, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_LINK, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::LINK, EXCHG_OUT_ACTION_INSERT_DDE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBED_SOURCE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBEDDED_OBJ, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBED_SOURCE_OLE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBEDDED_OBJ_OLE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};

/* --- DOC_TEXTFRAME_WEB --- */
SotAction_Impl const aEXCHG_DEST_DOC_TEXTFRAME_WEB_Def[] =
{
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::PNG, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::JPEG, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::BITMAP, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SVXB, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::HTML, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::HTML_SIMPLE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::STRING, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SONLK, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::INET_IMAGE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::NETSCAPE_IMAGE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SOLK, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::NONE, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::LINK, EXCHG_IN_ACTION_MOVE },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_DOC_TEXTFRAME_WEB_Move[] =
{
    { SotClipboardFormatId::SONLK, EXCHG_IN_ACTION_MOVE },
    { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_MOVE },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::HTML, EXCHG_OUT_ACTION_INSERT_HTML, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_OUT_ACTION_INSERT_HTML, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::HTML_SIMPLE, EXCHG_OUT_ACTION_INSERT_HTML, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::RTF, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::RICHTEXT, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::NETSCAPE_IMAGE, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::STRING, EXCHG_OUT_ACTION_INSERT_STRING, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::JPEG, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::LINK, EXCHG_OUT_ACTION_INSERT_DDE },
    { SotClipboardFormatId::SVIM, EXCHG_OUT_ACTION_INSERT_IMAGEMAP, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_DOC_TEXTFRAME_WEB_Copy[] =
{
    { SotClipboardFormatId::SBA_DATAEXCHANGE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SBA_CTRLDATAEXCHANGE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SBA_FIELDDATAEXCHANGE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::XFORMS, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SONLK, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::HTML, EXCHG_OUT_ACTION_INSERT_HTML, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_OUT_ACTION_INSERT_HTML, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::HTML_SIMPLE, EXCHG_OUT_ACTION_INSERT_HTML, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::RTF, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::RICHTEXT, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::NETSCAPE_IMAGE, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::STRING, EXCHG_OUT_ACTION_INSERT_STRING, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::LINK, EXCHG_OUT_ACTION_INSERT_DDE },
    { SotClipboardFormatId::SVIM, EXCHG_OUT_ACTION_INSERT_IMAGEMAP, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_DOC_TEXTFRAME_WEB_Link[] =
{
    { SotClipboardFormatId::SONLK, EXCHG_IN_ACTION_LINK },
    { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_LINK, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_LINK, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::LINK, EXCHG_OUT_ACTION_INSERT_DDE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};

/* --- DOC_GRAPHOBJ --- */
SotAction_Impl const aEXCHG_DEST_DOC_GRAPHOBJ_Def[] =
{
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::DRAWING, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::PDF, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::PNG, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::JPEG, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::BITMAP, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SVXB, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::HTML, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::HTML_SIMPLE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::STRING, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SOLK, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::NONE, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_IN_ACTION_COPY },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_DOC_GRAPHOBJ_Move[] =
{
    { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_REPLACE_DRAWOBJ, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_REPLACE_SVXB, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_REPLACE_GDIMETAFILE, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::PDF, EXCHG_OUT_ACTION_INSERT_GRAPH, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_REPLACE_BITMAP, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::JPEG, EXCHG_OUT_ACTION_REPLACE_BITMAP, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_REPLACE_BITMAP, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_REPLACE_GRAPH, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_REPLACE_GRAPH, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_REPLACE_GRAPH, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_REPLACE_GRAPH, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_REPLACE_GRAPH, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_DOC_GRAPHOBJ_Copy[] =
{
    { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_INSERT_DRAWOBJ, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::PDF, EXCHG_OUT_ACTION_INSERT_GRAPH, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::JPEG, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_INSERT_GRAPH, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_GRAPH, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_GRAPH, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_GRAPH, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_DOC_GRAPHOBJ_Link[] =
{
    { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::JPEG, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};

/* --- DOC_LNKD_GRAPHOBJ --- */
SotAction_Impl const aEXCHG_DEST_DOC_LNKD_GRAPHOBJ_Def[] =
{
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::DRAWING, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::PNG, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::JPEG, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::BITMAP, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SVXB, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::HTML, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::HTML_SIMPLE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::STRING, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SOLK, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::NONE, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_COPY },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_DOC_LNKD_GRAPHOBJ_Move[] =
{
    { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_REPLACE_DRAWOBJ, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_REPLACE_SVXB, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_REPLACE_GDIMETAFILE, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_REPLACE_BITMAP, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::JPEG, EXCHG_OUT_ACTION_REPLACE_BITMAP, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_REPLACE_BITMAP, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_REPLACE_GRAPH, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_REPLACE_GRAPH, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_REPLACE_GRAPH, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_REPLACE_GRAPH, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_REPLACE_GRAPH, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_DOC_LNKD_GRAPHOBJ_Copy[] =
{
    { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_INSERT_DRAWOBJ, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::JPEG, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_INSERT_GRAPH, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_GRAPH, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_GRAPH, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_GRAPH, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_DOC_LNKD_GRAPHOBJ_Link[] =
{
    { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::JPEG, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};

/* --- DOC_GRAPH_W_IMAP --- */
SotAction_Impl const aEXCHG_DEST_DOC_GRAPH_W_IMAP_Def[] =
{
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::DRAWING, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::PNG, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::JPEG, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::BITMAP, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SVXB, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::HTML, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::HTML_SIMPLE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::STRING, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SOLK, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::NONE, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_COPY },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_DOC_GRAPH_W_IMAP_Move[] =
{
    { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_REPLACE_DRAWOBJ, SotExchangeActionFlags::ReplaceImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_REPLACE_SVXB, SotExchangeActionFlags::ReplaceImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_REPLACE_GDIMETAFILE, SotExchangeActionFlags::ReplaceImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_REPLACE_BITMAP, SotExchangeActionFlags::ReplaceImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::JPEG, EXCHG_OUT_ACTION_REPLACE_BITMAP, SotExchangeActionFlags::ReplaceImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_REPLACE_BITMAP, SotExchangeActionFlags::ReplaceImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_REPLACE_GRAPH, SotExchangeActionFlags::ReplaceImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_REPLACE_GRAPH, SotExchangeActionFlags::ReplaceImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_REPLACE_GRAPH, SotExchangeActionFlags::ReplaceImageMap  | SotExchangeActionFlags::InsertTargetUrl, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_REPLACE_GRAPH, SotExchangeActionFlags::ReplaceImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_REPLACE_GRAPH, SotExchangeActionFlags::ReplaceImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_DOC_GRAPH_W_IMAP_Copy[] =
{
    { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_INSERT_DRAWOBJ, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::JPEG, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_INSERT_GRAPH, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_GRAPH, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_GRAPH, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_GRAPH, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_DOC_GRAPH_W_IMAP_Link[] =
{
    { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::JPEG, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};

/* --- DOC_LNKD_GRAPH_W_IMAP --- */
SotAction_Impl const aEXCHG_DEST_DOC_LNKD_GRAPH_W_IMAP_Def[] =
{
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::DRAWING, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::PNG, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::JPEG, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::BITMAP, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SVXB, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::HTML, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::HTML_SIMPLE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::STRING, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SOLK, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::NONE, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_COPY },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_DOC_LNKD_GRAPH_W_IMAP_Move[] =
{
    { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_REPLACE_DRAWOBJ, SotExchangeActionFlags::ReplaceImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_REPLACE_SVXB, SotExchangeActionFlags::ReplaceImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_REPLACE_GDIMETAFILE, SotExchangeActionFlags::ReplaceImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_REPLACE_BITMAP, SotExchangeActionFlags::ReplaceImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::JPEG, EXCHG_OUT_ACTION_REPLACE_BITMAP, SotExchangeActionFlags::ReplaceImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_REPLACE_BITMAP, SotExchangeActionFlags::ReplaceImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_REPLACE_GRAPH, SotExchangeActionFlags::ReplaceImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_REPLACE_GRAPH, SotExchangeActionFlags::ReplaceImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_REPLACE_GRAPH, SotExchangeActionFlags::ReplaceImageMap  | SotExchangeActionFlags::InsertTargetUrl, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_REPLACE_GRAPH, SotExchangeActionFlags::ReplaceImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_REPLACE_GRAPH, SotExchangeActionFlags::ReplaceImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_DOC_LNKD_GRAPH_W_IMAP_Copy[] =
{
    { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_INSERT_DRAWOBJ, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::JPEG, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_INSERT_GRAPH, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_GRAPH, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_GRAPH, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_GRAPH, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_DOC_LNKD_GRAPH_W_IMAP_Link[] =
{
    { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::JPEG, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};

/* --- DOC_IMAPREGION --- */
SotAction_Impl const aEXCHG_DEST_DOC_IMAPREGION_Def[] =
{
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::NONE, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_COPY },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_DOC_IMAPREGION_Copy[] =
{
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertTargetUrl, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};

/* --- DOC_DRAWOBJ --- */
SotAction_Impl const aEXCHG_DEST_DOC_DRAWOBJ_Def[] =
{
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::DRAWING, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::PNG, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::JPEG, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::BITMAP, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SVXB, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::HTML, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::HTML_SIMPLE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::STRING, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SOLK, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::NONE, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_IN_ACTION_COPY },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_DOC_DRAWOBJ_Copy[] =
{
    { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_REPLACE_DRAWOBJ, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_REPLACE_SVXB, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_REPLACE_GDIMETAFILE, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_REPLACE_BITMAP, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::JPEG, EXCHG_OUT_ACTION_REPLACE_BITMAP, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_REPLACE_BITMAP, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_REPLACE_GRAPH, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_REPLACE_GRAPH, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_REPLACE_GRAPH, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_REPLACE_GRAPH, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_REPLACE_GRAPH, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_DOC_DRAWOBJ_Move[] =
{
    { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_INSERT_DRAWOBJ, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::JPEG, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_INSERT_GRAPH, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_GRAPH, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_GRAPH, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_GRAPH, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_DOC_DRAWOBJ_Link[] =
{
    { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::JPEG, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};

/* --- DOC_URLBUTTON --- */
SotAction_Impl const aEXCHG_DEST_DOC_URLBUTTON_Def[] =
{
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::DRAWING, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::PNG, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::JPEG, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::BITMAP, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SVXB, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SOLK, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::NONE, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_IN_ACTION_COPY },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_DOC_URLBUTTON_Move[] =
{
    { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_REPLACE_DRAWOBJ, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_REPLACE_SVXB, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_REPLACE_GDIMETAFILE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_DOC_URLBUTTON_Copy[] =
{
    { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_INSERT_DRAWOBJ, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::JPEG, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertTargetUrl, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};

/* --- DOC_URLFIELD --- */
SotAction_Impl const aEXCHG_DEST_DOC_URLFIELD_Def[] =
{
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::NONE, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_IN_ACTION_COPY },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_DOC_URLFIELD_Copy[] =
{
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertTargetUrl, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_DOC_URLFIELD_Link[] =
{
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_HYPERLINK },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};

/* --- DOC_GROUPOBJ --- */
SotAction_Impl const aEXCHG_DEST_DOC_GROUPOBJ_Def[] =
{
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::DRAWING, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::PNG, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::JPEG, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::BITMAP, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SVXB, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::HTML, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::HTML_SIMPLE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::STRING, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SOLK, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::NONE, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_IN_ACTION_COPY },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_DOC_GROUPOBJ_Move[] =
{
    { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_REPLACE_DRAWOBJ, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_REPLACE_SVXB, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_REPLACE_GDIMETAFILE, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_REPLACE_BITMAP, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::JPEG, EXCHG_OUT_ACTION_REPLACE_BITMAP, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_REPLACE_BITMAP, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_REPLACE_GRAPH, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_REPLACE_GRAPH, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_REPLACE_GRAPH, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_REPLACE_GRAPH, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_REPLACE_GRAPH, SotExchangeActionFlags::InsertImageMap  | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_DOC_GROUPOBJ_Copy[] =
{
    { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_INSERT_DRAWOBJ, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::JPEG, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_INSERT_GRAPH, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_GRAPH, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_GRAPH, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_GRAPH, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_DOC_GROUPOBJ_Link[] =
{
    { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::JPEG, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_GET_ATTRIBUTES, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};

/* --- SWDOC_FREE_AREA --- */
SotAction_Impl const aEXCHG_DEST_SWDOC_FREE_AREA_Def[] =
{
    { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::HTML, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::HTML_SIMPLE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::RTF, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::RICHTEXT, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SVIM, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::NETSCAPE_IMAGE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::STRING, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::NONE, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::DRAWING, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SVXB, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::PDF, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::PNG, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::JPEG, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::BITMAP, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SONLK, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SOLK, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SD_OLE, EXCHG_IN_ACTION_MOVE },
    { SotClipboardFormatId::EMBED_SOURCE, EXCHG_IN_ACTION_MOVE },
    { SotClipboardFormatId::EMBEDDED_OBJ, EXCHG_IN_ACTION_MOVE },
    { SotClipboardFormatId::EMBED_SOURCE_OLE, EXCHG_IN_ACTION_MOVE },
    { SotClipboardFormatId::EMBEDDED_OBJ_OLE, EXCHG_IN_ACTION_MOVE },
    { SotClipboardFormatId::LINK, EXCHG_IN_ACTION_MOVE },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_SWDOC_FREE_AREA_Move[] =
{
    { SotClipboardFormatId::SONLK, EXCHG_IN_ACTION_MOVE },
    { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_MOVE },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_INSERT_DRAWOBJ, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SD_OLE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBED_SOURCE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBEDDED_OBJ, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::HTML, EXCHG_OUT_ACTION_INSERT_HTML, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_OUT_ACTION_INSERT_HTML, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::HTML_SIMPLE, EXCHG_OUT_ACTION_INSERT_HTML, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::RTF, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::RICHTEXT, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::STRING, EXCHG_OUT_ACTION_INSERT_STRING },
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::PDF, EXCHG_OUT_ACTION_INSERT_GRAPH, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::JPEG, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBED_SOURCE_OLE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBEDDED_OBJ_OLE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::LINK, EXCHG_OUT_ACTION_INSERT_DDE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVIM, EXCHG_OUT_ACTION_INSERT_IMAGEMAP, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_SWDOC_FREE_AREA_Copy[] =
{
    { SotClipboardFormatId::SBA_DATAEXCHANGE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SBA_CTRLDATAEXCHANGE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SBA_FIELDDATAEXCHANGE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::XFORMS, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SONLK, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_INSERT_DRAWOBJ, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SD_OLE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::RICHTEXT, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBED_SOURCE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBEDDED_OBJ, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
#ifndef MACOSX
    { SotClipboardFormatId::RTF, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::InsertTargetUrl, 0 },
#endif
    { SotClipboardFormatId::PDF, EXCHG_OUT_ACTION_INSERT_GRAPH, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::JPEG, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::HTML, EXCHG_OUT_ACTION_INSERT_HTML, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_OUT_ACTION_INSERT_HTML, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::HTML_SIMPLE, EXCHG_OUT_ACTION_INSERT_HTML, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
#ifdef MACOSX
    { SotClipboardFormatId::RTF, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::InsertTargetUrl, 0 },
#endif
    { SotClipboardFormatId::NETSCAPE_IMAGE, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::STRING, EXCHG_OUT_ACTION_INSERT_STRING, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBED_SOURCE_OLE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBEDDED_OBJ_OLE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::LINK, EXCHG_OUT_ACTION_INSERT_DDE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVIM, EXCHG_OUT_ACTION_INSERT_IMAGEMAP, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_SWDOC_FREE_AREA_Link[] =
{
    { SotClipboardFormatId::SONLK, EXCHG_IN_ACTION_LINK },
    { SotClipboardFormatId::SBA_DATAEXCHANGE, EXCHG_IN_ACTION_LINK },
    { SotClipboardFormatId::SBA_CTRLDATAEXCHANGE, EXCHG_IN_ACTION_LINK },
    { SotClipboardFormatId::SBA_FIELDDATAEXCHANGE, EXCHG_IN_ACTION_LINK },
    { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_LINK },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_LINK, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertTargetUrl, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::LINK, EXCHG_OUT_ACTION_INSERT_DDE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBED_SOURCE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBEDDED_OBJ, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBED_SOURCE_OLE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBEDDED_OBJ_OLE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};

/* --- SWDOC_FREE_AREA_WEB --- */
SotAction_Impl const aEXCHG_DEST_SWDOC_FREE_AREA_WEB_Def[] =
{
    { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SOLK, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::HTML, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::HTML_SIMPLE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::RTF, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SVIM, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::NETSCAPE_IMAGE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::STRING, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::NONE, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SVXB, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::PNG, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::JPEG, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::BITMAP, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SONLK, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::LINK, EXCHG_IN_ACTION_MOVE },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_SWDOC_FREE_AREA_WEB_Move[] =
{
    { SotClipboardFormatId::SONLK, EXCHG_IN_ACTION_MOVE },
    { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_MOVE },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::HTML, EXCHG_OUT_ACTION_INSERT_HTML, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_OUT_ACTION_INSERT_HTML, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::HTML_SIMPLE, EXCHG_OUT_ACTION_INSERT_HTML, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::RTF, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::STRING, EXCHG_OUT_ACTION_INSERT_STRING },
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::JPEG, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::LINK, EXCHG_OUT_ACTION_INSERT_DDE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVIM, EXCHG_OUT_ACTION_INSERT_IMAGEMAP, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_SWDOC_FREE_AREA_WEB_Copy[] =
{
    { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SONLK, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::HTML, EXCHG_OUT_ACTION_INSERT_HTML, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_OUT_ACTION_INSERT_HTML, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::HTML_SIMPLE, EXCHG_OUT_ACTION_INSERT_HTML, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::RTF, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::NETSCAPE_IMAGE, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::STRING, EXCHG_OUT_ACTION_INSERT_STRING, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::JPEG, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::LINK, EXCHG_OUT_ACTION_INSERT_DDE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVIM, EXCHG_OUT_ACTION_INSERT_IMAGEMAP, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_SWDOC_FREE_AREA_WEB_Link[] =
{
    { SotClipboardFormatId::SONLK, EXCHG_IN_ACTION_LINK },
    { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_LINK },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_LINK, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SOLK, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertTargetUrl, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::LINK, EXCHG_OUT_ACTION_INSERT_DDE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};

/* --- SCDOC_FREE_AREA --- */
SotAction_Impl const aEXCHG_DEST_SCDOC_FREE_AREA_Def[] =
{
    { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::HTML, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::HTML_SIMPLE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SVIM, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::STRING, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::NONE, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::DRAWING, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SVXB, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::PNG, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::JPEG, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::BITMAP, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SD_OLE, EXCHG_IN_ACTION_MOVE },
    { SotClipboardFormatId::EMBED_SOURCE, EXCHG_IN_ACTION_MOVE },
    { SotClipboardFormatId::EMBEDDED_OBJ, EXCHG_IN_ACTION_MOVE },
    { SotClipboardFormatId::EMBED_SOURCE_OLE, EXCHG_IN_ACTION_MOVE },
    { SotClipboardFormatId::EMBEDDED_OBJ_OLE, EXCHG_IN_ACTION_MOVE },
    { SotClipboardFormatId::LINK, EXCHG_IN_ACTION_MOVE },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_SCDOC_FREE_AREA_Move[] =
{
    { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_MOVE },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_INSERT_DRAWOBJ, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SD_OLE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBED_SOURCE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBEDDED_OBJ, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::BIFF_5, EXCHG_IN_ACTION_MOVE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::BIFF__5,EXCHG_IN_ACTION_MOVE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::HTML, EXCHG_OUT_ACTION_INSERT_HTML, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_OUT_ACTION_INSERT_HTML, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::HTML_SIMPLE, EXCHG_OUT_ACTION_INSERT_HTML, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::STRING, EXCHG_OUT_ACTION_INSERT_STRING, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE,SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::JPEG, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBED_SOURCE_OLE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBEDDED_OBJ_OLE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::LINK, EXCHG_OUT_ACTION_INSERT_DDE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVIM, EXCHG_OUT_ACTION_INSERT_IMAGEMAP, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::BIFF_8, EXCHG_IN_ACTION_MOVE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_SCDOC_FREE_AREA_Copy[] =
{
    { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_INSERT_DRAWOBJ, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SD_OLE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBED_SOURCE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBEDDED_OBJ, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::BIFF_5, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::BIFF__5,EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::JPEG, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::HTML, EXCHG_OUT_ACTION_INSERT_HTML, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_OUT_ACTION_INSERT_HTML, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::HTML_SIMPLE, EXCHG_OUT_ACTION_INSERT_HTML, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::STRING, EXCHG_OUT_ACTION_INSERT_STRING, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBED_SOURCE_OLE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBEDDED_OBJ_OLE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::LINK, EXCHG_OUT_ACTION_INSERT_DDE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVIM, EXCHG_OUT_ACTION_INSERT_IMAGEMAP, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::BIFF_8, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_SCDOC_FREE_AREA_Link[] =
{
    { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_LINK },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_LINK, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertTargetUrl, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::LINK, EXCHG_OUT_ACTION_INSERT_DDE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBED_SOURCE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBEDDED_OBJ, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBED_SOURCE_OLE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBEDDED_OBJ_OLE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};

/* --- SDDOC_FREE_AREA --- */
SotAction_Impl const aEXCHG_DEST_SDDOC_FREE_AREA_Def[] =
{
    { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::HTML, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::HTML_SIMPLE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SVIM, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::STRING, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_IN_ACTION_COPY, SotExchangeActionFlags::NONE, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::DRAWING, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SVXB, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::PNG, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::JPEG, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::BITMAP, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SD_OLE, EXCHG_IN_ACTION_MOVE },
    { SotClipboardFormatId::EMBED_SOURCE, EXCHG_IN_ACTION_MOVE },
    { SotClipboardFormatId::EMBEDDED_OBJ, EXCHG_IN_ACTION_MOVE },
    { SotClipboardFormatId::EMBED_SOURCE_OLE, EXCHG_IN_ACTION_MOVE },
    { SotClipboardFormatId::EMBEDDED_OBJ_OLE, EXCHG_IN_ACTION_MOVE },
    { SotClipboardFormatId::LINK, EXCHG_IN_ACTION_MOVE },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_SDDOC_FREE_AREA_Move[] =
{
    { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_MOVE },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_INSERT_DRAWOBJ, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SD_OLE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBED_SOURCE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBEDDED_OBJ, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::HTML, EXCHG_OUT_ACTION_INSERT_HTML, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_OUT_ACTION_INSERT_HTML, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::HTML_SIMPLE, EXCHG_OUT_ACTION_INSERT_HTML, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::STRING, EXCHG_OUT_ACTION_INSERT_STRING, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::JPEG, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBED_SOURCE_OLE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBEDDED_OBJ_OLE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::LINK, EXCHG_OUT_ACTION_INSERT_DDE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVIM, EXCHG_OUT_ACTION_INSERT_IMAGEMAP, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_SDDOC_FREE_AREA_Copy[] =
{
    { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_COPY },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_OUT_ACTION_INSERT_FILE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::DRAWING, EXCHG_OUT_ACTION_INSERT_DRAWOBJ, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVXB, EXCHG_OUT_ACTION_INSERT_SVXB, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SD_OLE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBED_SOURCE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBEDDED_OBJ, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::GDIMETAFILE, EXCHG_OUT_ACTION_INSERT_GDIMETAFILE, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::PNG, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::JPEG, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::BITMAP, EXCHG_OUT_ACTION_INSERT_BITMAP, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::HTML, EXCHG_OUT_ACTION_INSERT_HTML, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::HTML_NO_COMMENT, EXCHG_OUT_ACTION_INSERT_HTML, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::HTML_SIMPLE, EXCHG_OUT_ACTION_INSERT_HTML, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::STRING, EXCHG_OUT_ACTION_INSERT_STRING, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBED_SOURCE_OLE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBEDDED_OBJ_OLE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::LINK, EXCHG_OUT_ACTION_INSERT_DDE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::SVIM, EXCHG_OUT_ACTION_INSERT_IMAGEMAP, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};
SotAction_Impl const aEXCHG_DEST_SDDOC_FREE_AREA_Link[] =
{
    { SotClipboardFormatId::FILE_LIST, EXCHG_IN_ACTION_LINK },
    { SotClipboardFormatId::SIMPLE_FILE, EXCHG_IN_ACTION_LINK, SotExchangeActionFlags::InsertImageMap | SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::NETSCAPE_BOOKMARK, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::FILEGRPDESCRIPTOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertTargetUrl, FILEGRPDSC_ONLY_URL },
    { SotClipboardFormatId::UNIFORMRESOURCELOCATOR, EXCHG_OUT_ACTION_INSERT_HYPERLINK, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::LINK, EXCHG_OUT_ACTION_INSERT_DDE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBED_SOURCE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBEDDED_OBJ, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBED_SOURCE_OLE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { SotClipboardFormatId::EMBEDDED_OBJ_OLE, EXCHG_OUT_ACTION_INSERT_OLE, SotExchangeActionFlags::InsertTargetUrl, 0 },
    { static_cast<SotClipboardFormatId>(0xffff), 0 }
};

/* --- exchange destinations --- */
SotDestinationEntry_Impl const aDestinationArray[] =
{
    { SotExchangeDest::DOC_OLEOBJ,
        aEXCHG_DEST_DOC_OLEOBJ_Def,
        aEXCHG_DEST_DOC_OLEOBJ_Move,
        aEmptyArr,
        aEXCHG_DEST_DOC_OLEOBJ_Link
    },
    { SotExchangeDest::CHARTDOC_OLEOBJ,
        aEXCHG_DEST_CHARTDOC_OLEOBJ_Def,
        aEXCHG_DEST_CHARTDOC_OLEOBJ_Move,
        aEmptyArr,
        aEXCHG_DEST_CHARTDOC_OLEOBJ_Link
    },
    { SotExchangeDest::DOC_TEXTFRAME,
        aEXCHG_DEST_DOC_TEXTFRAME_Def,
        aEXCHG_DEST_DOC_TEXTFRAME_Move,
        aEXCHG_DEST_DOC_TEXTFRAME_Copy,
        aEXCHG_DEST_DOC_TEXTFRAME_Link
    },
    { SotExchangeDest::DOC_GRAPHOBJ,
        aEXCHG_DEST_DOC_GRAPHOBJ_Def,
        aEXCHG_DEST_DOC_GRAPHOBJ_Move,
        aEXCHG_DEST_DOC_GRAPHOBJ_Copy,
        aEXCHG_DEST_DOC_GRAPHOBJ_Link
    },
    { SotExchangeDest::DOC_LNKD_GRAPHOBJ,
        aEXCHG_DEST_DOC_LNKD_GRAPHOBJ_Def,
        aEXCHG_DEST_DOC_LNKD_GRAPHOBJ_Move,
        aEXCHG_DEST_DOC_LNKD_GRAPHOBJ_Copy,
        aEXCHG_DEST_DOC_LNKD_GRAPHOBJ_Link
    },
    { SotExchangeDest::DOC_GRAPH_W_IMAP,
        aEXCHG_DEST_DOC_GRAPH_W_IMAP_Def,
        aEXCHG_DEST_DOC_GRAPH_W_IMAP_Move,
        aEXCHG_DEST_DOC_GRAPH_W_IMAP_Copy,
        aEXCHG_DEST_DOC_GRAPH_W_IMAP_Link
    },
    { SotExchangeDest::DOC_LNKD_GRAPH_W_IMAP,
        aEXCHG_DEST_DOC_LNKD_GRAPH_W_IMAP_Def,
        aEXCHG_DEST_DOC_LNKD_GRAPH_W_IMAP_Move,
        aEXCHG_DEST_DOC_LNKD_GRAPH_W_IMAP_Copy,
        aEXCHG_DEST_DOC_LNKD_GRAPH_W_IMAP_Link
    },
    { SotExchangeDest::DOC_IMAPREGION,
        aEXCHG_DEST_DOC_IMAPREGION_Def,
        aEXCHG_DEST_DOC_IMAPREGION_Copy,
        aEmptyArr,
        aEmptyArr
    },
    { SotExchangeDest::DOC_DRAWOBJ,
        aEXCHG_DEST_DOC_DRAWOBJ_Def,
        aEXCHG_DEST_DOC_DRAWOBJ_Copy,
        aEXCHG_DEST_DOC_DRAWOBJ_Move,
        aEXCHG_DEST_DOC_DRAWOBJ_Link
    },
    { SotExchangeDest::DOC_URLBUTTON,
        aEXCHG_DEST_DOC_URLBUTTON_Def,
        aEXCHG_DEST_DOC_URLBUTTON_Move,
        aEXCHG_DEST_DOC_URLBUTTON_Copy,
        aEmptyArr
    },
    { SotExchangeDest::DOC_URLFIELD,
        aEXCHG_DEST_DOC_URLFIELD_Def,
        aEmptyArr,
        aEXCHG_DEST_DOC_URLFIELD_Copy,
        aEXCHG_DEST_DOC_URLFIELD_Link
    },
    { SotExchangeDest::DOC_GROUPOBJ,
        aEXCHG_DEST_DOC_GROUPOBJ_Def,
        aEXCHG_DEST_DOC_GROUPOBJ_Move,
        aEXCHG_DEST_DOC_GROUPOBJ_Copy,
        aEXCHG_DEST_DOC_GROUPOBJ_Link
    },
    { SotExchangeDest::SWDOC_FREE_AREA,
        aEXCHG_DEST_SWDOC_FREE_AREA_Def,
        aEXCHG_DEST_SWDOC_FREE_AREA_Move,
        aEXCHG_DEST_SWDOC_FREE_AREA_Copy,
        aEXCHG_DEST_SWDOC_FREE_AREA_Link
    },
    { SotExchangeDest::SCDOC_FREE_AREA,
        aEXCHG_DEST_SCDOC_FREE_AREA_Def,
        aEXCHG_DEST_SCDOC_FREE_AREA_Move,
        aEXCHG_DEST_SCDOC_FREE_AREA_Copy,
        aEXCHG_DEST_SCDOC_FREE_AREA_Link
    },
    { SotExchangeDest::SDDOC_FREE_AREA,
        aEXCHG_DEST_SDDOC_FREE_AREA_Def,
        aEXCHG_DEST_SDDOC_FREE_AREA_Move,
        aEXCHG_DEST_SDDOC_FREE_AREA_Copy,
        aEXCHG_DEST_SDDOC_FREE_AREA_Link
    },
    { SotExchangeDest::DOC_TEXTFRAME_WEB,
        aEXCHG_DEST_DOC_TEXTFRAME_WEB_Def,
        aEXCHG_DEST_DOC_TEXTFRAME_WEB_Move,
        aEXCHG_DEST_DOC_TEXTFRAME_WEB_Copy,
        aEXCHG_DEST_DOC_TEXTFRAME_WEB_Link
    },
    { SotExchangeDest::SWDOC_FREE_AREA_WEB,
        aEXCHG_DEST_SWDOC_FREE_AREA_WEB_Def,
        aEXCHG_DEST_SWDOC_FREE_AREA_WEB_Move,
        aEXCHG_DEST_SWDOC_FREE_AREA_WEB_Copy,
        aEXCHG_DEST_SWDOC_FREE_AREA_WEB_Link
    },
    {
        static_cast<SotExchangeDest>(0xffff), nullptr, nullptr, nullptr, nullptr
    }
};

} // namespace

namespace sot
{
const SotAction_Impl* GetExchangeDestinationWriterFreeAreaCopy()
{
    return aEXCHG_DEST_SWDOC_FREE_AREA_Copy;
}
}

// - new style GetExchange methods -

bool IsFormatSupported( const DataFlavorExVector& rDataFlavorExVector, SotClipboardFormatId nId )
{
    return std::any_of(rDataFlavorExVector.begin(), rDataFlavorExVector.end(),
        [nId](const DataFlavorEx& rDataFlavorEx) { return nId == rDataFlavorEx.mnSotId; });
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
#ifdef _WIN32
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
                                FILEGROUPDESCRIPTOR const * pFDesc = reinterpret_cast<FILEGROUPDESCRIPTOR const *>(aSeq.getConstArray());

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
                            const Reference< XTransferable >* pxTransferable,
                            SotExchangeActionFlags* pActionFlags )
{
    try
    {
        if( !rDataFlavorExVector.empty() )
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
                                            if (pActionFlags)
                                                *pActionFlags = pCur->nFlags;
                                            return pCur->nAction;
                                        }
                                        pCur++;
                                    }
                                }
                            }
                        }
                    }
                    if (pActionFlags)
                        *pActionFlags = pArray->nFlags;
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


sal_uInt8 SotExchange::GetExchangeAction( const DataFlavorExVector& rDataFlavorExVector,
                                       SotExchangeDest nDestination,
                                       sal_uInt16 nSourceOptions,
                                       sal_uInt8 nUserAction,
                                       SotClipboardFormatId& rFormat,
                                       sal_uInt8& rDefaultAction,
                                       SotClipboardFormatId nOnlyTestFormat,
                                       const Reference< XTransferable >* pxTransferable,
                                       SotExchangeActionFlags* pActionFlags )
{
    rFormat = SotClipboardFormatId::STRING;

    //Todo: incorporate a binary search
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

    rFormat = SotClipboardFormatId::NONE;

    /* Handling the default action using the following procedure:
     *
     * - The target is asked for the default action
     * - If the source supports this action, it is taken over
     * - Otherwise, from the actions made available by the source, one leading
     *   to a most likely non-empty result action is selected. This is done in
     *   the following order: Copy -> Link -> Move
     */
    if( nUserAction == EXCHG_IN_ACTION_DEFAULT )
    {
            nUserAction = GetTransferableAction_Impl(
                rDataFlavorExVector, pEntry->aDefaultActions,
                rFormat, nOnlyTestFormat, pxTransferable, pActionFlags );
            // Does the source support the action?
            if( !(nUserAction & nSourceOptions ))
            {
                // No -> Check all actions of the source
                rDefaultAction = (EXCHG_IN_ACTION_COPY & nSourceOptions);
                if( rDefaultAction )
                {
                    nUserAction = GetTransferableAction_Impl(
                        rDataFlavorExVector, pEntry->aCopyActions,
                        rFormat, nOnlyTestFormat, pxTransferable, pActionFlags );
                    if ( nUserAction )
                        return nUserAction;
                }
                rDefaultAction = (EXCHG_IN_ACTION_LINK & nSourceOptions);
                if( rDefaultAction )
                {
                    nUserAction = GetTransferableAction_Impl(
                        rDataFlavorExVector, pEntry->aLinkActions,
                        rFormat, nOnlyTestFormat, pxTransferable, pActionFlags );
                    if ( nUserAction )
                        return nUserAction;
                }
                rDefaultAction = (EXCHG_IN_ACTION_MOVE & nSourceOptions);
                if( rDefaultAction )
                {
                    nUserAction = GetTransferableAction_Impl(
                        rDataFlavorExVector, pEntry->aMoveActions,
                        rFormat, nOnlyTestFormat, pxTransferable, pActionFlags );
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
                            rFormat, nOnlyTestFormat, pxTransferable, pActionFlags );
        break;

    case EXCHG_IN_ACTION_COPY:
        nUserAction = GetTransferableAction_Impl(
                            rDataFlavorExVector, pEntry->aCopyActions,
                            rFormat, nOnlyTestFormat, pxTransferable, pActionFlags );
        break;

    case EXCHG_IN_ACTION_LINK:
        nUserAction = GetTransferableAction_Impl(
                            rDataFlavorExVector, pEntry->aLinkActions,
                            rFormat, nOnlyTestFormat, pxTransferable, pActionFlags );
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
