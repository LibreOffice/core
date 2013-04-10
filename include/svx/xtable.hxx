/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef _XTABLE_HXX
#define _XTABLE_HXX

#include <rtl/ref.hxx>
#include <svx/xpoly.hxx>
#include <svx/xdash.hxx>
#include <svx/xhatch.hxx>
#include <svx/xgrad.hxx>
#include <svx/xflasit.hxx>
#include <svx/xlnasit.hxx>

#include <tools/color.hxx>
#include <tools/string.hxx>
#include <tools/contnr.hxx>

#include <cppuhelper/weak.hxx>

#include <svx/svxdllapi.h>
#include <com/sun/star/embed/XStorage.hpp>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <svtools/grfmgr.hxx>
#include <svx/XPropertyEntry.hxx>

class Color;
class Bitmap;
class VirtualDevice;
class XOutdevItemPool;

// Standard-Vergleichsstring
extern sal_Unicode pszStandard[]; // "standard"

// Funktion zum Konvertieren in echte RGB-Farben, da mit
// enum COL_NAME nicht verglichen werden kann.
SVX_DLLPUBLIC Color RGB_Color( ColorData nColorName );

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
private:
    GraphicObject   maGraphicObject;

public:
    XBitmapEntry(const GraphicObject& rGraphicObject, const String& rName)
    :   XPropertyEntry(rName),
        maGraphicObject(rGraphicObject)
    {
    }

    XBitmapEntry(const XBitmapEntry& rOther)
    :   XPropertyEntry(rOther),
        maGraphicObject(rOther.maGraphicObject)
    {
    }

    const GraphicObject& GetGraphicObject() const
    {
        return maGraphicObject;
    }

    void SetGraphicObject(const GraphicObject& rGraphicObject)
    {
        maGraphicObject = rGraphicObject;
    }
};

// --------------------
// class XPropertyList
// --------------------

enum XPropertyListType {
    XCOLOR_LIST,
    XLINE_END_LIST,
    XDASH_LIST,
    XHATCH_LIST,
    XGRADIENT_LIST,
    XBITMAP_LIST,
    XPROPERTY_LIST_COUNT
};

typedef rtl::Reference< class XPropertyList > XPropertyListRef;

class XDashList ; typedef rtl::Reference< class XDashList > XDashListRef;
class XHatchList ; typedef rtl::Reference< class XHatchList > XHatchListRef;
class XColorList ; typedef rtl::Reference< class XColorList > XColorListRef;
class XBitmapList ; typedef rtl::Reference< class XBitmapList > XBitmapListRef;
class XLineEndList ; typedef rtl::Reference< class XLineEndList > XLineEndListRef;
class XGradientList ; typedef rtl::Reference< class XGradientList > XGradientListRef;

class SVX_DLLPUBLIC XPropertyList : public cppu::OWeakObject
{
 private:
    SAL_DLLPRIVATE void* operator new(size_t);
 protected:
    SAL_DLLPRIVATE void operator delete(void *);
protected:
    typedef ::std::vector< XPropertyEntry* > XPropertyEntryList_impl;
    typedef ::std::vector< Bitmap* > BitmapList_impl;

    XPropertyListType   meType;
    String              maName; // not persistent
    String              maPath;
    XOutdevItemPool*    mpXPool;

    XPropertyEntryList_impl maList;

    bool                mbListDirty;
    bool                mbEmbedInDocument;

                        XPropertyList( XPropertyListType t, const String& rPath,
                                       XOutdevItemPool* pXPool = NULL );

    virtual Bitmap      CreateBitmapForUI( long nIndex ) = 0;

public:
    virtual             ~XPropertyList();

    XPropertyListType   Type() const { return meType; }
    long                Count() const;

    void                Insert( XPropertyEntry* pEntry, long nIndex = CONTAINER_APPEND );
    XPropertyEntry*     Replace( XPropertyEntry* pEntry, long nIndex );
    XPropertyEntry*     Remove( long nIndex );

                        // Note: Get(long) & Get( String& ) are ambiguous
    XPropertyEntry*     Get( long nIndex, sal_uInt16 nDummy ) const;
    long                Get(const String& rName);
    Bitmap              GetUiBitmap( long nIndex ) const;

    const String&       GetName() const { return maName; }
    void                SetName( const String& rString );
    const String&       GetPath() const { return maPath; }
    void                SetPath( const String& rString ) { maPath = rString; }
    bool                IsDirty() const { return mbListDirty; }
    void                SetDirty( bool bDirty = sal_True )
                            { mbListDirty = bDirty; }
    bool                IsEmbedInDocument() const { return mbEmbedInDocument; }
    void                SetEmbedInDocument(bool b) { mbEmbedInDocument = b; }

    static OUString GetDefaultExt(XPropertyListType t);
    static OUString GetDefaultExtFilter(XPropertyListType t);
    OUString        GetDefaultExt() const { return GetDefaultExt( meType ); }

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >
        createInstance() = 0;
    bool                Load();
    bool                LoadFrom( const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::embed::XStorage > &xStorage,
                                  const OUString &rURL );
    bool                Save();
    bool                SaveTo  ( const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::embed::XStorage > &xStorage,
                                  const OUString &rURL,
                                  OUString *pOptName );
    virtual sal_Bool    Create() = 0;

