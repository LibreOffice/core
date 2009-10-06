/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vmldrawingfragment.hxx,v $
 * $Revision: 1.5 $
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

struct ShapeTypeModel;
class ShapeType;

struct ShapeClientData;
struct ShapeModel;
class ShapeBase;
class GroupShape;

class ShapeContainer;

// ============================================================================

class ShapeClientDataContext : public ::oox::core::ContextHandler2
{
public:
    explicit            ShapeClientDataContext(
                            ::oox::core::ContextHandler2Helper& rParent,
                            const AttributeList& rAttribs,
                            ShapeClientData& rClientData );

    virtual ::oox::core::ContextHandlerRef
                        onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onEndElement( const ::rtl::OUString& rChars );

private:
    ShapeClientData&    mrClientData;
};

// ============================================================================

class ShapeContextBase : public ::oox::core::ContextHandler2
{
public:
    static ::oox::core::ContextHandlerRef
                        createShapeContext(
                            ::oox::core::ContextHandler2Helper& rParent,
                            sal_Int32 nElement,
                            const AttributeList& rAttribs,
                            ShapeContainer& rShapes );

protected:
    explicit            ShapeContextBase( ::oox::core::ContextHandler2Helper& rParent );
};

// ============================================================================

class ShapeTypeContext : public ShapeContextBase
{
public:
    explicit            ShapeTypeContext(
                            ::oox::core::ContextHandler2Helper& rParent,
                            const AttributeList& rAttribs,
                            ShapeType& rShapeType );

    virtual ::oox::core::ContextHandlerRef
                        onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );

private:
    /** Processes the 'style' attribute. */
    void                setStyle( const ::rtl::OUString& rStyle );

private:
    ShapeTypeModel&     mrTypeModel;
};

// ============================================================================

class ShapeContext : public ShapeTypeContext
{
public:
    explicit            ShapeContext(
                            ::oox::core::ContextHandler2Helper& rParent,
                            const AttributeList& rAttribs,
                            ShapeBase& rShape );

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
                            const AttributeList& rAttribs,
                            GroupShape& rShape );

    virtual ::oox::core::ContextHandlerRef
                        onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );

private:
    ShapeContainer&     mrShapes;
};

// ============================================================================

} // namespace vml
} // namespace oox

#endif

