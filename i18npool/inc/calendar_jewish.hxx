/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: calendar_jewish.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:49:29 $
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
#ifndef _I18N_CALENDAR_JEWISH_HXX_
#define _I18N_CALENDAR_JEWISH_HXX_

#include "calendar_gregorian.hxx"

//  ----------------------------------------------------
//  class Calendar_jewish
//  ----------------------------------------------------

namespace com { namespace sun { namespace star { namespace i18n {

class Calendar_jewish : public Calendar_gregorian
{
public:
    // Constructors
    Calendar_jewish();

    // Methods in XExtendedCalendar
    virtual rtl::OUString SAL_CALL getDisplayString( sal_Int32 nCalendarDisplayCode, sal_Int16 nNativeNumberMode ) throw (com::sun::star::uno::RuntimeException);

protected:
    void SAL_CALL mapToGregorian() throw(com::sun::star::uno::RuntimeException);
    void SAL_CALL mapFromGregorian() throw(com::sun::star::uno::RuntimeException);
};

} } } }

#endif
