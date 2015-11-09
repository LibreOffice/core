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

package com.sun.star.wizards.common;

import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.*;

public class PlaceholderTextElement extends TextElement
{

    private String hint;
    private String placeHolderText;
    private XMultiServiceFactory xmsf;

    public PlaceholderTextElement(XTextRange textRange, String placeHolderText_, String hint_, XMultiServiceFactory xmsf_)
    {
        super(textRange);
        placeHolderText = placeHolderText_;
        hint = hint_;
        xmsf = xmsf_;
    }

    @Override
    public void write(Object textRange)
    {
        super.write(textRange);
        if (text.equals(PropertyNames.EMPTY_STRING))
        {
            XTextRange xTextRange = UnoRuntime.queryInterface(XTextRange.class, textRange);
            try
            {
                XTextContent xTextContent = createPlaceHolder(xmsf, placeHolderText, hint);
                xTextRange.getText().insertTextContent(xTextRange.getStart(), xTextContent, true);
            }
            catch (com.sun.star.lang.IllegalArgumentException ex)
            {
                ex.printStackTrace();
            }
        }
    }

    private static XTextContent createPlaceHolder(XMultiServiceFactory xmsf, String ph, String hint)
    {
        Object placeHolder;
        try
        {
            placeHolder = xmsf.createInstance("com.sun.star.text.TextField.JumpEdit");
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
            return null;
        }
        Helper.setUnoPropertyValue(placeHolder, "PlaceHolder", ph);
        Helper.setUnoPropertyValue(placeHolder, "Hint", hint);
        Helper.setUnoPropertyValue(placeHolder, "PlaceHolderType", Short.valueOf(PlaceholderType.TEXT));
        return UnoRuntime.queryInterface(XTextContent.class, placeHolder);

    }

}


