/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xtable.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 17:00:52 $
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
#ifndef _XTABLE_HXX
#define _XTABLE_HXX

// include ---------------------------------------------------------------

#ifndef _XPOLY_HXX
#include <svx/xpoly.hxx>
#endif
#ifndef _SVX_XDASH_HXX
#include <svx/xdash.hxx>
#endif
#ifndef _SVX_XHATCH_HXX
#include <svx/xhatch.hxx>
#endif
#ifndef _SVX__XGRADIENT_HXX
#include <svx/xgrad.hxx>
#endif
#ifndef _SVX_XBITMAP_HXX
#include <svx/xbitmap.hxx>
#endif
#ifndef _SVX_XFLASIT_HXX
#include <svx/xflasit.hxx>
#endif
#ifndef _SVX_XLNASIT_HXX
#include <svx/xlnasit.hxx>
#endif

#ifndef _SV_COLOR_HXX
#include <tools/color.hxx>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _TABLE_HXX //autogen
#include <tools/table.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif

class Color;
class Bitmap;
class VirtualDevice;
class XOutdevItemPool;
class XOutputDevice;

// Breite und Hoehe der LB-Bitmaps
#define BITMAP_WIDTH  32
#define BITMAP_HEIGHT 12

// Standard-Vergleichsstring
extern sal_Unicode __FAR_DATA pszStandard[]; // "standard"

// Funktion zum Konvertieren in echte RGB-Farben, da mit
// enum COL_NAME nicht verglichen werden kann.
SVX_DLLPUBLIC Color RGB_Color( ColorData nColorName );

// ---------------------
// class XPropertyEntry
// ---------------------

class XPropertyEntry
{
protected:
    String  aName;

            XPropertyEntry(const String& rName) : aName(rName) {}
            XPropertyEntry(const XPropertyEntry& rOther): aName(rOther.aName) {}
public:

    virtual        ~XPropertyEntry() {}
    void            SetName(const String& rName)    { aName = rName; }
    String&         GetName()                       { return aName; }
};

// ------------------
// class XColorEntry
// ------------------

class XColorEntry : public XPropertyEntry
{
    Color   aColor;

public:
            XColorEntry(const Color& rColor, const String& rName) :
                XPropertyEntry(rName), aColor(rColor) {}
            XColorEntry(const XColorEntry& rOther) :
                XPropertyEntry(rOther), aColor(rOther.aColor) {}

    void    SetColor(const Color& rColor)   { aColor = rColor; }
    Color&  GetColor()                      { return aColor; }
};

// --------------------
// class XLineEndEntry
// --------------------

class XLineEndEntry : public XPropertyEntry
{
    basegfx::B2DPolyPolygon aB2DPolyPolygon;

public:
    XLineEndEntry(const basegfx::B2DPolyPolygon& rB2DPolyPolygon, const String& rName)
    :   XPropertyEntry(rName),
        aB2DPolyPolygon(rB2DPolyPolygon)
    {}

    XLineEndEntry(const XLineEndEntry& rOther)
    :   XPropertyEntry(rOther),
        aB2DPolyPolygon(rOther.aB2DPolyPolygon)
    {}

    void SetLineEnd(const basegfx::B2DPolyPolygon& rB2DPolyPolygon)
    {
        aB2DPolyPolygon = rB2DPolyPolygon;
    }

    basegfx::B2DPolyPolygon& GetLineEnd()
    {
        return aB2DPolyPolygon;
    }
};

// ------------------
// class XDashEntry
// ------------------

class XDashEntry : public XPropertyEntry
{
    XDash   aDash;

public:
            XDashEntry(const XDash& rDash, const String& rName) :
                XPropertyEntry(rName), aDash(rDash) {}
            XDashEntry(const XDashEntry& rOther) :
                XPropertyEntry(rOther), aDash(rOther.aDash) {}

    void    SetDash(const XDash& rDash)    { aDash = rDash; }
    XDash&  GetDash()                      { return aDash; }
};

// ------------------
// class XHatchEntry
// ------------------

class XHatchEntry : public XPropertyEntry
{
    XHatch  aHatch;

public:
            XHatchEntry(const XHatch& rHatch, const String& rName) :
                XPropertyEntry(rName), aHatch(rHatch) {}
            XHatchEntry(const XHatchEntry& rOther) :
                XPropertyEntry(rOther), aHatch(rOther.aHatch) {}

