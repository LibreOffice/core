/*************************************************************************
 *
 *  $RCSfile: CGPublish.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $  $Date: 2004-05-19 13:18:10 $
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
 */

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
     * for this publisher.
     * This one will be edited to result the "url"
     * field, which is the true url, ucb uses to publish.
     */
    public String url;

    public void setURL(String url) {
        throw new NullPointerException("CGPublish.setURL not allowed.");
        //cp_URL = url;
    }

    public String getURL() {
        return JavaTools.convertfromURLNotation(cp_URL);
    }

    private String ftpURL() {
        return "ftp://" + cp_Username +
          ((password!=null && password.length()>0) ? ":" + password : "")
          + "@" + cp_URL.substring(7);
    }
}
