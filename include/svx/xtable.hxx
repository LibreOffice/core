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
#ifndef INCLUDED_SVX_XTABLE_HXX
#define INCLUDED_SVX_XTABLE_HXX

#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <svx/xpoly.hxx>
#include <svx/xdash.hxx>
#include <svx/xhatch.hxx>
#include <svx/xgrad.hxx>
#include <svx/xflasit.hxx>
#include <svx/xlnasit.hxx>

#include <tools/color.hxx>
#include <tools/contnr.hxx>

#include <cppuhelper/weak.hxx>

#include <svx/svxdllapi.h>
#include <com/sun/star/embed/XStorage.hpp>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <svtools/grfmgr.hxx>
#include <svx/XPropertyEntry.hxx>

#include <limits>

class Color;
class Bitmap;
class VirtualDevice;

// standard reference string
extern sal_Unicode pszStandard[]; // "standard"

// function to convert in real RGB_Colours;
// it can't be compared with enum COL_NAME
SVX_DLLPUBLIC Color RGB_Color( ColorData nColorName );

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

enum XPropertyListType {
    UNKNOWN_XPROPERTYLISTTYPE = -1,
    XCOLOR_LIST,
    XLINE_END_LIST,
    XDASH_LIST,
    XHATCH_LIST,
    XGRADIENT_LIST,
    XBITMAP_LIST,
    XPATTERN_LIST,
    XPROPERTY_LIST_COUNT
};

typedef rtl::Reference< class XPropertyList > XPropertyListRef;

class XDashList ; typedef rtl::Reference< class XDashList > XDashListRef;
class XHatchList ; typedef rtl::Reference< class XHatchList > XHatchListRef;
class XColorList ; typedef rtl::Reference< class XColorList > XColorListRef;
class XBitmapList ; typedef rtl::Reference< class XBitmapList > XBitmapListRef;
class XPatternList ; typedef rtl::Reference< class XPatternList > XPatternListRef;
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

    XPropertyListType   meType;
    OUString            maName; // not persistent
    OUString            maPath;
    OUString            maReferer;

    XPropertyEntryList_impl maList;

    bool                mbListDirty;
    bool                mbEmbedInDocument;

    XPropertyList(XPropertyListType t, const OUString& rPath, const OUString& rReferer);

    virtual Bitmap CreateBitmapForUI(long nIndex) = 0;

public:
    virtual ~XPropertyList();

    XPropertyListType Type() const { return meType; }
    long Count() const;

    void Insert(XPropertyEntry* pEntry, long nIndex = ::std::numeric_limits<long>::max());
    XPropertyEntry* Replace(XPropertyEntry* pEntry, long nIndex);
    XPropertyEntry* Remove(long nIndex);

    XPropertyEntry* Get(long nIndex) const;
    long GetIndex(const OUString& rName) const;
    Bitmap GetUiBitmap(long nIndex) const;

    const OUString& GetName() const { return maName; }
    void SetName(const OUString& rString);

    const OUString& GetPath() const { return maPath; }
    void SetPath(const OUString& rString) { maPath = rString; }

    void SetDirty(bool bDirty = true) { mbListDirty = bDirty; }

    bool IsEmbedInDocument() const { return mbEmbedInDocument; }
    void SetEmbedInDocument(bool b) { mbEmbedInDocument = b; }

    static OUString GetDefaultExt(XPropertyListType t);
    static OUString GetDefaultExtFilter(XPropertyListType t);
    OUString GetDefaultExt() const { return GetDefaultExt(meType); }

    virtual css::uno::Reference< css::container::XNameContainer >
        createInstance() = 0;
    bool Load();
    bool LoadFrom(const css::uno::Reference<
                       css::embed::XStorage > &xStorage,
                  const OUString &rURL, const OUString &rReferer);
    bool Save();
    bool SaveTo  (const css::uno::Reference<
                       css::embed::XStorage > &xStorage,
                   const OUString &rURL,
                                  OUString *pOptName);
    virtual bool Create() = 0;

    // Factory method for sub-classes
    static XPropertyListRef CreatePropertyList(XPropertyListType t,
                                               const OUString& rPath,
                                               const OUString& rReferer);
    // as above but initializes name as expected
    static XPropertyListRef CreatePropertyListFromURL(XPropertyListType t,
                                                       const OUString & rUrl);

    // helper accessors
    static inline XDashListRef AsDashList(
        rtl::Reference<XPropertyList> const & plist);
    static inline XHatchListRef AsHatchList(
        rtl::Reference<XPropertyList> const & plist);
    static inline XColorListRef AsColorList(
        rtl::Reference<XPropertyList> const & plist);
    static inline XBitmapListRef AsBitmapList(
        rtl::Reference<XPropertyList> const & plist);
    static inline XPatternListRef AsPatternList(
        rtl::Reference<XPropertyList> const & plist);
    static inline XLineEndListRef AsLineEndList(
        rtl::Reference<XPropertyList> const & plist);
    static inline XGradientListRef AsGradientList(
        rtl::Reference<XPropertyList> const & plist);
};

