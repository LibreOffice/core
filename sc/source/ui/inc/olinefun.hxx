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

#include <types.hxx>

class ScDocShell;
class ScRange;

class ScOutlineDocFunc
{
private:
    ScDocShell& rDocShell;

public:
                ScOutlineDocFunc( ScDocShell& rDocSh ): rDocShell(rDocSh) {}

    void        MakeOutline( const ScRange& rRange, bool bColumns, bool bRecord, bool bApi );
    void        RemoveOutline( const ScRange& rRange, bool bColumns, bool bRecord, bool bApi );
    bool        RemoveAllOutlines( SCTAB nTab, bool bRecord );
    void        AutoOutline( const ScRange& rRange, bool bRecord );

    bool        SelectLevel( SCTAB nTab, bool bColumns, sal_uInt16 nLevel,
                                    bool bRecord, bool bPaint );

    bool        ShowMarkedOutlines( const ScRange& rRange, bool bRecord );
    bool        HideMarkedOutlines( const ScRange& rRange, bool bRecord );

    void        ShowOutline( SCTAB nTab, bool bColumns, sal_uInt16 nLevel, sal_uInt16 nEntry,
                                    bool bRecord, bool bPaint );
    bool        HideOutline( SCTAB nTab, bool bColumns, sal_uInt16 nLevel, sal_uInt16 nEntry,
                                    bool bRecord, bool bPaint );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
