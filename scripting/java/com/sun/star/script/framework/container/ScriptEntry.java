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

package com.sun.star.script.framework.container;
import java.util.Map;
import java.util.HashMap;
public class ScriptEntry implements Cloneable {

    private String language;
    private String languagename;
    private String location;
    private String logicalname = "";
    private String description = "";

    private Map languagedepprops;

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
        this.languagedepprops =  new HashMap();
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
                       String logicalname, String location, Map languagedepprops) {
        this( language, languagename, logicalname, location );
        this.languagedepprops = languagedepprops;
    }

    public ScriptEntry(String language, String languagename,
                       String logicalname, String location,
                       Map languagedepprops, String description) {
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

    public Map getLanguageProperties()
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