class SVX_DLLPUBLIC XColorList : public XPropertyList
{
protected:
    virtual Bitmap  CreateBitmapForUI(long nIndex) override;

public:
    XColorList(const OUString& rPath, const OUString& rReferer)
        : XPropertyList(XCOLOR_LIST, rPath, rReferer) {}

    using XPropertyList::Replace;
    using XPropertyList::Remove;

    XColorEntry* Replace(long nIndex, XColorEntry* pEntry);
    XColorEntry* Remove(long nIndex);
    XColorEntry* GetColor(long nIndex) const;
    virtual css::uno::Reference< css::container::XNameContainer > createInstance() override;
    virtual bool Create() override;

    static XColorListRef CreateStdColorList();
    static XColorListRef GetStdColorList(); // returns a singleton
};

class SVX_DLLPUBLIC XLineEndList : public XPropertyList
{
protected:
    virtual Bitmap CreateBitmapForUI(long nIndex) override;

public:
    XLineEndList(const OUString& rPath, const OUString& rReferer);
    virtual ~XLineEndList();

    using XPropertyList::Remove;
    XLineEndEntry* Remove(long nIndex);
    XLineEndEntry* GetLineEnd(long nIndex) const;

    virtual css::uno::Reference< css::container::XNameContainer > createInstance() override;
    virtual bool Create() override;
};

class SVX_DLLPUBLIC XDashList : public XPropertyList
{
private:
    Bitmap              maBitmapSolidLine;
    OUString            maStringSolidLine;
    OUString            maStringNoLine;

protected:
    static Bitmap ImpCreateBitmapForXDash(const XDash* pDash);
    virtual Bitmap CreateBitmapForUI(long nIndex) override;

public:
    XDashList(const OUString& rPath, const OUString& rReferer);
    virtual ~XDashList();

    using XPropertyList::Replace;
    XDashEntry* Replace(XDashEntry* pEntry, long nIndex);
    using XPropertyList::Remove;
    XDashEntry* Remove(long nIndex);
    XDashEntry* GetDash(long nIndex) const;

    virtual css::uno::Reference< css::container::XNameContainer > createInstance() override;
    virtual bool Create() override;

    // Special call to get a bitmap for the solid line representation. It
    // creates a bitmap fitting in size and style to the ones you get by
    // using GetUiBitmap for existing entries.
    Bitmap const & GetBitmapForUISolidLine() const;

    // Special calls to get the translated strings for the UI entry for no
    // line style (XLINE_NONE) and solid line style (XLINE_SOLID) for dialogs
    OUString const & GetStringForUiSolidLine() const;
    OUString const & GetStringForUiNoLine() const;
};

class SVX_DLLPUBLIC XHatchList : public XPropertyList
{
private:
    Bitmap CreateBitmap(long nIndex, const Size& rSize) const;
protected:
    virtual Bitmap CreateBitmapForUI(long nIndex) override;
public:
    XHatchList(const OUString& rPath, const OUString& rReferer);
    virtual ~XHatchList();

