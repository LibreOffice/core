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

#include "cell.hxx"

#include "document.hxx"
#include "formulacell.hxx"

#include <svl/broadcast.hxx>

// STATIC DATA -----------------------------------------------------------

#ifdef USE_MEMPOOL
IMPL_FIXEDMEMPOOL_NEWDEL( ScValueCell )
IMPL_FIXEDMEMPOOL_NEWDEL( ScStringCell )
IMPL_FIXEDMEMPOOL_NEWDEL( ScNoteCell )
#endif

// ============================================================================

ScBaseCell::ScBaseCell( CellType eNewType ) :
    mpBroadcaster( 0 ),
    eCellType( sal::static_int_cast<sal_uInt8>(eNewType) )
{
}

ScBaseCell::ScBaseCell( const ScBaseCell& rCell ) :
    mpBroadcaster( 0 ),
    eCellType( rCell.eCellType )
{
}

ScBaseCell::~ScBaseCell()
{
    delete mpBroadcaster;
    OSL_ENSURE( eCellType == CELLTYPE_DESTROYED, "BaseCell Destructor" );
}

namespace {

ScBaseCell* lclCloneCell( const ScBaseCell& rSrcCell, ScDocument& rDestDoc, const ScAddress& rDestPos, int nCloneFlags )
{
    switch( rSrcCell.GetCellType() )
    {
        case CELLTYPE_VALUE:
            return new ScValueCell( static_cast< const ScValueCell& >( rSrcCell ) );
        case CELLTYPE_STRING:
            return new ScStringCell( static_cast< const ScStringCell& >( rSrcCell ) );
        case CELLTYPE_EDIT:
            return new ScEditCell(static_cast<const ScEditCell&>(rSrcCell), rDestDoc, rDestPos);
        case CELLTYPE_FORMULA:
            return new ScFormulaCell( static_cast< const ScFormulaCell& >( rSrcCell ), rDestDoc, rDestPos, nCloneFlags );
        case CELLTYPE_NOTE:
            return new ScNoteCell;
        default:;
    }
    OSL_FAIL( "lclCloneCell - unknown cell type" );
    return 0;
}

} // namespace

ScBaseCell* ScBaseCell::Clone( ScDocument& rDestDoc, int nCloneFlags ) const
{
    // notes will not be cloned -> cell address only needed for formula cells
    ScAddress aDestPos;
    if( eCellType == CELLTYPE_FORMULA )
        aDestPos = static_cast< const ScFormulaCell* >( this )->aPos;
    return lclCloneCell( *this, rDestDoc, aDestPos, nCloneFlags );
}

ScBaseCell* ScBaseCell::Clone( ScDocument& rDestDoc, const ScAddress& rDestPos, int nCloneFlags ) const
{
    return lclCloneCell( *this, rDestDoc, rDestPos, nCloneFlags );
}

void ScBaseCell::Delete()
{
    switch (eCellType)
    {
        case CELLTYPE_VALUE:
            delete (ScValueCell*) this;
            break;
        case CELLTYPE_STRING:
            delete (ScStringCell*) this;
            break;
        case CELLTYPE_EDIT:
            delete (ScEditCell*) this;
            break;
        case CELLTYPE_FORMULA:
            delete (ScFormulaCell*) this;
            break;
        case CELLTYPE_NOTE:
            delete (ScNoteCell*) this;
            break;
        default:
            OSL_FAIL("Attempt to Delete() an unknown CELLTYPE");
            break;
    }
}

bool ScBaseCell::IsBlank() const
{
    if(eCellType == CELLTYPE_NOTE)
        return true;

    return false;
}

void ScBaseCell::TakeBroadcaster( SvtBroadcaster* pBroadcaster )
{
    delete mpBroadcaster;
    mpBroadcaster = pBroadcaster;
}

SvtBroadcaster* ScBaseCell::ReleaseBroadcaster()
{
    SvtBroadcaster* pBroadcaster = mpBroadcaster;
    mpBroadcaster = 0;
    return pBroadcaster;
}

void ScBaseCell::DeleteBroadcaster()
{
    DELETEZ( mpBroadcaster );
}

sal_uInt16 ScBaseCell::GetErrorCode() const
{
    switch ( eCellType )
    {
        case CELLTYPE_FORMULA :
            return ((ScFormulaCell*)this)->GetErrCode();
        default:
            return 0;
    }
}


bool ScBaseCell::HasEmptyData() const
{
    switch ( eCellType )
    {
        case CELLTYPE_NOTE :
            return true;
        case CELLTYPE_FORMULA :
            return ((ScFormulaCell*)this)->IsEmpty();
        default:
            return false;
    }
}


bool ScBaseCell::HasValueData() const
{
    switch ( eCellType )
    {
        case CELLTYPE_VALUE :
            return true;
        case CELLTYPE_FORMULA :
            return ((ScFormulaCell*)this)->IsValue();
        default:
            return false;
    }
}


bool ScBaseCell::HasStringData() const
{
    switch ( eCellType )
    {
        case CELLTYPE_STRING :
        case CELLTYPE_EDIT :
            return true;
        case CELLTYPE_FORMULA :
            return !((ScFormulaCell*)this)->IsValue();
        default:
            return false;
    }
}

rtl::OUString ScBaseCell::GetStringData() const
{
    rtl::OUString aStr;
    switch ( eCellType )
    {
        case CELLTYPE_STRING:
            aStr = ((const ScStringCell*)this)->GetString();
            break;
        case CELLTYPE_EDIT:
            aStr = ((const ScEditCell*)this)->GetString();
            break;
        case CELLTYPE_FORMULA:
            aStr = ((ScFormulaCell*)this)->GetString();      // On the Formula cells are not constant
            break;
    }
    return aStr;
}

ScNoteCell::ScNoteCell( SvtBroadcaster* pBC ) :
    ScBaseCell( CELLTYPE_NOTE )
{
    TakeBroadcaster( pBC );
}

#if OSL_DEBUG_LEVEL > 0
ScNoteCell::~ScNoteCell()
{
    eCellType = CELLTYPE_DESTROYED;
}
#endif

// ============================================================================

ScValueCell::ScValueCell( double fValue ) :
    ScBaseCell( CELLTYPE_VALUE ),
    mfValue( fValue )
{
}

#if OSL_DEBUG_LEVEL > 0
ScValueCell::~ScValueCell()
{
    eCellType = CELLTYPE_DESTROYED;
}
#endif

// ============================================================================

ScStringCell::ScStringCell( const rtl::OUString& rString ) :
    ScBaseCell( CELLTYPE_STRING ),
    maString( rString.intern() )
{
}

#if OSL_DEBUG_LEVEL > 0
ScStringCell::~ScStringCell()
{
    eCellType = CELLTYPE_DESTROYED;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
