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
package com.sun.star.wizards.web.data;

import com.sun.star.wizards.common.ConfigGroup;
import com.sun.star.wizards.common.PropertyNames;

/**
 *
 * A Class which describes the publishing arguments
 * in a session.
 * Each session can contain different publishers, which are configured
 * through such a CGPublish object.
 */
public class CGPublish extends ConfigGroup
{

    public boolean cp_Publish;
    public String cp_URL;
    public String cp_Username;
    public String password;
    /**
     * cp_URL is the url given by the user
     * for this publisher. (in UCB URL form)
     * This one will be edited to result the "url"
     * field, which is the true url, ucb uses to publish.
     * It is used for example to add ftp username and password, or zip url
     */
    public String url;
    /**
     * if the user approved overwriting files in this publisher target
     */
    public boolean overwriteApproved;

    /**
     * here I get an URL from user input, and parse it to
     * a UCB url...
     * @param url
     */
    public void setURL(String path)
    {
        try
        {
            this.cp_URL = ((CGSettings) this.root).getFileAccess().getURL(path);
            overwriteApproved = false;
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }
    }

    public String getURL()
    {
        try
        {
            return ((CGSettings) this.root).getFileAccess().getPath(cp_URL, null);
        }
        catch (Exception e)
        {
            e.printStackTrace();
            return PropertyNames.EMPTY_STRING;
        }
    }

    private String ftpURL()
    {
        return "ftp://" + cp_Username +
                ((password != null && password.length() > 0) ? ":" + password : PropertyNames.EMPTY_STRING) + "@" + cp_URL.substring(7);
    }
}
