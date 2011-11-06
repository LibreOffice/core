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



#ifndef OOX_VML_VMLSHAPECONTAINER_HXX
#define OOX_VML_VMLSHAPECONTAINER_HXX

#include <com/sun/star/awt/Rectangle.hpp>
#include "oox/helper/refmap.hxx"
#include "oox/helper/refvector.hxx"

namespace com { namespace sun { namespace star {
    namespace drawing { class XShapes; }
} } }

namespace oox {
namespace vml {

class Drawing;
class ShapeType;
class ShapeBase;

// ============================================================================

struct ShapeParentAnchor
{
    ::com::sun::star::awt::Rectangle maShapeRect;
    ::com::sun::star::awt::Rectangle maCoordSys;
};

// ============================================================================

/** Container that holds a list of shapes and shape templates. */
class ShapeContainer
{
public:
    explicit            ShapeContainer( Drawing& rDrawing );
                        ~ShapeContainer();

    /** Returns the drawing this shape container is part of. */
    inline Drawing&     getDrawing() { return mrDrawing; }

    /** Creates and returns a new shape template object. */
    ShapeType&          createShapeType();
    /** Creates and returns a new shape object of the specified type. */
    template< typename ShapeT >
    ShapeT&             createShape();

    /** Final processing after import of the drawing fragment. */
    void                finalizeFragmentImport();

    /** Returns true, if this contaikner does not contain any shapes. */
    inline bool         empty() const { return maShapes.empty(); }

    /** Returns the shape template with the passed identifier.
        @param bDeep  True = searches in all group shapes too. */
    const ShapeType*    getShapeTypeById( const ::rtl::OUString& rShapeId, bool bDeep ) const;
    /** Returns the shape with the passed identifier.
        @param bDeep  True = searches in all group shapes too. */
    const ShapeBase*    getShapeById( const ::rtl::OUString& rShapeId, bool bDeep ) const;

    /** Searches for a shape type by using the passed functor that takes a
        constant reference of a ShapeType object. */
    template< typename Functor >
    const ShapeType*    findShapeType( const Functor& rFunctor ) const;
    /** Searches for a shape by using the passed functor that takes a constant
        reference of a ShapeBase object. */
    template< typename Functor >
    const ShapeBase*    findShape( const Functor& rFunctor ) const;

    /** Returns the first shape in the collection (Word only). */
    const ShapeBase*    getFirstShape() const;

    /** Creates and inserts all UNO shapes into the passed container. */
    void                convertAndInsert(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            const ShapeParentAnchor* pParentAnchor = 0 ) const;

private:
    typedef RefVector< ShapeType >                  ShapeTypeVector;
    typedef RefVector< ShapeBase >                  ShapeVector;
    typedef RefMap< ::rtl::OUString, ShapeType >    ShapeTypeMap;
    typedef RefMap< ::rtl::OUString, ShapeBase >    ShapeMap;

    Drawing&            mrDrawing;          /// The VML drawing page that contains this shape.
    ShapeTypeVector     maTypes;            /// All shape templates.
    ShapeVector         maShapes;           /// All shape definitions.
    ShapeTypeMap        maTypesById;        /// All shape templates mapped by identifier.
    ShapeMap            maShapesById;       /// All shape definitions mapped by identifier.
};

// ----------------------------------------------------------------------------

template< typename ShapeT >
ShapeT& ShapeContainer::createShape()
{
    ::boost::shared_ptr< ShapeT > xShape( new ShapeT( mrDrawing ) );
    maShapes.push_back( xShape );
    return *xShape;
}

template< typename Functor >
const ShapeType* ShapeContainer::findShapeType( const Functor& rFunctor ) const
{
    return maTypes.findIf( rFunctor ).get();
}

template< typename Functor >
const ShapeBase* ShapeContainer::findShape( const Functor& rFunctor ) const
{
    return maShapes.findIf( rFunctor ).get();
}

// ============================================================================

} // namespace vml
} // namespace oox

#endif
