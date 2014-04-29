/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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

    private static final Integer SMALL_IMAGE = 512;
    private static final Integer MEDIUM_IMAGE = 640;
    private static final Integer LARGE_IMAGE = 800;

    public boolean export(CGDocument source, String targetDirectory, XMultiServiceFactory xmsf, Task task) throws IOException
    {

        /* here set some filter specific properties.
         * other properties, which are not dependent on
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
