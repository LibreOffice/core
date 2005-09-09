/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BinaryOnlyFilter.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:06:56 $
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

package org.openoffice.idesupport.filter;

public class BinaryOnlyFilter implements FileFilter {
    private static final String[] EXTENSIONS = {".class", ".jar", ".bsh"};
    private static final String DESCRIPTION = "Executable Files Only";
    private static final BinaryOnlyFilter filter = new BinaryOnlyFilter();

    private BinaryOnlyFilter() {
    }

    public static BinaryOnlyFilter getInstance() {
        return filter;
    }
    public boolean validate(String name) {
        for (int i = 0; i < EXTENSIONS.length; i++)
            if (name.endsWith(EXTENSIONS[i]))
                return true;
        return false;
    }

    public String toString() {
        /* StringBuffer buf = new StringBuffer(DESCRIPTION + ": ");

        for (int i = 0; i < EXTENSIONS.length - 1; i++)
            buf.append("<" + EXTENSIONS[i] + "> ");
        buf.append("<" + EXTENSIONS[EXTENSIONS.length - 1] + ">");

        return buf.toString(); */
        return DESCRIPTION;
    }
}
