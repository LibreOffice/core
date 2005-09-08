/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BookmarkDsc.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:31:36 $
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

package util;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.text.XTextContent;

/**
 * describes a Bookmark to be inserted in a container
 */
public class BookmarkDsc extends InstDescr {

    final String service = "com.sun.star.text.Bookmark";
    String ifcName = "com.sun.star.text.XTextContent";
    private String name = null;


    public BookmarkDsc() {
            initBookmark();
    }

    public BookmarkDsc( String name ) {
            this.name = name;
            initBookmark();
    }

    public String getName() {
            return name;
    }

    public String getIfcName() {
            return ifcName;
    }

    public String getService() {
            return service;
    }

    private void initBookmark() {
            try {
            ifcClass = Class.forName( ifcName );
        }
        catch( ClassNotFoundException cnfE ) {
        }
    }
    public XInterface createInstance( XMultiServiceFactory docMSF ) {
            Object ServiceObj = null;

            try {
                    ServiceObj = docMSF.createInstance( service );
            }
            catch( com.sun.star.uno.Exception cssuE ){
            }
            XTextContent BM = (XTextContent)UnoRuntime.queryInterface( ifcClass,
                                                                                                                            ServiceObj );
            return BM;
    }
}