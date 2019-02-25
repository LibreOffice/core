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

#ifndef INCLUDED_OOX_DRAWINGML_MISCCONTEXTS_HXX
#define INCLUDED_OOX_DRAWINGML_MISCCONTEXTS_HXX

#include <drawingml/colorchoicecontext.hxx>

#include <drawingml/fillproperties.hxx>

namespace oox {
namespace drawingml {


/** Context handler that imports the a:solidFill element. */
class SolidFillContext : public ColorContext
{
public:
    explicit            SolidFillContext(
                            ::oox::core::ContextHandler2Helper const & rParent,
                            FillProperties& rFillProps );
};


/** Context handler that imports the a:gradFill element. */
class GradientFillContext : public ::oox::core::ContextHandler2
{
public:
    explicit            GradientFillContext(
                            ::oox::core::ContextHandler2Helper const & rParent,
                            const ::oox::AttributeList& rAttribs,
                            GradientFillProperties& rGradientProps );

    virtual ::oox::core::ContextHandlerRef
                        onCreateContext(
                            sal_Int32 nElement,
                            const ::oox::AttributeList& rAttribs ) override;

private:
    GradientFillProperties& mrGradientProps;
};

/** Context handler that imports the a:pattFill element. */
class PatternFillContext : public ::oox::core::ContextHandler2
{
public:
    explicit            PatternFillContext(
                            ::oox::core::ContextHandler2Helper const & rParent,
                            const ::oox::AttributeList& rAttribs,
                            PatternFillProperties& rPatternProps );

    virtual ::oox::core::ContextHandlerRef
                        onCreateContext(
                            sal_Int32 nElement,
                            const ::oox::AttributeList& rAttribs ) override;

private:
    PatternFillProperties& mrPatternProps;
};


/** Context handler that imports a14:imgProps, a14:imgLayer, a14:imgEffect containers
    and the a14:artistic* effect tags defined in the MS-ODRAWXML extension. */
class ArtisticEffectContext : public ::oox::core::ContextHandler2
{
public:
    explicit            ArtisticEffectContext(
                            ::oox::core::ContextHandler2Helper const & rParent,
                            ArtisticEffectProperties& rEffect );
    virtual             ~ArtisticEffectContext() override;

    virtual ::oox::core::ContextHandlerRef
                        onCreateContext(
                            sal_Int32 nElement,
                            const ::oox::AttributeList& rAttribs ) override;

private:
    ArtisticEffectProperties& maEffect;
};


/** Context handler that imports the a:extLst element inside a:blip and its
    children a:ext, which can contain transformations to the bitmap. */
class BlipExtensionContext : public ::oox::core::ContextHandler2
{
public:
    explicit            BlipExtensionContext(
                            ::oox::core::ContextHandler2Helper const & rParent,
                            BlipFillProperties& rBlipProps );
    virtual             ~BlipExtensionContext() override;

    virtual ::oox::core::ContextHandlerRef
                        onCreateContext(
                            sal_Int32 nElement,
                            const ::oox::AttributeList& rAttribs ) override;

private:
    BlipFillProperties& mrBlipProps;
};


/** Context handler that imports the a:duotone element containing the colors
    of a bitmap duotone transformation. */
class DuotoneContext : public ::oox::core::ContextHandler2
{
public:
    explicit            DuotoneContext(
                            ::oox::core::ContextHandler2Helper const & rParent,
                            BlipFillProperties& rBlipProps );
    virtual             ~DuotoneContext() override;

    virtual ::oox::core::ContextHandlerRef
                        onCreateContext(
                            sal_Int32 nElement,
                            const ::oox::AttributeList& rAttribs ) override;

private:
    BlipFillProperties& mrBlipProps;
    int                 mnColorIndex;
};


/** Context handler that imports the a:clrChange element containing the colors
    of a bitmap color change transformation. */
class ColorChangeContext : public ::oox::core::ContextHandler2
{
public:
    explicit            ColorChangeContext(
                            ::oox::core::ContextHandler2Helper const & rParent,
                            const ::oox::AttributeList& rAttribs,
                            BlipFillProperties& rBlipProps );
    virtual             ~ColorChangeContext() override;

    virtual ::oox::core::ContextHandlerRef
                        onCreateContext(
                            sal_Int32 nElement,
                            const ::oox::AttributeList& rAttribs ) override;

private:
    BlipFillProperties& mrBlipProps;
    bool                mbUseAlpha;
};

/** Context handler that imports the a:blip element containing the fill bitmap
    and bitmap color transformation settings. */
class BlipContext : public ::oox::core::ContextHandler2
{
public:
    explicit            BlipContext(
                            ::oox::core::ContextHandler2Helper const & rParent,
                            const ::oox::AttributeList& rAttribs,
                            BlipFillProperties& rBlipProps );

    virtual ::oox::core::ContextHandlerRef
                        onCreateContext(
                            sal_Int32 nElement,
                            const ::oox::AttributeList& rAttribs ) override;

private:
    BlipFillProperties& mrBlipProps;
};

/** Context handler that imports the a:blipFill element. */
class BlipFillContext : public ::oox::core::ContextHandler2
{
public:
    explicit            BlipFillContext(
                            ::oox::core::ContextHandler2Helper const & rParent,
                            const ::oox::AttributeList& rAttribs,
                            BlipFillProperties& rBlipProps );

    virtual ::oox::core::ContextHandlerRef
                        onCreateContext(
                            sal_Int32 nElement,
                            const ::oox::AttributeList& rAttribs ) override;

private:
    BlipFillProperties& mrBlipProps;
};

/** Context handler for elements that contain a fill property element
    (a:noFill, a:solidFill, a:gradFill, a:pattFill, a:blipFill, a:grpFill). */
class FillPropertiesContext : public ::oox::core::ContextHandler2
{
public:
    explicit            FillPropertiesContext(
                            ::oox::core::ContextHandler2Helper const & rParent,
                            FillProperties& rFillProps );

    virtual ::oox::core::ContextHandlerRef
                        onCreateContext(
                            sal_Int32 nElement,
                            const ::oox::AttributeList& rAttribs ) override;

    static ::oox::core::ContextHandlerRef
                        createFillContext(
                            ::oox::core::ContextHandler2Helper const & rParent,
                            sal_Int32 nElement,
                            const ::oox::AttributeList& rAttribs,
                            FillProperties& rFillProps );

private:
    FillProperties&     mrFillProps;
};

/** Context handler for elements that contain a fill property element
    (a:noFill, a:solidFill, a:gradFill, a:pattFill, a:blipFill, a:grpFill).

    This context handler takes a simple color instead of a fill properties
    struct. The imported fill properties are converted automatically to the
    best fitting solid color.
 */
class SimpleFillPropertiesContext final : private FillProperties, public FillPropertiesContext
{
public:
    explicit            SimpleFillPropertiesContext(
                            ::oox::core::ContextHandler2Helper const & rParent,
                            Color& rColor );
    virtual             ~SimpleFillPropertiesContext() override;

private:
    Color&              mrColor;
};

} // namespace drawingml
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
