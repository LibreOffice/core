/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _XTABLE_HXX
#define _XTABLE_HXX

// include ---------------------------------------------------------------

#include <svx/xpoly.hxx>
#include <svx/xdash.hxx>
#include <svx/xhatch.hxx>
#include <svx/xgrad.hxx>
#include <svx/xbitmap.hxx>
#include <svx/xflasit.hxx>
#include <svx/xlnasit.hxx>

#ifndef _SV_COLOR_HXX
#include <tools/color.hxx>
#endif
#include <tools/string.hxx>

#ifndef _TABLE_HXX //autogen
#include <tools/table.hxx>
#endif
#include "svx/svxdllapi.h"
#include <basegfx/polygon/b2dpolypolygon.hxx>

class Color;
class Bitmap;
class VirtualDevice;
class XOutdevItemPool;

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

    sal_Bool                bTableDirty;
    sal_Bool                bBitmapsDirty;
    sal_Bool                bOwnPool;

                        XPropertyTable( const String& rPath,
                                        XOutdevItemPool* pXPool = NULL,
                                        sal_uInt16 nInitSize = 16,
                                        sal_uInt16 nReSize = 16 );
                        XPropertyTable( SvStream& rIn );
    void                Clear();

public:
    virtual             ~XPropertyTable();

    long                Count() const;

    sal_Bool                Insert(long nIndex, XPropertyEntry* pEntry);
    XPropertyEntry*     Replace(long nIndex, XPropertyEntry* pEntry);
    XPropertyEntry*     Remove(long nIndex, sal_uInt16 nDummy);
    XPropertyEntry*     Get( long nIndex, sal_uInt16 nDummy ) const;

    long                Get(const String& rName);
    Bitmap*             GetBitmap( long nIndex ) const;

    const String&       GetName() const { return aName; }
    void                SetName( const String& rString );
    const String&       GetPath() const { return aPath; }
    void                SetPath( const String& rString ) { aPath = rString; }
    sal_Bool                IsDirty() const { return bTableDirty && bBitmapsDirty; }
    void                SetDirty( sal_Bool bDirty = sal_True )
                            { bTableDirty = bDirty; bBitmapsDirty = bDirty; }

    virtual sal_Bool        Load() = 0;
    virtual sal_Bool        Save() = 0;
    virtual sal_Bool        Create() = 0;
    virtual sal_Bool        CreateBitmapsForUI() = 0;
    virtual Bitmap*     CreateBitmapForUI( long nIndex, sal_Bool bDelete = sal_True ) = 0;
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

    sal_Bool                bListDirty;
    sal_Bool                bBitmapsDirty;
    sal_Bool                bOwnPool;

                        XPropertyList(  const String& rPath,
                                        XOutdevItemPool* pXPool = NULL,
                                        sal_uInt16 nInitSize = 16,
                                        sal_uInt16 nReSize = 16 );
                        XPropertyList( SvStream& rIn );
    void                Clear();

public:
    virtual             ~XPropertyList();

    long                Count() const;

    void                Insert( XPropertyEntry* pEntry, long nIndex = LIST_APPEND );
    XPropertyEntry*     Replace( XPropertyEntry* pEntry, long nIndex );
    XPropertyEntry*     Remove( long nIndex, sal_uInt16 nDummy );
    XPropertyEntry*     Get( long nIndex, sal_uInt16 nDummy ) const;

    long                Get(const String& rName);
    Bitmap*             GetBitmap( long nIndex ) const;

    const String&       GetName() const { return aName; }
    void                SetName( const String& rString );
    const String&       GetPath() const { return aPath; }
    void                SetPath( const String& rString ) { aPath = rString; }
    sal_Bool                IsDirty() const { return bListDirty && bBitmapsDirty; }
    void                SetDirty( sal_Bool bDirty = sal_True )
                            { bListDirty = bDirty; bBitmapsDirty = bDirty; }

    virtual sal_Bool        Load() = 0;
    virtual sal_Bool        Save() = 0;
    virtual sal_Bool        Create() = 0;
    virtual sal_Bool        CreateBitmapsForUI() = 0;
    virtual Bitmap*     CreateBitmapForUI( long nIndex, sal_Bool bDelete = sal_True ) = 0;
};

