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
package org.libreoffice.report.pentaho.loader;

import java.io.Serializable;

/**
 * Creation-Date: Feb 22, 2007, 8:51:42 PM
 *
 */
public class InputResourceKey implements Serializable
{

    private static final long serialVersionUID = 2819901838705793075L;
    private final Object inputRepositoryId;
    private final String path;

    public InputResourceKey(final Object inputRepositoryId, final String path)
    {
        this.inputRepositoryId = inputRepositoryId;
        this.path = path;
    }

    public Object getInputRepositoryId()
    {
        return inputRepositoryId;
    }

    public String getPath()
    {
        return path;
    }

    public String toString()
    {
        return "InputResourceKey{" + "inputRepositoryId=" + inputRepositoryId + ", path='" + path + '\'' + '}';
    }
}
