/*************************************************************************
 *
 *  $RCSfile: condedit.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 14:24:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// include ---------------------------------------------------------------

#pragma hdrstop

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
using namespace ::rtl;
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

sal_Int8 ConditionEdit::AcceptDrop( const AcceptDropEvent& rEvt )
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