    using XPropertyList::Replace;
    XHatchEntry* Replace(XHatchEntry* pEntry, long nIndex);
    using XPropertyList::Remove;
    XHatchEntry* Remove(long nIndex);
    XHatchEntry* GetHatch(long nIndex) const;
    Bitmap GetBitmapForPreview(long nIndex, const Size& rSize);

    virtual css::uno::Reference< css::container::XNameContainer > createInstance() override;
    virtual bool Create() override;
};

class SVX_DLLPUBLIC XGradientList : public XPropertyList
{
private:
    Bitmap CreateBitmap(long nIndex, const Size& rSize) const;

protected:
    virtual Bitmap CreateBitmapForUI(long nIndex) override;

public:
    XGradientList(const OUString& rPath, const OUString& rReferer);
    virtual ~XGradientList();

    using XPropertyList::Replace;
    XGradientEntry* Replace(XGradientEntry* pEntry, long nIndex);
    using XPropertyList::Remove;
    XGradientEntry* Remove(long nIndex);
    XGradientEntry* GetGradient(long nIndex) const;
    Bitmap GetBitmapForPreview(long nIndex, const Size& rSize);

    virtual css::uno::Reference< css::container::XNameContainer > createInstance() override;
    virtual bool Create() override;
};

class SVX_DLLPUBLIC XBitmapList : public XPropertyList
{
private:
    Bitmap CreateBitmap( long nIndex, const Size& rSize ) const;

protected:
    virtual Bitmap CreateBitmapForUI(long nIndex) override;

public:
    XBitmapList(const OUString& rPath, const OUString& rReferer)
        : XPropertyList(XBITMAP_LIST, rPath, rReferer) {}

    using XPropertyList::Replace;
    using XPropertyList::Remove;
    XBitmapEntry* Remove(long nIndex);
    XBitmapEntry* GetBitmap(long nIndex) const;
    Bitmap GetBitmapForPreview(long nIndex, const Size& rSize);

    virtual css::uno::Reference< css::container::XNameContainer > createInstance() override;
    virtual bool Create() override;
};

class SVX_DLLPUBLIC XPatternList : public XPropertyList
{
protected:
    virtual Bitmap CreateBitmapForUI(long nIndex) override;

public:
    XPatternList(const OUString& rPath, const OUString& rReferer)
        : XPropertyList(XPATTERN_LIST, rPath, rReferer) {}

    using XPropertyList::Replace;
    using XPropertyList::Remove;

    XBitmapEntry* Remove(long nIndex);
    XBitmapEntry* GetBitmap(long nIndex) const;

    virtual css::uno::Reference< css::container::XNameContainer > createInstance() override;
    virtual bool Create() override;
};

// FIXME: could add type checking too ...
inline XDashListRef  XPropertyList::AsDashList(
    rtl::Reference<XPropertyList> const & plist)
{ return XDashListRef( static_cast<XDashList *> (plist.get()) ); }
inline XHatchListRef XPropertyList::AsHatchList(
    rtl::Reference<XPropertyList> const & plist)
{ return XHatchListRef( static_cast<XHatchList *> (plist.get()) ); }
inline XColorListRef XPropertyList::AsColorList(
    rtl::Reference<XPropertyList> const & plist)
{ return XColorListRef( static_cast<XColorList *> (plist.get()) ); }
inline XBitmapListRef XPropertyList::AsBitmapList(
    rtl::Reference<XPropertyList> const & plist)
{ return XBitmapListRef( static_cast<XBitmapList *> (plist.get()) ); }
inline XPatternListRef XPropertyList::AsPatternList(
    rtl::Reference<XPropertyList> const & plist)
{ return XPatternListRef( static_cast<XPatternList *> (plist.get()) ); }
inline XLineEndListRef XPropertyList::AsLineEndList(
    rtl::Reference<XPropertyList> const & plist)
{ return XLineEndListRef( static_cast<XLineEndList *> (plist.get()) ); }
inline XGradientListRef XPropertyList::AsGradientList(
    rtl::Reference<XPropertyList> const & plist)
{ return XGradientListRef( static_cast<XGradientList *> (plist.get()) ); }

#endif // INCLUDED_SVX_XTABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