    void    SetHatch(const XHatch& rHatch)  { aHatch = rHatch; }
    XHatch& GetHatch()                      { return aHatch; }
};

// ---------------------
// class XGradientEntry
// ---------------------

class XGradientEntry : public XPropertyEntry
{
    XGradient  aGradient;

public:
                XGradientEntry(const XGradient& rGradient, const String& rName):
                    XPropertyEntry(rName), aGradient(rGradient) {}
                XGradientEntry(const XGradientEntry& rOther) :
                    XPropertyEntry(rOther), aGradient(rOther.aGradient) {}

    void        SetGradient(const XGradient& rGrad) { aGradient = rGrad; }
    XGradient&  GetGradient()                       { return aGradient; }
};

// ---------------------
// class XBitmapEntry
// ---------------------

class XBitmapEntry : public XPropertyEntry
{
    XOBitmap aXOBitmap;

public:
            XBitmapEntry( const XOBitmap& rXOBitmap, const String& rName ):
                XPropertyEntry( rName ), aXOBitmap( rXOBitmap ) {}
            XBitmapEntry( const XBitmapEntry& rOther ) :
                XPropertyEntry( rOther ), aXOBitmap( rOther.aXOBitmap ) {}

    void     SetXBitmap(const XOBitmap& rXOBitmap) { aXOBitmap = rXOBitmap; }
    XOBitmap& GetXBitmap()                    { return aXOBitmap; }
};

// ---------------------
// class XPropertyTable
// ---------------------

class SVX_DLLPUBLIC XPropertyTable
{
protected:
    String              aName; // nicht persistent !
    String              aPath;
    XOutdevItemPool*    pXPool;

    Table               aTable;
    Table*              pBmpTable;

    BOOL                bTableDirty;
    BOOL                bBitmapsDirty;
    BOOL                bOwnPool;

                        XPropertyTable( const String& rPath,
                                        XOutdevItemPool* pXPool = NULL,
                                        USHORT nInitSize = 16,
                                        USHORT nReSize = 16 );
                        XPropertyTable( SvStream& rIn );
    void                Clear();

public:
    virtual             ~XPropertyTable();

    long                Count() const;

    BOOL                Insert(long nIndex, XPropertyEntry* pEntry);
    XPropertyEntry*     Replace(long nIndex, XPropertyEntry* pEntry);
    XPropertyEntry*     Remove(long nIndex, USHORT nDummy);
    XPropertyEntry*     Get( long nIndex, USHORT nDummy ) const;

    long                Get(const String& rName);
    Bitmap*             GetBitmap( long nIndex ) const;

    const String&       GetName() const { return aName; }
    void                SetName( const String& rString );
    const String&       GetPath() const { return aPath; }
    void                SetPath( const String& rString ) { aPath = rString; }
    BOOL                IsDirty() const { return bTableDirty && bBitmapsDirty; }
    void                SetDirty( BOOL bDirty = TRUE )
                            { bTableDirty = bDirty; bBitmapsDirty = bDirty; }

    virtual BOOL        Load() = 0;
    virtual BOOL        Save() = 0;
    virtual BOOL        Create() = 0;
    virtual BOOL        CreateBitmapsForUI() = 0;
    virtual Bitmap*     CreateBitmapForUI( long nIndex, BOOL bDelete = TRUE ) = 0;
};

// --------------------
// class XPropertyList
// --------------------

class SVX_DLLPUBLIC XPropertyList
{
protected:
    String              aName; // nicht persistent !
    String              aPath;
    XOutdevItemPool*    pXPool;

    List                aList;
    List*               pBmpList;

    BOOL                bListDirty;
    BOOL                bBitmapsDirty;
    BOOL                bOwnPool;

                        XPropertyList(  const String& rPath,
                                        XOutdevItemPool* pXPool = NULL,
                                        USHORT nInitSize = 16,
                                        USHORT nReSize = 16 );
                        XPropertyList( SvStream& rIn );
    void                Clear();

public:
    virtual             ~XPropertyList();

    long                Count() const;

