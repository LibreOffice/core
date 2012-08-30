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

package complex.tdoc;

import com.sun.star.container.XChild;
import com.sun.star.container.XNamed;
import com.sun.star.uno.UnoRuntime;
import share.LogWriter;

/*
* Testing <code>com.sun.star.container.XChild</code>
* interface methods :
* <ul>
*  <li><code> getParent()</code></li>
*  <li><code> setParent()</code></li>
* </ul>
* @see com.sun.star.container.XChild
*/
public class _XChild {

    public XChild oObj = null;
    public Object gotten = null;
    public LogWriter log = null;


    /**
    * Test calls the method and checks return value and that
    * no exceptions were thrown. Parent returned is stored.<p>
    * Has <b> OK </b> status if the method returns not null value
    * and no exceptions were thrown. <p>
    */
    public boolean _getParent(boolean hasParent) {
        gotten = oObj.getParent();
        if (!hasParent)
            return gotten == null;
        XNamed the_name = (XNamed) UnoRuntime.queryInterface(XNamed.class,gotten);
        if (the_name != null)
            log.println("Parent:"+the_name.getName());
        return gotten != null;
    }

    /**
    * Sets existing parent and checks that
    * no exceptions were thrown. <p>
    * Has <b> OK </b> status if no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getParent() </code> : to get the parent. </li>
    * </ul>
    */
    public boolean _setParent(boolean supported) {
//        requiredMethod("getParent()") ;

        String parentComment = null;//String) tEnv.getObjRelation("cannotSwitchParent");

        if (parentComment != null) {
            log.println(parentComment);
            return true;
        }

        try {
            oObj.setParent(gotten);
        }
        catch (com.sun.star.lang.NoSupportException ex) {
            log.println("Exception occurred during setParent() - " + (supported?"FAILED":"OK"));
            if (supported) {
                ex.printStackTrace((java.io.PrintWriter)log);
                return false;
            }
        }
        return true;
    }

}  // finish class _XChild


