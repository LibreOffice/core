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
package com.sun.star.wizards.web.data;

import com.sun.star.wizards.ui.UIConsts;
import java.util.HashMap;
import java.util.Map;

import javax.xml.transform.*;
import javax.xml.transform.stream.StreamSource;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.FileAccess;

public class CGLayout extends ConfigSetItem
{

    public String cp_Name;
    public String cp_FSName;
    private Map<String, Templates> templates;

    private void createTemplates(XMultiServiceFactory xmsf) throws Exception
    {

        templates = new HashMap<String, Templates>(3);

        TransformerFactory tf = TransformerFactory.newInstance();

        String workPath = getSettings().workPath;
        FileAccess fa = new FileAccess(xmsf);
        String stylesheetPath = fa.getURL(getSettings().workPath, "layouts/" + cp_FSName);

        String[] files = fa.listFiles(stylesheetPath, false);

        for (int i = 0; i < files.length; i++)
        {
            if (FileAccess.getExtension(files[i]).equals("xsl"))
            {
                templates.put(FileAccess.getFilename(files[i]), tf.newTemplates(new StreamSource(files[i])));
            }
        }
    }

    public Object[] getImageUrls()
    {
        Object[] sRetUrls = new Object[1];
        int ResId = UIConsts.RID_IMG_WEB + (cp_Index * 2);
        return new Integer[]
                {
                    new Integer(ResId), new Integer(ResId + 1)
                };
    }

    public Map<String, Templates> getTemplates(XMultiServiceFactory xmsf) throws Exception
    {

        // TODO uncomment...
        // if (templates==null)
        createTemplates(xmsf);

        return templates;
    }
}
