/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ExtMap.java,v $
 * $Revision: 1.3 $
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
/*
 * A special HashMap,
 * can be constructed of
 * two Arrays
 */
package com.sun.star.tooling.converter;

import java.util.HashMap;
import java.util.Map;

/**
 * @author Christian Schmidt
 *
 * Create a Hash Map from two Arrays
 *
 */
public class ExtMap extends HashMap {

    /**
     *
     */
    public ExtMap() {
        super();

    }

    /**
     * @see java.util.HashMap
     * @param arg0
     */
    public ExtMap(int arg0) {
        super(arg0);

    }

    /**
     * @param arg0
     * @param arg1
     */
    public ExtMap(int arg0, float arg1) {
        super(arg0, arg1);

    }

    /**
     * @param arg0
     */
    public ExtMap(Map arg0) {
        super(arg0);

    }

    // create a new Map from two string arrays
    public ExtMap(String[] names, String[] content) {
        super(names.length);
        if (content == null)
            content = new String[names.length];
        for (int i = 0; i < names.length; i++) {
            if (i >= content.length) {
                break;
            } else {
                this.put(names[i], content[i]);
            }
        }

    }

}