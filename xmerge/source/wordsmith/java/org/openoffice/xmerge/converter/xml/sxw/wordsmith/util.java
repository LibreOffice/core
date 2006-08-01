/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: util.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 13:16:31 $
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

package org.openoffice.xmerge.converter.xml.sxw.wordsmith;

/**
 *  WordSmith utility class.
 *
 *  @author   David Proulx
 */
class util {

    /**
     *  Convert 2 bytes to an integer.
     *
     *  @param  data   <code>byte</code> data to convert.
     *  @param  index  Index to convert.
     *
     *  @return  Converted integer.
     */
    static int intFrom2bytes(byte[] data, int index) {
        return (((data[index] & 0xFF) << 8)
                | (data[index+1] & 0xFF));

    }


    /**
     *  Convert 4 bytes to an integer.
     *
     *  @param  data   <code>byte</code> data to convert.
     *  @param  index  Index to convert.
     *
     *  @return  Converted integer.
     */
    static int intFrom4bytes(byte[] data, int index) {
        return (((data[index] & 0xFF) << 24)
          | ((data[index + 1] & 0xFF) << 16)
          | ((data[index + 2] & 0xFF) << 8)
                | (data[index+3] & 0xFF));

    }
}

