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
package com.sun.star.wizards.web.export;

import com.sun.star.io.IOException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.ui.event.Task;
import com.sun.star.wizards.web.data.CGDocument;
import com.sun.star.wizards.web.data.CGSession;

/**
 * @author rpiterman
 *
 * To change the template for this generated type comment go to
 * Window>Preferences>Java>Code Generation>Code and Comments
 */
public class ImpressHTMLExporter extends ConfiguredExporter
{

    private static final Integer SMALL_IMAGE = new Integer(512);
    private static final Integer MEDIUM_IMAGE = new Integer(640);
    private static final Integer LARGE_IMAGE = new Integer(800);

    public boolean export(CGDocument source, String targetDirectory, XMultiServiceFactory xmsf, Task task) throws IOException
    {

        /* here set some filter specific properties.
         * other properties, which are not dependant on
         * user input are set through the exporter
         * configuration.
         */

        CGSession session = getSession(source);

        props.put("Author", source.cp_Author);
        props.put("Email", session.cp_GeneralInfo.cp_Email);
        props.put("HomepageURL", getHomepageURL(session));
        props.put("UserText", source.cp_Title);

        props.put(PropertyNames.PROPERTY_WIDTH, getImageWidth(session));

        /*
         * props.put("BackColor",...);
         * props.put("TextColor",...);
         * props.put("LinkColor",...);
         * props.put("VLinkColor",...);
         * props.put("ALinkColor",...);
         */
        props.put("UseButtonSet", new Integer(session.cp_Design.cp_IconSet));


        //now export
        return super.export(source, targetDirectory, xmsf, task);

    }

    private String getHomepageURL(CGSession session)
    {
        return "../" +
                (exporter.cp_OwnDirectory ? "../index.html" : "index.html");
    }

    private Integer getImageWidth(CGSession session)
    {
        switch (session.cp_Design.cp_OptimizeDisplaySize)
        {
            case 0:
                return SMALL_IMAGE;
            case 1:
                return MEDIUM_IMAGE;
            case 2:
                return LARGE_IMAGE;
        }
        return MEDIUM_IMAGE;
    }

    private CGSession getSession(CGDocument doc)
    {
        return doc.getSettings().cp_DefaultSession;
    }
}
