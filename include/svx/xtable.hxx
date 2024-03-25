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
#include <svx/xdash.hxx>
#include <svx/xhatch.hxx>
#include <basegfx/utils/bgradient.hxx>

#include <tools/color.hxx>

#include <cppuhelper/weak.hxx>

#include <svx/svxdllapi.h>
#include <com/sun/star/embed/XStorage.hpp>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <vcl/GraphicObject.hxx>
#include <svx/XPropertyEntry.hxx>

#include <limits>
#include <memory>

class SVXCORE_DLLPUBLIC XColorEntry final : public XPropertyEntry
{
private:
    Color   m_aColor;

public:
    XColorEntry(const Color& rColor, const OUString& rName);

    const Color& GetColor() const
    {
        return m_aColor;
    }
};

class SVXCORE_DLLPUBLIC XLineEndEntry final : public XPropertyEntry
{
private:
    basegfx::B2DPolyPolygon aB2DPolyPolygon;

public:
    XLineEndEntry(basegfx::B2DPolyPolygon aB2DPolyPolygon, const OUString& rName);
    XLineEndEntry(const XLineEndEntry& rOther);

    const basegfx::B2DPolyPolygon& GetLineEnd() const
    {
        return aB2DPolyPolygon;
    }
};

class SVXCORE_DLLPUBLIC XDashEntry final : public XPropertyEntry
{
private:
    XDash   m_aDash;

public:
    XDashEntry(const XDash& rDash, const OUString& rName);
    XDashEntry(const XDashEntry& rOther);

    const XDash& GetDash() const
    {
        return m_aDash;
    }
};

class SVXCORE_DLLPUBLIC XHatchEntry final : public XPropertyEntry
{
private:
    XHatch  m_aHatch;

public:
    XHatchEntry(const XHatch& rHatch, const OUString& rName);
    XHatchEntry(const XHatchEntry& rOther);

    const XHatch& GetHatch() const
    {
        return m_aHatch;
    }
};

class SVXCORE_DLLPUBLIC XGradientEntry final : public XPropertyEntry
{
private:
    basegfx::BGradient  m_aGradient;

public:
    XGradientEntry(const basegfx::BGradient& rGradient, const OUString& rName);
    XGradientEntry(const XGradientEntry& rOther);

    const basegfx::BGradient& GetGradient() const
    {
        return m_aGradient;
    }
};

class SVXCORE_DLLPUBLIC XBitmapEntry final : public XPropertyEntry
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

enum class XPropertyListType {
    Unknown = -1,
    Color,
    LineEnd,
    Dash,
    Hatch,
    Gradient,
    Bitmap,
    Pattern,
    LAST = Pattern
};

typedef rtl::Reference< class XPropertyList > XPropertyListRef;

class XDashList ; typedef rtl::Reference< class XDashList > XDashListRef;
class XHatchList ; typedef rtl::Reference< class XHatchList > XHatchListRef;
class XColorList ; typedef rtl::Reference< class XColorList > XColorListRef;
class XBitmapList ; typedef rtl::Reference< class XBitmapList > XBitmapListRef;
class XPatternList ; typedef rtl::Reference< class XPatternList > XPatternListRef;
class XLineEndList ; typedef rtl::Reference< class XLineEndList > XLineEndListRef;
class XGradientList ; typedef rtl::Reference< class XGradientList > XGradientListRef;

class SVXCORE_DLLPUBLIC XPropertyList : public cppu::OWeakObject
{
protected:
    XPropertyListType   meType;
    OUString            maName; // not persistent
    OUString            maPath;
    OUString            maReferer;

    std::vector< std::unique_ptr<XPropertyEntry> > maList;

    bool                mbListDirty;
    bool                mbEmbedInDocument;

    XPropertyList(XPropertyListType t, OUString aPath, OUString aReferer);
    bool isValidIdx(tools::Long nIndex) const;
    virtual BitmapEx CreateBitmapForUI(tools::Long nIndex) = 0;

public:
    XPropertyList(const XPropertyList&) = delete;
    XPropertyList& operator=(const XPropertyList&) = delete;
    virtual ~XPropertyList() override;

    XPropertyListType Type() const { return meType; }
    tools::Long Count() const;

    void Insert(std::unique_ptr<XPropertyEntry> pEntry, tools::Long nIndex = std::numeric_limits<tools::Long>::max());
    void Replace(std::unique_ptr<XPropertyEntry> pEntry, tools::Long nIndex);
    void Remove(tools::Long nIndex);

    XPropertyEntry* Get(tools::Long nIndex) const;
    tools::Long GetIndex(std::u16string_view rName) const;
    BitmapEx GetUiBitmap(tools::Long nIndex) const;

    const OUString& GetName() const { return maName; }
    void SetName(const OUString& rString);

    const OUString& GetPath() const { return maPath; }
    void SetPath(const OUString& rString) { maPath = rString; }

    void SetDirty(bool bDirty) { mbListDirty = bDirty; }

    bool IsEmbedInDocument() const { return mbEmbedInDocument; }

    static OUString GetDefaultExt(XPropertyListType t);
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
                                                       std::u16string_view rUrl);

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

