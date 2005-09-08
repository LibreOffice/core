/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VTitle.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:52:29 $
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
#ifndef _CHART2_VTITLE_HXX
#define _CHART2_VTITLE_HXX

#ifndef _COM_SUN_STAR_CHART2_XTITLE_HPP_
#include <com/sun/star/chart2/XTitle.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class VTitle
{
public:
    VTitle( const ::com::sun::star::uno::Reference<
                  ::com::sun::star::chart2::XTitle > & xTitle
                  , double fAdditionalRotationAngleDegree=0.0 ); //the shape will be rotated by the sum of this value and the rotation given by the title model
    virtual ~VTitle();

    void SAL_CALL init( const ::com::sun::star::uno::Reference<
                     ::com::sun::star::drawing::XShapes >& xTargetPage
             , const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory);

    void    createShapes( const ::com::sun::star::awt::Point& rPos
                          , const ::com::sun::star::awt::Size& rReferenceSize );

    ::com::sun::star::awt::Size getFinalSize() const;
    void    changePosition( const ::com::sun::star::awt::Point& rPos );

private:
    ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShapes >            m_xTarget;
    ::com::sun::star::uno::Reference<
                    ::com::sun::star::lang::XMultiServiceFactory>   m_xShapeFactory;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XTitle >                  m_xTitle;
    ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShape >             m_xShape;

    double      m_fRotationAngleDegree;
    sal_Int32   m_nXPos;
    sal_Int32   m_nYPos;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif

