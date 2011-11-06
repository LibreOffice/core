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


package com.sun.star.wizards.web.data;

import com.sun.star.wizards.ui.UIConsts;
import java.util.Hashtable;
import java.util.Map;

import javax.xml.transform.*;
import javax.xml.transform.stream.StreamSource;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.FileAccess;

public class CGLayout extends ConfigSetItem
{

    public String cp_Name;
    public String cp_FSName;
    private Map templates;

    private void createTemplates(XMultiServiceFactory xmsf) throws Exception
    {

        templates = new Hashtable(3);

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

    public Map getTemplates(XMultiServiceFactory xmsf) throws Exception
    {

        // TODO uncomment...
        // if (templates==null)
        createTemplates(xmsf);

        return templates;
    }
}
