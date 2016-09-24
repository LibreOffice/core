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
package org.libreoffice.report.pentaho.model;

import java.util.ArrayList;
import java.util.List;

public class ObjectOleElement extends ReportElement
{

    private String url;
    private final List<String> masterfields;
    private final List<String> detailfields;
    private String classid;

    public ObjectOleElement()
    {
        masterfields = new ArrayList<String>();
        detailfields = new ArrayList<String>();
    }

    public String getClassid()
    {
        return classid;
    }

    public List<String> getDetailfields()
    {
        return detailfields;
    }

    public List<String> getMasterfields()
    {
        return masterfields;
    }

    public String getUrl()
    {
        return url;
    }

    public void setClassId(final String classid)
    {
        this.classid = classid;
    }

    public void setUrl(final String _url)
    {
        url = _url;
    }

    public void addMasterDetailFields(final String master, final String detail)
    {
        if (master != null)
        {
            masterfields.add(master);
            detailfields.add(detail == null ? master : detail);
        }
    }
}
