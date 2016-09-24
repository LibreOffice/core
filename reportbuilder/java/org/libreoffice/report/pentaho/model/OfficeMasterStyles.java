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

import java.util.HashMap;
import java.util.Map;

import org.jfree.report.structure.Element;
import org.jfree.report.structure.Section;


/**
 * The master-styles section can have either a master-page, handout-master
 * or draw-layer-set. (The latter ones are ignored for the reporting purposes,
 * they are PowerPoint related.)
 *
 * There is no documentation how the system selects a master-page if there is
 * no master-page assigned to the paragraph. However, it seems as if the
 * master-page called 'Standard' is used as initial default.
 *
 * @since 13.03.2007
 */
public class OfficeMasterStyles extends Element
{

    private final Map<String,OfficeMasterPage> masterPages;
    private final Section otherNodes;

    public OfficeMasterStyles()
    {
        masterPages = new HashMap<String,OfficeMasterPage>();
        otherNodes = new Section();
    }

    public void addMasterPage(final OfficeMasterPage masterPage)
    {
        if (masterPage == null)
        {
            throw new NullPointerException();
        }
        this.masterPages.put(masterPage.getStyleName(), masterPage);
    }

    public OfficeMasterPage getMasterPage(final String name)
    {
        return masterPages.get(name);
    }

    public OfficeMasterPage[] getAllMasterPages()
    {
        return masterPages.values().toArray(new OfficeMasterPage[masterPages.size()]);
    }

    public Section getOtherNodes()
    {
        return otherNodes;
    }
}
