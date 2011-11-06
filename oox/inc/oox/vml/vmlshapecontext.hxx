/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
