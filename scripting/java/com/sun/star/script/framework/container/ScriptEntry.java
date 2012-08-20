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
import java.util.Map;
import java.util.HashMap;
public class ScriptEntry implements Cloneable {

    private String language;
    private String languagename;
    private String location;
    private String logicalname = "";
    private String description = "";

    private Map<String,String> languagedepprops;

    public ScriptEntry(String language, String languagename,
                       String logicalname, String location) {
        this.language = language;
        this.languagename = languagename;
        // logical name/ function name concept
        // needs to be reworked, in meantime
        // function name ( from xml ) will be used
        // as logical name also
        this.logicalname = languagename;
        this.location = location;
        this.languagedepprops =  new HashMap<String,String>();
    }

    public ScriptEntry(ScriptEntry entry)
    {
        this.language = entry.language;
        this.languagename = entry.languagename;
        this.logicalname = entry.languagename;
        this.location = entry.location;
        this.languagedepprops = entry.languagedepprops;
        this.description = entry.description;
    }

    public ScriptEntry(String language, String languagename,
                       String logicalname, String location, Map<String,String> languagedepprops) {
        this( language, languagename, logicalname, location );
        this.languagedepprops = languagedepprops;
    }

    public ScriptEntry(String language, String languagename,
                       String logicalname, String location,
                       Map<String,String> languagedepprops, String description) {
        this( language, languagename, logicalname, location );
        this.languagedepprops = languagedepprops;
        this.description = description;
    }

    public ScriptEntry(String languagename, String location) {
        this("Java", languagename, languagename, location);
    }

    public Object clone() throws CloneNotSupportedException {
        return super.clone();
    }

    public boolean equals(ScriptEntry other) {
        if (language.equals(other.getLanguage()) &&
            languagename.equals(other.getLanguageName()) &&
            logicalname.equals(other.getLogicalName()) &&
            languagedepprops.equals(other.getLanguageProperties()) &&
            location.equals(other.getLocation()))
            return true;
        return false;
    }

    public Map<String,String> getLanguageProperties()
    {
        return languagedepprops;
    }

    public String getLanguageName() {
        return languagename;
    }

    public String getLogicalName() {
        return logicalname;
    }

    public void setLogicalName(String name) {
        logicalname = name;
    }

    public String getLanguage() {
        return language;
    }

    public String getLocation() {
        return location;
    }

    public String getDescription() {
        return description;
    }

    public String toString() {
        return "\nLogicalName = " + logicalname +  "\nLanguageName = " + languagename + "\nLocation = " + location + "\nLanguaguageProperties = " + languagedepprops;
    }
}
