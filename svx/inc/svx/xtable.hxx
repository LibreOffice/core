/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <tools/color.hxx>
#include <tools/string.hxx>

#include <tools/table.hxx>
#include "svx/svxdllapi.h"
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <com/sun/star/container/XNameContainer.hpp>

class Color;
class Bitmap;
class VirtualDevice;
class XOutdevItemPool;

// Breite und Hoehe der LB-Bitmaps
#define BITMAP_WIDTH  32
#define BITMAP_HEIGHT 12

// Standard-Vergleichsstring
extern sal_Unicode pszStandard[]; // "standard"

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
            XColorEntry(const Color& rColor, const String& rName)
                : XPropertyEntry(rName)
                , aColor(rColor)
                {}

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

    void     SetXBitmap(const XOBitmap& rXOBitmap) { aXOBitmap = rXOBitmap; }
    XOBitmap& GetXBitmap()                    { return aXOBitmap; }
};

// --------------------
// class XPropertyList
// --------------------

class SVX_DLLPUBLIC XPropertyList
{
protected:
    typedef ::std::vector< XPropertyEntry* > XPropertyEntryList_impl;
    typedef ::std::vector< Bitmap* > BitmapList_impl;

    String              aName; // not persistent
    String              aPath;
    XOutdevItemPool*    pXPool;
    const char *        pDefaultExt;

    XPropertyEntryList_impl aList;
    BitmapList_impl*        pBmpList;

    sal_Bool            bListDirty;
    sal_Bool            bBitmapsDirty;
    sal_Bool            bOwnPool;

                        XPropertyList(
                            const char *_pDefaultExtension,
                            const String& rPath,
                            XOutdevItemPool* pXPool = NULL
                        );
    void                Clear();

public:
    virtual             ~XPropertyList();

    long                Count() const;

    void                Insert( XPropertyEntry* pEntry, long nIndex = LIST_APPEND );
    XPropertyEntry*     Replace( XPropertyEntry* pEntry, long nIndex );
    XPropertyEntry*     Remove( long nIndex );

                        // Note: Get(long) & Get( String& ) are ambiguous
    XPropertyEntry*     Get( long nIndex, sal_uInt16 nDummy ) const;
    long                Get(const String& rName);

    Bitmap*             GetBitmap( long nIndex ) const;

    const String&       GetName() const { return aName; }
    void                SetName( const String& rString );
    const String&       GetPath() const { return aPath; }
    void                SetPath( const String& rString ) { aPath = rString; }
    sal_Bool            IsDirty() const { return bListDirty && bBitmapsDirty; }
    void                SetDirty( sal_Bool bDirty = sal_True )
                            { bListDirty = bDirty; bBitmapsDirty = bDirty; }

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >
        createInstance() = 0;
    sal_Bool            Load();
    sal_Bool            Save();
    virtual sal_Bool    Create() = 0;
    virtual sal_Bool    CreateBitmapsForUI() = 0;
    virtual Bitmap*     CreateBitmapForUI( long nIndex, sal_Bool bDelete = sal_True ) = 0;
};

// ------------------
// class XColorList
// ------------------

class SVX_DLLPUBLIC XColorList : public XPropertyList
{
public:
    explicit        XColorList(
                        const String& rPath,
                        XOutdevItemPool* pXPool = NULL
                    );
    virtual         ~XColorList();

    using XPropertyList::Replace;
    using XPropertyList::Remove;
    using XPropertyList::Get;

    XColorEntry*        Replace(long nIndex, XColorEntry* pEntry );
    XColorEntry*        Remove(long nIndex);
    XColorEntry*        GetColor(long nIndex) const;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > createInstance();
    virtual sal_Bool    Create();
    virtual sal_Bool    CreateBitmapsForUI();
    virtual Bitmap*     CreateBitmapForUI( long nIndex, sal_Bool bDelete = sal_True );

    static XColorList& GetStdColorTable();
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
    explicit        XLineEndList(
                        const String& rPath,
                        XOutdevItemPool* pXPool = 0
                    );
    virtual         ~XLineEndList();

    using XPropertyList::Remove;
    XLineEndEntry* Remove(long nIndex);
    using XPropertyList::Get;
    XLineEndEntry* GetLineEnd(long nIndex) const;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > createInstance();
    virtual sal_Bool Create();
    virtual sal_Bool CreateBitmapsForUI();
    virtual Bitmap* CreateBitmapForUI(long nIndex, sal_Bool bDelete = sal_True);
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
    explicit            XDashList(
                            const String& rPath,
                            XOutdevItemPool* pXPool = 0
                        );
    virtual             ~XDashList();

    using XPropertyList::Replace;
    XDashEntry*         Replace(XDashEntry* pEntry, long nIndex);
    using XPropertyList::Remove;
    XDashEntry*         Remove(long nIndex);
    using XPropertyList::Get;
    XDashEntry*         GetDash(long nIndex) const;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > createInstance();
    virtual sal_Bool    Create();
    virtual sal_Bool    CreateBitmapsForUI();
    virtual Bitmap*     CreateBitmapForUI(long nIndex, sal_Bool bDelete = sal_True);
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
    explicit        XHatchList(
                        const String& rPath,
                        XOutdevItemPool* pXPool = 0
                    );
    ~XHatchList();

    using XPropertyList::Replace;
    XHatchEntry* Replace(XHatchEntry* pEntry, long nIndex);
    using XPropertyList::Remove;
    XHatchEntry* Remove(long nIndex);
    using XPropertyList::Get;
    XHatchEntry* GetHatch(long nIndex) const;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > createInstance();
    virtual sal_Bool Create();
    virtual sal_Bool CreateBitmapsForUI();
    virtual Bitmap* CreateBitmapForUI(long nIndex, sal_Bool bDelete = sal_True);
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
    explicit    XGradientList(
                    const String& rPath,
                    XOutdevItemPool* pXPool = 0
                );
    virtual     ~XGradientList();

    using XPropertyList::Replace;
    XGradientEntry*     Replace(XGradientEntry* pEntry, long nIndex);
    using XPropertyList::Remove;
    XGradientEntry*     Remove(long nIndex);
    using XPropertyList::Get;
    XGradientEntry*     GetGradient(long nIndex) const;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > createInstance();
    virtual sal_Bool    Create();
    virtual sal_Bool    CreateBitmapsForUI();
    virtual Bitmap*     CreateBitmapForUI(long nIndex, sal_Bool bDelete = sal_True);
};

// -------------------
// class XBitmapList
// -------------------

class SVX_DLLPUBLIC XBitmapList : public XPropertyList
{
public:
    explicit        XBitmapList(
                        const String& rPath,
                        XOutdevItemPool* pXPool = NULL
                    );
    virtual         ~XBitmapList();

    using XPropertyList::Replace;
    using XPropertyList::Remove;
    XBitmapEntry*   Remove(long nIndex);
    using XPropertyList::Get;
    XBitmapEntry*   GetBitmap(long nIndex) const;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > createInstance();
    virtual sal_Bool    Create();
    virtual sal_Bool    CreateBitmapsForUI();
    virtual Bitmap*     CreateBitmapForUI( long nIndex, sal_Bool bDelete = sal_True );
};

#endif // _XTABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
