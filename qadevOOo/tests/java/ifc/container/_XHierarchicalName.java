/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XHierarchicalName.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:21:18 $
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
package ifc.container;

import com.sun.star.container.XHierarchicalName;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;


public class _XHierarchicalName extends MultiMethodTest {
    public XHierarchicalName oObj;

    public void _composeHierarchicalName() {
        String expName = (String) tEnv.getObjRelation("HierachicalName");
        String element = (String) tEnv.getObjRelation("ElementName");
        boolean res = false;

        try {
            String hn = oObj.composeHierarchicalName(element);
            res = hn.startsWith(expName);

            if (!res) {
                log.println("Getting : " + hn +
                            " but expected it to start with " + expName);
            }
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Exception " + e.getMessage());
        } catch (com.sun.star.lang.NoSupportException e) {
            log.println("Exception " + e.getMessage());
        }

        tRes.tested("composeHierarchicalName()", res);
    }

    public void _getHierarchicalName() {
        String hName = oObj.getHierarchicalName();
        String expName = (String) tEnv.getObjRelation("HierachicalName");
        boolean res = true;
System.out.println("### "+hName);
        if (expName != null) {
            res = hName.startsWith(expName);

            if (!res) {
                log.println("Expected the name to start with " + expName);
                log.println("got " + hName);
            }
        } else {
            throw new StatusException(Status.failed(
                                              "ObjectRelation 'HierachicalName' missing"));
        }

        tRes.tested("getHierarchicalName()", res);
    }
}