    // Factory method for sub-classes
    static XPropertyListRef CreatePropertyList( XPropertyListType t,
                                                const String& rPath,
                                                XOutdevItemPool* pXPool = NULL );
    // as above but initializes name as expected
    static XPropertyListRef CreatePropertyListFromURL( XPropertyListType t,
                                                       const OUString & rUrl,
                                                       XOutdevItemPool* pXPool = NULL );

    // helper accessors
    inline XDashListRef  AsDashList();
    inline XHatchListRef AsHatchList();
    inline XColorListRef AsColorList();
    inline XBitmapListRef AsBitmapList();
    inline XLineEndListRef AsLineEndList();
    inline XGradientListRef AsGradientList();

    sal_uInt32 getUiBitmapWidth() const;
    sal_uInt32 getUiBitmapHeight() const;
    sal_uInt32 getUiBitmapLineWidth() const;
};

// ------------------
// class XColorList
// ------------------

class SVX_DLLPUBLIC XColorList : public XPropertyList
{
protected:
    virtual Bitmap  CreateBitmapForUI( long nIndex );

public:
    explicit        XColorList( const String& rPath,
                                XOutdevItemPool* pXInPool = NULL ) :
        XPropertyList( XCOLOR_LIST, rPath, pXInPool ) {}

    using XPropertyList::Replace;
    using XPropertyList::Remove;
    using XPropertyList::Get;

    XColorEntry*         Replace(long nIndex, XColorEntry* pEntry );
    XColorEntry*         Remove(long nIndex);
    XColorEntry*         GetColor(long nIndex) const;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > createInstance();
    virtual sal_Bool     Create();

    static XColorListRef CreateStdColorList();
    static XColorListRef GetStdColorList(); // returns a singleton
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

protected:
    virtual Bitmap CreateBitmapForUI(long nIndex);

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
};

// -------------------
// class XDashList
// -------------------
class impXDashList;

class SVX_DLLPUBLIC XDashList : public XPropertyList
{
private:
    impXDashList*       mpData;
    Bitmap              maBitmapSolidLine;
    String              maStringSolidLine;
    String              maStringNoLine;

    void impCreate();
    void impDestroy();

protected:
    Bitmap ImpCreateBitmapForXDash(const XDash* pDash);
    virtual Bitmap CreateBitmapForUI(long nIndex);

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

    // Special call to get a bitmap for the solid line representation. It
    // creates a bitmap fitting in size and style to the ones you get by
    // using GetUiBitmap for existing entries.
    Bitmap GetBitmapForUISolidLine() const;

    // Special calls to get the translated strings for the UI entry for no
    // line style (XLINE_NONE) and solid line style (XLINE_SOLID) for dialogs
    String GetStringForUiSolidLine() const;
    String GetStringForUiNoLine() const;
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

protected:
    virtual Bitmap CreateBitmapForUI(long nIndex);

public:
    explicit XHatchList( const String& rPath,
                         XOutdevItemPool* pXPool = 0 );
    virtual ~XHatchList();

    using XPropertyList::Replace;
    XHatchEntry* Replace(XHatchEntry* pEntry, long nIndex);
    using XPropertyList::Remove;
    XHatchEntry* Remove(long nIndex);
    using XPropertyList::Get;
    XHatchEntry* GetHatch(long nIndex) const;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > createInstance();
    virtual sal_Bool Create();
};

// -------------------
// class XGradientList
// -------------------
class impXGradientList;

class SVX_DLLPUBLIC XGradientList : public XPropertyList
{
private:
    impXGradientList* mpData;

    void impCreate();
    void impDestroy();

protected:
    virtual Bitmap CreateBitmapForUI(long nIndex);

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
};

// -------------------
// class XBitmapList
// -------------------

class SVX_DLLPUBLIC XBitmapList : public XPropertyList
{
protected:
    virtual Bitmap CreateBitmapForUI( long nIndex );

public:
    explicit        XBitmapList( const String& rPath,
                                 XOutdevItemPool* pXInPool = NULL )
                        : XPropertyList( XBITMAP_LIST, rPath, pXInPool ) {}

    using XPropertyList::Replace;
    using XPropertyList::Remove;
    XBitmapEntry*   Remove(long nIndex);
    using XPropertyList::Get;
    XBitmapEntry*   GetBitmap(long nIndex) const;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > createInstance();
    virtual sal_Bool    Create();
};


// FIXME: could add type checking too ...
inline XDashListRef  XPropertyList::AsDashList() { return XDashListRef( static_cast<XDashList *> (this) ); }
inline XHatchListRef XPropertyList::AsHatchList() { return XHatchListRef( static_cast<XHatchList *> (this) ); }
inline XColorListRef XPropertyList::AsColorList() { return XColorListRef( static_cast<XColorList *> (this) ); }
inline XBitmapListRef XPropertyList::AsBitmapList() { return XBitmapListRef( static_cast<XBitmapList *> (this) ); }
inline XLineEndListRef XPropertyList::AsLineEndList() { return XLineEndListRef( static_cast<XLineEndList *> (this) ); }
inline XGradientListRef XPropertyList::AsGradientList() { return XGradientListRef( static_cast<XGradientList *> (this) ); }

#endif // _XTABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
