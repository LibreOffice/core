/*
 *  ************************************************************************
 *
 *  DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  This file is part of OpenOffice.org.
 *
 *  OpenOffice.org is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License version 3
 *  only, as published by the Free Software Foundation.
 *
 *  OpenOffice.org is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License version 3 for more details
 *  (a copy is included in the LICENSE file that accompanied this code).
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  version 3 along with OpenOffice.org.  If not, see
 *  <http://www.openoffice.org/license.html>
 *  for a copy of the LGPLv3 License.
 *
 *  ************************************************************************
 */

package complex.reportdesign;

import java.io.File;
import java.net.URI;
import java.net.URISyntaxException;

/**
 *
 * @author ll93751
 */
public class FileURL
{
    String m_sFileURL;

    public FileURL(String _sFileURL)
    {
        m_sFileURL = _sFileURL;
    }
    public boolean exists()
    {
        try
        {
            final URI aURI = new URI(m_sFileURL);
            final File aFile = new File(aURI);
            return aFile.exists();
        }
        catch (URISyntaxException ex)
        {
            System.out.println("Error: URI is wrong. '" + m_sFileURL + "': " + ex.getMessage());
        }
        return false;
    }
}
