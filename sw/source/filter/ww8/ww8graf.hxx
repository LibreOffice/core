/*************************************************************************
 *
 *  $RCSfile: ww8graf.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:58 $
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

#ifndef _WW8GRAF_HXX
#define _WW8GRAF_HXX


#define nbxRelPageBorder    1 // Blatt-Rand
#define nbxRelPgMargin      0 // Page Print Area
#define nbxRelText          2 // Spalten (Absatz) -Rand   = FRAME Print Area

#define nbyRelPageBorder    1
#define nbyRelPgMargin      0
#define nbyRelText          2


struct WW8_FSPA
{
    long nSpId;     //Shape Identifier. Used in conjunction with the office art data (found via fcDggInfo in the FIB) to find the actual data for this shape.
    long nXaLeft;   //left of rectangle enclosing shape relative to the origin of the shape
    long nYaTop;        //top of rectangle enclosing shape relative to the origin of the shape
    long nXaRight;  //right of rectangle enclosing shape relative to the origin of the shape
    long nYaBottom;//bottom of the rectangle enclosing shape relative to the origin of the shape
    USHORT bHdr:1;
    //0001 1 in the undo doc when shape is from the header doc, 0 otherwise (undefined when not in the undo doc)
    USHORT nbx:2;
    //0006 x position of shape relative to anchor CP
    //0 relative to page margin
    //1 relative to top of page
    //2 relative to text (column for horizontal text; paragraph for vertical text)
    //3 reserved for future use
    USHORT nby:2;
    //0018 y position of shape relative to anchor CP
    //0 relative to page margin
    //1 relative to top of page
    //2 relative to text (paragraph for horizontal text; column for vertical text)
    USHORT nwr:4;
    //01E0 text wrapping mode
    //0 like 2, but doesn't require absolute object
    //1 no text next to shape
    //2 wrap around absolute object
    //3 wrap as if no object present
    //4 wrap tightly around object
    //5 wrap tightly, but allow holes
    //6-15 reserved for future use
    USHORT nwrk:4;
    //1E00 text wrapping mode type (valid only for wrapping modes 2 and 4
    //0 wrap both sides
    //1 wrap only on left
    //2 wrap only on right
    //3 wrap only on largest side
    USHORT bRcaSimple:1;
    //2000 when set, temporarily overrides bx, by, forcing the xaLeft, xaRight, yaTop, and yaBottom fields to all be page relative.
    USHORT bBelowText:1;
    //4000
    //1 shape is below text
    //0 shape is above text
    USHORT bAnchorLock:1;
    //8000  1 anchor is locked
    //      0 anchor is not locked
    long nTxbx; //count of textboxes in shape (undo doc only)
};


struct WW8_FSPA_SHADOW  // alle Member an gleicher Position und Groesse,
{                                               // wegen:  pF = (WW8_FSPA*)pFS;
    SVBT32 nSpId;
    SVBT32 nXaLeft;
    SVBT32 nYaTop;
    SVBT32 nXaRight;
    SVBT32 nYaBottom;
    SVBT16 aBits1;
    SVBT32 nTxbx;
};

struct WW8_TXBXS
{
    SVBT32 cTxbx__iNextReuse;
    SVBT32 cReusable;
    SVBT16 fReusable;
    SVBT32 reserved;
    SVBT32 ShapeId;
    SVBT32 txidUndo;
};

void WW8FSPAShadowToReal( WW8_FSPA_SHADOW* pFSPAS, WW8_FSPA* pPic );


/*************************************************************************
      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/ww8/ww8graf.hxx,v 1.1.1.1 2000-09-18 17:14:58 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.11  2000/09/18 16:04:59  willem.vandorp
      OpenOffice header added.

      Revision 1.10  2000/05/16 11:35:11  khz
      Unicode code-conversion

      Revision 1.9  1998/11/25 17:50:46  JP
      Compilerfehler unter UNX


      Rev 1.8   25 Nov 1998 18:50:46   JP
   Compilerfehler unter UNX

      Rev 1.7   20 Nov 1998 17:56:00   KHZ
   Task #59640# Hilfsattribute SwFmtVertOrient und SwFmtHoriOrient (2)

      Rev 1.6   29 Oct 1998 15:27:26   KHZ
   Task #58199# defines fuer rel. Abstand-Orientierung von Shapes geaendert

      Rev 1.5   03 Sep 1998 22:14:22   KHZ
   Task #55189# Textboxen

      Rev 1.4   27 Aug 1998 10:12:30   KHZ
   Task #55189# Escher-Import fuer Draw, Calc und Writer

      Rev 1.3   22 Jul 1998 16:14:16   HR
   ww8graf fuer UNX

      Rev 1.2   21 Jul 1998 14:51:48   KHZ
   Task #52607# (WW 97 Import)

      Rev 1.1   21 Jul 1998 12:33:28   KHZ
   als MSDrawingObject eingebettete Grafik (Teil 1)

      Rev 1.0   16 Jun 1998 10:56:50   KHZ
   Initial revision.

*************************************************************************/
#endif

