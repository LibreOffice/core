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
package org.libreoffice.report.pentaho.parser;

public final class AttributeSpecification
{

    private final String namespaceUrl;
    private final String attributName;
    private final String mapperImplementation;

    public AttributeSpecification(String namespaceUrl, String attributName, String mapperImplementation)
    {
        this.namespaceUrl = namespaceUrl;
        this.attributName = attributName;
        this.mapperImplementation = mapperImplementation;
    }

    public String getAttributName()
    {
        return attributName;
    }

    public String getMapperImplementation()
    {
        return mapperImplementation;
    }

    public String getNamespaceUrl()
    {
        return namespaceUrl;
    }
}
