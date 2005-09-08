/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XExporter.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:28:37 $
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

package ifc.document;


import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.document.XExporter;
import com.sun.star.lang.XComponent;

/**
* Testing <code>com.sun.star.document.XExporter</code>
* interface methods :
* <ul>
*  <li><code> setSourceDocument()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'SourceDocument'</code> (of type <code>XComponent</code>):
*   the source document to be passed to the method. </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.document.XExporter
*/
public class _XExporter extends MultiMethodTest {

    public XExporter oObj = null;
    public XComponent source = null ;

    /**
    * Retrieves object relations.
    * @throws StatusException If one of relations not found.
    */
    public void before() {
        source = (XComponent) tEnv.getObjRelation("SourceDocument") ;

        if (source == null) throw new StatusException(Status.failed
            ("Relation not found")) ;
    }

    /**
    * Just calls the method. <p>
    * Has <b> OK </b> status if no runtime exceptions occured.
        * Usually this interface is supported both with <code>XFilter</code>
        * where source document setting is checked.
    */
    public void _setSourceDocument() {
        boolean result = true ;

        try {
            oObj.setSourceDocument(source);
        }
        catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception while checking :");
            ex.printStackTrace(log);
            result = false;
        }

        tRes.tested("setSourceDocument()", result) ;
    }
}


