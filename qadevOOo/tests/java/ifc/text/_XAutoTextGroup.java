/*************************************************************************
 *
 *  $RCSfile: _XAutoTextGroup.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:15:13 $
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

package ifc.text;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;
import util.ValueComparer;

import com.sun.star.text.XAutoTextGroup;
import com.sun.star.text.XTextRange;

/**
 * Testing <code>com.sun.star.text.XAutoTextGroup</code>
 * interface methods :
 * <ul>
 *  <li><code> getTitles()</code></li>
 *  <li><code> renameByName()</code></li>
 *  <li><code> insertNewByName()</code></li>
 *  <li><code> removeByName()</code></li>
 * </ul> <p>
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'TextRange'</code> (of type <code>XTextRange</code>):
 *   the range for which an entry is added. </li>
 * <ul> <p>
 * Test is multithread compilant. <p>
 * @see com.sun.star.text.XAutoTextGroup
 */
public class _XAutoTextGroup extends MultiMethodTest {

    public XAutoTextGroup oObj = null;

    /**
     * Unique number among different interface threads.
     */
    protected static int uniq = 0 ;

    /**
     * Unique string for AutoTextEntry names among different
     * threads.
     */
    protected String str = null ;

    /**
     * Prefix for unique string.
     * @see #str
     */
    protected static final String pref = "XAutoTextGroup" ;
    protected XTextRange oRange = null;

    /**
     * Constructs a unique string for current interface thread
     * for naming purposes. All old entries which names are
     * started with prefix used for entry names, are deleted
     * from the group (they can remain after previous unsuccessfull
     * test runs). The relation is obtained.
     *
     * @throws StatusException if the relation is not found.
     */
    public void before() {
        str = pref + uniq++ ;
        String[] names = oObj.getElementNames() ;
        for (int i = 0; i < names.length; i++) {
            log.println("  " + names[i]);
            if (names[i].toUpperCase().indexOf(pref.toUpperCase()) > 0) {
                try {
                    log.println("  ... removing ...");
                    oObj.removeByName(names[i]) ;
                } catch (com.sun.star.container.NoSuchElementException e) {
                    log.println("Element '" + names[i] + "' not found.");
                }
            }
        }

        oRange = (XTextRange) tEnv.getObjRelation("TextRange");
        if (oRange == null) {
            throw new StatusException(Status.failed("No relation found")) ;
        }
    }

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns not
     * <code>null</code> value.
     */
    public void _getTitles() {

        String[] titles = oObj.getTitles();
        tRes.tested("getTitles()",titles != null);
    }

    /**
     * Firsts inserts a new <code>AutoTextEntry</code> using a range
     * from relation, entry titles are checked before and after
     * insertion, second tries to add an entry with the same name. <p>
     *
     * Has <b>OK</b> status if in the first case titles are changed,
     * and in the second case <code>ElementExistException</code> is
     * thrown.
     */
    public void _insertNewByName() {

        boolean result = false;

        try {
            String[] before = oObj.getTitles();
            oObj.insertNewByName(str, "For " + str,oRange);
            String[] after = oObj.getTitles();
            result = !util.ValueComparer.equalValue(before, after);
        }
        catch (com.sun.star.container.ElementExistException ex) {
            log.println("Exception occured while testing insertNewByName");
            ex.printStackTrace(log);
            result = false;
        }

        try {
            oObj.insertNewByName(str, "For " + str, oRange);
            log.println(
                "com::sun::star::container::ElementExistsException wasn't thrown");
            oObj.removeByName(str);
            result &= false;
        } catch (com.sun.star.container.ElementExistException ex) {
            result &= true;
        } catch (com.sun.star.container.NoSuchElementException ex) {
            log.println("Wrong exception was thrown :");
            ex.printStackTrace(log);
            result &= false;
        }

        tRes.tested("insertNewByName()",result);

    }

