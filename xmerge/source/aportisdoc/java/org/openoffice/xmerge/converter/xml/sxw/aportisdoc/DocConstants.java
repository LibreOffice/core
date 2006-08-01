/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DocConstants.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 12:40:03 $
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

package org.openoffice.xmerge.converter.xml.sxw.aportisdoc;

import org.openoffice.xmerge.converter.palm.PdbUtil;

/**
 *  Constants used for encoding and decoding the AportisDoc format.
 *
 *  @author   Herbie Ong
 */
interface DocConstants {

    /** Creator id. */
    public static final int CREATOR_ID = PdbUtil.intID("REAd");

    /** Type id. */
    public static final int TYPE_ID = PdbUtil.intID("TEXt");

    /** Constant for uncompressed version. */
    public static final short UNCOMPRESSED = 1;

    /** Constant for compressed version. */
    public static final short COMPRESSED = 2;

    /** Constant used for spare fields. */
    public static final int SPARE = 0;

    /** AportisDoc record size. */
    public static final short TEXT_RECORD_SIZE = 4096;

    /** Constant for encoding scheme. */
    public static final String ENCODING = "8859_1";

    /** Constant for TAB character. */
    public final static char TAB_CHAR = '\t';

    /** Constant for EOL character. */
    public final static char EOL_CHAR = '\n';

    /** Constant for SPACE character. */
    public final static char SPACE_CHAR = ' ';
}

