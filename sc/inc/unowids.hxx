/*************************************************************************
 *
 *  $RCSfile: unowids.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: nn $ $Date: 2001-04-17 19:33:00 $
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

#define SC_WID_UNO_END              ( SC_WID_UNO_START + 24 )


inline BOOL IsScUnoWid( USHORT nWid )
{
    return nWid >= SC_WID_UNO_START && nWid <= SC_WID_UNO_END;
}

inline BOOL IsScItemWid( USHORT nWid )
{
    return nWid >= ATTR_STARTINDEX && nWid <= ATTR_ENDINDEX;    // incl. page
}

#endif

