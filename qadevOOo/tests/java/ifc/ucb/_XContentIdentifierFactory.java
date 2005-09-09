/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XContentIdentifierFactory.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:30:52 $
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

package ifc.ucb;

import lib.MultiMethodTest;
import util.utils;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.XContentIdentifier;
import com.sun.star.ucb.XContentIdentifierFactory;

/**
* Testing <code>com.sun.star.ucb.XContentIdentifierFactory</code>
* interface methods :
* <ul>
*  <li><code> createContentIdentifier()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* After test completion object environment has to be recreated.
* @see com.sun.star.ucb.XContentIdentifierFactory
*/
public class _XContentIdentifierFactory extends MultiMethodTest {
    /**
     * Contains the tested object.
     */
    public XContentIdentifierFactory oObj;

    /**
     * Calls the tested method with a file url and
     * gets an identifier. <p>
     * Has <b> OK </b> status if content identifier an provider sheme are
     * proper. <p>
     */
    public void _createContentIdentifier() {
        // creating a content identifier string - tmp url
        String tmpDir = utils.getOfficeTemp((XMultiServiceFactory)tParam.getMSF());

        String contentId = utils.getFullURL(tmpDir);

        // the scheme is file
        String scheme = "file";

        XContentIdentifier identifier = oObj.createContentIdentifier(contentId);

        // verifying results
        String resId = identifier.getContentIdentifier();
        String resScheme = identifier.getContentProviderScheme();

        tRes.tested("createContentIdentifier()", contentId.equals(resId)
                && scheme.equals(resScheme));
    }
}
