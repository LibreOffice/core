/*************************************************************************
 *
 *  $RCSfile: drawitem.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:00:55 $
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
#ifndef _SVX_DRAWITEM_HXX
#define _SVX_DRAWITEM_HXX

// include ---------------------------------------------------------------

#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif

//==================================================================
//  SvxColorTableItem
//==================================================================

#ifdef ITEMID_COLOR_TABLE
class XColorTable;

class SvxColorTableItem: public SfxPoolItem
{
    XColorTable*            pColorTable;

public:
                            TYPEINFO();
                            SvxColorTableItem();
                            SvxColorTableItem( XColorTable* pTable,
                                    USHORT nWhich = ITEMID_COLOR_TABLE );
                            SvxColorTableItem( const SvxColorTableItem& );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const International * = 0 ) const;

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    XColorTable*            GetColorTable() const { return pColorTable; }
    void                    SetColorTable( XColorTable* pTable ) {
                                    pColorTable = pTable; }
};

#endif

//==================================================================
//  SvxGradientListItem
//==================================================================

#ifdef ITEMID_GRADIENT_LIST
class XGradientList;

class SvxGradientListItem: public SfxPoolItem
{
    XGradientList*              pGradientList;

public:
                            TYPEINFO();
                            SvxGradientListItem();
                            SvxGradientListItem( XGradientList* pList,
                                    USHORT nWhich = ITEMID_GRADIENT_LIST );
                            SvxGradientListItem( const SvxGradientListItem& );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const International * = 0 ) const;

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    XGradientList*          GetGradientList() const { return pGradientList; }
    void                    SetGradientList( XGradientList* pList ) {
                                    pGradientList = pList; }
};

#endif

//==================================================================
//  SvxHatchListItem
//==================================================================

#ifdef ITEMID_HATCH_LIST
class XHatchList;

class SvxHatchListItem: public SfxPoolItem
{
    XHatchList*             pHatchList;

public:
                            TYPEINFO();
                            SvxHatchListItem();
                            SvxHatchListItem( XHatchList* pList,
                                    USHORT nWhich = ITEMID_HATCH_LIST );
                            SvxHatchListItem( const SvxHatchListItem& );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const International * = 0 ) const;

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    XHatchList*             GetHatchList() const { return pHatchList; }
    void                    SetHatchList( XHatchList* pList ) {
                                    pHatchList = pList; }
};

#endif

//==================================================================
//  SvxBitmapListItem
//==================================================================

#ifdef ITEMID_BITMAP_LIST
class XBitmapList;

class SvxBitmapListItem: public SfxPoolItem
{
    XBitmapList*                pBitmapList;

public:
                            TYPEINFO();
                            SvxBitmapListItem();
                            SvxBitmapListItem( XBitmapList* pBL,
                                    USHORT nWhich = ITEMID_BITMAP_LIST );
                            SvxBitmapListItem( const SvxBitmapListItem& );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const International * = 0 ) const;

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    XBitmapList*            GetBitmapList() const { return pBitmapList; }
    void                    SetBitmapList( XBitmapList* pList ) {
                                    pBitmapList = pList; }
};

#endif

//==================================================================
//  SvxDashListItem
//==================================================================

#ifdef ITEMID_DASH_LIST
class XDashList;

class SvxDashListItem: public SfxPoolItem
{
    XDashList*              pDashList;

public:
                            TYPEINFO();
                            SvxDashListItem();
                            SvxDashListItem( XDashList* pList,
                                    USHORT nWhich = ITEMID_DASH_LIST );
                            SvxDashListItem( const SvxDashListItem& );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const International * = 0 ) const;

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    XDashList*              GetDashList() const { return pDashList; }
    void                    SetDashList( XDashList* pList ) {
                                    pDashList = pList; }
};

#endif

//==================================================================
//  SvxLineEndListItem
//==================================================================

#ifdef ITEMID_LINEEND_LIST
class XLineEndList;

class SvxLineEndListItem: public SfxPoolItem
{
    XLineEndList*           pLineEndList;

public:
                            TYPEINFO();
                            SvxLineEndListItem();
                            SvxLineEndListItem( XLineEndList* pList,
                                    USHORT nWhich = ITEMID_LINEEND_LIST );
                            SvxLineEndListItem( const SvxLineEndListItem& );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const International * = 0 ) const;

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    XLineEndList*           GetLineEndList() const { return pLineEndList; }
    void                    SetLineEndList( XLineEndList* pList ) {
                                    pLineEndList = pList; }
};

#endif


#endif

