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

import com.sun.star.report.pentaho.OfficeNamespaces;

import org.jfree.report.structure.Section;

/**
 * A page layout describes the physical properties of a page. It is equal to
 * an @page rule in CSS.
 *
 * @author Thomas Morgner
 * @since 13.03.2007
 */
public class PageLayout extends Section
{

    public PageLayout()
    {
        setNamespace(OfficeNamespaces.STYLE_NS);
        setType("page-layout");
    }

    public String getStyleName()
    {
        return (String) getAttribute(OfficeNamespaces.STYLE_NS, "name");
    }

    public void setStyleName(final String name)
    {
        setAttribute(OfficeNamespaces.STYLE_NS, "name", name);
    }

    public Section getHeaderStyle()
    {
        return (Section) findFirstChild(OfficeNamespaces.STYLE_NS, "header-style");
    }

    public Section getFooterStyle()
    {
        return (Section) findFirstChild(OfficeNamespaces.STYLE_NS, "footer-style");
    }
}
