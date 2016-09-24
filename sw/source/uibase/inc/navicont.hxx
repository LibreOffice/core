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

#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_NAVICONT_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_NAVICONT_HXX

#include <rtl/ustring.hxx>

class SwDocShell;
class TransferDataContainer;
class TransferableDataHelper;
enum class RegionMode;

/*
    navigator bookmark for distinct identification in Sw
*/

class NaviContentBookmark
{
    OUString        aUrl;       // URL including jump mark
    OUString        aDescr;     // description
    long            nDocSh;     // address of DocShell
    RegionMode      nDefDrag;   // description contains defaultDragType

public:
    NaviContentBookmark();
    NaviContentBookmark( const OUString &rUrl, const OUString& rDesc,
                            RegionMode nDragType, const SwDocShell* );

    const OUString& GetURL() const              { return aUrl; }
    const OUString& GetDescription() const      { return aDescr; }
    RegionMode      GetDefaultDragType() const  { return nDefDrag; }
    void            Copy( TransferDataContainer& rData ) const;
    bool            Paste( TransferableDataHelper& rData );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
