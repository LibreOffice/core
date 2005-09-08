/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XMimeContentTypeFactory.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:24:14 $
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
            log.println("Exception occured : " ) ;
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


