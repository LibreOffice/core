/*************************************************************************
 *
 *  $RCSfile: shapeimporter.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 19:20:09 $
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

#ifndef _SLIDESHOW_SHAPEIMPORTER_HXX
#define _SLIDESHOW_SHAPEIMPORTER_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#include <shape.hxx>

namespace com { namespace sun { namespace star
{
    namespace drawing
    {
        class XShapes;
    }
    namespace beans
    {
        class XPropertySet;
    }
} } }

// -----------------------------------------------------------------------------

namespace presentation
{
    namespace internal
    {
        /** This class imports all shapes from a given XShapes object
         */
        class ShapeImporter
        {
        public:
            /** Create shape importer.

                @param xPage
                Page containing the shapes

                @param xActualPage
                Actual page that's imported - if xPage is a master
                page, this argument must refer to the using, i.e the
                page that embeds this specific masterpage. Otherwise,
                this argument is probably equal to xPage.

                @param nOrdNumStart
                Each shape receives a z order number, in order of
                import (which relies on the fact that the API returns
                the shapes in draw order - which it does,
                currently). Since we might mix several pages on screen
                (e.g. master page and foreground page), this value can
                be used as an offset to distinguish those pages.

                @param bConvertingMasterPage
                When true, then the master page is imported. Otherwise, this
                object imports the draw page.
             */
            ShapeImporter( const ::com::sun::star::uno::Reference<
                                     ::com::sun::star::drawing::XDrawPage >&    xPage,
                           const ::com::sun::star::uno::Reference<
                                     ::com::sun::star::drawing::XDrawPage >&    xActualPage,
                           sal_Int32                                        nOrdNumStart,
                           bool                                             bConvertingMasterPage );

            /** This method imports presentation-visible shapes (and
                skips all others).

                @return the generated Shape, or NULL for no more shapes.
             */
            ShapeSharedPtr importShape(); // throw (ConversionFailedException)

            /** Test whether import is done.

                @return true, if all shapes are imported via the
                importShape() call.
             */
            bool isImportDone() const;

        private:
            bool isSkip( const ::com::sun::star::uno::Reference<  ::com::sun::star::drawing::XShape >&      xCurrShape,
                         const ::com::sun::star::uno::Reference<  ::com::sun::star::beans::XPropertySet >&  xPropSet ) const;

            ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >    mxPage;
            ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >      mxShapes;
            sal_Int32                                                                   mnCurrShape;
            sal_Int32                                                                   mnNumShapes;
            sal_Int32                                                                   mnOrdNumStart;
            bool                                                                        mbConvertingMasterPage;
        };
    }
}

#endif
