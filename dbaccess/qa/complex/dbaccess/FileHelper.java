/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Parser.java,v $
 * $Revision: 1.1.6.2 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package complex.dbaccess;

public class FileHelper
{
    static public String getOOoCompatibleFileURL( String _javaFileURL )
    {
        String returnURL = _javaFileURL;
        if ( ( returnURL.indexOf( "file:/" ) == 0 ) && ( returnURL.indexOf( "file:///" ) == -1 ) )
        {
            // for some reason, the URLs here in Java start with "file:/" only, instead of "file:///"
            // Some of the office code doesn't like this ...
            returnURL = "file:///" + returnURL.substring( 6 );
        }
        return returnURL;
    }
}
