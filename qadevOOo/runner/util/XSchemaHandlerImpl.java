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
package util;

public class XSchemaHandlerImpl
    implements com.sun.star.configuration.backend.XSchemaHandler {
    private String calls = "";
    private final String ls = System.getProperty("line.separator");

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