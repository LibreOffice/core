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


#include <memory>
#include <XPropertyTable.hxx>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <o3tl/any.hxx>
#include <vcl/svapp.hxx>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <svx/xdef.hxx>

#include <svx/unoapi.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <docmodel/uno/UnoGradientTools.hxx>

using namespace com::sun::star;
using namespace ::cppu;

namespace {

class SvxUnoXPropertyTable : public WeakImplHelper< container::XNameContainer, lang::XServiceInfo >
{
private:
    XPropertyList& mrList;
    sal_Int16 mnWhich;

    tools::Long getCount() const { return mrList.Count(); }
    const XPropertyEntry* get(tools::Long index) const;
public:
    SvxUnoXPropertyTable( sal_Int16 nWhich, XPropertyList& rList ) noexcept;

    /// @throws uno::RuntimeException
    virtual uno::Any getAny( const XPropertyEntry* pEntry ) const = 0;
    /// @throws uno::RuntimeException
    /// @throws lang::IllegalArgumentException
    virtual std::unique_ptr<XPropertyEntry> createEntry(const OUString& rName, const uno::Any& rAny) const = 0;

    // XServiceInfo
    virtual sal_Bool SAL_CALL supportsService( const  OUString& ServiceName ) override;

    // XNameContainer
    virtual void SAL_CALL insertByName( const  OUString& aName, const  uno::Any& aElement ) override;
    virtual void SAL_CALL removeByName( const  OUString& Name ) override;

    // XNameReplace
    virtual void SAL_CALL replaceByName( const  OUString& aName, const  uno::Any& aElement ) override;

    // XNameAccess
    virtual uno::Any SAL_CALL getByName( const  OUString& aName ) override;
    virtual uno::Sequence<  OUString > SAL_CALL getElementNames(  ) override;
    virtual sal_Bool SAL_CALL hasByName( const  OUString& aName ) override;

    // XElementAccess
    virtual sal_Bool SAL_CALL hasElements(  ) override;
};

}

SvxUnoXPropertyTable::SvxUnoXPropertyTable( sal_Int16 nWhich, XPropertyList& rList ) noexcept
: mrList( rList ), mnWhich( nWhich )
{
}

const XPropertyEntry* SvxUnoXPropertyTable::get(tools::Long index) const
{
   return mrList.Get(index);
}

