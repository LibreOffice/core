/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ImpressHTMLExporter.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 10:03:48 $
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
