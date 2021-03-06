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

#include <vector>
#include <svl/hint.hxx>
#include "rangelst.hxx"

struct ScUnoRefEntry
{
    sal_Int64   nObjectId;
    ScRangeList aRanges;

    ScUnoRefEntry( sal_Int64 nId, const ScRangeList& rOldRanges ) :
        nObjectId( nId ),
        aRanges( rOldRanges )
    {
    }
};

/** List of RefUpdate changes made to UNO objects during ScUpdateRefHint broadcast.
*/

class ScUnoRefList
{
private:
    ::std::vector<ScUnoRefEntry> aEntries;

public:
                ScUnoRefList();
                ~ScUnoRefList();

    void        Add( sal_Int64 nId, const ScRangeList& rOldRanges );
    void        Undo( ScDocument* pDoc );

    bool        IsEmpty() const     { return aEntries.empty(); }
};

/** Hint to restore a UNO object to its old state (used during undo).
*/

class ScUnoRefUndoHint final : public SfxHint
{
    ScUnoRefEntry   aEntry;

public:
                ScUnoRefUndoHint( const ScUnoRefEntry& rRefEntry );
                virtual ~ScUnoRefUndoHint() override;

    sal_Int64   GetObjectId() const         { return aEntry.nObjectId; }
    const ScRangeList& GetRanges() const    { return aEntry.aRanges; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