// XServiceInfo
sal_Bool SAL_CALL SvxUnoXPropertyTable::supportsService( const  OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

// XNameContainer
void SAL_CALL SvxUnoXPropertyTable::insertByName( const  OUString& aName, const  uno::Any& aElement )
{
    SolarMutexGuard aGuard;

    if( hasByName( aName ) )
        throw container::ElementExistException();

    OUString aInternalName = SvxUnogetInternalNameForItem(mnWhich, aName);

    std::unique_ptr<XPropertyEntry> pNewEntry(createEntry(aInternalName, aElement));
    if (!pNewEntry)
        throw lang::IllegalArgumentException();

    mrList.Insert(std::move(pNewEntry));
}

void SAL_CALL SvxUnoXPropertyTable::removeByName( const  OUString& Name )
{
    SolarMutexGuard aGuard;

    OUString aInternalName = SvxUnogetInternalNameForItem(mnWhich, Name);

    const tools::Long nCount = getCount();
    tools::Long i;
    for( i = 0; i < nCount; i++ )
    {
        const XPropertyEntry* pEntry = get(i);
        if (pEntry && aInternalName == pEntry->GetName())
        {
            mrList.Remove(i);
            return;
        }
    }

    throw container::NoSuchElementException();
}

// XNameReplace
void SAL_CALL SvxUnoXPropertyTable::replaceByName( const  OUString& aName, const  uno::Any& aElement )
{
    SolarMutexGuard aGuard;

    OUString aInternalName = SvxUnogetInternalNameForItem(mnWhich, aName);

    const tools::Long nCount = getCount();
    tools::Long i;
    for( i = 0; i < nCount; i++ )
    {
        const XPropertyEntry* pEntry = get(i);
        if (pEntry && aInternalName == pEntry->GetName())
        {
            std::unique_ptr<XPropertyEntry> pNewEntry(createEntry(aInternalName, aElement));
            if (!pNewEntry)
                throw lang::IllegalArgumentException();

            mrList.Replace(std::move(pNewEntry), i);
            return;
        }
    }

    throw container::NoSuchElementException();
}

// XNameAccess
uno::Any SAL_CALL SvxUnoXPropertyTable::getByName( const  OUString& aName )
{
    SolarMutexGuard aGuard;

    OUString aInternalName = SvxUnogetInternalNameForItem(mnWhich, aName);

    const tools::Long nCount = getCount();
    tools::Long i;
    for( i = 0; i < nCount; i++ )
    {
        const XPropertyEntry* pEntry = get(i);

        if (pEntry && aInternalName == pEntry->GetName())
            return getAny( pEntry );
    }

    throw container::NoSuchElementException();
}

uno::Sequence<  OUString > SAL_CALL SvxUnoXPropertyTable::getElementNames()
{
    SolarMutexGuard aGuard;

    const tools::Long nCount = getCount();
    uno::Sequence< OUString > aNames( nCount );
    OUString* pNames = aNames.getArray();
    tools::Long i;
    for( i = 0; i < nCount; i++ )
    {
        const XPropertyEntry* pEntry = get(i);

        if (pEntry)
            *pNames++ = SvxUnogetApiNameForItem(mnWhich, pEntry->GetName());
    }

    return aNames;
}

sal_Bool SAL_CALL SvxUnoXPropertyTable::hasByName( const  OUString& aName )
{
    SolarMutexGuard aGuard;

    OUString aInternalName = SvxUnogetInternalNameForItem(mnWhich, aName);

    const tools::Long nCount = mrList.Count();
    tools::Long i;
    for( i = 0; i < nCount; i++ )
    {
        const XPropertyEntry* pEntry = get(i);
        if (pEntry && aInternalName == pEntry->GetName())
            return true;
    }

    return false;
}

// XElementAccess
sal_Bool SAL_CALL SvxUnoXPropertyTable::hasElements(  )
{
    SolarMutexGuard aGuard;

    return getCount() != 0;
}

namespace {

class SvxUnoXColorTable : public SvxUnoXPropertyTable
{
public:
    explicit SvxUnoXColorTable( XPropertyList& rList ) noexcept : SvxUnoXPropertyTable( XATTR_LINECOLOR, rList ) {};

    // SvxUnoXPropertyTable
    virtual uno::Any getAny( const XPropertyEntry* pEntry ) const noexcept override;
    virtual std::unique_ptr<XPropertyEntry> createEntry(const OUString& rName, const uno::Any& rAny) const override;

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) override;
};

}

uno::Reference< container::XNameContainer > SvxUnoXColorTable_createInstance( XPropertyList& rList ) noexcept
{
    return new SvxUnoXColorTable( rList );
}

// SvxUnoXPropertyTable
uno::Any SvxUnoXColorTable::getAny( const XPropertyEntry* pEntry ) const noexcept
{
    return uno::Any( static_cast<sal_Int32>(static_cast<const XColorEntry*>(pEntry)->GetColor()) );
}

std::unique_ptr<XPropertyEntry> SvxUnoXColorTable::createEntry(const OUString& rName, const uno::Any& rAny) const
{
    Color aColor;
    if( !(rAny >>= aColor) )
        return std::unique_ptr<XPropertyEntry>();

    return std::make_unique<XColorEntry>(aColor, rName);
}

// XElementAccess
uno::Type SAL_CALL SvxUnoXColorTable::getElementType()
{
    return ::cppu::UnoType<sal_Int32>::get();
}

// XServiceInfo
OUString SAL_CALL SvxUnoXColorTable::getImplementationName(  )
{
    return u"SvxUnoXColorTable"_ustr;
}

uno::Sequence<  OUString > SAL_CALL SvxUnoXColorTable::getSupportedServiceNames(  )
{
    return { u"com.sun.star.drawing.ColorTable"_ustr };
}

namespace {

class SvxUnoXLineEndTable : public SvxUnoXPropertyTable
{
public:
    explicit SvxUnoXLineEndTable( XPropertyList& rTable ) noexcept : SvxUnoXPropertyTable( XATTR_LINEEND, rTable ) {};

