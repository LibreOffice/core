/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: warnbox.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 14:11:54 $
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
#include "precompiled_sc.hxx"

#ifndef SC_WARNBOX_HXX
#include "warnbox.hxx"
#endif

#include "scmod.hxx"
#include "inputopt.hxx"
#include "scresid.hxx"
#include "sc.hrc"


// ============================================================================

ScCbWarningBox::ScCbWarningBox( Window* pParent, const String& rMsgStr, bool bDefYes ) :
    WarningBox( pParent, WB_YES_NO | (bDefYes ? WB_DEF_YES : WB_DEF_NO), rMsgStr )
{
    SetDefaultCheckBoxText();
}

sal_Int16 ScCbWarningBox::Execute()
{
    sal_Int16 nRet = (GetStyle() & WB_DEF_YES) ? RET_YES : RET_NO;
    if( IsDialogEnabled() )
    {
        nRet = WarningBox::Execute();
        if( GetCheckBoxState() )
            DisableDialog();
    }
    return nRet;
}

bool ScCbWarningBox::IsDialogEnabled()
{
    return true;
}

void ScCbWarningBox::DisableDialog()
{
}


// ----------------------------------------------------------------------------

ScReplaceWarnBox::ScReplaceWarnBox( Window* pParent ) :
    ScCbWarningBox( pParent, String( ScResId( STR_REPLCELLSWARN ) ), true )
{
    SetHelpId( HID_SC_REPLCELLSWARN );
}

bool ScReplaceWarnBox::IsDialogEnabled()
{
    return SC_MOD()->GetInputOptions().GetReplaceCellsWarn() == TRUE;
}

void ScReplaceWarnBox::DisableDialog()
{
    ScModule* pScMod = SC_MOD();
    ScInputOptions aInputOpt( pScMod->GetInputOptions() );
    aInputOpt.SetReplaceCellsWarn( FALSE );
    pScMod->SetInputOptions( aInputOpt );
}


// ============================================================================

