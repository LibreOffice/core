/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ExtensionVerifier.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:54:46 $
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
public class ExtensionVerifier implements UCB.Verifier {
    private String extension;

    public ExtensionVerifier(String extension_) {
        extension="." + extension_;
    }

    /**
     * @return true if the given object is
     * a String which does not end with the
     * given extension.
     */
    public boolean verify(Object object) {
        if (object instanceof String)
          return !((String)object).endsWith(extension);
        return false;
    }

}
