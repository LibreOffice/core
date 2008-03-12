/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: condedit.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 12:59:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

// include ---------------------------------------------------------------


#ifndef _SOT_FORMATS_HXX //autogen
#include <sot/formats.hxx>
#endif

#ifndef _CONDEDIT_HXX
#include <condedit.hxx>
#endif
#ifndef _SVX_DBAEXCHANGE_HXX_
#include <svx/dbaexchange.hxx>
#endif
using namespace ::svx;
using ::rtl::OUString;
using namespace ::com::sun::star::uno;
#define DB_DD_DELIM 0x0b

// STATIC DATA -----------------------------------------------------------

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

ConditionEdit::ConditionEdit( Window* pParent, const ResId& rResId )
    : Edit( pParent, rResId ),
    DropTargetHelper( this ),
    bBrackets( TRUE ), bEnableDrop( TRUE )
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


