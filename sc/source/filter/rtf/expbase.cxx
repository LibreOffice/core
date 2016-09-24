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

#include "expbase.hxx"
#include "document.hxx"
#include "editutil.hxx"

ScExportBase::ScExportBase( SvStream& rStrmP, ScDocument* pDocP,
                const ScRange& rRangeP )
            :
            rStrm( rStrmP ),
            aRange( rRangeP ),
            pDoc( pDocP ),
            pFormatter( pDocP->GetFormatTable() ),
            pEditEngine( nullptr )
{
}

ScExportBase::~ScExportBase()
{
    delete pEditEngine;
}

bool ScExportBase::GetDataArea( SCTAB nTab, SCCOL& nStartCol,
            SCROW& nStartRow, SCCOL& nEndCol, SCROW& nEndRow ) const
{
    pDoc->GetDataStart( nTab, nStartCol, nStartRow );
    pDoc->GetPrintArea( nTab, nEndCol, nEndRow );
    return TrimDataArea( nTab, nStartCol, nStartRow, nEndCol, nEndRow );
}

bool ScExportBase::TrimDataArea( SCTAB nTab, SCCOL& nStartCol,
            SCROW& nStartRow, SCCOL& nEndCol, SCROW& nEndRow ) const
{
    while ( nStartCol <= nEndCol && pDoc->ColHidden(nStartCol, nTab))
        ++nStartCol;
    while ( nStartCol <= nEndCol && pDoc->ColHidden(nEndCol, nTab))
        --nEndCol;
    nStartRow = pDoc->FirstVisibleRow(nStartRow, nEndRow, nTab);
    nEndRow = pDoc->LastVisibleRow(nStartRow, nEndRow, nTab);
    return nStartCol <= nEndCol && nStartRow <= nEndRow && nEndRow !=
        ::std::numeric_limits<SCROW>::max();
}

bool ScExportBase::IsEmptyTable( SCTAB nTab ) const
{
    if ( !pDoc->HasTable( nTab ) || !pDoc->IsVisible( nTab ) )
        return true;
    SCCOL nStartCol, nEndCol;
    SCROW nStartRow, nEndRow;
    return !GetDataArea( nTab, nStartCol, nStartRow, nEndCol, nEndRow );
}

ScFieldEditEngine& ScExportBase::GetEditEngine() const
{
    if ( !pEditEngine )
        const_cast<ScExportBase*>(this)->pEditEngine = new ScFieldEditEngine(pDoc, pDoc->GetEditPool());
    return *pEditEngine;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
