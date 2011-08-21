/*************************************************************************
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

package ifc.text;

import lib.MultiPropertyTest;

import com.sun.star.text.ControlCharacter;

/**
 * Testing <code>com.sun.star.text.TextPortion</code>
 * service properties :
 * <ul>
 *  <li><code> TextPortionType</code></li>
 *  <li><code> ControlCharacter</code></li>
 * </ul> <p>
 * Properties testing is automated by <code>lib.MultiPropertyTest</code>.
 * @see com.sun.star.text.TextPortion
 */
public class _TextPortion extends MultiPropertyTest {

    /**
     * This property can be void, so if old value is <code> null </code>
     * new value must be specified.
     */
    public void _ControlCharacter() {
        testProperty("ControlCharacter",
            new Short(ControlCharacter.LINE_BREAK),
            new Short(ControlCharacter.PARAGRAPH_BREAK)) ;
    }

} //finish class _TextPortion

