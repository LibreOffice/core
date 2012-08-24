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
import com.sun.star.wizards.common.TemplateElement;

import java.util.ArrayList;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XSearchable;
import com.sun.star.util.XSearchDescriptor;
import com.sun.star.container.XIndexAccess;
import com.sun.star.text.*;
import com.sun.star.wizards.text.*;
import com.sun.star.wizards.common.TextElement;
import com.sun.star.wizards.common.PlaceholderTextElement;

class ParaStyled implements TemplateElement
{

    String paraStyle;

    ParaStyled(String paraStyle_)
    {
        paraStyle = paraStyle_;
    }

    void format(Object textRange)
    {
        XText o;
        o = UnoRuntime.queryInterface(XText.class, textRange);
        if (o == null)
        {
            o = UnoRuntime.queryInterface(XTextRange.class, textRange).getText();
        }
        XTextRange xtr = UnoRuntime.queryInterface(XTextRange.class, textRange);
        XTextCursor cursor = o.createTextCursorByRange(xtr);

        Helper.setUnoPropertyValue(cursor, "ParaStyleName", paraStyle);
    }

    public void write(Object textRange)
    {
        format(textRange);
    }
}

