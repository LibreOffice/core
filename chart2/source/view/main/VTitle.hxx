/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef _CHART2_VTITLE_HXX
#define _CHART2_VTITLE_HXX

#include <com/sun/star/chart2/XTitle.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

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
                  ::com::sun::star::chart2::XTitle > & xTitle );
    virtual ~VTitle();

    void    init( const ::com::sun::star::uno::Reference<
                     ::com::sun::star::drawing::XShapes >& xTargetPage
             , const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory
             , const rtl::OUString& rCID );

    void    createShapes( const ::com::sun::star::awt::Point& rPos
                          , const ::com::sun::star::awt::Size& rReferenceSize );

    double getRotationAnglePi() const;
    ::com::sun::star::awt::Size getUnrotatedSize() const;
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
    rtl::OUString   m_aCID;

    double      m_fRotationAngleDegree;
    sal_Int32   m_nXPos;
    sal_Int32   m_nYPos;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
