/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <sot/formats.hxx>

#include <condedit.hxx>
#include <svx/dbaexchange.hxx>
using namespace ::svx;
using ::rtl::OUString;
using namespace ::com::sun::star::uno;
#define DB_DD_DELIM 0x0b

// STATIC DATA -----------------------------------------------------------

ConditionEdit::ConditionEdit( Window* pParent, const ResId& rResId )
    : Edit( pParent, rResId ),
    DropTargetHelper( this ),
    bBrackets( sal_True ), bEnableDrop( sal_True )
{
}

/*--------------------------------------------------------------------
    Beschreibung: Drop moeglich, bzw Format bekannt?
 --------------------------------------------------------------------*/
sal_Int8 ConditionEdit::AcceptDrop( const AcceptDropEvent& /*rEvt*/ )
{
    return OColumnTransferable::canExtractColumnDescriptor
        ( GetDataFlavorExVector(),
                                CTF_COLUMN_DESCRIPTOR )
                ? DND_ACTION_COPY
                : DND_ACTION_NONE;
}

sal_Int8 ConditionEdit::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    sal_Int8 nRet = DND_ACTION_NONE;
    if( bEnableDrop )
    {
        String sTxt;
        TransferableDataHelper aData( rEvt.maDropEvent.Transferable );

            DataFlavorExVector& rVector = aData.GetDataFlavorExVector();
            if(OColumnTransferable::canExtractColumnDescriptor(rVector, CTF_COLUMN_DESCRIPTOR))
            {
                ODataAccessDescriptor aColDesc = OColumnTransferable::extractColumnDescriptor(
                                                                    aData);
                String sDBName;
                if (bBrackets)
                    sDBName += '[';
                OUString sTmp;
                sTmp = aColDesc.getDataSource();
                sDBName += String(sTmp);
                sDBName += '.';

                aColDesc[daCommand] >>= sTmp;
                sDBName += String(sTmp);
                sDBName += '.';

                aColDesc[daColumnName] >>= sTmp;
                sDBName += String(sTmp);
                if (bBrackets)
                    sDBName += ']';

                SetText( sDBName );
                nRet = DND_ACTION_COPY;
            }
    }
    return nRet;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
