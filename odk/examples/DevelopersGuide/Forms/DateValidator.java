/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DateValidator.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2006-03-15 09:27:30 $
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

import com.sun.star.form.binding.*;

/**
 *
 * @author  fs@openoffice.org
 */
public class DateValidator extends ControlValidator
{

    /** Creates a new instance of NumericValidator */
    public DateValidator( )
    {
    }

    public String explainInvalid( Object Value )
    {
        try
        {
            if ( isVoid( Value ) )
                return "empty input";

            com.sun.star.util.Date dateValue = (com.sun.star.util.Date)Value;
            if ( isDedicatedInvalidDate( dateValue ) )
                return "this is no valid date";

            if ( !isNextMonthsDate( dateValue ) )
                return "date must denote a day in the current month";
        }
        catch( java.lang.Exception e )
        {
            return "oops. What did you enter for this to happen?";
        }
        return "";
    }

    public boolean isValid( Object Value )
    {
        try
        {
            if ( isVoid( Value ) )
                return false;

            com.sun.star.util.Date dateValue = (com.sun.star.util.Date)
                com.sun.star.uno.AnyConverter.toObject(
                    com.sun.star.util.Date.class, Value);
            if ( isDedicatedInvalidDate( dateValue ) )
                return false;

            if ( !isNextMonthsDate( dateValue ) )
                return false;
            return true;
        }
        catch( java.lang.Exception e )
        {
            e.printStackTrace( System.err );
        }
        return false;
    }

    private boolean isDedicatedInvalidDate( com.sun.star.util.Date dateValue )
    {
        return ( dateValue.Day == 0 ) && ( dateValue.Month == 0 ) && ( dateValue.Year == 0 );
    }

    private boolean isNextMonthsDate( com.sun.star.util.Date dateValue )
    {
        int overallMonth = dateValue.Year * 12 + dateValue.Month - 1;

        int todaysMonth = new java.util.Date().getMonth();
        int todaysYear = new java.util.Date().getYear() + 1900;
        int todaysOverallMonth = todaysYear * 12 + todaysMonth;

        return overallMonth == todaysOverallMonth;
    }
}