    void                Insert( XPropertyEntry* pEntry, long nIndex = LIST_APPEND );
    XPropertyEntry*     Replace( XPropertyEntry* pEntry, long nIndex );
    XPropertyEntry*     Remove( long nIndex, USHORT nDummy );
    XPropertyEntry*     Get( long nIndex, USHORT nDummy ) const;

    long                Get(const String& rName);
    Bitmap*             GetBitmap( long nIndex ) const;

    const String&       GetName() const { return aName; }
    void                SetName( const String& rString );
    const String&       GetPath() const { return aPath; }
    void                SetPath( const String& rString ) { aPath = rString; }
    BOOL                IsDirty() const { return bListDirty && bBitmapsDirty; }
    void                SetDirty( BOOL bDirty = TRUE )
                            { bListDirty = bDirty; bBitmapsDirty = bDirty; }

    virtual BOOL        Load() = 0;
    virtual BOOL        Save() = 0;
    virtual BOOL        Create() = 0;
    virtual BOOL        CreateBitmapsForUI() = 0;
    virtual Bitmap*     CreateBitmapForUI( long nIndex, BOOL bDelete = TRUE ) = 0;
};

// ------------------
// class XColorTable
// ------------------

class SVX_DLLPUBLIC XColorTable : public XPropertyTable
{
public:
                    XColorTable( const String& rPath,
                                 XOutdevItemPool* pXPool = NULL,
                                 USHORT nInitSize = 16,
                                 USHORT nReSize = 16 );
    virtual         ~XColorTable();

    using XPropertyTable::Replace;
    XColorEntry*    Replace(long nIndex, XColorEntry* pEntry );
    using XPropertyTable::Remove;
    XColorEntry*    Remove(long nIndex);
    using XPropertyTable::Get;
    XColorEntry*    GetColor(long nIndex) const;

    virtual BOOL    Load();
    virtual BOOL    Save();
    virtual BOOL    Create();
    virtual BOOL    CreateBitmapsForUI();
    virtual Bitmap* CreateBitmapForUI( long nIndex, BOOL bDelete = TRUE );

    static XColorTable* GetStdColorTable();
};

// -------------------
// class XColorList
// -------------------

class XColorList : public XPropertyList
{
public:
                    XColorList( const String& rPath,
                                XOutdevItemPool* pXPool = NULL,
                                USHORT nInitSize = 16,
                                USHORT nReSize = 16 );
    virtual         ~XColorList();

    using XPropertyList::Replace;
    XColorEntry*    Replace(XColorEntry* pEntry, long nIndex );
    using XPropertyList::Remove;
    XColorEntry*    Remove(long nIndex);
    using XPropertyList::Get;
    XColorEntry*    GetColor(long nIndex) const;

    virtual BOOL    Load();
    virtual BOOL    Save();
    virtual BOOL    Create();
    virtual BOOL    CreateBitmapsForUI();
    virtual Bitmap* CreateBitmapForUI( long nIndex, BOOL bDelete = TRUE );
};

// --------------------
// class XLineEndTable
// --------------------

class XLineEndTable : public XPropertyTable
{
public:
                    XLineEndTable( const String& rPath,
                                    XOutdevItemPool* pXPool = NULL,
                                    USHORT nInitSize = 16,
                                    USHORT nReSize = 16 );
    virtual         ~XLineEndTable();

    using XPropertyTable::Replace;
    XLineEndEntry*  Replace(long nIndex, XLineEndEntry* pEntry );
    using XPropertyTable::Remove;
    XLineEndEntry*  Remove(long nIndex);
    using XPropertyTable::Get;
    XLineEndEntry*  GetLineEnd(long nIndex) const;

    virtual BOOL    Load();
    virtual BOOL    Save();
    virtual BOOL    Create();
    virtual BOOL    CreateBitmapsForUI();
    virtual Bitmap* CreateBitmapForUI( long nIndex, BOOL bDelete = TRUE );
};

// -------------------
// class XLineEndList
// -------------------

class SVX_DLLPUBLIC XLineEndList : public XPropertyList
{
protected:
    VirtualDevice*      pVD;
    XOutputDevice*      pXOut;
    XFillAttrSetItem*   pXFSet;
    XLineAttrSetItem*   pXLSet;

public:
                    XLineEndList( const String& rPath,
                                    XOutdevItemPool* pXPool = NULL,
                                    USHORT nInitSize = 16,
                                    USHORT nReSize = 16 );
    virtual         ~XLineEndList();

