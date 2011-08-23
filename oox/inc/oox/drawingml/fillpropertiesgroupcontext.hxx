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

#ifndef OOX_DRAWINGML_FILLPROPERTIESGROUPCONTEXT_HPP
#define OOX_DRAWINGML_FILLPROPERTIESGROUPCONTEXT_HPP

#include "oox/drawingml/colorchoicecontext.hxx"
#include "oox/drawingml/fillproperties.hxx"

namespace oox {
namespace drawingml {

// ============================================================================

/** Context handler that imports the a:solidFill element. */
class SolidFillContext : public ColorContext
{
public:
    explicit            SolidFillContext(
                            ::oox::core::ContextHandler& rParent,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& rxAttribs,
                            FillProperties& rFillProps );
};

// ============================================================================

/** Context handler that imports the a:gradFill element. */
class GradientFillContext : public ::oox::core::ContextHandler
{
public:
    explicit            GradientFillContext(
                            ::oox::core::ContextHandler& rParent,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& rxAttribs,
                            GradientFillProperties& rGradientProps );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL
                        createFastChildContext(
                            sal_Int32 nElement,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& rxAttribs )
                        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );

private:
    GradientFillProperties& mrGradientProps;
};

// ============================================================================

/** Context handler that imports the a:pattFill element. */
class PatternFillContext : public ::oox::core::ContextHandler
{
public:
    explicit            PatternFillContext(
                            ::oox::core::ContextHandler& rParent,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& rxAttribs,
                            PatternFillProperties& rPatternProps );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL
                        createFastChildContext(
                            sal_Int32 nElement,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& rxAttribs )
                        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );

private:
    PatternFillProperties& mrPatternProps;
};

// ============================================================================
// ============================================================================

/** Context handler that imports the a:clrChange element containing the colors
    of a bitmap color change transformation. */
class ColorChangeContext : public ::oox::core::ContextHandler
{
public:
    explicit            ColorChangeContext(
                            ::oox::core::ContextHandler& rParent,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& rxAttribs,
                            BlipFillProperties& rBlipProps );
    virtual             ~ColorChangeContext();

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL
                        createFastChildContext(
                            sal_Int32 nElement,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& rxAttribs )
                        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );

private:
    BlipFillProperties& mrBlipProps;
    bool                mbUseAlpha;
};

// ============================================================================

/** Context handler that imports the a:blip element containing the fill bitmap
    and bitmap color transformation settings. */
class BlipContext : public ::oox::core::ContextHandler
{
public:
    explicit            BlipContext(
                            ::oox::core::ContextHandler& rParent,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& rxAttribs,
                            BlipFillProperties& rBlipProps );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL
                        createFastChildContext(
                            sal_Int32 nElement,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& rxAttribs )
                        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );

private:
    BlipFillProperties& mrBlipProps;
};

// ============================================================================

/** Context handler that imports the a:blipFill element. */
class BlipFillContext : public ::oox::core::ContextHandler
{
public:
    explicit            BlipFillContext(
                            ::oox::core::ContextHandler& rParent,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& rxAttribs,
                            BlipFillProperties& rBlipProps );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL
                        createFastChildContext(
                            sal_Int32 nElement,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& rxAttribs )
                        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );

private:
    BlipFillProperties& mrBlipProps;
};

// ============================================================================
// ============================================================================

/** Context handler for elements that contain a fill property element
    (a:noFill, a:solidFill, a:gradFill, a:pattFill, a:blipFill, a:grpFill). */
class FillPropertiesContext : public ::oox::core::ContextHandler
{
public:
    explicit            FillPropertiesContext(
                            ::oox::core::ContextHandler& rParent,
                            FillProperties& rFillProps );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL
                        createFastChildContext(
                            sal_Int32 nElement,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& rxAttribs )
                        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );

    static ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler >
                        createFillContext(
                            ::oox::core::ContextHandler& rParent,
                            sal_Int32 nElement,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& rxAttribs,
                            FillProperties& rFillProps );

protected:
    FillProperties&     mrFillProps;
};

// ============================================================================

/** Context handler for elements that contain a fill property element
    (a:noFill, a:solidFill, a:gradFill, a:pattFill, a:blipFill, a:grpFill).

    This context handler takes a simple color instead of a fill properties
    struct. The imported fill properties are converted automatically to the
    best fitting solid color.
 */
class SimpleFillPropertiesContext : private FillProperties, public FillPropertiesContext
{
public:
    explicit            SimpleFillPropertiesContext(
                            ::oox::core::ContextHandler& rParent,
                            Color& rColor );
    virtual             ~SimpleFillPropertiesContext();

protected:
    Color&              mrColor;
};

// ============================================================================

} // namespace drawingml
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
