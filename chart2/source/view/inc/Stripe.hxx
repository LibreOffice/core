/*************************************************************************
 *
 *  $RCSfile: Stripe.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-07 16:44:23 $
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
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _CHART_STRIPE_HXX
#define _CHART_STRIPE_HXX

#ifndef _COM_SUN_STAR_DRAWING_POSITION3D_HPP_
#include <com/sun/star/drawing/Position3D.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_DIRECTION3D_HPP_
#include <com/sun/star/drawing/Direction3D.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/** A Stripe represents a 2 dimensional foursquare plane in a 3 dimaensional room.

@todo could: it is not necessary to have 4 point members here; it would be sufficient to have one point and 2 directions
*/

class Stripe
{
public:
    Stripe( const ::com::sun::star::drawing::Position3D& rPoint1
        , const ::com::sun::star::drawing::Direction3D& rDirectionToPoint2
        , const ::com::sun::star::drawing::Direction3D& rDirectionToPoint4 );

/*
    Stripe( const ::com::sun::star::drawing::Position3D& rPoint1
        , const ::com::sun::star::drawing::Position3D& rPoint2
        , const ::com::sun::star::drawing::Position3D& rPoint3
        , const ::com::sun::star::drawing::Position3D& rPoint4 );*/


    ::com::sun::star::uno::Any      getPolyPolygonShape3D() const;
    ::com::sun::star::uno::Any      getNormalsPolyPolygonShape3D() const;

    ::com::sun::star::drawing::Position3D GetPosition1() const { return m_aPoint1; }
    ::com::sun::star::drawing::Position3D GetPosition2() const { return m_aPoint2; }
    ::com::sun::star::drawing::Position3D GetPosition3() const { return m_aPoint3; }
    ::com::sun::star::drawing::Position3D GetPosition4() const { return m_aPoint4; }

    ::com::sun::star::drawing::Direction3D GetDirectionTo4() const;

private:
    ::com::sun::star::drawing::Position3D m_aPoint1;
    ::com::sun::star::drawing::Position3D m_aPoint2;
    ::com::sun::star::drawing::Position3D m_aPoint3;
    ::com::sun::star::drawing::Position3D m_aPoint4;

    ::com::sun::star::drawing::Direction3D
                                    getNormal() const;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
