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

#pragma once
#if 1

#include <tools/string.hxx>

class SwDocShell;
class TransferDataContainer;
class TransferableDataHelper;

/*
    navigator bookmark for distinct identification in Sw
*/

class NaviContentBookmark
{
    String          aUrl;       // URL including jump mark
    String          aDescr;     // description
    long            nDocSh;     // address of DocShell
    sal_uInt16          nDefDrag;   // description contains defaultDragType

public:
    NaviContentBookmark();
    NaviContentBookmark( const String &rUrl, const String& rDesc,
                            sal_uInt16 nDragType, const SwDocShell* );

    const String&   GetURL() const              { return aUrl; }
    const String&   GetDescription() const      { return aDescr; }
    sal_uInt16          GetDefaultDragType() const  { return nDefDrag; }
    long            GetDocShell() const         { return nDocSh; }
    void            Copy( TransferDataContainer& rData ) const;
    sal_Bool            Paste( TransferableDataHelper& rData );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
