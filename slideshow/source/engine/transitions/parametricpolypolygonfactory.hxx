/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: parametricpolypolygonfactory.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:55:50 $
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

#ifndef _SLIDESHOW_PARAMETRICPOLYPOLYGONFACTORY_HXX
#define _SLIDESHOW_PARAMETRICPOLYPOLYGONFACTORY_HXX

#include <parametricpolypolygon.hxx>

namespace presentation
{
    namespace internal
    {
        /* Definition of Transitionfactory class */

        class ParametricPolyPolygonFactory
        {
        public:
            static ParametricPolyPolygonSharedPtr createClipPolyPolygon( sal_Int16 nTransitionType,
                                                                         sal_Int16 nTransitionSubType );

        private:
            // default: disabled copy/assignment
            ParametricPolyPolygonFactory(const ParametricPolyPolygonFactory&);
            ParametricPolyPolygonFactory& operator=( const ParametricPolyPolygonFactory& );

            // static factory
            ParametricPolyPolygonFactory();
            ~ParametricPolyPolygonFactory();
        };
    }
}

#endif /* _SLIDESHOW_PARAMETRICPOLYPOLYGONFACTORY_HXX */
