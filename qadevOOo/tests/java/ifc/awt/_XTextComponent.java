/*************************************************************************
 *
 *  $RCSfile: _XTextComponent.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 10:13:12 $
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

package ifc.awt;

import lib.MultiMethodTest;
import util.ValueComparer;

import com.sun.star.awt.Selection;
import com.sun.star.awt.TextEvent;
import com.sun.star.awt.XTextComponent;
import com.sun.star.awt.XTextListener;
import com.sun.star.lang.EventObject;

/**
* Testing <code>com.sun.star.awt.XTextComponent</code>
* interface methods:
* <ul>
*  <li><code> addTextListener() </code></li>
*  <li><code> removeTextListener() </code></li>
*  <li><code> setText() </code></li>
*  <li><code> getText() </code></li>
*  <li><code> insertText() </code></li>
*  <li><code> getSelectedText() </code></li>
*  <li><code> setSelection() </code></li>
*  <li><code> getSelection() </code></li>
*  <li><code> setEditable() </code></li>
*  <li><code> isEditable() </code></li>
*  <li><code> setMaxTextLen() </code></li>
*  <li><code> getMaxTextLen() </code></li>
* </ul><p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'XTextComponent.onlyNumbers'</code> (of type <code>Object</code>):
*  needed for checking if component can contain only numeric values </li>
* </ul><p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.awt.XTextComponent
*/
public class _XTextComponent extends MultiMethodTest {
    public XTextComponent oObj = null;
    public boolean textChanged = false;
    // indicates that component can contain only numeric values
    private boolean num = false ;

    /**
    * Listener implementation which just set flag when listener
    * method is called.
    */
    protected class MyChangeListener implements XTextListener {
        public void disposing ( EventObject oEvent ) {}
        public void textChanged(TextEvent ev) {
            textChanged = true;
        }
    }

    XTextListener listener = new MyChangeListener();

    /**
    * Retrieves object relation, then sets flag 'num' to 'true'
    * if relation is not null.
    */
    public void before() {
        if (tEnv.getObjRelation("XTextComponent.onlyNumbers") != null)
            num = true;
    }

    /**
    * After test calls the method, a new text is set to the object. Then
    * we check if listener was called, and set a new text value
    * to the object.<p>
    * Has <b> OK </b> status if listener was called.
    */
    public void _addTextListener(){
        oObj.addTextListener(listener);
        oObj.setText("Listen");
        try {
            Thread.sleep(500);
        } catch(java.lang.InterruptedException e) {
            e.printStackTrace(log);
        }
        if (!textChanged) {
            log.println("Listener wasn't called after changing Text");
        }

        tRes.tested("addTextListener()",textChanged);
    }

    /**
    * After setting flag 'textChanged' to false, test calls the method.
    * Then a new text value is set to the object. <p>
    * Has <b> OK </b> status if listener was not called. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li><code> addTextListener() </code>: adds listener to the object.</li>
    * </ul>
    */
    public void _removeTextListener() {
        requiredMethod("addTextListener()");
        textChanged = false;
        oObj.removeTextListener(listener);
        oObj.setText("Do not listen");
        tRes.tested("removeTextListener()",!textChanged);
    }

    /**
    * At first we're setting some string variable 'newText' depending of a kind
    * of object we are working with. Then test calls the method. <p>
    * Has <b> OK </b> status if set value is equal to a value obtained after.
    */
    public void _setText() {
        String newText = num ? "823" : "setText" ;
        if (tEnv.getTestCase().getObjectName().equals("OTimeControl")) {
            newText = "8:15";
        }
        log.println("Setting text to : '" + newText + "'") ;
        oObj.setText(newText);
        log.println("Getting text : '" + oObj.getText() + "'") ;
        tRes.tested("setText()",oObj.getText().equals(newText));
    }

