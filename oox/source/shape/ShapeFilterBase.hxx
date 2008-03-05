/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ShapeFilterBase.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:53:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef OOX_SHAPE_SHAPEFILTERBASE_HXX
#define OOX_SHAPE_SHAPEFILTERBASE_HXX

#include <boost/shared_ptr.hpp>
#include <rtl/ref.hxx>
#include "oox/vml/drawing.hxx"
#include "oox/core/xmlfilterbase.hxx"

namespace oox {
namespace shape {

// ============================================================================


class ShapeFilterBase : public core::XmlFilterBase
{
public:
    typedef boost::shared_ptr<ShapeFilterBase> Pointer_t;

    explicit            ShapeFilterBase(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxFactory );

    virtual             ~ShapeFilterBase();

    /** Has to be implemented by each filter to resolve scheme colors. */
    virtual sal_Int32   getSchemeClr( sal_Int32 nColorSchemeToken ) const;

    /** Has to be implemented by each filter to return drawings collection. */
    virtual const ::oox::vml::DrawingPtr getDrawings();

    virtual rtl::OUString implGetImplementationName() const;

    virtual bool importDocument() { return true; }
    virtual bool exportDocument() { return true; }
};

// ============================================================================

} // namespace shape
} // namespace oox

#endif

