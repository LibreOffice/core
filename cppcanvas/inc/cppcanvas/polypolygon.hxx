/*************************************************************************
 *
 *  $RCSfile: polypolygon.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: thb $ $Date: 2004-03-18 10:40:59 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _CPPCANVAS_POLYPOLYGON_HXX
#define _CPPCANVAS_POLYPOLYGON_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <external/boost/shared_ptr.hpp>
#endif

#ifndef _CPPCANVAS_CANVASGRAPHIC_HXX
#include <cppcanvas/canvasgraphic.hxx>
#endif

namespace basegfx
{
    class B2DPolygon;
    class B2DPolyPolygon;
}

namespace drafts { namespace com { namespace sun { namespace star { namespace rendering
{
    class  XPolyPolygon2D;
} } } } }


/* Definition of PolyPolygon interface */

namespace cppcanvas
{

    /** This interface defines a PolyPolygon canvas object

        Consider this object part of the view, and not of the model
        data. Although the given polygon is typically copied and held
        internally (to facilitate migration to incompatible canvases),
        ::basegfx::B2DPolygon et al. are ref-counted copy-on-write
        classes, i.e. memory shouldn't be wasted. On the other hand,
        the API polygon created internally _does_ necessarily
        duplicate the data held, but can be easily flushed away via
        flush().
     */
    class PolyPolygon : public virtual CanvasGraphic
    {
    public:
        virtual void addPolygon( const ::basegfx::B2DPolygon& rPoly ) = 0;
        virtual void addPolyPolygon( const ::basegfx::B2DPolyPolygon& rPoly ) = 0;

        /** Set polygon fill color
         */
        virtual void                        setRGBAFillColor( Color::IntSRGBA ) = 0;
        /** Set polygon line color
         */
        virtual void                        setRGBALineColor( Color::IntSRGBA ) = 0;
        /** Get polygon fill color
         */
        virtual Color::IntSRGBA             getRGBAFillColor() const = 0;
        /** Get polygon line color
         */
        virtual Color::IntSRGBA             getRGBALineColor() const = 0;

        virtual void                        setStrokeWidth( const double& rStrokeWidth ) = 0;
        virtual double                      getStrokeWidth() const = 0;

        virtual ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::rendering::XPolyPolygon2D > getUNOPolyPolygon() const = 0;
    };

    typedef ::boost::shared_ptr< ::cppcanvas::PolyPolygon > PolyPolygonSharedPtr;
}

#endif /* _CPPCANVAS_POLYPOLYGON_HXX */
