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

#ifndef OOX_VML_VMLSHAPECONTEXT_HXX
#define OOX_VML_VMLSHAPECONTEXT_HXX

#include "oox/core/contexthandler2.hxx"

namespace oox {
namespace vml {

class Drawing;

struct ShapeTypeModel;
class ShapeType;

struct ClientData;
struct ShapeModel;
class ShapeBase;
class GroupShape;

class ShapeContainer;

// ============================================================================

class ShapeLayoutContext : public ::oox::core::ContextHandler2
{
public:
    explicit            ShapeLayoutContext(
                            ::oox::core::ContextHandler2Helper& rParent,
                            Drawing& rDrawing );

    virtual ::oox::core::ContextHandlerRef
                        onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );

private:
    Drawing&            mrDrawing;
};

// ============================================================================

class ClientDataContext : public ::oox::core::ContextHandler2
{
public:
    explicit            ClientDataContext(
                            ::oox::core::ContextHandler2Helper& rParent,
                            ClientData& rClientData,
                            const AttributeList& rAttribs );

    virtual ::oox::core::ContextHandlerRef
                        onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onCharacters( const ::rtl::OUString& rChars );
    virtual void        onEndElement();

private:
    ClientData&         mrClientData;
    ::rtl::OUString     maElementText;
};

// ============================================================================

class ShapeContextBase : public ::oox::core::ContextHandler2
{
public:
    static ::oox::core::ContextHandlerRef
                        createShapeContext(
                            ::oox::core::ContextHandler2Helper& rParent,
                            ShapeContainer& rShapes,
                            sal_Int32 nElement,
                            const AttributeList& rAttribs );

protected:
    explicit            ShapeContextBase( ::oox::core::ContextHandler2Helper& rParent );
};

// ============================================================================

class ShapeTypeContext : public ShapeContextBase
{
public:
    explicit            ShapeTypeContext(
                            ::oox::core::ContextHandler2Helper& rParent,
                            ShapeType& rShapeType,
                            const AttributeList& rAttribs );

    virtual ::oox::core::ContextHandlerRef
                        onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );

private:
    /** Processes the 'style' attribute. */
    void                setStyle( const ::rtl::OUString& rStyle );

    /** Resolve a relation identifier to a fragment path. */
    OptValue< ::rtl::OUString > decodeFragmentPath( const AttributeList& rAttribs, sal_Int32 nToken ) const;

private:
    ShapeTypeModel&     mrTypeModel;
};

// ============================================================================

class ShapeContext : public ShapeTypeContext
{
public:
    explicit            ShapeContext(
                            ::oox::core::ContextHandler2Helper& rParent,
                            ShapeBase& rShape,
                            const AttributeList& rAttribs );

    virtual ::oox::core::ContextHandlerRef
                        onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );

private:
    /** Processes the 'points' attribute. */
    void                setPoints( const ::rtl::OUString& rPoints );

private:
    ShapeModel&         mrShapeModel;
};

// ============================================================================

class GroupShapeContext : public ShapeContext
{
public:
    explicit            GroupShapeContext(
                            ::oox::core::ContextHandler2Helper& rParent,
                            GroupShape& rShape,
                            const AttributeList& rAttribs );

    virtual ::oox::core::ContextHandlerRef
                        onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );

private:
    ShapeContainer&     mrShapes;
};

// ============================================================================

} // namespace vml
} // namespace oox

#endif