// ------------------
// class XColorTable
// ------------------

class SVX_DLLPUBLIC XColorTable : public XPropertyTable
{
public:
                    XColorTable( const String& rPath,
                                 XOutdevItemPool* pXPool = NULL,
                                 sal_uInt16 nInitSize = 16,
                                 sal_uInt16 nReSize = 16 );
    virtual         ~XColorTable();

    using XPropertyTable::Replace;
    XColorEntry*    Replace(long nIndex, XColorEntry* pEntry );
    using XPropertyTable::Remove;
    XColorEntry*    Remove(long nIndex);
    using XPropertyTable::Get;
    XColorEntry*    GetColor(long nIndex) const;

    virtual sal_Bool    Load();
    virtual sal_Bool    Save();
    virtual sal_Bool    Create();
    virtual sal_Bool    CreateBitmapsForUI();
    virtual Bitmap* CreateBitmapForUI( long nIndex, sal_Bool bDelete = sal_True );

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
                                sal_uInt16 nInitSize = 16,
                                sal_uInt16 nReSize = 16 );
    virtual         ~XColorList();

    using XPropertyList::Replace;
    XColorEntry*    Replace(XColorEntry* pEntry, long nIndex );
    using XPropertyList::Remove;
    XColorEntry*    Remove(long nIndex);
    using XPropertyList::Get;
    XColorEntry*    GetColor(long nIndex) const;

    virtual sal_Bool    Load();
    virtual sal_Bool    Save();
    virtual sal_Bool    Create();
    virtual sal_Bool    CreateBitmapsForUI();
    virtual Bitmap* CreateBitmapForUI( long nIndex, sal_Bool bDelete = sal_True );
};

// --------------------
// class XLineEndTable
// --------------------

class XLineEndTable : public XPropertyTable
{
public:
                    XLineEndTable( const String& rPath,
                                    XOutdevItemPool* pXPool = NULL,
                                    sal_uInt16 nInitSize = 16,
                                    sal_uInt16 nReSize = 16 );
    virtual         ~XLineEndTable();

    using XPropertyTable::Replace;
    XLineEndEntry*  Replace(long nIndex, XLineEndEntry* pEntry );
    using XPropertyTable::Remove;
    XLineEndEntry*  Remove(long nIndex);
    using XPropertyTable::Get;
    XLineEndEntry*  GetLineEnd(long nIndex) const;

    virtual sal_Bool    Load();
    virtual sal_Bool    Save();
    virtual sal_Bool    Create();
    virtual sal_Bool    CreateBitmapsForUI();
    virtual Bitmap* CreateBitmapForUI( long nIndex, sal_Bool bDelete = sal_True );
};

// -------------------
// class XLineEndList
// -------------------
class impXLineEndList;

class SVX_DLLPUBLIC XLineEndList : public XPropertyList
{
private:
    impXLineEndList*    mpData;

    void impCreate();
    void impDestroy();

public:
    XLineEndList(const String& rPath, XOutdevItemPool* pXPool = 0, sal_uInt16 nInitSize = 16, sal_uInt16 nReSize = 16);
    virtual ~XLineEndList();

    using XPropertyList::Replace;
    XLineEndEntry* Replace(XLineEndEntry* pEntry, long nIndex);
    using XPropertyList::Remove;
    XLineEndEntry* Remove(long nIndex);
    using XPropertyList::Get;
    XLineEndEntry* GetLineEnd(long nIndex) const;

    virtual sal_Bool Load();
    virtual sal_Bool Save();
    virtual sal_Bool Create();
    virtual sal_Bool CreateBitmapsForUI();
    virtual Bitmap* CreateBitmapForUI(long nIndex, sal_Bool bDelete = sal_True);
};

