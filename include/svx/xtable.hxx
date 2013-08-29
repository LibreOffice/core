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

// Standard-Vergleichsstring
extern sal_Unicode pszStandard[]; // "standard"

// Funktion zum Konvertieren in echte RGB-Farben, da mit
// enum COL_NAME nicht verglichen werden kann.
SVX_DLLPUBLIC Color RGB_Color( ColorData nColorName );

// ------------------
// class XColorEntry
// ------------------

class SVX_DLLPUBLIC XColorEntry : public XPropertyEntry
{
private:
    Color   aColor;

public:
    XColorEntry(const Color& rColor, const OUString& rName);
    XColorEntry(const XColorEntry& rOther);

    const Color& GetColor() const
    {
        return aColor;
    }
};

// --------------------
// class XLineEndEntry
// --------------------

class SVX_DLLPUBLIC XLineEndEntry : public XPropertyEntry
{
private:
    basegfx::B2DPolyPolygon aB2DPolyPolygon;

public:
    XLineEndEntry(const basegfx::B2DPolyPolygon& rB2DPolyPolygon, const OUString& rName);
    XLineEndEntry(const XLineEndEntry& rOther);

    const basegfx::B2DPolyPolygon& GetLineEnd() const
    {
        return aB2DPolyPolygon;
    }
};

// ------------------
// class XDashEntry
// ------------------

class SVX_DLLPUBLIC XDashEntry : public XPropertyEntry
{
private:
    XDash   aDash;

public:
    XDashEntry(const XDash& rDash, const OUString& rName);
    XDashEntry(const XDashEntry& rOther);

    const XDash& GetDash() const
    {
        return aDash;
    }
};

// ------------------
// class XHatchEntry
// ------------------

class SVX_DLLPUBLIC XHatchEntry : public XPropertyEntry
{
private:
    XHatch  aHatch;

public:
    XHatchEntry(const XHatch& rHatch, const OUString& rName);
    XHatchEntry(const XHatchEntry& rOther);

    const XHatch& GetHatch() const
    {
        return aHatch;
    }
};

// ---------------------
// class XGradientEntry
// ---------------------

class SVX_DLLPUBLIC XGradientEntry : public XPropertyEntry
{
private:
    XGradient  aGradient;

public:
    XGradientEntry(const XGradient& rGradient, const OUString& rName);
    XGradientEntry(const XGradientEntry& rOther);

    const XGradient& GetGradient() const
    {
        return aGradient;
    }
};

// ---------------------
// class XBitmapEntry
// ---------------------

class SVX_DLLPUBLIC XBitmapEntry : public XPropertyEntry
{
private:
    GraphicObject   maGraphicObject;

public:
    XBitmapEntry(const GraphicObject& rGraphicObject, const OUString& rName);
    XBitmapEntry(const XBitmapEntry& rOther);

    const GraphicObject& GetGraphicObject() const
    {
        return maGraphicObject;
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
    OUString            maName; // not persistent
    OUString            maPath;

    XPropertyEntryList_impl maList;

    bool                mbListDirty;
    bool                mbEmbedInDocument;

    XPropertyList(XPropertyListType t, const OUString& rPath);

    virtual Bitmap CreateBitmapForUI(long nIndex) = 0;

public:
    virtual ~XPropertyList();

    XPropertyListType Type() const { return meType; }
    long Count() const;

    void Insert(XPropertyEntry* pEntry, long nIndex = CONTAINER_APPEND);
    XPropertyEntry* Replace(XPropertyEntry* pEntry, long nIndex);
    XPropertyEntry* Remove(long nIndex);

    XPropertyEntry* Get(long nIndex) const;
    long GetIndex(const OUString& rName) const;
    Bitmap GetUiBitmap(long nIndex) const;

    const OUString& GetName() const { return maName; }
    void SetName(const OUString& rString);

    const OUString& GetPath() const { return maPath; }
    void SetPath(const OUString& rString) { maPath = rString; }

    bool IsDirty() const { return mbListDirty; }
    void SetDirty(bool bDirty = true) { mbListDirty = bDirty; }

    bool IsEmbedInDocument() const { return mbEmbedInDocument; }
    void SetEmbedInDocument(bool b) { mbEmbedInDocument = b; }

    static OUString GetDefaultExt(XPropertyListType t);
    static OUString GetDefaultExtFilter(XPropertyListType t);
    OUString GetDefaultExt() const { return GetDefaultExt(meType); }

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >
        createInstance() = 0;
    bool Load();
    bool LoadFrom(const ::com::sun::star::uno::Reference<
                       ::com::sun::star::embed::XStorage > &xStorage,
                   const OUString &rURL);
    bool Save();
    bool SaveTo  (const ::com::sun::star::uno::Reference<
                       ::com::sun::star::embed::XStorage > &xStorage,
                   const OUString &rURL,
                                  OUString *pOptName);
    virtual bool Create() = 0;

    // Factory method for sub-classes
    static XPropertyListRef CreatePropertyList(XPropertyListType t,
                                                const OUString& rPath);
    // as above but initializes name as expected
    static XPropertyListRef CreatePropertyListFromURL(XPropertyListType t,
                                                       const OUString & rUrl);

    // helper accessors
    inline XDashListRef  AsDashList();
    inline XHatchListRef AsHatchList();
    inline XColorListRef AsColorList();
    inline XBitmapListRef AsBitmapList();
    inline XLineEndListRef AsLineEndList();
    inline XGradientListRef AsGradientList();
};

// ------------------
// class XColorList
// ------------------

class SVX_DLLPUBLIC XColorList : public XPropertyList
{
protected:
    virtual Bitmap  CreateBitmapForUI(long nIndex);

public:
    explicit XColorList(const OUString& rPath)
        : XPropertyList(XCOLOR_LIST, rPath) {}

