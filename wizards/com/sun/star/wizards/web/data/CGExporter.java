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
