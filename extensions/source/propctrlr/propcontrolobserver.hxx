/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propcontrolobserver.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 12:02:30 $
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
#ifndef PROPCONTROLOBSERVER_HXX
#define PROPCONTROLOBSERVER_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_INSPECTION_XPROPERTYCONTROL_HPP_
#include <com/sun/star/inspection/XPropertyControl.hpp>
#endif
/** === end UNO includes === **/

//........................................................................
namespace pcr
{
//........................................................................

    //====================================================================
    //= IPropertyControlObserver
    //====================================================================
    /** non-UNO version of the XPropertyControlObserver
    */
    class IPropertyControlObserver
    {
    public:
        virtual void    focusGained( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >& _Control ) = 0;
        virtual void    valueChanged( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >& _Control ) = 0;
    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // PROPCONTROLOBSERVER_HXX

