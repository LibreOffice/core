/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TextValidator.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:11:06 $
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
public class TextValidator extends ControlValidator
{

    /** Creates a new instance of NumericValidator */
    public TextValidator( )
    {
    }

    public String explainInvalid( Object Value )
    {
        try
        {
            String value = (String)Value;
            if ( containsZs( value ) )
                return "No Z's allowed here";
            if ( !isProperChunks( value ) )
                return "Need 3 * n characters";
        }
        catch( java.lang.Exception e )
        {
            return "ooops. Unknown error";
        }
        return "";
    }

    public boolean isValid( Object Value )
    {
        try
        {
            String value = (String)Value;
            if ( containsZs( value ) )
                return false;
            if ( !isProperChunks( value ) )
                return false;
            return true;
        }
        catch( java.lang.Exception e )
        {
        }
        return false;
    }

    private boolean isProperChunks( String value )
    {
        return ( value.length() % 3 ) == 0;
    }

    private boolean containsZs( String value )
    {
        if  (  ( value.indexOf( 'Z' ) != -1 )
            || ( value.indexOf( 'z' ) != -1 )
            )
            return true;
        return false;
    }
}
