/*************************************************************************
 *
 *  $RCSfile: _XScriptInfo.java,v $
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

public class _XScriptInfo extends MultiMethodTest {

    public XScriptInfo oObj = null;

    /**
    * Retrieves object relation.
    */
    public void before() throws StatusException {
    }

    public void _getLogicalName() {

        log.println("In _XScriptInfo.getLogicalName()");
        if ( !oObj.getLogicalName().equals( "MemoryUtils.MemUsage" ) ){
            log.println("Expected logical name = MemoryUtils.MemUsage, got "                    + oObj.getLogicalName() );
            tRes.tested("getImplementations()", false);
            return;
        }

        tRes.tested("getLogicalName()", true);
    }
    public void _getParcelURI() {
        log.println("In _XScriptInfo._getParcelUR()");
        if ( oObj.getParcelURI().length() == 0 ){
            log.println( "Expected parcel uri to be set up" );
            tRes.tested( "getParcelURI()", false );
            return;
        }
        tRes.tested("getParcelURI()", true);
    }
    public void _getLanguage() {
        log.println("In _XScriptInfo.getLanguage()");
        if ( !oObj.getLanguage().equals( "Java" ) ){
            log.println("Expected language to be Java, got " +
                oObj.getLanguage().length() );
            tRes.tested("getLanguage()", false);
            return;
        }
        tRes.tested("getLanguage()", true);
    }

    public void _getFunctionName() {
        log.println("In _XScriptInfo.getFunctionName()");
        if ( !oObj.getFunctionName().equals( "MemoryUsage.updateMemoryUsage" ) ) {
            log.println(
                "Expected functionName to be MemoryUsage.updateMemoryUsage, got ->" +
                oObj.getFunctionName() + "<- instead." );
            tRes.tested("getFunctionName()", false);
            return;
        }
        tRes.tested("getFunctionName()", true);
    }
    public void _getLanguageProperties() {
        log.println("In _XScriptInfo.getLanguageProperties()");
        XPropertySet langProps = oObj.getLanguageProperties();
        try{

            String classPath = (String) langProps.getPropertyValue("classpath");            if ( classPath == null || !classPath.equals( "MemUsage.jar" ) ) {
                log.println(
                   "Expected classpath property to be MemUsage.jar, got ->" +
                    classPath + "<- instead." );
                tRes.tested("getLanguageProperties()", false);
                return;
            }
        }
        catch( com.sun.star.uno.Exception e) {
            log.println("getLanguageProperties: failed:" + e);
            tRes.tested("getLanguageProperties()", false);
            return;
        }
        tRes.tested("getLanguageProperties()", true);
    }
    public void _getFileSetNames() {
        log.println("In _XScriptInfo.getFileSetNames()");
        String[] fileSets = oObj.getFileSetNames();
        log.println("Got filesets of length " + fileSets.length + " first fileset is called " + fileSets[0] );
        if ( fileSets == null || fileSets.length != 1 ||
             !fileSets[0].equals("delivered") ) {
           log.println(
                       "Expected filesets with 1 element with value \"delivered\"");
           tRes.tested("getFileSetNames()", false);
           return;
        }
        tRes.tested("getFileSetNames()", true);
    }
    public void _getFilesInFileSet() {
        log.println("In _XScriptInfo.getFilesInFileSet()");
        String[] filesInFileSet = oObj.getFilesInFileSet( oObj.getFileSetNames()[0] );
        if ( filesInFileSet == null || filesInFileSet.length != 1 ||
            !filesInFileSet[0].equals( "google.jar" ) ) {
            log.println(
                "Expected a list of one file named \"google.jar\" for fileSet named \"delivered\"" );
            tRes.tested("getFilesInFileSet()", false);
            return;
        }
        tRes.tested("getFilesInFileSet()", true);
    }
    public void _getDescription() {
        log.println("In _XScriptInfo.getDescription()");
        if ( oObj.getDescription().length() != 0 ){
            log.println("Last test. Expected no description for MemoryUtils.MemUsage" + " got ->" + oObj.getDescription() + "<- instead" );
            tRes.tested("getDescription()", false);
            return;
        }
        tRes.tested("getDescription()", true);
    }

}
