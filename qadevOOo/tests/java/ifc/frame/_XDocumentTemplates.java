/*************************************************************************
 *
 *  $RCSfile: _XDocumentTemplates.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 10:39:13 $
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

package ifc.frame;

import lib.MultiMethodTest;
import util.utils;

import com.sun.star.beans.Property;
import com.sun.star.frame.XDocumentTemplates;
import com.sun.star.frame.XStorable;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XRow;
import com.sun.star.ucb.Command;
import com.sun.star.ucb.OpenCommandArgument2;
import com.sun.star.ucb.XCommandProcessor;
import com.sun.star.ucb.XContent;
import com.sun.star.ucb.XContentAccess;
import com.sun.star.ucb.XDynamicResultSet;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
/**
* Testing <code>com.sun.star.frame.XDesktop</code>
* interface methods:
* <ul>
*  <li><code> getContent() </code></li>
*  <li><code> storeTemplate() </code></li>
*  <li><code> addTemplate() </code></li>
*  <li><code> removeTemplate() </code></li>
*  <li><code> renameTemplate() </code></li>
*  <li><code> addGroup() </code></li>
*  <li><code> removeGroup() </code></li>
*  <li><code> renameGroup() </code></li>
*  <li><code> update() </code></li>
* </ul><p>
*/
public class _XDocumentTemplates extends MultiMethodTest {
    public XDocumentTemplates oObj = null; // oObj filled by MultiMethodTest
    protected XContent content = null;
    /**
    * Test calls the method and prints contents list to log.<p>
    * Has <b> OK </b> status if the XContent isn't empty.
    */
    public void _getContent() {
        content = oObj.getContent();
        log.println("Content list:" + getContentList(content));
        tRes.tested("getContent()", content != null);
    }

    /**
    * Test calls the method and checks that new group was added. <p>
    * Has <b> OK </b> status if method returns true and new group was added.
    */
    public void _addGroup() {
        requiredMethod("getContent()");
        if (getSubContent(content, "XDocumentTemplatesTemp") != null ) {
            oObj.removeGroup("XDocumentTemplatesTemp");
        }
        if (getSubContent(content, "XDocumentTemplates") != null ) {
            oObj.removeGroup("XDocumentTemplates");
        }
        boolean res  = oObj.addGroup("XDocumentTemplatesTemp");
        log.println("Method returned: " + res);
        res &= getSubContent(content, "XDocumentTemplatesTemp") != null;
        tRes.tested("addGroup()", res);
    }

    XContent groupContent = null;
    /**
    * Test calls the method and checks that content has no group with old name
    * and that content has group with new name. <p>
    * Has <b> OK </b> status if method returns true, content has no group with
    * old name and content has group with new name.<p>
    */
    public void _renameGroup() {
        requiredMethod("addGroup()");
        boolean res = oObj.renameGroup("XDocumentTemplatesTemp",
            "XDocumentTemplates");
        log.println("Method returned: " + res);
        groupContent = getSubContent(content, "XDocumentTemplates");
        res &= getSubContent(content, "XDocumentTemplatesTemp") == null;
        res &= groupContent != null;
        tRes.tested("renameGroup()", res);
    }

    /**
    * Test calls the method and checks that group content has new template. <p>
    * Has <b> OK </b> status if method returns true and group content has new
    * template.<p>
    */
    public void _addTemplate() {
        requiredMethod("renameGroup()");
        String testDoc = utils.getFullTestURL("report.stw");
        log.println("Adding template from " + testDoc);
        boolean res = oObj.addTemplate("XDocumentTemplates",
            "ANewTemplateTemp",testDoc);
        log.println("Method returned: " + res);
        res &= getSubContent(groupContent, "ANewTemplateTemp") != null;
        tRes.tested("addTemplate()", res);
    }

    /**
    * Test calls the method and checks that group content has no template with
    * old name and that group content has template with new name. <p>
    * Has <b> OK </b> status if method returns true, group content has no
    * template with old name and group content has template with new name.<p>
    */
    public void _renameTemplate() {
        requiredMethod("addTemplate()");
        boolean res = oObj.renameTemplate("XDocumentTemplates",
                                          "ANewTemplateTemp",
                                          "ANewTemplate");
        log.println("Method returned: " + res);
        res &= getSubContent(groupContent, "ANewTemplateTemp") == null;
        res &= getSubContent(groupContent, "ANewTemplate") != null;

        tRes.tested("renameTemplate()", res);
    }

