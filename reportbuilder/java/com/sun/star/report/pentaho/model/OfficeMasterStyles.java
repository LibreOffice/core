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


package com.sun.star.report.pentaho.model;

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
 * @author Thomas Morgner
 * @since 13.03.2007
 */
public class OfficeMasterStyles extends Element
{

    private final Map masterPages;
    private final Section otherNodes;

    public OfficeMasterStyles()
    {
        masterPages = new HashMap();
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
        return (OfficeMasterPage) masterPages.get(name);
    }

    public OfficeMasterPage[] getAllMasterPages()
    {
        return (OfficeMasterPage[]) masterPages.values().toArray(new OfficeMasterPage[masterPages.size()]);
    }

    public Section getOtherNodes()
    {
        return otherNodes;
    }
}
