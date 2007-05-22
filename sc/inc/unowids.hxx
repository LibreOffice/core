/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unowids.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 19:40:18 $
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

#ifndef SC_UNOWIDS_HXX
#define SC_UNOWIDS_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef SC_ITEMS_HXX
#include "scitems.hxx"
#endif


//  WIDs for uno property maps,
//  never stored in files

#define SC_WID_UNO_START    1200

#define SC_WID_UNO_CELLSTYL         ( SC_WID_UNO_START + 0 )
#define SC_WID_UNO_CHCOLHDR         ( SC_WID_UNO_START + 1 )
#define SC_WID_UNO_CHROWHDR         ( SC_WID_UNO_START + 2 )
#define SC_WID_UNO_CONDFMT          ( SC_WID_UNO_START + 3 )
#define SC_WID_UNO_CONDLOC          ( SC_WID_UNO_START + 4 )
#define SC_WID_UNO_CONDXML          ( SC_WID_UNO_START + 5 )
#define SC_WID_UNO_TBLBORD          ( SC_WID_UNO_START + 6 )
#define SC_WID_UNO_VALIDAT          ( SC_WID_UNO_START + 7 )
#define SC_WID_UNO_VALILOC          ( SC_WID_UNO_START + 8 )
#define SC_WID_UNO_VALIXML          ( SC_WID_UNO_START + 9 )
#define SC_WID_UNO_POS              ( SC_WID_UNO_START + 10 )
#define SC_WID_UNO_SIZE             ( SC_WID_UNO_START + 11 )
#define SC_WID_UNO_FORMLOC          ( SC_WID_UNO_START + 12 )
#define SC_WID_UNO_FORMRT           ( SC_WID_UNO_START + 13 )
#define SC_WID_UNO_PAGESTL          ( SC_WID_UNO_START + 14 )
#define SC_WID_UNO_CELLVIS          ( SC_WID_UNO_START + 15 )
#define SC_WID_UNO_LINKDISPBIT      ( SC_WID_UNO_START + 16 )
#define SC_WID_UNO_LINKDISPNAME     ( SC_WID_UNO_START + 17 )
#define SC_WID_UNO_CELLWID          ( SC_WID_UNO_START + 18 )
#define SC_WID_UNO_OWIDTH           ( SC_WID_UNO_START + 19 )
#define SC_WID_UNO_NEWPAGE          ( SC_WID_UNO_START + 20 )
#define SC_WID_UNO_MANPAGE          ( SC_WID_UNO_START + 21 )
#define SC_WID_UNO_CELLHGT          ( SC_WID_UNO_START + 22 )
#define SC_WID_UNO_CELLFILT         ( SC_WID_UNO_START + 23 )
#define SC_WID_UNO_OHEIGHT          ( SC_WID_UNO_START + 24 )
#define SC_WID_UNO_DISPNAME         ( SC_WID_UNO_START + 25 )
#define SC_WID_UNO_HEADERSET        ( SC_WID_UNO_START + 26 )
#define SC_WID_UNO_FOOTERSET        ( SC_WID_UNO_START + 27 )
#define SC_WID_UNO_NUMRULES         ( SC_WID_UNO_START + 28 )
#define SC_WID_UNO_ISACTIVE         ( SC_WID_UNO_START + 29 )
#define SC_WID_UNO_BORDCOL          ( SC_WID_UNO_START + 30 )
#define SC_WID_UNO_PROTECT          ( SC_WID_UNO_START + 31 )
#define SC_WID_UNO_SHOWBORD         ( SC_WID_UNO_START + 32 )
#define SC_WID_UNO_PRINTBORD        ( SC_WID_UNO_START + 33 )
#define SC_WID_UNO_COPYBACK         ( SC_WID_UNO_START + 34 )
#define SC_WID_UNO_COPYSTYL         ( SC_WID_UNO_START + 35 )
#define SC_WID_UNO_COPYFORM         ( SC_WID_UNO_START + 36 )
#define SC_WID_UNO_TABLAYOUT        ( SC_WID_UNO_START + 37 )
#define SC_WID_UNO_AUTOPRINT        ( SC_WID_UNO_START + 38 )
#define SC_WID_UNO_ABSNAME          ( SC_WID_UNO_START + 39 )
#define SC_WID_UNO_END              ( SC_WID_UNO_START + 39 )


inline BOOL IsScUnoWid( USHORT nWid )
{
    return nWid >= SC_WID_UNO_START && nWid <= SC_WID_UNO_END;
}

inline BOOL IsScItemWid( USHORT nWid )
{
    return nWid >= ATTR_STARTINDEX && nWid <= ATTR_ENDINDEX;    // incl. page
}

#endif

