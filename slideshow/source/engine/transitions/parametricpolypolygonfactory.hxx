/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: parametricpolypolygonfactory.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 15:43:42 $
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
#include <boost/utility.hpp>

namespace slideshow
{
    namespace internal
    {
        /* Definition of Transitionfactory class */

        class ParametricPolyPolygonFactory : private boost::noncopyable
        {
        public:
            static ParametricPolyPolygonSharedPtr createClipPolyPolygon( sal_Int16 nTransitionType,
                                                                         sal_Int16 nTransitionSubType );

        private:
            // static factory
            ParametricPolyPolygonFactory();
            ~ParametricPolyPolygonFactory();
        };
    }
}

#endif /* _SLIDESHOW_PARAMETRICPOLYPOLYGONFACTORY_HXX */