    /**
    * Test calls the method and checks that group content has new template. <p>
    * Has <b> OK </b> status if method returns true and new template was created.<p>
    */
    public void _storeTemplate() {
        requiredMethod("renameGroup()");
        XStorable store = (XStorable) tEnv.getObjRelation("Store");
        boolean res = oObj.storeTemplate("XDocumentTemplates",
                                         "NewStoreTemplate",
                                         store);
        log.println("Method returned: " + res);
        res &= getSubContent(groupContent, "NewStoreTemplate") != null;
        tRes.tested("storeTemplate()", res);
    }

    /**
    * Test calls the method and checks that group content has no deleted template. <p>
    * Has <b> OK </b> status if method returns true and group content has no
    * deleted template.<p>
    */
    public void _removeTemplate() {
        requiredMethod("renameTemplate()");
        boolean res = oObj.removeTemplate("XDocumentTemplates", "ANewTemplate");
        log.println("Method returned: " + res);
        res &= getSubContent(groupContent, "ANewTemplate") == null;
        tRes.tested("removeTemplate()", res);
    }

    /**
    * Test calls the method and checks that content has no deleted group. <p>
    * Has <b> OK </b> status if method returns true and content has no deleted
    * group.<p>
    */
    public void _removeGroup() {
        requiredMethod("renameGroup()");
        executeMethod("renameTemplate()");
        boolean res = oObj.removeGroup("XDocumentTemplates");
        log.println("Method returned: " + res);
        res &= getSubContent(content, "XDocumentTemplates") == null;
        tRes.tested("removeGroup()", res);
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if no exception occurs.<p>
    */
    public void _update() {
        oObj.update();
        tRes.tested("update()",true);
    }

    /**
     * Returns the string representation of content passed as parameter.
     */
    protected String getContentList(XContent content) {
        XResultSet statRes = getStatResultSet(content);
        String ret = "";
        try {
            statRes.first();
            XRow row = (XRow)UnoRuntime.queryInterface(XRow.class, statRes);
            while(! statRes.isAfterLast()) {
                ret += "\n    " + row.getString(1);
                statRes.next();
            }
        } catch (com.sun.star.sdbc.SQLException e) {
            log.println("Exception occured:" + e);
        }

        return ret;
    }

    protected XResultSet getStatResultSet(XContent content) {
        XResultSet statResSet = null;
        try {
            statResSet = getDynaResultSet(content).getStaticResultSet();
        } catch(com.sun.star.ucb.ListenerAlreadySetException e) {
            log.println("Exception occured:" + e);
        }
        return statResSet;
    }

    protected XDynamicResultSet getDynaResultSet(XContent content) {
        Command command = new Command();
        OpenCommandArgument2 comArg = new OpenCommandArgument2();
        Property[] comProps = new Property[1];
        comArg.Mode = com.sun.star.ucb.OpenMode.ALL;
        comProps[0] = new Property();
        comProps[0].Name = "Title";
        comArg.Properties = comProps;

        command.Name = "open";
        command.Handle = -1;
        command.Argument = comArg;

        XCommandProcessor comProc = (XCommandProcessor)
            UnoRuntime.queryInterface(XCommandProcessor.class, content);

        XDynamicResultSet DynResSet = null;
        try {
            DynResSet = (XDynamicResultSet) AnyConverter.toObject(
                new Type(XDynamicResultSet.class),comProc.execute(command, 0, null));
        } catch(com.sun.star.ucb.CommandAbortedException e) {
            log.println("Couldn't execute command:" + e);
        } catch(com.sun.star.uno.Exception e) {
            log.println("Couldn't execute command:" + e);
        }

        return DynResSet;
    }

    protected XContent getSubContent(XContent content, String subName) {
        XResultSet statRes = getStatResultSet(content);
        XRow row = (XRow)UnoRuntime.queryInterface(XRow.class, statRes);
        XContentAccess contAcc = (XContentAccess)
            UnoRuntime.queryInterface(XContentAccess.class, statRes);
        XContent subContent = null;
        try {
            statRes.first();
            while(!statRes.isAfterLast()) {
                if ( subName.equals(row.getString(1)) ) {
                    subContent = contAcc.queryContent();
                }
                statRes.next();
            }
        } catch(com.sun.star.sdbc.SQLException e) {
            log.println("Exception occured:" + e);
        }

        return subContent;
    }
}