    // SvxUnoXPropertyTable
    virtual uno::Any getAny( const XPropertyEntry* pEntry ) const noexcept override;
    virtual std::unique_ptr<XPropertyEntry> createEntry(const OUString& rName, const uno::Any& rAny) const override;

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) override;
};

}

uno::Reference< container::XNameContainer > SvxUnoXLineEndTable_createInstance( XPropertyList& rTable ) noexcept
{
    return new SvxUnoXLineEndTable( rTable );
}

// SvxUnoXPropertyTable
uno::Any SvxUnoXLineEndTable::getAny( const XPropertyEntry* pEntry ) const noexcept
{
    drawing::PolyPolygonBezierCoords aBezier;
    basegfx::utils::B2DPolyPolygonToUnoPolyPolygonBezierCoords( static_cast<const XLineEndEntry*>(pEntry)->GetLineEnd(),
                                                          aBezier );
    return uno::Any(aBezier);
}

std::unique_ptr<XPropertyEntry> SvxUnoXLineEndTable::createEntry(const OUString& rName, const uno::Any& rAny) const
{
    auto pCoords = o3tl::tryAccess<drawing::PolyPolygonBezierCoords>(rAny);
    if( !pCoords )
        return std::unique_ptr<XLineEndEntry>();

    basegfx::B2DPolyPolygon aPolyPolygon;
    if( pCoords->Coordinates.getLength() > 0 )
        aPolyPolygon = basegfx::utils::UnoPolyPolygonBezierCoordsToB2DPolyPolygon( *pCoords );

    // #86265# make sure polygon is closed
    aPolyPolygon.setClosed(true);

    return std::make_unique<XLineEndEntry>(aPolyPolygon, rName);
}

// XElementAccess
uno::Type SAL_CALL SvxUnoXLineEndTable::getElementType()
{
    return cppu::UnoType<drawing::PolyPolygonBezierCoords>::get();
}

// XServiceInfo
OUString SAL_CALL SvxUnoXLineEndTable::getImplementationName(  )
{
    return u"SvxUnoXLineEndTable"_ustr;
}

uno::Sequence<  OUString > SAL_CALL SvxUnoXLineEndTable::getSupportedServiceNames(  )
{
    return { u"com.sun.star.drawing.LineEndTable"_ustr };
}

namespace {

class SvxUnoXDashTable : public SvxUnoXPropertyTable
{
public:
    explicit SvxUnoXDashTable( XPropertyList& rTable ) noexcept : SvxUnoXPropertyTable( XATTR_LINEDASH, rTable ) {};

    // SvxUnoXPropertyTable
    virtual uno::Any getAny( const XPropertyEntry* pEntry ) const noexcept override;
    virtual std::unique_ptr<XPropertyEntry> createEntry(const OUString& rName, const uno::Any& rAny) const override;

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) override;
};

}

uno::Reference< container::XNameContainer > SvxUnoXDashTable_createInstance( XPropertyList& rTable ) noexcept
{
    return new SvxUnoXDashTable( rTable );
}

// SvxUnoXPropertyTable
uno::Any SvxUnoXDashTable::getAny( const XPropertyEntry* pEntry ) const noexcept
{
    const XDash& rXD = static_cast<const XDashEntry*>(pEntry)->GetDash();

    drawing::LineDash aLineDash;

    aLineDash.Style = static_cast<css::drawing::DashStyle>(static_cast<sal_uInt16>(rXD.GetDashStyle()));
    aLineDash.Dots = rXD.GetDots();
    aLineDash.DotLen = rXD.GetDotLen();
    aLineDash.Dashes = rXD.GetDashes();
    aLineDash.DashLen = rXD.GetDashLen();
    aLineDash.Distance = rXD.GetDistance();

    return uno::Any(aLineDash);
}

std::unique_ptr<XPropertyEntry> SvxUnoXDashTable::createEntry(const OUString& rName, const uno::Any& rAny) const
{
    drawing::LineDash aLineDash;
    if(!(rAny >>= aLineDash))
        return std::unique_ptr<XDashEntry>();

    XDash aXDash;

    aXDash.SetDashStyle(static_cast<css::drawing::DashStyle>(static_cast<sal_uInt16>(aLineDash.Style)));
    aXDash.SetDots(aLineDash.Dots);
    aXDash.SetDotLen(aLineDash.DotLen);
    aXDash.SetDashes(aLineDash.Dashes);
    aXDash.SetDashLen(aLineDash.DashLen);
    aXDash.SetDistance(aLineDash.Distance);

    return std::make_unique<XDashEntry>(aXDash, rName);
}

