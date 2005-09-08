/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XFormDocumentsSupplier.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:26:27 $
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

package ifc.sdb;

import lib.MultiMethodTest;

import com.sun.star.container.XNameAccess;
import com.sun.star.sdb.XFormDocumentsSupplier;

/**
* <code>com.sun.star.sdb.XFormDocumentsSupplier</code> interface
* testing.
* @see com.sun.star.sdb.XFormDocumentsSupplier
*/
public class _XFormDocumentsSupplier extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XFormDocumentsSupplier oObj = null ;

    /**
    * Has OK status if not null returned. <p>
    */
    public void _getFormDocuments() {

        XNameAccess docs = oObj.getFormDocuments() ;

        String[] docNames = docs.getElementNames() ;
        if (docNames != null) {
            log.println("Totally " + docNames.length + " documents :") ;
            for (int i = 0; i < docNames.length; i++)
                log.println("  " + docNames[i]) ;
        }

        tRes.tested("getFormDocuments()", docNames != null) ;
    }

}  // finish class _XFormDocumentsSupplier


