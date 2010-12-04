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

package ifc.datatransfer;

import lib.MultiMethodTest;

import com.sun.star.datatransfer.XMimeContentType;
import com.sun.star.datatransfer.XMimeContentTypeFactory;

/**
* Testing <code>com.sun.star.datatransfer.XMimeContentTypeFactory</code>
* interface methods :
* <ul>
*  <li><code> createMimeContentType()</code></li>
* </ul> <p>
* Test is multithread compilant. <p>
* @see com.sun.star.datatransfer.XMimeContentTypeFactory
*/
public class _XMimeContentTypeFactory extends MultiMethodTest {

    public XMimeContentTypeFactory oObj = null;

    /**
    * First tries to create 'image/jpeg' MIME type and checks that
    * valid <code>XMimeContentType</code> object was created.
    * Second tries to create type with wrong argument and exception
    * throwing is checked. <p>
    * Has <b>OK</b> status if in the first case valid object is
    * returned and in the second case <code>IllegalArgumentException</code>
    * was thrown.
    */
    public void _createMimeContentType() {
        boolean result = true ;
        XMimeContentType type = null;

        try {
            type = oObj.createMimeContentType("image/jpeg") ;

            if (type != null) {
                String typeS = type.getFullMediaType() ;

                log.println("MediaType = '" + type.getMediaType() + "'") ;
                log.println("MediaSubType = '" + type.getMediaSubtype() + "'") ;
                log.println("FullMediaType = '" + typeS + "'") ;

                result = "image/jpeg".equals(typeS) ;
            } else {
                log.println("!!! Null was returned !!!") ;
                result = false ;
            }
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Exception occurred : " ) ;
            e.printStackTrace(log) ;
            result = false ;
        }

        try {
            type = oObj.createMimeContentType("nosuchtype") ;

            log.println("!!! No exception was thrown on wrong MIME type !!!") ;
            result = false ;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Right exception was thrown." ) ;
        }

        tRes.tested("createMimeContentType()", result) ;
    }
}