class SVXCORE_DLLPUBLIC XColorList final : public XPropertyList
{
    virtual BitmapEx  CreateBitmapForUI(tools::Long nIndex) override;

public:
    XColorList(const OUString& rPath, const OUString& rReferer)
        : XPropertyList(XPropertyListType::Color, rPath, rReferer) {}

    void Replace(tools::Long nIndex, std::unique_ptr<XColorEntry> pEntry);
    XColorEntry* GetColor(tools::Long nIndex) const;
    tools::Long GetIndexOfColor( const Color& rColor) const;
    virtual css::uno::Reference< css::container::XNameContainer > createInstance() override;
    virtual bool Create() override;

    static XColorListRef CreateStdColorList();
    static XColorListRef GetStdColorList(); // returns a singleton
};

class SVXCORE_DLLPUBLIC XLineEndList final : public XPropertyList
{
    virtual BitmapEx CreateBitmapForUI(tools::Long nIndex) override;

public:
    XLineEndList(const OUString& rPath, const OUString& rReferer);
    virtual ~XLineEndList() override;

    XLineEndEntry* GetLineEnd(tools::Long nIndex) const;

    virtual css::uno::Reference< css::container::XNameContainer > createInstance() override;
    virtual bool Create() override;
};

class SVXCORE_DLLPUBLIC XDashList final : public XPropertyList
{
private:
    BitmapEx            maBitmapSolidLine;
    OUString            maStringSolidLine;
    OUString            maStringNoLine;

    static double ImpGetDefaultLineThickness();
    virtual BitmapEx CreateBitmapForUI(tools::Long nIndex) override;

public:
    XDashList(const OUString& rPath, const OUString& rReferer);
    virtual ~XDashList() override;

    void Replace(std::unique_ptr<XDashEntry> pEntry, tools::Long nIndex);
    XDashEntry* GetDash(tools::Long nIndex) const;

    virtual css::uno::Reference< css::container::XNameContainer > createInstance() override;
    virtual bool Create() override;

    // Special call to get a bitmap for the solid line representation. It
    // creates a bitmap fitting in size and style to the ones you get by
    // using GetUiBitmap for existing entries.
    BitmapEx const & GetBitmapForUISolidLine() const;

    static BitmapEx CreateBitmapForXDash(const XDash* pDash, double fLineThickness);

    // Special calls to get the translated strings for the UI entry for no
    // line style (XLINE_NONE) and solid line style (XLINE_SOLID) for dialogs
    OUString const & GetStringForUiSolidLine() const;
    OUString const & GetStringForUiNoLine() const;
};

class SVXCORE_DLLPUBLIC XHatchList final : public XPropertyList
{
private:
    BitmapEx CreateBitmap(tools::Long nIndex, const Size& rSize) const;
    virtual BitmapEx CreateBitmapForUI(tools::Long nIndex) override;
public:
    XHatchList(const OUString& rPath, const OUString& rReferer);
    virtual ~XHatchList() override;

    void Replace(std::unique_ptr<XHatchEntry> pEntry, tools::Long nIndex);
    XHatchEntry* GetHatch(tools::Long nIndex) const;
    BitmapEx GetBitmapForPreview(tools::Long nIndex, const Size& rSize);

    virtual css::uno::Reference< css::container::XNameContainer > createInstance() override;
    virtual bool Create() override;
};

class SVXCORE_DLLPUBLIC XGradientList final : public XPropertyList
{
private:
    BitmapEx CreateBitmap(tools::Long nIndex, const Size& rSize) const;
    virtual BitmapEx CreateBitmapForUI(tools::Long nIndex) override;

public:
    XGradientList(const OUString& rPath, const OUString& rReferer);
    virtual ~XGradientList() override;

    void Replace(std::unique_ptr<XGradientEntry> pEntry, tools::Long nIndex);
    XGradientEntry* GetGradient(tools::Long nIndex) const;
    BitmapEx GetBitmapForPreview(tools::Long nIndex, const Size& rSize);

    virtual css::uno::Reference< css::container::XNameContainer > createInstance() override;
    virtual bool Create() override;
};

class SVXCORE_DLLPUBLIC XBitmapList final : public XPropertyList
{
private:
    BitmapEx CreateBitmap( tools::Long nIndex, const Size& rSize ) const;
    virtual BitmapEx CreateBitmapForUI(tools::Long nIndex) override;

public:
    XBitmapList(const OUString& rPath, const OUString& rReferer)
        : XPropertyList(XPropertyListType::Bitmap, rPath, rReferer) {}

    XBitmapEntry* GetBitmap(tools::Long nIndex) const;
    BitmapEx GetBitmapForPreview(tools::Long nIndex, const Size& rSize);

    virtual css::uno::Reference< css::container::XNameContainer > createInstance() override;
    virtual bool Create() override;
};

class SVXCORE_DLLPUBLIC XPatternList final : public XPropertyList
{
private:
    BitmapEx CreateBitmap( tools::Long nIndex, const Size& rSize ) const;
    virtual BitmapEx CreateBitmapForUI(tools::Long nIndex) override;

public:
    XPatternList(const OUString& rPath, const OUString& rReferer)
        : XPropertyList(XPropertyListType::Pattern, rPath, rReferer) {}

    XBitmapEntry* GetBitmap(tools::Long nIndex) const;
    BitmapEx GetBitmapForPreview(tools::Long nIndex, const Size& rSize);

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
