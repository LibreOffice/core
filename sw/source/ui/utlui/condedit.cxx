/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

// include ---------------------------------------------------------------


#include <sot/formats.hxx>

#ifndef _CONDEDIT_HXX
#include <condedit.hxx>
#endif
#include <svx/dbaexchange.hxx>
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