    using XPropertyList::Replace;
    XLineEndEntry*  Replace(XLineEndEntry* pEntry, long nIndex );
    using XPropertyList::Remove;
    XLineEndEntry*  Remove(long nIndex);
    using XPropertyList::Get;
    XLineEndEntry*  GetLineEnd(long nIndex) const;

    virtual BOOL    Load();
    virtual BOOL    Save();
    virtual BOOL    Create();
    virtual BOOL    CreateBitmapsForUI();
    virtual Bitmap* CreateBitmapForUI( long nIndex, BOOL bDelete = TRUE );
};

// --------------------
// class XDashTable
// --------------------

class XDashTable : public XPropertyTable
{
public:
                    XDashTable( const String& rPath,
                                XOutdevItemPool* pXPool = NULL,
                                USHORT nInitSize = 16,
                                USHORT nReSize = 16 );
    virtual         ~XDashTable();

    using XPropertyTable::Replace;
    XDashEntry*     Replace(long nIndex, XDashEntry* pEntry );
    using XPropertyTable::Remove;
    XDashEntry*     Remove(long nIndex);
    using XPropertyTable::Get;
    XDashEntry*     GetDash(long nIndex) const;

    virtual BOOL    Load();
    virtual BOOL    Save();
    virtual BOOL    Create();
    virtual BOOL    CreateBitmapsForUI();
    virtual Bitmap* CreateBitmapForUI( long nIndex, BOOL bDelete = TRUE );
};

// -------------------
// class XDashList
// -------------------

class SVX_DLLPUBLIC XDashList : public XPropertyList
{
protected:
    VirtualDevice*      pVD;
    XOutputDevice*      pXOut;
    XFillAttrSetItem*   pXFSet;
    XLineAttrSetItem*   pXLSet;

public:
                    XDashList( const String& rPath,
                                XOutdevItemPool* pXPool = NULL,
                                USHORT nInitSize = 16,
                                USHORT nReSize = 16 );
    virtual         ~XDashList();

    using XPropertyList::Replace;
    XDashEntry* Replace(XDashEntry* pEntry, long nIndex );
    using XPropertyList::Remove;
    XDashEntry* Remove(long nIndex);
    using XPropertyList::Get;
    XDashEntry* GetDash(long nIndex) const;

    virtual BOOL    Load();
    virtual BOOL    Save();
    virtual BOOL    Create();
    virtual BOOL    CreateBitmapsForUI();
    virtual Bitmap* CreateBitmapForUI( long nIndex, BOOL bDelete = TRUE );
};

// --------------------
// class XHatchTable
// --------------------

class XHatchTable : public XPropertyTable
{
public:
                    XHatchTable( const String& rPath,
                                    XOutdevItemPool* pXPool = NULL,
                                    USHORT nInitSize = 16,
                                    USHORT nReSize = 16 );
    virtual         ~XHatchTable();

    using XPropertyTable::Replace;
    XHatchEntry*    Replace(long nIndex, XHatchEntry* pEntry );
    using XPropertyTable::Remove;
    XHatchEntry*    Remove(long nIndex);
    using XPropertyTable::Get;
    XHatchEntry*    GetHatch(long nIndex) const;

    virtual BOOL    Load();
    virtual BOOL    Save();
    virtual BOOL    Create();
    virtual BOOL    CreateBitmapsForUI();
    virtual Bitmap* CreateBitmapForUI( long nIndex, BOOL bDelete = TRUE );
};

// -------------------
// class XHatchList
// -------------------

class SVX_DLLPUBLIC XHatchList : public XPropertyList
{
protected:
    VirtualDevice*      pVD;
    XOutputDevice*      pXOut;
    XFillAttrSetItem*   pXFSet;

public:
                    XHatchList( const String& rPath,
                                XOutdevItemPool* pXPool = NULL,
                                USHORT nInitSize = 16,
                                USHORT nReSize = 16 );
                    ~XHatchList();

    using XPropertyList::Replace;
    XHatchEntry*    Replace(XHatchEntry* pEntry, long nIndex );
    using XPropertyList::Remove;
    XHatchEntry*    Remove(long nIndex);
    using XPropertyList::Get;
    XHatchEntry*    GetHatch(long nIndex) const;

