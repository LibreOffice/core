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

public class XLayerHandlerImpl
    implements com.sun.star.configuration.backend.XLayerHandler {
    private String calls = "";
    private final String ls = System.getProperty("line.separator");

    public void addOrReplaceNode(String str, short param)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("addOrReplace(" + str + "," + param + ");" + ls);
    }

    public void addOrReplaceNodeFromTemplate(String str,
                                             com.sun.star.configuration.backend.TemplateIdentifier templateIdentifier,
                                             short param)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("addOrReplaceNodeFromTemplate(" + str + "," + templateIdentifier + ");" + ls);
    }

    public void addProperty(String str, short param,
                            com.sun.star.uno.Type type)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("addProperty(" + str + "," + param + "," + type + ");" + ls);
    }

    public void addPropertyWithValue(String str, short param, Object obj)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("addPropertyWithValue(" + str + "," + param + "," + obj + ");" + ls);
    }

    public void dropNode(String str)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("dropNode(" + str + ");" + ls);
    }

    public void endLayer()
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("endLayer();" + ls);
    }

    public void endNode()
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("endNode();" + ls);
    }

    public void endProperty()
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("endProperty();" + ls);
    }

    public void overrideNode(String str, short param, boolean param2)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("overrideNode(" + str + "," + param + "," + param2 + ");" + ls);
    }

    public void overrideProperty(String str, short param,
                                 com.sun.star.uno.Type type, boolean param3)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("overrideProperty(" + str + "," + param + "," + type + "," + param3 + ");" + ls);
    }

    public void setPropertyValue(Object obj)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("setPropertyValue(" + obj + ");" + ls);
    }

    public void setPropertyValueForLocale(Object obj, String str)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("setPropertyValueForLocale(" + obj + "," + str + ");" + ls);
    }

    public void startLayer()
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls = "startLayer();" + ls;
    }

    public String getCalls() {
        return calls;
    }
}