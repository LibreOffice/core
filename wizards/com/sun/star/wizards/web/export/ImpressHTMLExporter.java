/*************************************************************************
 *
 *  $RCSfile: ImpressHTMLExporter.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $  $Date: 2004-05-19 13:20:45 $
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

package com.sun.star.wizards.web.export;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.ui.event.Task;
import com.sun.star.wizards.web.data.CGDocument;
import com.sun.star.wizards.web.data.CGSession;

/**
 * @author rpiterman
 *
 * To change the template for this generated type comment go to
 * Window>Preferences>Java>Code Generation>Code and Comments
 */
public class ImpressHTMLExporter extends ConfiguredExporter {

    private static final Integer SMALL_IMAGE = new Integer(512);
    private static final Integer MEDIUM_IMAGE = new Integer(640);
    private static final Integer LARGE_IMAGE = new Integer(800);

    public boolean export(CGDocument source, String targetDirectory ,XMultiServiceFactory xmsf, Task task) {

        /* here set some filter specific properties.
         * other properties, which are not dependant on
         * user input are set through the exporter
         * configuration.
         */

        CGSession session = getSession(source);

        props.put("Author",source.cp_Author);
        props.put("Email",session.cp_GeneralInfo.cp_Email);
        props.put("HomepageURL",getHomepageURL(session));
        props.put("UserText",source.cp_Title);

        props.put("Width",getImageWidth(session));

        /*
         * props.put("BackColor",...);
         * props.put("TextColor",...);
         * props.put("LinkColor",...);
         * props.put("VLinkColor",...);
         * props.put("ALinkColor",...);
         */
        props.put("UseButtonSet",new Integer(session.cp_Design.cp_IconSet));


        //now export
        return super.export(source, targetDirectory, xmsf, task);

    }

    private String getHomepageURL(CGSession session) {
        return "../" +
             ( exporter.cp_OwnDirectory ? "../index.html" : "index.html" ) ;
    }

    private Integer getImageWidth(CGSession session) {
        switch (session.cp_Design.cp_OptimizeDisplaySize) {
            case 0 : return SMALL_IMAGE;
            case 1 : return MEDIUM_IMAGE;
            case 2 : return LARGE_IMAGE;
        }
        return MEDIUM_IMAGE;
    }

    private CGSession getSession(CGDocument doc) {
        return doc.getSettings().cp_DefaultSession;
    }

}
