/*************************************************************************
 *
 *  $RCSfile: ScriptEntry.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-19 08:22:10 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