// XElementAccess
uno::Type SAL_CALL SvxUnoXDashTable::getElementType()
{
    return cppu::UnoType<drawing::LineDash>::get();
}

// XServiceInfo
OUString SAL_CALL SvxUnoXDashTable::getImplementationName(  )
{
    return u"SvxUnoXDashTable"_ustr;
}

uno::Sequence<  OUString > SAL_CALL SvxUnoXDashTable::getSupportedServiceNames(  )
{
    return { u"com.sun.star.drawing.DashTable"_ustr };
}

namespace {

class SvxUnoXHatchTable : public SvxUnoXPropertyTable
{
public:
    explicit SvxUnoXHatchTable( XPropertyList& rTable ) noexcept : SvxUnoXPropertyTable( XATTR_FILLHATCH, rTable ) {};

    // SvxUnoXPropertyTable
    virtual uno::Any getAny( const XPropertyEntry* pEntry ) const noexcept override;
    virtual std::unique_ptr<XPropertyEntry> createEntry(const OUString& rName, const uno::Any& rAny) const override;

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) override;
};

}

uno::Reference< container::XNameContainer > SvxUnoXHatchTable_createInstance( XPropertyList& rTable ) noexcept
{
    return new SvxUnoXHatchTable( rTable );
}

// SvxUnoXPropertyTable
uno::Any SvxUnoXHatchTable::getAny( const XPropertyEntry* pEntry ) const noexcept
{
    const XHatch& aHatch = static_cast<const XHatchEntry*>(pEntry)->GetHatch();

    drawing::Hatch aUnoHatch;

    aUnoHatch.Style = aHatch.GetHatchStyle();
    aUnoHatch.Color = sal_Int32(aHatch.GetColor());
    aUnoHatch.Distance = aHatch.GetDistance();
    aUnoHatch.Angle = aHatch.GetAngle().get();

    return uno::Any(aUnoHatch);
}

std::unique_ptr<XPropertyEntry> SvxUnoXHatchTable::createEntry(const OUString& rName, const uno::Any& rAny) const
{
    drawing::Hatch aUnoHatch;
    if(!(rAny >>= aUnoHatch))
        return std::unique_ptr<XHatchEntry>();

    XHatch aXHatch;
    aXHatch.SetHatchStyle( aUnoHatch.Style );
    aXHatch.SetColor( Color(ColorTransparency, aUnoHatch.Color) );
    aXHatch.SetDistance( aUnoHatch.Distance );
    aXHatch.SetAngle( Degree10(aUnoHatch.Angle) );

    return std::make_unique<XHatchEntry>(aXHatch, rName);
}

// XElementAccess
uno::Type SAL_CALL SvxUnoXHatchTable::getElementType()
{
    return cppu::UnoType<drawing::Hatch>::get();
}

// XServiceInfo
OUString SAL_CALL SvxUnoXHatchTable::getImplementationName(  )
{
    return u"SvxUnoXHatchTable"_ustr;
}

uno::Sequence<  OUString > SAL_CALL SvxUnoXHatchTable::getSupportedServiceNames(  )
{
    return { u"com.sun.star.drawing.HatchTable"_ustr };
}

namespace {

class SvxUnoXGradientTable : public SvxUnoXPropertyTable
{
public:
    explicit SvxUnoXGradientTable( XPropertyList& rTable ) noexcept : SvxUnoXPropertyTable( XATTR_FILLGRADIENT, rTable ) {};

    // SvxUnoXPropertyTable
    virtual uno::Any getAny( const XPropertyEntry* pEntry ) const noexcept override;
    virtual std::unique_ptr<XPropertyEntry> createEntry(const OUString& rName, const uno::Any& rAny) const override;

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) override;
};

}

uno::Reference< container::XNameContainer > SvxUnoXGradientTable_createInstance( XPropertyList& rTable ) noexcept
{
    return new SvxUnoXGradientTable( rTable );
}

