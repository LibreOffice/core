/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CGLayout.java,v $
 * $Revision: 1.8 $
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