    virtual BOOL    Load();
    virtual BOOL    Save();
    virtual BOOL    Create();
    virtual BOOL    CreateBitmapsForUI();
    virtual Bitmap* CreateBitmapForUI( long nIndex, BOOL bDelete = TRUE );
};

// ---------------------
// class XGradientTable
// ---------------------

class XGradientTable : public XPropertyTable
{
public:
                    XGradientTable( const String& rPath,
                                    XOutdevItemPool* pXPool = NULL,
                                    USHORT nInitSize = 16,
                                    USHORT nReSize = 16 );
    virtual         ~XGradientTable();

    using XPropertyTable::Replace;
    XGradientEntry* Replace(long nIndex, XGradientEntry* pEntry );
    using XPropertyTable::Remove;
    XGradientEntry* Remove(long nIndex);
    using XPropertyTable::Get;
    XGradientEntry* GetGradient(long nIndex) const;

    virtual BOOL    Load();
    virtual BOOL    Save();
    virtual BOOL    Create();
    virtual BOOL    CreateBitmapsForUI();
    virtual Bitmap* CreateBitmapForUI( long nIndex, BOOL bDelete = TRUE );
};

// -------------------
// class XGradientList
// -------------------

class SVX_DLLPUBLIC XGradientList : public XPropertyList
{
protected:
    VirtualDevice*      pVD;
    XOutputDevice*      pXOut;
    XFillAttrSetItem*   pXFSet;

public:
                    XGradientList( const String& rPath,
                                    XOutdevItemPool* pXPool = NULL,
                                    USHORT nInitSize = 16,
                                    USHORT nReSize = 16 );
    virtual         ~XGradientList();

    using XPropertyList::Replace;
    XGradientEntry* Replace(XGradientEntry* pEntry, long nIndex );
    using XPropertyList::Remove;
    XGradientEntry* Remove(long nIndex);
    using XPropertyList::Get;
    XGradientEntry* GetGradient(long nIndex) const;

    virtual BOOL    Load();
    virtual BOOL    Save();
    virtual BOOL    Create();
    virtual BOOL    CreateBitmapsForUI();
    virtual Bitmap* CreateBitmapForUI( long nIndex, BOOL bDelete = TRUE );
};

// ---------------------
// class XBitmapTable
// ---------------------

class XBitmapTable : public XPropertyTable
{
public:
                    XBitmapTable( const String& rPath,
                                    XOutdevItemPool* pXPool = NULL,
                                    USHORT nInitSize = 16,
                                    USHORT nReSize = 16 );
    virtual         ~XBitmapTable();

    using XPropertyTable::Replace;
    XBitmapEntry*   Replace(long nIndex, XBitmapEntry* pEntry );
    using XPropertyTable::Remove;
    XBitmapEntry*   Remove(long nIndex);
    using XPropertyTable::Get;
    XBitmapEntry*   GetBitmap(long nIndex) const;

    virtual BOOL    Load();
    virtual BOOL    Save();
    virtual BOOL    Create();
    virtual BOOL    CreateBitmapsForUI();
    virtual Bitmap* CreateBitmapForUI( long nIndex, BOOL bDelete = TRUE );
};

// -------------------
// class XBitmapList
// -------------------

class SVX_DLLPUBLIC XBitmapList : public XPropertyList
{
public:
                    XBitmapList( const String& rPath,
                                    XOutdevItemPool* pXPool = NULL,
                                    USHORT nInitSize = 16,
                                    USHORT nReSize = 16 );
    virtual         ~XBitmapList();

    using XPropertyList::Replace;
    XBitmapEntry*   Replace(XBitmapEntry* pEntry, long nIndex );
    using XPropertyList::Remove;
    XBitmapEntry*   Remove(long nIndex);
    using XPropertyList::Get;
    XBitmapEntry*   GetBitmap(long nIndex) const;

    virtual BOOL    Load();
    virtual BOOL    Save();
    virtual BOOL    Create();
    virtual BOOL    CreateBitmapsForUI();
    virtual Bitmap* CreateBitmapForUI( long nIndex, BOOL bDelete = TRUE );
};

#endif // _XTABLE_HXX