// --------------------
// class XDashTable
// --------------------

class XDashTable : public XPropertyTable
{
public:
                    XDashTable( const String& rPath,
                                XOutdevItemPool* pXPool = NULL,
                                sal_uInt16 nInitSize = 16,
                                sal_uInt16 nReSize = 16 );
    virtual         ~XDashTable();

    using XPropertyTable::Replace;
    XDashEntry*     Replace(long nIndex, XDashEntry* pEntry );
    using XPropertyTable::Remove;
    XDashEntry*     Remove(long nIndex);
    using XPropertyTable::Get;
    XDashEntry*     GetDash(long nIndex) const;

    virtual sal_Bool    Load();
    virtual sal_Bool    Save();
    virtual sal_Bool    Create();
    virtual sal_Bool    CreateBitmapsForUI();
    virtual Bitmap* CreateBitmapForUI( long nIndex, sal_Bool bDelete = sal_True );
};

// -------------------
// class XDashList
// -------------------
class impXDashList;

class SVX_DLLPUBLIC XDashList : public XPropertyList
{
private:
    impXDashList*       mpData;

    void impCreate();
    void impDestroy();

public:
    XDashList(const String& rPath, XOutdevItemPool* pXPool = 0, sal_uInt16 nInitSize = 16, sal_uInt16 nReSize = 16);
    virtual ~XDashList();

    using XPropertyList::Replace;
    XDashEntry* Replace(XDashEntry* pEntry, long nIndex);
    using XPropertyList::Remove;
    XDashEntry* Remove(long nIndex);
    using XPropertyList::Get;
    XDashEntry* GetDash(long nIndex) const;

    virtual sal_Bool Load();
    virtual sal_Bool Save();
    virtual sal_Bool Create();
    virtual sal_Bool CreateBitmapsForUI();
    virtual Bitmap* CreateBitmapForUI(long nIndex, sal_Bool bDelete = sal_True);
};

// --------------------
// class XHatchTable
// --------------------

class XHatchTable : public XPropertyTable
{
public:
                    XHatchTable( const String& rPath,
                                    XOutdevItemPool* pXPool = NULL,
                                    sal_uInt16 nInitSize = 16,
                                    sal_uInt16 nReSize = 16 );
    virtual         ~XHatchTable();

    using XPropertyTable::Replace;
    XHatchEntry*    Replace(long nIndex, XHatchEntry* pEntry );
    using XPropertyTable::Remove;
    XHatchEntry*    Remove(long nIndex);
    using XPropertyTable::Get;
    XHatchEntry*    GetHatch(long nIndex) const;

    virtual sal_Bool    Load();
    virtual sal_Bool    Save();
    virtual sal_Bool    Create();
    virtual sal_Bool    CreateBitmapsForUI();
    virtual Bitmap* CreateBitmapForUI( long nIndex, sal_Bool bDelete = sal_True );
};

// -------------------
// class XHatchList
// -------------------
class impXHatchList;

class SVX_DLLPUBLIC XHatchList : public XPropertyList
{
private:
    impXHatchList*      mpData;

    void impCreate();
    void impDestroy();

public:
    XHatchList(const String& rPath, XOutdevItemPool* pXPool = 0, sal_uInt16 nInitSize = 16, sal_uInt16 nReSize = 16);
    ~XHatchList();

    using XPropertyList::Replace;
    XHatchEntry* Replace(XHatchEntry* pEntry, long nIndex);
    using XPropertyList::Remove;
    XHatchEntry* Remove(long nIndex);
    using XPropertyList::Get;
    XHatchEntry* GetHatch(long nIndex) const;

    virtual sal_Bool Load();
    virtual sal_Bool Save();
    virtual sal_Bool Create();
    virtual sal_Bool CreateBitmapsForUI();
    virtual Bitmap* CreateBitmapForUI(long nIndex, sal_Bool bDelete = sal_True);
};

// ---------------------
// class XGradientTable
// ---------------------

