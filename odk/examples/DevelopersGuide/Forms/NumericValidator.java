/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NumericValidator.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:09:16 $
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
public class NumericValidator extends ControlValidator
{

    /** Creates a new instance of NumericValidator */
    public NumericValidator( )
    {
    }

    public String explainInvalid( Object Value )
    {
        try
        {
            double value = ((Double)Value).doubleValue();
            if ( Double.compare( Double.NaN, value ) == 0 )
                return "This is NotANumber";
            if ( !isProperRange( value ) )
                return "The value must be between 0 and 100";
            if ( !isProperDigitCount( value ) )
                return "The value must have at most one decimal digit";
        }
        catch( java.lang.Exception e )
        {
            return "This is no valid number";
        }
        return "";
    }

    public boolean isValid( Object Value )
    {
        try
        {
            double value = ((Double)Value).doubleValue();
            if ( Double.compare( Double.NaN, value ) == 0 )
                return false;
            if ( !isProperRange( value ) )
                return false;
            if ( !isProperDigitCount( value ) )
                return false;
            return true;
        }
        catch( java.lang.Exception e )
        {
        }
        return false;
    }

    private boolean isProperRange( double value)
    {
        return ( value >= 0 ) && ( value <= 100 );
    }

    private boolean isProperDigitCount( double value)
    {
        return ( java.lang.Math.floor( value * 10 ) == value * 10 );
    }
}