    /**
    * At first we're setting some string variable 'newText' depending of a kind
    * of object we are working with. Then we set text to the object and call
    * the method. <p>
    * Has <b> OK </b> status if set value is equal to a value obtained using
    * getText() method.
    */
    public void _getText() {
        String newText = num ? "823" : "setText" ;
        if (tEnv.getTestCase().getObjectName().equals("OTimeControl")) {
            newText = "8:15";
        }
        oObj.setText(newText);
        tRes.tested("getText()",oObj.getText().equals(newText));
    }

    /**
    * At first we're setting string variables 'text' and 'itext' depending
    * of a kind of object we are working with. Next, value from 'text' variable
    * is set to an object using setText(), then the method insertText() is called.
    * <p>
    * Has <b> OK </b> status if text is inserted to the object.
    */
    public void _insertText() {
        String text = num ? "753" :  "iText" ;
        String itext = num ? "6" :  "insert" ;
        log.println("Setting text to : '" + text + "'") ;
        oObj.setText(text);
        log.println("Iserting text to (0,1) : '" + itext + "'") ;
        oObj.insertText(new Selection(0,1), itext);
        log.println("getText() returns: " + oObj.getText());
        tRes.tested("insertText()", oObj.getText().equals
            (num ? "653" : "insertText"));
    }

    /**
    * After text is set to the object, test calls the method.<p>
    * Has <b> OK </b> status if selected text is equal to first three symbols
    * of text added before.
    */
    public void _getSelectedText() {
        String text = num ? "753" :  "txt" ;
        oObj.setText(text);
        oObj.setSelection(new Selection(0,3));
        boolean result = oObj.getSelectedText().equals(text);

        if (! result) {
            System.out.println("Getting '"+oObj.getSelectedText()+"'");
            System.out.println("Expected '"+text+"'");
        }

        tRes.tested("getSelectedText()",result);
    }

    /**
    * After setting new text to an object, and defining selection variable,
    * test calls the method. <p>
    * Has <b> OK </b> status if selection set before is equal to a selection we
    * got using getSelection().
    */
    public void _setSelection() {
        oObj.setText("setSelection");
        Selection sel = new Selection(0,3);
        oObj.setSelection(sel);
        tRes.tested("setSelection()", ValueComparer.equalValue
            (oObj.getSelection(), sel));
    }

    /**
    * After setting new text to an object, and defining selection variable,
    * test calls the method. <p>
    * Has <b> OK </b> status if selection set before is equal to a selection we
    * got using getSelection().
    */
    public void _getSelection() {
        oObj.setText("getSelection");
        Selection sel = new Selection(2,3);
        oObj.setSelection(sel);
        tRes.tested("getSelection()", ValueComparer.equalValue
            (oObj.getSelection(), sel));
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if method has changed a property 'Editable'.
    */
    public void _setEditable(){
        oObj.setEditable(true);
        tRes.tested("setEditable()", oObj.isEditable());
    }

    /**
    * First we set 'Editable' variable to false. Then test calls the method.<p>
    * Has <b> OK </b> status if method returns value we set before.
    */
    public void _isEditable(){
        oObj.setEditable(false);
        tRes.tested("isEditable()", ! oObj.isEditable());
    }

    /**
    * Test calls the method. Then new text value is set to the object. <p>
    * Has <b> OK </b> status if text, returned by getText() is a string of
    * length we set before.
    */
    public void _setMaxTextLen() {
        oObj.setMaxTextLen((short)10);
        //oObj.setText("0123456789ABCDE");
        //String get = oObj.getText();
        //tRes.tested("setMaxTextLen()",get.length() == 10);
        tRes.tested("setMaxTextLen()",oObj.getMaxTextLen()==10);
    }

    /**
    * At first we set MaxTextLen, then test calls the method. <p>
    * Has <b> OK </b> status if method returns a value we set before.
    */
    public void _getMaxTextLen() {
        oObj.setMaxTextLen((short)15);
        log.println("getMaxTextLen() returns: "+oObj.getMaxTextLen());
        tRes.tested("getMaxTextLen()",oObj.getMaxTextLen()==15);
    }

}

