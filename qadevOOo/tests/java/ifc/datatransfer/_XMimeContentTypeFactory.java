/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
* Test is multithread compliant. <p>
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

        if (result == true) {
            try {
                oObj.createMimeContentType("nosuchtype") ;

                log.println("!!! No exception was thrown on wrong MIME type !!!") ;
                result = false ;
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                log.println("Right exception was thrown." ) ;
            }
        }

        tRes.tested("createMimeContentType()", result) ;
    }
}


