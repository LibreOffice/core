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

package com.sun.star.script.framework.container;

import java.util.HashMap;
import java.util.Map;

public class ScriptEntry {

    private final String language;
    private final String languagename;
    private final String logicalname;
    private final String description;

    private final Map<String, String> languagedepprops;

    protected ScriptEntry(ScriptEntry entry) {
        this.language = entry.language;
        this.languagename = entry.languagename;
        this.logicalname = entry.languagename;
        this.languagedepprops = entry.languagedepprops;
        this.description = entry.description;
    }

    public ScriptEntry(String language, String languagename) {
        this(language, languagename, new HashMap<String, String>(), "");
    }

    public ScriptEntry(String language, String languagename,
                       Map<String, String> languagedepprops,
                       String description) {
        this.language = language;
        this.languagename = languagename;
        // logical name/ function name concept
        // needs to be reworked, in meantime
        // function name ( from xml ) will be used
        // as logical name also
        this.logicalname = languagename;
        this.languagedepprops = languagedepprops;
        this.description = description;
    }

    public Map<String, String> getLanguageProperties() {
        return languagedepprops;
    }

    public String getLanguageName() {
        return languagename;
    }

    public String getLogicalName() {
        return logicalname;
    }

    public String getLanguage() {
        return language;
    }

    public String getDescription() {
        return description;
    }

    @Override
    public String toString() {
        return "\nLogicalName = " + logicalname +
               "\nLanguageName = " + languagename +
               "\nLanguaguageProperties = " + languagedepprops;
    }
}
