/*************************************************************************
 *
 *  $RCSfile: txtcfg.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:26 $
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

#ifndef _TXTCFG_HXX
#define _TXTCFG_HXX

#ifdef DEBUG
#include "dbgloop.hxx"  // DBG_LOOP
#else
#ifdef DBG_LOOP     //kann per precompiled hereinkommen
#undef DBG_LOOP
#undef DBG_LOOP_RESET
#endif
#define DBG_LOOP
#define DBG_LOOP_RESET
#endif

// Toleranzwert in der Formatierung und Textausgabe.
#define SLOPPY_TWIPS    5

#define CONSTCHAR( name, string ) static const sal_Char __FAR_DATA name[] = string

// Allgemeines ...

#ifndef CONST
#define CONST const
#endif

/*************************************************************************

      $Log: not supported by cvs2svn $
      Revision 1.37  2000/09/18 16:04:22  willem.vandorp
      OpenOffice header added.

      Revision 1.36  2000/09/08 13:25:44  willem.vandorp
      Header and footer replaced

      Revision 1.35  2000/04/13 09:59:17  ama
      Unicode changes

      Revision 1.34  1997/12/11 15:44:48  AMA
      Fix: Text-Verzeichnis wieder mit DEBUG uebersetzbar


      Rev 1.33   11 Dec 1997 16:44:48   AMA
   Fix: Text-Verzeichnis wieder mit DEBUG uebersetzbar

      Rev 1.32   05 Dec 1997 08:43:28   MA
   includes

      Rev 1.32   05 Dec 1997 08:41:28   MA
   includes

      Rev 1.31   15 Oct 1997 12:31:44   AMA
   Opt: DBG_LOOP kann jetzt auch resetted werden

      Rev 1.30   25 Jun 1996 20:36:02   MA
   headerfiles

      Rev 1.29   07 Feb 1996 08:57:46   OS
   _GDIMTF_HXX definiert

      Rev 1.28   19 Apr 1995 15:08:30   ER
   kein define _unqidx_hxx mehr

      Rev 1.27   18 Apr 1995 17:29:04   AMA
   DBGLOOP nur noch bei DEBUG=sal_True

      Rev 1.26   15 Dec 1994 20:51:38   SWG
   *ARR* Ersetzungen, svmem, __far_data etc.

      Rev 1.25   26 Apr 1994 17:39:20   BP
   6922: Toleranz von SLOPPY_TWIPS

      Rev 1.24   16 Nov 1993 14:10:04   BP
   chg: Renovierung, mit win088h1_bp zusammengemischt

      Rev 1.23   08 Nov 1993 14:26:00   BP
   dbg: Umstellung auf dbgloop.hxx, Loop-Kontrolle bei not PRODUCT

      Rev 1.21   05 Oct 1993 17:21:16   BP
   chg: Aufraeumarbeiten nach dem final

      Rev 1.20   14 Sep 1993 08:27:00   SWG
   INLINE Definition wech

      Rev 1.19   12 Jul 1993 13:53:14   ER
   NEWTXT fuer MAC

      Rev 1.18   02 Jul 1993 10:59:28   BP
   new: PRODUCT mit NEWTXT verdrahtet, NEWTXT ohne SCRMET

      Rev 1.17   27 Jun 1993 12:16:24   BP
   chg: OLDTXT ist abhaengig von NEWTXT

      Rev 1.16   22 Jun 1993 15:39:10   BP
   new: OLDTXT enabled fuer neue und alte Textformatierung

      Rev 1.15   08 Jun 1993 16:53:24   ER
   else fuer DGB_LOOP

      Rev 1.14   08 Jun 1993 15:58:28   BP
   dbg: DBG_LOOP bemerkt Endlosschleifen.

      Rev 1.13   14 May 1993 09:12:46   OK
   Time Stamp rausgeflogen

      Rev 1.12   27 Apr 1993 11:34:26   BP
   chg: CONSTCHAR-Makro

      Rev 1.11   20 Apr 1993 10:40:54   BP
   del: DbgPen, DbgColor in TxtFrm

      Rev 1.10   22 Mar 1993 10:58:40   BP
   bug: falsche Umlaute fuer PM2

      Rev 1.9   06 Mar 1993 17:51:36   SWG
   const Behandlung fuer SUN auskommentiert

      Rev 1.8   02 Mar 1993 09:33:56   BP
   new: CONSTCHAR fuer Daten ins __FAR_DATA

      Rev 1.7   21 Feb 1993 13:58:42   OK
   Umlautr fuer Windows NT

      Rev 1.6   29 Jan 1993 11:43:34   BP
   new: NOINLINE schaltet alle INLINEs (gewinnt 20 KB im Textverzeichnis)

      Rev 1.5   25 Jan 1993 15:29:36   BP
   new: IS_CHAR-Makros aus dem trlib.hxx uebernommen.

      Rev 1.4   18 Jan 1993 16:32:28   BP
   chg: nun gibt's nur noch einen Schalter

      Rev 1.3   15 Jan 1993 17:28:08   BP
   chg: der SV-Bug ist zu fatal - Umstellung auf SOS_Kerning

      Rev 1.2   15 Jan 1993 13:08:28   BP
   new: KERNING aktiviert.
   new: PAINTOFST

      Rev 1.1   14 Jan 1993 14:24:18   BP
   new: Schalter fuer SOS_Kerning
   bug: SCRMET statt SCRMETRIC

      Rev 1.0   14 Jan 1993 12:56:10   BP
   Initial revision.

*************************************************************************/

#endif
