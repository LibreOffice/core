/*************************************************************************
 *
 *  $RCSfile: _XScriptInfoAccess.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change:$Date: 2002-11-20 14:29:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package ifc.script.framework.storage;

import drafts.com.sun.star.script.framework.storage.XScriptStorageManager;
import drafts.com.sun.star.script.framework.storage.XScriptInfoAccess;
import drafts.com.sun.star.script.framework.storage.XScriptInfo;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.Exception;
import com.sun.star.beans.XPropertySet;

import java.io.PrintWriter;
import lib.MultiMethodTest;
import lib.StatusException;

public class _XScriptInfoAccess extends MultiMethodTest {

    public XScriptInfoAccess oObj = null;

    /**
    * Retrieves object relation.
    */
    public void before() throws StatusException {
    }

    public void _getScriptLogicalNames() {
        try {
            log.println("In _XScriptInfoAccess.getScriptLogicalNames()");
            String[] logicalNames = oObj.getScriptLogicalNames();
            log.println("Retrieved " + logicalNames.length + " entries from storage" );
            for ( int idx=0; idx < logicalNames.length; idx++ ){
                log.println("logical name [" + idx + "] = " + logicalNames[idx]);
            }
            if ( logicalNames.length == 0 ){
                log.println( "Failed:_XScriptInfoAccess.getScriptLogicalNames(), no logical names returned" );
                tRes.tested( "getScriptLogicalNames()", false );
            }

        }
        catch (com.sun.star.uno.Exception e) {
            log.println("Failed:_XScriptInfoAccess.getScriptLogicalNames() :" + e);
            tRes.tested("getScriptLogicalNames()", false);
            return;
        }
        tRes.tested("getScriptLogicalNames()", true);
    }

    public void _getImplementations() {
    // performs a basic check to see if 1 match (XScriptInfo) is returned
    // the XScriptInfo object is tested more completely in _XScriptInfo
    // which is drive from ScriptInfo
        try {
            log.println("In _XScriptInfoAccess._getImplementations()");
            XScriptInfo[] impls = oObj.getImplementations("script://MemoryUtils.MemUsage?location=document");
            // should only be one match
            log.println("_XScriptInfoAccess._getImplementations() returned " + impls.length + " items ");
            if ( impls.length != 1 ){
                log.println("Expected 1 implementation to be returned, got " +
                    impls.length + " instead.");
                tRes.tested("getImplementations()", false);
                return;
        }

            if ( !impls[0].getLogicalName().equals( "MemoryUtils.MemUsage" ) ){
                log.println("Expected logical name = MemoryUtils.MemUsage, got "
                    + impls[0].getLogicalName() );
                tRes.tested("getImplementations()", false);
                return;
            }

        }
        catch (com.sun.star.uno.Exception e) {
            log.println("getImplementations: failed:" + e);
            tRes.tested("getImplementations()()", false);
            return;
        }
        log.println("_XScriptInfoAccess._getImplementations() completed sucessfully");
        tRes.tested("getImplementations()", true);

    }

}
