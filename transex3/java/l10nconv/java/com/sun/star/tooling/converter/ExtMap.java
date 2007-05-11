/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ExtMap.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-11 09:08:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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