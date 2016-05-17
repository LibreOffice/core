/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

import java.io.File;
import java.net.MalformedURLException;

public class URLHelper
{
    /**
     * Because the office need URLs for loading/saving documents
     * we must convert used system paths.
     * And java use another notation for file URLs ... correct it.
     *
     * @param aSystemPath
     *          represent the file in system notation
     *
     * @return [String]
     *          a file url which represent the given system path
     */
    public static String getFileURLFromSystemPath( File aSystemPath )
    {
        String sFileURL = null;
        try
        {
            sFileURL = aSystemPath.toURI().toURL().toString();
        }
        catch( MalformedURLException exWrong )
        {
            sFileURL = null;
        }

        // problem of java: file URL's are coded with 1 slash instead of 2 or 3 ones!
        // => correct this problem first, otherwise office can't use these URL's
        if(
            (sFileURL                       != null ) &&
            (sFileURL.startsWith("file:/")  == true ) &&
            (sFileURL.startsWith("file://") == false)
          )
        {
            StringBuffer sWorkBuffer = new StringBuffer(sFileURL);
            sWorkBuffer.insert(6,"//");
            sFileURL = sWorkBuffer.toString();
        }

        return sFileURL;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
