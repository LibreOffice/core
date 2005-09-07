/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TimeValidator.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:11:22 $
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
public class TimeValidator extends ControlValidator
{

    /** Creates a new instance of NumericValidator */
    public TimeValidator( )
    {
    }

    public String explainInvalid( Object Value )
    {
        try
        {
            if ( isVoid( Value ) )
                return "empty input";

            com.sun.star.util.Time timeValue = (com.sun.star.util.Time)Value;
            if ( isInvalidTime( timeValue ) )
                return "this is no valid time";
            if ( !isFullHour( timeValue ) )
                return "time must denote a full hour";
        }
        catch( java.lang.Exception e )
        {
            return "this is no valid time";
        }
        return "";
    }

    public boolean isValid( Object Value )
    {
        try
        {
            if ( isVoid( Value ) )
                return false;

            com.sun.star.util.Time timeValue = (com.sun.star.util.Time)Value;
            if ( isInvalidTime( timeValue ) )
                return false;
            if ( !isFullHour( timeValue ) )
                return false;
            return true;
        }
        catch( java.lang.Exception e )
        {
            e.printStackTrace( System.err );
        }
        return false;
    }

    private boolean isInvalidTime( com.sun.star.util.Time timeValue )
    {
        return ( timeValue.Hours == -1 ) && ( timeValue.Minutes == -1 ) && ( timeValue.Seconds == -1 ) && ( timeValue.HundredthSeconds == -1 );
    }

    private boolean isFullHour( com.sun.star.util.Time timeValue )
    {
        return ( timeValue.Minutes == 0 ) && ( timeValue.Seconds == 0 ) && ( timeValue.HundredthSeconds == 0 );
    }
}
