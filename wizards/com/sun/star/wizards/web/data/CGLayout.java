/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CGLayout.java,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-03 13:56:21 $
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
 ************************************************************************/package com.sun.star.wizards.web.data;

import com.sun.star.wizards.ui.UIConsts;
import java.util.Hashtable;
import java.util.Map;

import javax.xml.transform.*;
import javax.xml.transform.stream.StreamSource;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.FileAccess;

public class CGLayout extends ConfigSetItem {
    public String cp_Name;
    public String cp_FSName;

    private Map templates;

    private void createTemplates(XMultiServiceFactory xmsf) throws Exception {

        templates = new Hashtable(3);

        TransformerFactory tf = TransformerFactory.newInstance();

        String workPath = getSettings().workPath;
        FileAccess fa = new FileAccess(xmsf);
        String stylesheetPath = fa.getURL(getSettings().workPath,"layouts/"+ cp_FSName);

        String[] files  = fa.listFiles(stylesheetPath,false);

        for (int i = 0; i<files.length; i++)
            if (FileAccess.getExtension(files[i]).equals("xsl"))
                templates.put(FileAccess.getFilename(files[i]), tf.newTemplates( new StreamSource(files[i]) ));

    }

    public Object[] getImageUrls() {
        Object[] sRetUrls = new Object[1];
        int ResId = UIConsts.RID_IMG_WEB + (cp_Index *2);
        return new Integer[]{new Integer(ResId), new Integer(ResId + 1)};
    }

    public Map getTemplates(XMultiServiceFactory xmsf) throws Exception {

        // TODO uncomment...
        // if (templates==null)
          createTemplates(xmsf);

        return templates;
    }

}
