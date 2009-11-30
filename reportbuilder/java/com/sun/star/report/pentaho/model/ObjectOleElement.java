/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ObjectOleElement.java,v $
 * $Revision: 1.4 $
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
package com.sun.star.report.pentaho.model;

import java.util.ArrayList;
import java.util.List;

/**
 *
 * @author Ocke Janssen
 */
public class ObjectOleElement extends ReportElement{

    private String url;
    private final List masterfields;
    private final List detailfields;
    private String classid;

    public String getClassid() {
        return classid;
    }

    public List getDetailfields() {
        return detailfields;
    }

    public List getMasterfields() {
        return masterfields;
    }

    public ObjectOleElement()
    {
        masterfields = new ArrayList();
        detailfields = new ArrayList();
    }

    public String getUrl() {
        return url;
    }

    public void setClassId(final String classid) {
        this.classid = classid;
    }
    public void setUrl(final String _url ){
        url = _url;
    }

    public void addMasterDetailFields(final String master,final String detail){
        if ( master != null ){
            masterfields.add(master);
            detailfields.add(detail == null ? master : detail);
        }
    }

}
