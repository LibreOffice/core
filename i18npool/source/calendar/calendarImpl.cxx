/*************************************************************************
 *
 *  $RCSfile: calendarImpl.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-30 14:39:09 $
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

#include "calendarImpl.hxx"
#include "localedata.hxx"
#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;
using namespace ::rtl;

#define ERROR RuntimeException()

CalendarImpl::CalendarImpl(const Reference< XMultiServiceFactory > &rxMSF) : xMSF(rxMSF)
{
}

CalendarImpl::~CalendarImpl()
{
    // Clear lookuptable
    for (sal_Int32 l = 0; l < lookupTable.size(); l++)
        delete lookupTable[l];
    lookupTable.clear();
}


void SAL_CALL
CalendarImpl::loadDefaultCalendar( const Locale& rLocale ) throw(RuntimeException)
{
    Sequence< Calendar> xC = LocaleData().getAllCalendars(rLocale);
    for (sal_Int32 i = 0; i < xC.getLength(); i++) {
        if (xC[i].Default) {
            loadCalendar(xC[i].Name, rLocale);
            return;
        }
    }
    throw ERROR;
}

void SAL_CALL
CalendarImpl::loadCalendar(const OUString& uniqueID, const Locale& rLocale ) throw (RuntimeException)
{
    Reference < XExtendedCalendar > xOldCalendar( xCalendar );  // backup
    sal_Int32 i;

    for (i = 0; i < lookupTable.size(); i++) {
        lookupTableItem *listItem = lookupTable[i];
        if (uniqueID == listItem->uniqueID) {
            xCalendar = listItem->xCalendar;
            break;
        }
    }

    if (i >= lookupTable.size()) {
        Reference < XInterface > xI = xMSF->createInstance(
                OUString::createFromAscii("com.sun.star.i18n.Calendar_") + uniqueID);

        if ( ! xI.is() ) {
            // check if the calendar is defined in localedata, load gregorian calendar service.
            Sequence< Calendar> xC = LocaleData().getAllCalendars(rLocale);
            for (i = 0; i < xC.getLength(); i++) {
                if (uniqueID == xC[i].Name) {
                    xI = xMSF->createInstance(
                        OUString::createFromAscii("com.sun.star.i18n.Calendar_gregorian"));
                    break;
                }
            }
        }

        if ( xI.is() )
            xI->queryInterface(::getCppuType((const Reference< XExtendedCalendar>*)0)) >>= xCalendar;
        else
            throw ERROR;

        lookupTable.push_back( new lookupTableItem(uniqueID, xCalendar) );
    }

    if ( !xCalendar.is() )
    {
        xCalendar = xOldCalendar;
        throw ERROR;
    }

    try
    {
        xCalendar->loadCalendar(uniqueID, rLocale);
    }
    catch ( Exception& e )
    {   // restore previous calendar and re-throw
        xCalendar = xOldCalendar;
        throw;
    }
}

Calendar SAL_CALL
CalendarImpl::getLoadedCalendar() throw(RuntimeException)
{
    if (xCalendar.is())
        return xCalendar->getLoadedCalendar();
    else
        throw ERROR ;
}

Sequence< OUString > SAL_CALL
CalendarImpl::getAllCalendars( const Locale& rLocale ) throw(RuntimeException)
{
    Sequence< Calendar> xC = LocaleData().getAllCalendars(rLocale);
    sal_Int32 nLen = xC.getLength();
    Sequence< OUString > xSeq( nLen );
    for (sal_Int32 i = 0; i < nLen; i++)
        xSeq[i] = xC[i].Name;
    return xSeq;
}

void SAL_CALL
CalendarImpl::setDateTime( double timeInDays ) throw(RuntimeException)
{
    if (xCalendar.is())
        xCalendar->setDateTime( timeInDays );
    else
        throw ERROR ;
}

double SAL_CALL
CalendarImpl::getDateTime() throw(RuntimeException)
{
    if (xCalendar.is())
        return xCalendar->getDateTime();
    else
        throw ERROR ;
}

OUString SAL_CALL
CalendarImpl::getUniqueID() throw(RuntimeException)
{
    if (xCalendar.is())
        return xCalendar->getUniqueID();
    else
        throw ERROR ;
}

void SAL_CALL
CalendarImpl::setValue( sal_Int16 fieldIndex, sal_Int16 value ) throw(RuntimeException)
{
    if (xCalendar.is())
        xCalendar->setValue( fieldIndex, value );
    else
        throw ERROR ;
}

sal_Int16 SAL_CALL
CalendarImpl::getValue( sal_Int16 fieldIndex ) throw(RuntimeException)
{
    if (xCalendar.is())
        return xCalendar->getValue( fieldIndex );
    else
        throw ERROR ;
}

void SAL_CALL
CalendarImpl::addValue( sal_Int16 fieldIndex, sal_Int32 amount ) throw(RuntimeException)
{
    if (xCalendar.is())
        xCalendar->addValue( fieldIndex, amount);
    else
        throw ERROR ;
}

sal_Int16 SAL_CALL
CalendarImpl::getFirstDayOfWeek() throw(RuntimeException)
{
    if (xCalendar.is())
        return xCalendar->getFirstDayOfWeek();
    else
        throw ERROR ;
}

void SAL_CALL
CalendarImpl::setFirstDayOfWeek( sal_Int16 day )
throw(RuntimeException)
{
    if (xCalendar.is())
        xCalendar->setFirstDayOfWeek(day);
    else
        throw ERROR ;
}

void SAL_CALL
CalendarImpl::setMinimumNumberOfDaysForFirstWeek( sal_Int16 days ) throw(RuntimeException)
{
    if (xCalendar.is())
        xCalendar->setMinimumNumberOfDaysForFirstWeek(days);
    else
        throw ERROR ;
}

sal_Int16 SAL_CALL
CalendarImpl::getMinimumNumberOfDaysForFirstWeek() throw(RuntimeException)
{
    if (xCalendar.is())
        return xCalendar->getMinimumNumberOfDaysForFirstWeek();
    else
        throw ERROR ;
}


OUString SAL_CALL
CalendarImpl::getDisplayName( sal_Int16 displayIndex, sal_Int16 idx, sal_Int16 nameType ) throw(RuntimeException)
{
    if (xCalendar.is())
        return xCalendar->getDisplayName( displayIndex, idx, nameType );
    else
        throw ERROR ;
}

sal_Int16 SAL_CALL
CalendarImpl::getNumberOfMonthsInYear() throw(RuntimeException)
{
    if (xCalendar.is())
        return xCalendar->getNumberOfMonthsInYear();
    else
        throw ERROR ;
}


sal_Int16 SAL_CALL
CalendarImpl::getNumberOfDaysInWeek() throw(RuntimeException)
{
    if (xCalendar.is())
        return xCalendar->getNumberOfDaysInWeek();
    else
        throw ERROR ;
}


Sequence< CalendarItem > SAL_CALL
CalendarImpl::getMonths() throw(RuntimeException)
{
    if (xCalendar.is())
        return xCalendar->getMonths();
    else
        throw ERROR ;
}


Sequence< CalendarItem > SAL_CALL
CalendarImpl::getDays() throw(RuntimeException)
{
    if (xCalendar.is())
        return xCalendar->getDays();
    else
        throw ERROR ;
}


sal_Bool SAL_CALL
CalendarImpl::isValid() throw(RuntimeException)
{
    if (xCalendar.is())
        return xCalendar->isValid();
    else
        throw ERROR ;
}

OUString SAL_CALL
CalendarImpl::getDisplayString( sal_Int32 nCalendarDisplayCode, sal_Int16 nNativeNumberMode )
    throw (RuntimeException)
{
    if (xCalendar.is())
        return xCalendar->getDisplayString(nCalendarDisplayCode, nNativeNumberMode);
    else
        throw ERROR ;
}

OUString SAL_CALL
CalendarImpl::getImplementationName(void) throw( RuntimeException )
{
    return OUString::createFromAscii("com.sun.star.i18n.CalendarImpl");
}

const sal_Char cCalendar[] = "com.sun.star.i18n.LocaleCalendar";

sal_Bool SAL_CALL
CalendarImpl::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return !rServiceName.compareToAscii(cCalendar);
}

Sequence< OUString > SAL_CALL
CalendarImpl::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii(cCalendar);
    return aRet;
}

