/*************************************************************************
 *
 *  $RCSfile: ww8graf.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: cmc $ $Date: 2002-08-22 11:30:32 $
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

#ifndef __SGI_STL_VECTOR
#include <vector>
#endif

#if defined WNT || defined WIN || defined OS2
#define __WW8_NEEDS_PACK
#pragma pack(2)
#endif

struct WW8_FSPA
{
public:
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
public:
    enum FSPAOrient {RelPgMargin, RelPageBorder, RelText};
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
    SVBT32 cTxbx_iNextReuse;
    SVBT32 cReusable;
    SVBT16 fReusable;
    SVBT32 reserved;
    SVBT32 ShapeId;
    SVBT32 txidUndo;
};

#ifdef __WW8_NEEDS_PACK
#pragma pack()
#endif

class wwZOrderer
{
private:
    //No of objects in doc before starting (always 0 unless using file->insert
    //and probably 0 then as well
    std::vector<ULONG> maEscherLayer;
    typedef std::vector<ULONG>::iterator myeiter;

    std::vector<short> maDrawHeight;
    typedef std::vector<short>::iterator myditer;

    ULONG mnNoInitialObjects;
    ULONG mnInlines;
    SdrPage* mpDrawPg;
    const SvxMSDffShapeOrders *mpShapeOrders;
    sal_Int8 mnHeaven;
    sal_Int8 mnHell;

    ULONG GetEscherObjectPos(ULONG nSpId);
    ULONG GetDrawingObjectPos(short nWwHeight);
public:
    wwZOrderer(SdrPage* pDrawPg, const SvxMSDffShapeOrders *pShapeOrders,
        sal_Int8 nHeaven, sal_Int8 nHell);
    void InsertTextLayerObject(SdrObject* pObject);
    /*
     cmc: We should have have seperate ZOrder classes for 95- and 97+ and
     instantiate the appropiate one at run time.
     */
    void InsertDrawingObject(SdrObject* pObj, short nWwHeight);
    void InsertEscherObject(SdrObject* pObject, ULONG nSpId);
};

void WW8FSPAShadowToReal( WW8_FSPA_SHADOW* pFSPAS, WW8_FSPA* pPic );
#endif
