/*************************************************************************
 *
 *  $RCSfile: textaction.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 20:57:20 $
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

#ifndef _CPPCANVAS_TEXTACTION_HXX
#define _CPPCANVAS_TEXTACTION_HXX

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_RENDERSTATE_HPP__
#include <drafts/com/sun/star/rendering/RenderState.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_STRINGCONTEXT_HPP__
#include <drafts/com/sun/star/rendering/StringContext.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_TEXTDIRECTION_HPP_
#include <drafts/com/sun/star/rendering/TextDirection.hpp>
#endif

#ifndef _COMPHELPER_OPTIONALVALUE_HXX
#include <comphelper/optionalvalue.hxx>
#endif
#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

#include <action.hxx>
#include <cppcanvas/canvas.hxx>

class Point;

namespace drafts { namespace com { namespace sun { namespace star { namespace rendering
{
    class  XCanvasFont;
    class  XTextLayout;
} } } } }

/* Definition of internal::LineAction class */

namespace cppcanvas
{
    namespace internal
    {
        struct OutDevState;

        class TextAction : public Action
        {
        public:
            TextAction( const ::Point&                                                  rStartPoint,
                        const ::rtl::OUString&                                          rText,
                        sal_Int32                                                       nStartPos,
                        sal_Int32                                                       nLen,
                        const CanvasSharedPtr&                                          rCanvas,
                        const OutDevState&                                              rState,
                        const ::comphelper::OptionalValue< ::basegfx::B2DHomMatrix >&   rTextTransform );
            TextAction( const ::Point&                                                  rStartPoint,
                        const ::rtl::OUString&                                          rText,
                        sal_Int32                                                       nStartPos,
                        sal_Int32                                                       nLen,
                        ::com::sun::star::uno::Sequence< double >                       aOffsets,
                        const CanvasSharedPtr&                                          rCanvas,
                        const OutDevState&                                              rState,
                        const ::comphelper::OptionalValue< ::basegfx::B2DHomMatrix >&   rTextTransform      );
            virtual ~TextAction();

            virtual bool render( const ::basegfx::B2DHomMatrix& rTransformation ) const;

        private:
            // default: disabled copy/assignment
            TextAction(const TextAction&);
            TextAction& operator = ( const TextAction& );

            void init( const ::Point&                                                   rStartPoint,
                       const OutDevState&                                               rState,
                       const ::comphelper::OptionalValue< ::basegfx::B2DHomMatrix >&    rTextTransform );

            // TODO(P2): This is potentially a real mass object (every character might be
            // a separate TextAction), thus, make it as lightweight as possible. For
            // example, share common RenderState among several TextActions, use maOffsets
            // for the translation.

            ::com::sun::star::uno::Reference<
                ::drafts::com::sun::star::rendering::XCanvasFont >  mxFont;
            ::com::sun::star::uno::Reference<
                ::drafts::com::sun::star::rendering::XTextLayout >  mxTextLayout;
            ::drafts::com::sun::star::rendering::StringContext      maStringContext;
            ::com::sun::star::uno::Sequence< double >               maOffsets;
            CanvasSharedPtr                                         mpCanvas;
            ::drafts::com::sun::star::rendering::RenderState        maState;
            sal_Int8                                                maTextDirection;
        };
    }
}

#endif /* _CPPCANVAS_TEXTACTION_HXX */
