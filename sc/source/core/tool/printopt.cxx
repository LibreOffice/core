/*************************************************************************
 *
 *  $RCSfile: printopt.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:18 $
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
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include <segmentc.hxx>

#include <tlstream.hxx>
#include "cfgids.hxx"
#include "printopt.hxx"
#include "scresid.hxx"
#include "sc.hrc"

// STATIC DATA -----------------------------------------------------------

#define SC_VERSION ((USHORT)251)

TYPEINIT1(ScTpPrintItem, SfxPoolItem);

SEG_EOFGLOBALS()

//========================================================================
//      ScPrintOptions - Dokument-Optionen
//========================================================================
#pragma SEG_FUNCDEF(printopt_01)

ScPrintOptions::ScPrintOptions()
{
    SetDefaults();
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(printopt_02)

ScPrintOptions::ScPrintOptions( const ScPrintOptions& rCpy )
        :   bWarnPrinterNotFound( rCpy.bWarnPrinterNotFound ),
            bWarnPageSize( rCpy.bWarnPageSize ),
            bWarnOrientation( rCpy.bWarnOrientation )
{
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(printopt_03)

__EXPORT ScPrintOptions::~ScPrintOptions()
{
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(printopt_04)

SvStream& operator>>( SvStream& rStream, ScPrintOptions& rOpt )
{
    rStream >> rOpt.bWarnPrinterNotFound;
    rStream >> rOpt.bWarnPageSize;
    rStream >> rOpt.bWarnOrientation;

    return rStream;
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(printopt_05)

SvStream& operator<<( SvStream& rStream, const ScPrintOptions& rOpt )
{
    rStream << rOpt.bWarnPrinterNotFound;
    rStream << rOpt.bWarnPageSize;
    rStream << rOpt.bWarnOrientation;

    return rStream;
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(printopt_06)

void ScPrintOptions::SetDefaults()
{
    bWarnPrinterNotFound = TRUE;
    bWarnPageSize        = FALSE;
    bWarnOrientation     = FALSE;
}

//========================================================================
//      ScTpPrintItem - Daten fuer die PrintOptions-TabPage
//========================================================================
#pragma SEG_FUNCDEF(printopt_07)

ScTpPrintItem::ScTpPrintItem( USHORT nWhich ) : SfxPoolItem( nWhich )
{
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(printopt_08)

ScTpPrintItem::ScTpPrintItem( USHORT nWhich, const ScPrintOptions& rOpt )
    :   SfxPoolItem ( nWhich ),
        theOptions  ( rOpt )
{
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(printopt_09)

ScTpPrintItem::ScTpPrintItem( const ScTpPrintItem& rItem )
    :   SfxPoolItem ( rItem ),
        theOptions  ( rItem.theOptions )
{
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(printopt_0a)

__EXPORT ScTpPrintItem::~ScTpPrintItem()
{
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(printopt_0b)

String __EXPORT ScTpPrintItem::GetValueText() const
{
    return "ScTpPrintItem";
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(printopt_0c)

int __EXPORT ScTpPrintItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal Which or Type" );

    const ScTpPrintItem& rPItem = (const ScTpPrintItem&)rItem;

    return ( theOptions == rPItem.theOptions );
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(printopt_0d)

SfxPoolItem* __EXPORT ScTpPrintItem::Clone( SfxItemPool * ) const
{
    return new ScTpPrintItem( *this );
}

//==================================================================
// CfgItem fuer Print-Optionen
//==================================================================
#pragma SEG_FUNCDEF(printopt_12)

ScPrintCfg::ScPrintCfg() : SfxConfigItem( SCCFG_PRINT )
{
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(printopt_0e)

int __EXPORT ScPrintCfg::Load( SvStream& rStream )
{
    USHORT  nVersion;

    rStream >> nVersion;
    rStream >> (ScPrintOptions&)*this;

    SetDefault( FALSE );

    return ( nVersion == SC_VERSION )
                ? SfxConfigItem::ERR_OK
                : SfxConfigItem::WARNING_VERSION;
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(printopt_11)

BOOL SEXPORT ScPrintCfg::Store( SvStream& rStream)
{
    rStream << SC_VERSION;
    rStream << *this;

    SetDefault( FALSE );

    return SfxConfigItem::ERR_OK;
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(printopt_0f)

void __EXPORT ScPrintCfg::UseDefault()
{
    SetDefaults();
    SetDefault( TRUE );
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(printopt_10)

String __EXPORT ScPrintCfg::GetName() const
{
    return String( ScResId( SCSTR_CFG_PRINT ) );
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.8  2000/09/17 14:08:42  willem.vandorp
    OpenOffice header added.

    Revision 1.7  2000/08/31 16:38:03  willem.vandorp
    Header and footer replaced

    Revision 1.6  1997/11/13 20:05:36  NN
    ifndef PCH raus


      Rev 1.5   13 Nov 1997 21:05:36   NN
   ifndef PCH raus

      Rev 1.4   06 Nov 1996 09:48:56   TRI
   includes

      Rev 1.3   17 Nov 1995 14:49:44   MO
   SetModified entfernt

      Rev 1.2   16 Nov 1995 20:41:44   MO
   SetDefault()/SetModified im Load/Save

      Rev 1.1   09 Oct 1995 11:49:02   MO
   ResId -> ScResId

      Rev 1.0   18 Sep 1995 17:04:30   MO
   Initial revision.

------------------------------------------------------------------------*/

#pragma SEG_EOFMODULE

