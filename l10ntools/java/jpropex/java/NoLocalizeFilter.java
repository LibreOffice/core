/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


import java.io.*;
import java.util.regex.*;

// exit if the sequence x-no-localize is found in stream!
public class NoLocalizeFilter extends FilterInputStream
{
    InputStream in;
    Pattern p = Pattern.compile("#[\\s]*x-no-translate");

    public NoLocalizeFilter( InputStream in ) {
        super(in);
        this.in = in;
    }
    public int read(byte[] b, int off, int len) throws IOException
    {
        String search = new String( b );
        Matcher m = p.matcher( search );
        if( m.find() )
        //if( search.contains("x-no-translate" ) )  // TODO: fixme!
        {
            System.out.println("found x-no-translate");
            in.close();
            close();
            System.exit( 0 );
        }
        return in.read( b , off , len );
    }
}
