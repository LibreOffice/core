/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ExceptParcelFilter.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:07:16 $
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

public class ExceptParcelFilter implements FileFilter {
    private static final String DESCRIPTION = "Remove specified Parcel";
    private static final ExceptParcelFilter filter = new ExceptParcelFilter();
    private static String parcelName = null;

    private ExceptParcelFilter() {
    }

    public void setParcelToRemove(String parcelName)
    {
        this.parcelName = parcelName;
    }

    public static ExceptParcelFilter getInstance() {
        return filter;
    }
    public boolean validate(String name) {
        if (name.startsWith(this.parcelName))
            return true;
        return false;
    }

    public String toString() {
        StringBuffer buf = new StringBuffer(DESCRIPTION + ": ");

        buf.append("<" + this.parcelName + ">");

        return buf.toString();
    }
}
