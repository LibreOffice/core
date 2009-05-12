/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ExtensionVerifier.java,v $
 * $Revision: 1.6 $
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
package com.sun.star.wizards.web;

import com.sun.star.wizards.common.UCB;

/**
 * @author rpiterman
 * Verifies all String that do not end with
 * the given extension.
 * This is used to exclude from a copy all the
 * xsl files, so I copy from a layout directory
 * all the files that do *not* end with xsl.
 *
 */
public class ExtensionVerifier implements UCB.Verifier
{

    private String extension;

    public ExtensionVerifier(String extension_)
    {
        extension = "." + extension_;
    }

    /**
     * @return true if the given object is
     * a String which does not end with the
     * given extension.
     */
    public boolean verify(Object object)
    {
        if (object instanceof String)
        {
            return !((String) object).endsWith(extension);
        }
        return false;
    }
}
