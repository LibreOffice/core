/*************************************************************************
 *
 *  $RCSfile: ScriptBrowseNode.java,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: toconnor $ $Date: 2003-10-29 15:01:09 $
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

package com.sun.star.script.framework.browse;

import drafts.com.sun.star.script.browse.XBrowseNode;
import drafts.com.sun.star.script.browse.BrowseNodeTypes;
import drafts.com.sun.star.script.provider.XScriptContext;

import com.sun.star.beans.PropertyAttribute;
import com.sun.star.lib.uno.helper.PropertySet;
import com.sun.star.uno.Type;

import com.sun.star.beans.XIntrospectionAccess;
import com.sun.star.script.XInvocation;

import java.io.File;

public class ScriptBrowseNode extends PropertySet
    implements XBrowseNode, XInvocation
{
    private String name;
    private ScriptEntry entry;
    private File basedir;

    public String uri;
    public boolean editable = false;
    public boolean deletable = false;

    public ScriptBrowseNode(ScriptEntry entry, String location) {
        this.entry = entry;

        uri = "vnd.sun.star.script://" + entry.getLanguageName() +
            "?" + "language=" + entry.getLanguage() +
            "&location=" + location;

        name = entry.getLanguageName();

        if (!entry.getLanguage().toLowerCase().equals("java") &&
            !location.equals("document"))
        {
            editable = true;
            deletable = true;
        }

        registerProperty("Deletable", new Type(boolean.class),
            (short)0, "deletable");
        registerProperty("Editable", new Type(boolean.class),
            (short)0, "editable");
        registerProperty("URI", new Type(String.class),
            (short)0, "uri");
    }

    public ScriptBrowseNode(ScriptEntry entry, String location, File basedir) {
        this(entry, location);
        this.basedir = basedir;
    }

    public String getName() {
        return name;
    }

    public XBrowseNode[] getChildNodes() {
        return new XBrowseNode[0];
    }

    public boolean hasChildNodes() {
        return false;
    }

    public short getType() {
        return BrowseNodeTypes.SCRIPT;
    }

    public String toString() {
        return getName();
    }

    // implementation of XInvocation interface
    public XIntrospectionAccess getIntrospection() {
        return null;
    }

    public Object invoke(String aFunctionName, Object[] aParams,
                         short[][] aOutParamIndex, Object[][] aOutParam)
        throws com.sun.star.lang.IllegalArgumentException,
               com.sun.star.script.CannotConvertException,
               com.sun.star.reflection.InvocationTargetException
    {
        System.out.println("in ScriptBrowseNode.invoke");

        if (aFunctionName.equals("Editable")) {
            if (!editable) {
                throw new com.sun.star.reflection.InvocationTargetException(
                    "Script not editable");
            }

            if (basedir == null || !basedir.exists()) {
                throw new com.sun.star.lang.IllegalArgumentException(
                    "Cannot find script directory: " +
                    basedir.getAbsolutePath());
            }

            File sourcefile = new File(basedir, entry.getLanguageName());
            if (!sourcefile.exists()) {
                throw new com.sun.star.lang.IllegalArgumentException(
                    "Cannot find script source file: " +
                    sourcefile.getAbsolutePath());
            }

            if (aParams == null || aParams.length < 1) {
                throw new com.sun.star.lang.IllegalArgumentException(
                    "XScriptContext not provided");
            }

            XScriptContext ctxt;
            try {
                ctxt = (XScriptContext) aParams[0];
            }
            catch (ClassCastException cce) {
                throw new com.sun.star.lang.IllegalArgumentException(
                    "Wrong type for editor parameter: " +
                    aParams[0].getClass().getName());
            }

            String name = "com.sun.star.script.framework.provider." +
                entry.getLanguage().toLowerCase() + ".ScriptEditorFor" +
                entry.getLanguage();

            try
            {
                Class c = Class.forName(name);
                Class[] types =
                    new Class[] { XScriptContext.class, String.class };

                java.lang.reflect.Method m = c.getMethod("edit", types);

                if (m != null) {
                    System.out.println("got a script editor");
                    Object[] args = new Object[] {
                        ctxt, sourcefile.getAbsolutePath() };
                    m.invoke(c.newInstance(), args);
                }
                else {
                    throw new com.sun.star.reflection.InvocationTargetException(
                        "No edit method found for Editor");
                }
            }
            catch ( ClassNotFoundException cnfe )
            {
                throw new com.sun.star.reflection.InvocationTargetException(
                    "Exception getting Editor: " + cnfe.getMessage());
            }
            catch ( NoSuchMethodException nsme )
            {
                throw new com.sun.star.reflection.InvocationTargetException(
                    "Exception getting Editor: " + nsme.getMessage());
            }
            catch ( InstantiationException ie )
            {
                throw new com.sun.star.reflection.InvocationTargetException(
                    "Exception getting Editor: " + ie.getMessage());
            }
            catch ( IllegalAccessException iae )
            {
                throw new com.sun.star.reflection.InvocationTargetException(
                    "Exception getting Editor: " + iae.getMessage());
            }
            catch ( java.lang.IllegalArgumentException iarge )
            {
                throw new com.sun.star.reflection.InvocationTargetException(
                    "Exception getting Editor: " + iarge.getMessage());
            }
            catch ( java.lang.reflect.InvocationTargetException ite )
            {
                throw new com.sun.star.reflection.InvocationTargetException(
                    "Exception getting Editor: " + ite.getMessage());
            }
        }
        else {
            throw new com.sun.star.lang.IllegalArgumentException(
                "Function " + aFunctionName + " not supported.");
        }

        return null;
    }

    public void setValue(String aPropertyName, Object aValue)
        throws com.sun.star.beans.UnknownPropertyException,
               com.sun.star.script.CannotConvertException,
               com.sun.star.reflection.InvocationTargetException
    {
    }

    public Object getValue(String aPropertyName)
        throws com.sun.star.beans.UnknownPropertyException
    {
        return null;
    }

    public boolean hasMethod(String aName) {
        return false;
    }

    public boolean hasProperty(String aName) {
        return false;
    }
}
