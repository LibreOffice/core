/*************************************************************************
 *
 *  $RCSfile: TextValidator.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-01-31 16:33:25 $
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
