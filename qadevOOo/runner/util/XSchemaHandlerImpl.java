/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: XSchemaHandlerImpl.java,v $
 * $Revision: 1.4 $
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
package util;

public class XSchemaHandlerImpl
    implements com.sun.star.configuration.backend.XSchemaHandler {
    protected String calls = "";
    protected String ls = System.getProperty("line.separator");

    public void addInstance(String str,
                            com.sun.star.configuration.backend.TemplateIdentifier templateIdentifier)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("addInstance();" + ls);
    }

    public void addItemType(com.sun.star.configuration.backend.TemplateIdentifier templateIdentifier)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("addItemType();" + ls);
    }

    public void addProperty(String str, short param,
                            com.sun.star.uno.Type type)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("addProperty();" + ls);
    }

    public void addPropertyWithDefault(String str, short param, Object obj)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("addPropertyWithDefault();" + ls);
    }

    public void endComponent()
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("endComponent();" + ls);
    }

    public void endNode()
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("endNode();" + ls);
    }

    public void endSchema()
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("endSchema();" + ls);
    }

    public void endTemplate()
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("endTemplate();" + ls);
    }

    public void importComponent(String str)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("importComponent();" + ls);
    }

    public void startComponent(String str)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("startComponent();" + ls);
    }

    public void startGroup(String str, short param)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("startGroup();" + ls);
    }

    public void startGroupTemplate(com.sun.star.configuration.backend.TemplateIdentifier templateIdentifier,
                                   short param)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("startGroupTemplate();" + ls);
    }

    public void startSchema()
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("startSchema();" + ls);
    }

    public void startSet(String str, short param,
                         com.sun.star.configuration.backend.TemplateIdentifier templateIdentifier)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("startSet();" + ls);
    }

    public void startSetTemplate(com.sun.star.configuration.backend.TemplateIdentifier templateIdentifier,
                                 short param,
                                 com.sun.star.configuration.backend.TemplateIdentifier templateIdentifier2)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("startSetTemplate();" + ls);
    }

    public String getCalls() {
        return calls;
    }

    public void cleanCalls() {
        calls = "";
    }
}