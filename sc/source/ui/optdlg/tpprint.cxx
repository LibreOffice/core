/*************************************************************************
 *
 *  $RCSfile: tpprint.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:03 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef PCH
#include "scitems.hxx"
#include <sv.hxx>
#include <svtool.hxx>
#include <tools.hxx>
#include <sfx.hxx>
#include <segmentc.hxx>
#endif

#include "printopt.hxx"
#include "uiitems.hxx"
#include "scresid.hxx"

#include "sc.hrc"       // -> Slot-IDs
#include "optdlg.hrc"

#define _TPPRINT_CXX
#include "tpprint.hxx"
#undef _TPPRINT_CXX


// STATIC DATA -----------------------------------------------------------

static USHORT pPrintOptRanges[] =
{
    SID_SCPRINTOPTIONS,
    SID_SCPRINTOPTIONS,
    0
};

SEG_EOFGLOBALS()

//========================================================================
#pragma SEG_FUNCDEF(tpprint_01)

ScTpPrintOptions::ScTpPrintOptions( Window*           pParent,
                                    const SfxItemSet& rCoreAttrs )
    :   SfxTabPage      ( pParent,
                          ScResId( RID_SCPAGE_PRINT ),
                          rCoreAttrs ),
        aBtnPrinterNotFound ( this, ScResId( BTN_WARNPRINTERNOTFOUND ) ),
        aBtnPageSize        ( this, ScResId( BTN_WARNPAPERSIZE ) ),
        aBtnOrientation     ( this, ScResId( BTN_WARNPAPERBIN ) ),
        aGbWarnings         ( this, ScResId( GB_WARNINGS ) ),
        nWhichPrint         ( GetWhich( SID_SCPRINTOPTIONS ) )
{
    FreeResource();
}

//-----------------------------------------------------------------------
#pragma SEG_FUNCDEF(tpprint_02)

__EXPORT ScTpPrintOptions::~ScTpPrintOptions()
{
}

//-----------------------------------------------------------------------
#pragma SEG_FUNCDEF(tpprint_03)

USHORT* __EXPORT ScTpPrintOptions::GetRanges()
{
    return pPrintOptRanges;
}

//-----------------------------------------------------------------------
#pragma SEG_FUNCDEF(tpprint_04)

SfxTabPage* __EXPORT ScTpPrintOptions::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return ( new ScTpPrintOptions( pParent, rAttrSet ) );
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(tpprint_06)

int __EXPORT ScTpPrintOptions::DeactivatePage( SfxItemSet* pSet )
{
    if ( pSet )
        FillItemSet( *pSet );

    return LEAVE_PAGE;
}

//-----------------------------------------------------------------------
#pragma SEG_FUNCDEF(tpprint_07)

void __EXPORT ScTpPrintOptions::Reset( const SfxItemSet& rCoreAttrs )
{
    const ScPrintOptions& rOpt = ((const ScTpPrintItem&)rCoreAttrs.Get(nWhichPrint)).
                                        GetPrintOptions();

    aBtnPrinterNotFound .Check( rOpt.GetWarnPrinterNotFound() );
    aBtnPageSize        .Check( rOpt.GetWarnPageSize() );
    aBtnOrientation     .Check( rOpt.GetWarnOrientation() );
    aBtnPrinterNotFound .SaveValue();
    aBtnPageSize        .SaveValue();
    aBtnOrientation     .SaveValue();
}

//-----------------------------------------------------------------------
#pragma SEG_FUNCDEF(tpprint_08)

BOOL __EXPORT ScTpPrintOptions::FillItemSet( SfxItemSet& rCoreAttrs )
{
    if (   aBtnPrinterNotFound.GetSavedValue() != aBtnPrinterNotFound.IsChecked()
        || aBtnPageSize       .GetSavedValue() != aBtnPageSize.IsChecked()
        || aBtnOrientation    .GetSavedValue() != aBtnOrientation.IsChecked() )
    {
        ScPrintOptions aOpt;

        aOpt.SetWarnPrinterNotFound ( aBtnPrinterNotFound.IsChecked() );
        aOpt.SetWarnPageSize        ( aBtnPageSize.IsChecked() );
        aOpt.SetWarnOrientation     ( aBtnOrientation.IsChecked() );

        rCoreAttrs.Put( ScTpPrintItem( nWhichPrint, aOpt ) );

        return TRUE;
    }
    else
        return FALSE;
}


/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.5  2000/09/17 14:09:16  willem.vandorp
    OpenOffice header added.

    Revision 1.4  2000/08/31 16:38:38  willem.vandorp
    Header and footer replaced

    Revision 1.3  1996/10/29 13:07:20  NN
    ueberall ScResId statt ResId


      Rev 1.2   29 Oct 1996 14:07:20   NN
   ueberall ScResId statt ResId

      Rev 1.1   28 Nov 1995 19:03:02   MO
   303a: HasExchangeSupport entfernt

      Rev 1.0   18 Sep 1995 17:10:18   MO
   Initial revision.

------------------------------------------------------------------------*/

#pragma SEG_EOFMODULE

