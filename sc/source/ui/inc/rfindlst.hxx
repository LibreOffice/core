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

#ifndef SC_RFINDLST_HXX
#define SC_RFINDLST_HXX

#include <tools/color.hxx>
#include "global.hxx"
#include "address.hxx"

//==================================================================

struct ScRangeFindData
{
    ScRange     aRef;
    sal_uInt16      nFlags;
    xub_StrLen  nSelStart;
    xub_StrLen  nSelEnd;

    ScRangeFindData( const ScRange& rR, sal_uInt16 nF, xub_StrLen nS, xub_StrLen nE ) :
        aRef(rR), nFlags(nF), nSelStart(nS), nSelEnd(nE) {}
};

class ScRangeFindList
{
    std::vector<ScRangeFindData> maEntries;
    String      aDocName;
    bool        bHidden;

public:
            ScRangeFindList(const String& rName);

    sal_uLong   Count() const                       { return maEntries.size(); }
    void    Insert( const ScRangeFindData &rNew )       { maEntries.push_back(rNew); }

    ScRangeFindData* GetObject( sal_uLong nIndex ) { return &(maEntries[nIndex]); }

    void    SetHidden( sal_Bool bSet )              { bHidden = bSet; }

    const String&   GetDocName() const          { return aDocName; }
    bool            IsHidden() const            { return bHidden; }

    static ColorData GetColorName( size_t nIndex );
};



#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
