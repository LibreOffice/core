/*************************************************************************
 *
 *  $RCSfile: warnbox.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dr $ $Date: 2002-07-11 10:55:11 $
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
        sal_Int16 nRet = WarningBox::Execute();
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

