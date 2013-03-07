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

import org.libreoffice.report.pentaho.OfficeNamespaces;
import org.libreoffice.report.pentaho.parser.office.DocumentContentReadHandler;

import org.pentaho.reporting.libraries.xmlns.parser.XmlDocumentInfo;
import org.pentaho.reporting.libraries.xmlns.parser.XmlFactoryModule;
import org.pentaho.reporting.libraries.xmlns.parser.XmlReadHandler;

public class StarXmlFactoryModule implements XmlFactoryModule
{

    public StarXmlFactoryModule()
    {
    }

    public XmlReadHandler createReadHandler(final XmlDocumentInfo documentInfo)
    {
        return new DocumentContentReadHandler();
    }

    public int getDocumentSupport(final XmlDocumentInfo documentInfo)
    {
        final String rootNamespace = documentInfo.getRootElementNameSpace();
        if (OfficeNamespaces.OFFICE_NS.equals(rootNamespace) && ("document-content".equals(documentInfo.getRootElement()) || "document".equals(documentInfo.getRootElement())))
        {
            return XmlFactoryModule.RECOGNIZED_BY_NAMESPACE;
        }
        return XmlFactoryModule.NOT_RECOGNIZED;
    }

    public String getDefaultNamespace(final XmlDocumentInfo documentInfo)
    {
        return null;
    }
}