    using XPropertyList::Replace;
    using XPropertyList::Remove;

    XColorEntry* Replace(long nIndex, XColorEntry* pEntry);
    XColorEntry* Remove(long nIndex);
    XColorEntry* GetColor(long nIndex) const;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > createInstance();
    virtual bool Create();

    static XColorListRef CreateStdColorList();
    static XColorListRef GetStdColorList(); // returns a singleton
};

// -------------------
// class XLineEndList
// -------------------

class SVX_DLLPUBLIC XLineEndList : public XPropertyList
{
protected:
    virtual Bitmap CreateBitmapForUI(long nIndex);

public:
    explicit XLineEndList(const OUString& rPath);
    virtual ~XLineEndList();

    using XPropertyList::Remove;
    XLineEndEntry* Remove(long nIndex);
    XLineEndEntry* GetLineEnd(long nIndex) const;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > createInstance();
    virtual bool Create();
};

// -------------------
// class XDashList
// -------------------

class SVX_DLLPUBLIC XDashList : public XPropertyList
{
private:
    Bitmap              maBitmapSolidLine;
    OUString            maStringSolidLine;
    OUString            maStringNoLine;

protected:
    Bitmap ImpCreateBitmapForXDash(const XDash* pDash);
    virtual Bitmap CreateBitmapForUI(long nIndex);

public:
    explicit XDashList(const OUString& rPath);
    virtual ~XDashList();

    using XPropertyList::Replace;
    XDashEntry* Replace(XDashEntry* pEntry, long nIndex);
    using XPropertyList::Remove;
    XDashEntry* Remove(long nIndex);
    XDashEntry* GetDash(long nIndex) const;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > createInstance();
    virtual bool Create();

    // Special call to get a bitmap for the solid line representation. It
    // creates a bitmap fitting in size and style to the ones you get by
    // using GetUiBitmap for existing entries.
    Bitmap GetBitmapForUISolidLine() const;

    // Special calls to get the translated strings for the UI entry for no
    // line style (XLINE_NONE) and solid line style (XLINE_SOLID) for dialogs
    OUString GetStringForUiSolidLine() const;
    OUString GetStringForUiNoLine() const;
};

// -------------------
// class XHatchList
// -------------------

class SVX_DLLPUBLIC XHatchList : public XPropertyList
{
protected:
    virtual Bitmap CreateBitmapForUI(long nIndex);

public:
    explicit XHatchList(const OUString& rPath);
    virtual ~XHatchList();

    using XPropertyList::Replace;
    XHatchEntry* Replace(XHatchEntry* pEntry, long nIndex);
    using XPropertyList::Remove;
    XHatchEntry* Remove(long nIndex);
    XHatchEntry* GetHatch(long nIndex) const;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > createInstance();
    virtual bool Create();
};

// -------------------
// class XGradientList
// -------------------
class SVX_DLLPUBLIC XGradientList : public XPropertyList
{
protected:
    virtual Bitmap CreateBitmapForUI(long nIndex);

public:
    explicit XGradientList(const OUString& rPath);
    virtual ~XGradientList();

    using XPropertyList::Replace;
    XGradientEntry* Replace(XGradientEntry* pEntry, long nIndex);
    using XPropertyList::Remove;
    XGradientEntry* Remove(long nIndex);
    XGradientEntry* GetGradient(long nIndex) const;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > createInstance();
    virtual bool Create();
};

// -------------------
// class XBitmapList
// -------------------

class SVX_DLLPUBLIC XBitmapList : public XPropertyList
{
protected:
    virtual Bitmap CreateBitmapForUI(long nIndex);

public:
    explicit XBitmapList(const OUString& rPath)
        : XPropertyList(XBITMAP_LIST, rPath) {}

    using XPropertyList::Replace;
    using XPropertyList::Remove;
    XBitmapEntry* Remove(long nIndex);
    XBitmapEntry* GetBitmap(long nIndex) const;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > createInstance();
    virtual bool Create();
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
