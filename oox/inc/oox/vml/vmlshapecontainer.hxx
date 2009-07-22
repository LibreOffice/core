/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vmlshapecoontainer.hxx,v $
 * $Revision: 1.1 $
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

#ifndef OOX_VML_VMLSHAPECONTAINER_HXX
#define OOX_VML_VMLSHAPECONTAINER_HXX

#include <com/sun/star/awt/Rectangle.hpp>
#include "oox/helper/containerhelper.hxx"

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
    explicit            ShapeContainer( const Drawing& rDrawing );
                        ~ShapeContainer();

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

    const Drawing&      mrDrawing;          /// The VML drawing page that contains this shape.
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

// ============================================================================

} // namespace vml
} // namespace oox

#endif