    /**
     * Removes <code>AutoTextEntry</code> added before and checks
     * titles of the group before and after removing. <p>
     * Has <b> OK </b> status if titles are not equal before and after
     * removing and no exceptions were thrown. <p>
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> insertNewByName() </code> : the entry is
     *   inserted here. </li>
     * </ul>
     */
    public void _removeByName() {
        requiredMethod("insertNewByName()") ;

        try {
            String[] before = oObj.getTitles();
            oObj.removeByName(str);
            String[] after = oObj.getTitles();
            tRes.tested("removeByName()",
                !ValueComparer.equalValue(before,after));
        }
        catch (com.sun.star.container.NoSuchElementException ex) {
            log.println("Exception occured while testing removeByName");
            ex.printStackTrace(log);
            tRes.tested("removeByName()",false);
        }
    }

    /**
     * Three cases are tested here :
     * <ol>
     *   <li> Trying to rename an entry to a name, which already
     *     exists in the group. <code>ElementExistException</code>
     *     must be thrown. </li>
     *   <li> Trying to rename an element with non-existing name.
     *     <code>IllegalArgumentException</code> must be thrown.</li>
     *   <li> The normal situation : no exceptions must be thrown
     *     and element with a new name must arise. </li>
     * </ol>
     *
     * Has <b>OK</b> status if all three cases were completed successfully.
     */
    public void _renameByName() {
        boolean result = false;

        try {
            String[] tit = oObj.getTitles() ;
            String[] names = oObj.getElementNames() ;
            oObj.insertNewByName(str,"For " + str,oRange);
            oObj.insertNewByName(str + "dup","For " + str,oRange);
            tit = oObj.getTitles() ;
            names = oObj.getElementNames() ;
            result = true;
        } catch (com.sun.star.container.ElementExistException e) {
            log.println("Unexpected exception occured :") ;
            e.printStackTrace(log);
        } finally {
            if (!result) {
                try {
                    oObj.removeByName(str);
                } catch (com.sun.star.container.NoSuchElementException e) {}
                try {
                    oObj.removeByName(str + "dup");
                } catch (com.sun.star.container.NoSuchElementException e) {}
                tRes.tested("renameByName()", false);
                return;
            }
        }


        try {
            oObj.renameByName(str, str + "dup", "For "+str);
            log.println(
                "com::sun::star::container::ElementExistsException wasn't thrown");
            result = false;
        } catch (com.sun.star.container.ElementExistException e) {
            result = true;
        } catch (com.sun.star.io.IOException e) {
            log.println("Wrong exception was thrown :");
            e.printStackTrace(log);
            result = false;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Wrong exception was thrown :");
            e.printStackTrace(log);
            result = false;
        } finally {
            try {
                oObj.removeByName(str);
            } catch (com.sun.star.container.NoSuchElementException e) {}
            try {
                oObj.removeByName(str + "dup");
            } catch (com.sun.star.container.NoSuchElementException e) {}
        }

        try {
            oObj.renameByName("~"+str,str,str);
            log.println(
                "com::sun::star::lang::IllegalArgumentException wasn't thrown");
            result &= false;
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            result &= true;
        } catch (com.sun.star.container.ElementExistException e) {
            log.println("Unexpected exception :") ;
            e.printStackTrace(log) ;
            result = false ;
        } catch (com.sun.star.io.IOException e) {
            log.println("Unexpected exception :") ;
            e.printStackTrace(log) ;
            result = false ;
        } finally {
            try {
                oObj.removeByName(str);
            } catch (com.sun.star.container.NoSuchElementException e) {}
        }

        try {
            oObj.insertNewByName(str, "For " + str, oRange);

            oObj.renameByName(str,str+"a",str+"b");
            result &= oObj.hasByName(str + "a");
        } catch (com.sun.star.container.ElementExistException ex) {
            log.println("Exception occured while testing renameByName");
            ex.printStackTrace(log);
            result &=false;
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception occured while testing renameByName");
            ex.printStackTrace(log);
            result &=false;
        } catch (com.sun.star.io.IOException ex) {
            log.println("Exception occured while testing renameByName");
            ex.printStackTrace(log);
            result &=false;
        } finally {
            try {
                oObj.removeByName(str);
            } catch (com.sun.star.container.NoSuchElementException e) {}
            try {
                oObj.removeByName(str + "a");
            } catch (com.sun.star.container.NoSuchElementException e) {}
        }

        tRes.tested("renameByName()",result);

    }

}  // finish class _XAutoTextGroup


