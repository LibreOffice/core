/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: transformer.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 13:23:15 $
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

#ifndef _GOODIES_GRAPHICTRANSFORMER_HXX
#define _GOODIES_GRAPHICTRANSFORMER_HXX

#include <cppuhelper/implbase1.hxx>

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_GRAPHIC_XGRAPHICTRANSFORMER_HPP_
#include <com/sun/star/graphic/XGraphicTransformer.hpp>
#endif

using namespace com::sun::star;

namespace unographic {

// ----------------------
// - GraphicTransformer -
// ----------------------

typedef ::cppu::WeakAggImplHelper1<
    ::com::sun::star::graphic::XGraphicTransformer
    > GraphicTransformer_UnoImplHelper1;
class GraphicTransformer : public GraphicTransformer_UnoImplHelper1
{
    public:

    GraphicTransformer();
    ~GraphicTransformer();

    // XGraphicTransformer
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > SAL_CALL colorChange(
        const ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >& rGraphic,
            sal_Int32 nColorFrom, sal_Int8 nTolerance, sal_Int32 nColorTo, sal_Int8 nAlphaTo )
                throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

};

}

#endif
