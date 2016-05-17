/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

import com.sun.star.script.provider.XScriptContext;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.lang.XComponent;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextRange;
import com.sun.star.text.XText;
import com.sun.star.beans.PropertyValue;
import com.sun.star.awt.ActionEvent;

public class SayHello
{
    public static void start(XScriptContext ctxt, ActionEvent e)
        throws Exception
    {
        SayHello.start(ctxt);
    }

    public static void start(XScriptContext ctxt)
        throws Exception
    {
        // getting the text document object
        XTextDocument xTextDocument = createDocument(ctxt);

        XText xText = xTextDocument.getText();
        XTextRange xTextRange = xText.getEnd();
        xTextRange.setString("Hello");
    }

    private static XTextDocument createDocument(XScriptContext ctxt)
        throws Exception
    {
        XComponentLoader loader = UnoRuntime.queryInterface(
            XComponentLoader.class, ctxt.getDesktop());

        XComponent comp = loader.loadComponentFromURL(
            "private:factory/swriter", "_blank", 4, new PropertyValue[0]);

        XTextDocument doc = UnoRuntime.queryInterface( XTextDocument.class, comp);

        return doc;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
