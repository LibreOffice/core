/*************************************************************************
 *
 *  $RCSfile: calendar_hijri.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: bustamam $ $Date: 2002-03-26 13:36:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
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
#ifndef _I18N_CALENDAR_HIJRI_HXX_
#define _I18N_CALENDAR_HIJRI_HXX_

#include "calendar_gregorian.hxx"


//  ----------------------------------------------------
//  class Calendar_hijri
//  ----------------------------------------------------

namespace com { namespace sun { namespace star { namespace i18n {

class Calendar_hijri : public Calendar_gregorian
{
public:

    // Constructors
    Calendar_hijri();

protected:
    sal_Bool SAL_CALL convertValue( sal_Int16 fieldIndex ) throw(com::sun::star::uno::RuntimeException);

    // radians per degree (pi/180)
    static const double RadPerDeg;

    // Synodic Period (mean time between 2 successive new moon: 29d, 12 hr, 44min, 3sec
    static const double SynPeriod;

    static const double SynMonth;           // Solar days in a year/SynPeriod

    // Julian day on Jan 1, 1900
    static const double jd1900;

    // Reference point: September 1984 25d 3h 10m UT. == 1405 Hijri == 1048 Synodial month from 1900
    static const sal_Int32 SynRef;
    static const sal_Int32 GregRef;

    // Local time (Saudi Arabia)
    static const double SA_TimeZone;        // Time Zone

    // Period between 1.30pm - 6:30pm
    static const double EveningPeriod;

    // "Leap" years
    static const sal_Int32 LeapYear[];

private:
    double NewMoon(sal_Int32 n);
    void getHijri(sal_Int32 *day, sal_Int32 *month, sal_Int32 *year);
    double getJulianDay();
};

} } } }

#endif
