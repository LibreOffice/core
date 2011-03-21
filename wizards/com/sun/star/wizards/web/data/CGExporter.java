/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

import com.sun.star.wizards.common.ConfigSet;
import com.sun.star.wizards.common.PropertyNames;

public class CGExporter extends ConfigSetItem
{

    public String cp_Name;
    public String cp_ExporterClass;
    public boolean cp_OwnDirectory;
    public boolean cp_SupportsFilename;
    public String cp_DefaultFilename;
    public String cp_Extension;
    public String cp_SupportedMimeTypes;
    public String cp_Icon;
    public String cp_TargetType;
    public boolean cp_Binary;
    public int cp_PageType;
    public String targetTypeName = PropertyNames.EMPTY_STRING;
    public ConfigSet cp_Arguments = new ConfigSet(CGArgument.class);

    public String toString()
    {
        return cp_Name;
    }

    public boolean supports(String mime)
    {
        return (cp_SupportedMimeTypes.equals(PropertyNames.EMPTY_STRING) || cp_SupportedMimeTypes.indexOf(mime) > -1);
    }
}