// SvxUnoXPropertyTable
uno::Any SvxUnoXGradientTable::getAny( const XPropertyEntry* pEntry ) const noexcept
{
    const basegfx::BGradient& aBGradient = static_cast<const XGradientEntry*>(pEntry)->GetGradient();

    awt::Gradient2 aGradient = model::gradient::createUnoGradient2(aBGradient);
    assert(aGradient.ColorStops.get() && "cid#1524745 aGradient.ColorStops._pSequence won't be null here");

    return uno::Any(aGradient);
}

std::unique_ptr<XPropertyEntry> SvxUnoXGradientTable::createEntry(const OUString& rName, const uno::Any& rAny) const
{
    if (!rAny.has<css::awt::Gradient>() || !rAny.has<css::awt::Gradient2>())
        return std::unique_ptr<XPropertyEntry>();

    const basegfx::BGradient aBGradient = model::gradient::getFromAny(rAny);
    return std::make_unique<XGradientEntry>(aBGradient, rName);
}

// XElementAccess
uno::Type SAL_CALL SvxUnoXGradientTable::getElementType()
{
    return cppu::UnoType<awt::Gradient>::get();
}

// XServiceInfo
OUString SAL_CALL SvxUnoXGradientTable::getImplementationName(  )
{
    return u"SvxUnoXGradientTable"_ustr;
}

uno::Sequence<  OUString > SAL_CALL SvxUnoXGradientTable::getSupportedServiceNames(  )
{
    return { u"com.sun.star.drawing.GradientTable"_ustr };
}

namespace {

class SvxUnoXBitmapTable : public SvxUnoXPropertyTable
{
public:
    explicit SvxUnoXBitmapTable( XPropertyList& rTable ) noexcept : SvxUnoXPropertyTable( XATTR_FILLBITMAP, rTable ) {};

    // SvxUnoXPropertyTable
    virtual uno::Any getAny( const XPropertyEntry* pEntry ) const override;
    virtual std::unique_ptr<XPropertyEntry> createEntry(const OUString& rName, const uno::Any& rAny) const override;

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) override;
};

}

uno::Reference< container::XNameContainer > SvxUnoXBitmapTable_createInstance( XPropertyList& rTable ) noexcept
{
    return new SvxUnoXBitmapTable( rTable );
}

// SvxUnoXPropertyTable
uno::Any SvxUnoXBitmapTable::getAny( const XPropertyEntry* pEntry ) const
{
    auto xBitmapEntry = static_cast<const XBitmapEntry*>(pEntry);
    css::uno::Reference<css::awt::XBitmap> xBitmap(xBitmapEntry->GetGraphicObject().GetGraphic().GetXGraphic(), uno::UNO_QUERY);
    return uno::Any(xBitmap);
}

std::unique_ptr<XPropertyEntry> SvxUnoXBitmapTable::createEntry(const OUString& rName, const uno::Any& rAny) const
{
    if (!rAny.has<uno::Reference<awt::XBitmap>>())
        return std::unique_ptr<XPropertyEntry>();

    auto xBitmap = rAny.get<uno::Reference<awt::XBitmap>>();
    if (!xBitmap.is())
        return nullptr;

    uno::Reference<graphic::XGraphic> xGraphic(xBitmap, uno::UNO_QUERY);
    if (!xGraphic.is())
        return nullptr;

    Graphic aGraphic(xGraphic);
    if (aGraphic.IsNone())
        return nullptr;

    GraphicObject aGraphicObject(std::move(aGraphic));
    return std::make_unique<XBitmapEntry>(aGraphicObject, rName);
}

// XElementAccess
uno::Type SAL_CALL SvxUnoXBitmapTable::getElementType()
{
    return ::cppu::UnoType<awt::XBitmap>::get();
}

// XServiceInfo
OUString SAL_CALL SvxUnoXBitmapTable::getImplementationName(  )
{
    return u"SvxUnoXBitmapTable"_ustr;
}

uno::Sequence<  OUString > SAL_CALL SvxUnoXBitmapTable::getSupportedServiceNames(  )
{
    return { u"com.sun.star.drawing.BitmapTable"_ustr };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
