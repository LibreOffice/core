/*
 ************************************************************************
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

package com.sun.star.wizards.web.export;

import java.util.Iterator;

import com.sun.star.wizards.web.data.CGArgument;
import com.sun.star.wizards.web.data.CGExporter;

/**
 * @author rpiterman
 *
 * To change the template for this generated type comment go to
 * Window>Preferences>Java>Code Generation>Code and Comments
 */
public class ConfiguredExporter extends FilterExporter
{

    /* (non-Javadoc)
     * @see com.sun.star.wizards.web.export.Exporter#init(com.sun.star.wizards.web.data.CGExporter)
     */
    public void init(CGExporter exporter)
    {
        super.init(exporter);
        for (Iterator i = exporter.cp_Arguments.keys().iterator(); i.hasNext();)
        {
            Object key = i.next();
            if (!key.equals("Filter"))
            {
                Object value = exporter.cp_Arguments.getElement(key);
                props.put(key, cast(((CGArgument) value).cp_Value));
            }
        }
    }

    private Object cast(String s)
    {
        String s1 = s.substring(1);
        char c = s.charAt(0);
        switch (c)
        {
            case '$':
                return s1;
            case '%':
                return Integer.valueOf(s1);
            case '#':
                return Short.valueOf(s1);
            case '&':
                return Double.valueOf(s1);
            case 'f':
                if (s.equals("false"))
                {
                    return Boolean.FALSE;
                }
                break;
            case 't':
                if (s.equals("true"))
                {
                    return Boolean.TRUE;
                }
                break;
        }
        return null;
    }
}