class XGradientTable : public XPropertyTable
{
public:
                    XGradientTable( const String& rPath,
                                    XOutdevItemPool* pXPool = NULL,
                                    sal_uInt16 nInitSize = 16,
                                    sal_uInt16 nReSize = 16 );
    virtual         ~XGradientTable();

    using XPropertyTable::Replace;
    XGradientEntry* Replace(long nIndex, XGradientEntry* pEntry );
    using XPropertyTable::Remove;
    XGradientEntry* Remove(long nIndex);
    using XPropertyTable::Get;
    XGradientEntry* GetGradient(long nIndex) const;

    virtual sal_Bool    Load();
    virtual sal_Bool    Save();
    virtual sal_Bool    Create();
    virtual sal_Bool    CreateBitmapsForUI();
    virtual Bitmap* CreateBitmapForUI( long nIndex, sal_Bool bDelete = sal_True );
};

// -------------------
// class XGradientList
// -------------------
class impXGradientList;

class SVX_DLLPUBLIC XGradientList : public XPropertyList
{
private:
    impXGradientList*   mpData;

    void impCreate();
    void impDestroy();

public:
    XGradientList(const String& rPath, XOutdevItemPool* pXPool = 0, sal_uInt16 nInitSize = 16, sal_uInt16 nReSize = 16);
    virtual ~XGradientList();

    using XPropertyList::Replace;
    XGradientEntry* Replace(XGradientEntry* pEntry, long nIndex);
    using XPropertyList::Remove;
    XGradientEntry* Remove(long nIndex);
    using XPropertyList::Get;
    XGradientEntry* GetGradient(long nIndex) const;

    virtual sal_Bool Load();
    virtual sal_Bool Save();
    virtual sal_Bool Create();
    virtual sal_Bool CreateBitmapsForUI();
    virtual Bitmap* CreateBitmapForUI(long nIndex, sal_Bool bDelete = sal_True);
};

// ---------------------
// class XBitmapTable
// ---------------------

class XBitmapTable : public XPropertyTable
{
public:
                    XBitmapTable( const String& rPath,
                                    XOutdevItemPool* pXPool = NULL,
                                    sal_uInt16 nInitSize = 16,
                                    sal_uInt16 nReSize = 16 );
    virtual         ~XBitmapTable();

    using XPropertyTable::Replace;
    XBitmapEntry*   Replace(long nIndex, XBitmapEntry* pEntry );
    using XPropertyTable::Remove;
    XBitmapEntry*   Remove(long nIndex);
    using XPropertyTable::Get;
    XBitmapEntry*   GetBitmap(long nIndex) const;

    virtual sal_Bool    Load();
    virtual sal_Bool    Save();
    virtual sal_Bool    Create();
    virtual sal_Bool    CreateBitmapsForUI();
    virtual Bitmap* CreateBitmapForUI( long nIndex, sal_Bool bDelete = sal_True );
};

// -------------------
// class XBitmapList
// -------------------

class SVX_DLLPUBLIC XBitmapList : public XPropertyList
{
public:
                    XBitmapList( const String& rPath,
                                    XOutdevItemPool* pXPool = NULL,
                                    sal_uInt16 nInitSize = 16,
                                    sal_uInt16 nReSize = 16 );
    virtual         ~XBitmapList();

    using XPropertyList::Replace;
    XBitmapEntry*   Replace(XBitmapEntry* pEntry, long nIndex );
    using XPropertyList::Remove;
    XBitmapEntry*   Remove(long nIndex);
    using XPropertyList::Get;
    XBitmapEntry*   GetBitmap(long nIndex) const;

    virtual sal_Bool    Load();
    virtual sal_Bool    Save();
    virtual sal_Bool    Create();
    virtual sal_Bool    CreateBitmapsForUI();
    virtual Bitmap* CreateBitmapForUI( long nIndex, sal_Bool bDelete = sal_True );
};

#endif // _XTABLE_HXX
