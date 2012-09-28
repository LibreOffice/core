/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package complex.reportdesign;

import java.io.File;
import java.net.URI;
import java.net.URISyntaxException;

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
