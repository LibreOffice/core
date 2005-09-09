/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CGPublish.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 10:02:17 $
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
package com.sun.star.wizards.web.data;

import com.sun.star.wizards.common.ConfigGroup;
import com.sun.star.wizards.common.JavaTools;


/**
 *
 * A Class which describes the publishing arguments
 * in a session.
 * Each session can contain different publishers, which are configured
 * through such a CGPublish object.
 */
public class CGPublish extends ConfigGroup {

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
    public void setURL(String path) {
        try {
            this.cp_URL = ((CGSettings)this.root).getFileAccess().getURL(path);
            overwriteApproved = false;
        }
        catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    public String getURL() {
        try {
            return  ((CGSettings)this.root).getFileAccess().getPath(cp_URL, null );
        } catch (Exception e) {
            e.printStackTrace();
            return "";
        }
    }

    private String ftpURL() {
        return "ftp://" + cp_Username +
          ((password!=null && password.length()>0) ? ":" + password : "")
          + "@" + cp_URL.substring(7);
    }
}
