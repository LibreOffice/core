/*************************************************************************
 *
 *  $RCSfile: _XModuleUIConfigurationManager.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Date: 2004-02-25 18:11:09 $
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

package ifc.ui;

import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.uno.UnoRuntime;
import drafts.com.sun.star.ui.XModuleUIConfigurationManager;
import drafts.com.sun.star.ui.XModuleUIConfigurationManagerSupplier;
import drafts.com.sun.star.ui.XUIConfigurationManager;
import lib.MultiMethodTest;

public class _XModuleUIConfigurationManager extends MultiMethodTest {

    public XModuleUIConfigurationManager oObj = null;
    private String resourceUrl = null;

    public void before() {
        resourceUrl = (String)tEnv.getObjRelation("XModuleUIConfigurationManager.ResourceURL");
    }

    public void _isDefaultSettings() {
        boolean result;
        try {
            result = oObj.isDefaultSettings(resourceUrl);
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("'" + resourceUrl + "' is an illegal resource.");
            result = false;
        }
        String notPossibleUrl = "private:resource/menubar/dingsbums";
        try {
            result &= !oObj.isDefaultSettings(notPossibleUrl);
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("'" + notPossibleUrl + "' is an illegal resource.");
            result = false;
        }

        tRes.tested("isDefaultSettings()", result);
    }

    public void _getDefaultSettings() {
        boolean result;
        try {
            XIndexAccess xIndexAcc = oObj.getDefaultSettings(resourceUrl);
            result = xIndexAcc != null;
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("'" + resourceUrl + "' is an illegal resource.");
            result = false;
        }
        catch(com.sun.star.container.NoSuchElementException e) {
            log.println("No resource '" + resourceUrl + "' available.");
            result = false;
        }
        tRes.tested("getDefaultSettings()", result);
    }

}