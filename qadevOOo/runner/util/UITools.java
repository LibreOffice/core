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

package util;

import java.awt.Robot;
import java.awt.event.InputEvent;
import java.io.PrintWriter;
import java.util.ArrayList;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleAction;
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleEditableText;
import com.sun.star.accessibility.XAccessibleSelection;
import com.sun.star.accessibility.XAccessibleText;
import com.sun.star.accessibility.XAccessibleValue;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.awt.XTopWindow;
import com.sun.star.awt.XWindow;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;


/**
 * This class supports some functions to handle easily accessible objects
 */
public class UITools {

    private static final AccessibilityTools mAT = new AccessibilityTools();
    private final XAccessible mXRoot;
    private final XMultiServiceFactory mMSF;

    public UITools(XMultiServiceFactory msf, XModel xModel)
    {
        mMSF = msf;
        mXRoot = makeRoot(mMSF, xModel);
    }

    public UITools(XMultiServiceFactory msf, XTextDocument xTextDoc)
    {
        mMSF = msf;
        XModel xModel = UnoRuntime.queryInterface(XModel.class, xTextDoc);
        mXRoot = makeRoot(mMSF, xModel);
    }

    public UITools(XMultiServiceFactory msf, XWindow xWindow)
    {
        mMSF = msf;
        mXRoot = makeRoot(xWindow);
    }

    private static XAccessible makeRoot(XMultiServiceFactory msf, XModel aModel)
    {
        XWindow xWindow = mAT.getCurrentWindow(msf, aModel);
        return mAT.getAccessibleObject(xWindow);
    }


    private static String getString(XInterface xInt)
    {
        XAccessibleText oText = UnoRuntime.queryInterface(XAccessibleText.class, xInt);
        return oText.getText();
    }

    private static void setString(XInterface xInt, String cText)
    {
        XAccessibleEditableText oText = UnoRuntime.queryInterface(XAccessibleEditableText.class, xInt);

        oText.setText(cText);
    }

    private static Object getValue(XInterface xInt)
    {
         XAccessibleValue oValue = UnoRuntime.queryInterface(XAccessibleValue.class, xInt);
         return oValue.getCurrentValue();
    }

    private static XAccessible makeRoot(XWindow xWindow)
    {
        return mAT.getAccessibleObject(xWindow);
    }

    /**
     * get the root element of the accessible tree
     * @return the root element
     */
    public XAccessible getRoot()
    {
        return mXRoot;
    }

    /**
     * Helper mathod: set a text into AccessibleEdit field
     * @param textfiledName is the name of the text field
     * @param stringToSet is the string to set
     * @throws java.lang.Exception if something fail
     */
    public void setTextEditFiledText(String textfiledName, String stringToSet)
                        throws java.lang.Exception
    {
        XInterface oTextField = mAT.getAccessibleObjectForRole(mXRoot,
                                            AccessibleRole.TEXT, textfiledName);
        setString(oTextField, stringToSet);
    }

    /**
     * returns the button by the given name
     * @param buttonName is name name of the button to get
     * @return a XAccessibleContext of the button
     * @throws java.lang.Exception if something fail
     */
    public XAccessibleContext getButton(String buttonName) throws java.lang.Exception
    {
        return mAT.getAccessibleObjectForRole
                                (mXRoot, AccessibleRole.PUSH_BUTTON, buttonName);
    }

    /**
     * Helper method: gets button via accessibility and 'click' it</code>
     * @param buttonName is name name of the button to click
     * @throws java.lang.Exception if something fail
     */

     public void clickButton(String buttonName) throws java.lang.Exception
     {

        XAccessibleContext oButton =mAT.getAccessibleObjectForRole
                                (mXRoot, AccessibleRole.PUSH_BUTTON, buttonName);
        if (oButton == null){
            throw new Exception("Could not get button '" + buttonName + "'");
        }
        XAccessibleAction oAction = UnoRuntime.queryInterface(XAccessibleAction.class, oButton);

        // "click" the button
        try{
            oAction.doAccessibleAction(0);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
          throw new Exception("Could not do accessible action with '" +
                               buttonName + "'" + e.toString());
        }
     }



    /**
     * Helper method: gets button via accessibility and 'click' it
     *  @param buttonName   The name of the button in the accessibility tree
     *  @param toBePressed  desired state of the toggle button
     *
     *  @return true if the state of the button could be changed in the desired manner
     */
     private boolean clickToggleButton(String buttonName, boolean toBePressed)
     {
        XAccessibleContext oButton =mAT.getAccessibleObjectForRole
                                (mXRoot, AccessibleRole.TOGGLE_BUTTON, buttonName);

        if (oButton != null){
            boolean isChecked = oButton.getAccessibleStateSet().contains(com.sun.star.accessibility.AccessibleStateType.CHECKED);
            if((isChecked && !toBePressed) || (!isChecked && toBePressed)){
                XAccessibleAction oAction = UnoRuntime.queryInterface(XAccessibleAction.class, oButton);
                try{
                    // "click" the button
                    oAction.doAccessibleAction(0);
                    return true;
                } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                  System.out.println("Could not do accessible action with '"
                        + buttonName + "'" + e.toString());
                  return false;
                }
            }else
                //no need to press togglebar, do nothing
                return true;
        } else{
            System.out.println("Could not get button '" + buttonName + "'");
            return false;
        }
     }

    /**
     * Deactivates toggle button via Accessibility
     *  @param buttonName   The name of the button in the Accessibility tree
     *
     *  @return true if the button could be set to deactivated
     */
    public boolean deactivateToggleButton(String buttonName){
        return clickToggleButton(buttonName, false);
    }

    /**
     * Activates toggle button via Accessibility
     *  @param buttonName   The name of the button in the Accessibility tree
     *
     *  @return true if the button could be set to activated
     */
    public boolean activateToggleButton(String buttonName){
        return clickToggleButton(buttonName, true);
    }

     /**
      * returns the value of named radio button
      * @param buttonName the name of the button to get the value of
      * @throws java.lang.Exception if something fail
      * @return Integer
      */
     public Integer getRadioButtonValue(String buttonName)
            throws java.lang.Exception
     {
        try {
            XInterface xRB =mAT.getAccessibleObjectForRole(mXRoot,
                                       AccessibleRole.RADIO_BUTTON, buttonName);

            return (Integer) getValue(xRB);
        } catch (Exception e) {
          throw new Exception("Could not get value from RadioButton '"
                                          + buttonName + "' : " + e.toString());
        }
     }

     /**
      * returns the named graphic
      * @param GraphicName the name of the graphic
      * @return XInterface
      * @throws java.lang.Exception if something fail
      */
     public XInterface getGraphic(String GraphicName) throws java.lang.Exception
     {
        return mAT.getAccessibleObjectForRole(mXRoot, AccessibleRole.GRAPHIC,
                                             GraphicName);
     }


     /**
      * set a named radio button the a given value
      * @param buttonName the name of the button to set
      * @param iValue the value to set
      * @throws java.lang.Exception if something fail
      */
     public void setRadioButtonValue(String buttonName, int iValue)
            throws java.lang.Exception
     {
        try {
            XInterface xRB =mAT.getAccessibleObjectForRole(mXRoot, AccessibleRole.RADIO_BUTTON, buttonName);
            if(xRB == null)
                System.out.println("AccessibleObjectForRole couldn't be found for " + buttonName);
            XAccessibleValue oValue = UnoRuntime.queryInterface(XAccessibleValue.class, xRB);
            if(oValue == null)
                System.out.println("XAccessibleValue couldn't be queried for " + buttonName);
            oValue.setCurrentValue(new Integer(iValue));
        } catch (Exception e) {
          e.printStackTrace();

          throw new Exception("Could not set value to RadioButton '"
                                          + buttonName + "' : " + e.toString());
        }

     }

     /**
      * select an item in nanmed listbox
      * @param ListBoxName the name of the listbox
      * @param nChildIndex the index of the item to set
      * @throws java.lang.Exception if something fail
      */
     public void selectListboxItem(String ListBoxName, int nChildIndex)
            throws java.lang.Exception
     {
        try {
            XAccessibleContext xListBox = null;

            xListBox =mAT.getAccessibleObjectForRole(mXRoot,
                                         AccessibleRole.COMBO_BOX, ListBoxName);
            if (xListBox == null){
                xListBox =mAT.getAccessibleObjectForRole(mXRoot,
                                             AccessibleRole.PANEL, ListBoxName);
            }
            XAccessible xListBoxAccess = UnoRuntime.queryInterface(XAccessible.class, xListBox);

            // if a List is not pulled to be open all entries are not visiblle, therefore the
            // boolean argument
            XAccessibleContext xList =mAT.getAccessibleObjectForRole(
                                          xListBoxAccess, AccessibleRole.LIST, true);
            XAccessibleSelection xListSelect = UnoRuntime.queryInterface(XAccessibleSelection.class, xList);

            xListSelect.selectAccessibleChild(nChildIndex);

        } catch (Exception e) {
          throw new Exception("Could not select item '" +nChildIndex+
                        "' in listbox '" + ListBoxName + "' : " + e.toString());
        }
     }

     /**
      * This method returns all entries as XInterface of a list box
      * @param ListBoxName the name of the listbox
      * @return Object[] containing XInterface
      * @throws java.lang.Exception if something fail
      */

     public Object[] getListBoxObjects(String ListBoxName)
            throws java.lang.Exception
     {
         ArrayList<XInterface> Items = new ArrayList<XInterface>();
         try {
            XAccessibleContext xListBox = null;
            XAccessibleContext xList = null;

            xListBox =mAT.getAccessibleObjectForRole(mXRoot,
                                         AccessibleRole.COMBO_BOX, ListBoxName);
            if (xListBox == null){
                xListBox =mAT.getAccessibleObjectForRole(mXRoot,
                                             AccessibleRole.PANEL, ListBoxName);
            }

            if (xListBox == null){
                // get the list of TreeListBox
                xList =mAT.getAccessibleObjectForRole(mXRoot,
                                              AccessibleRole.TREE, ListBoxName);

            // all other list boxes have a children of kind of LIST
            } else {

                XAccessible xListBoxAccess = UnoRuntime.queryInterface(XAccessible.class, xListBox);
                // if a List is not pulled to be open all entries are not visiblle, therefore the
                // boolean argument
                xList =mAT.getAccessibleObjectForRole(
                                              xListBoxAccess, AccessibleRole.LIST, true);
            }

            for (int i=0;i<xList.getAccessibleChildCount();i++) {
                try {
                    XAccessible xChild = xList.getAccessibleChild(i);
                    XAccessibleContext xChildCont =
                                                  xChild.getAccessibleContext();
                    XInterface xChildInterface = UnoRuntime.queryInterface(XInterface.class, xChildCont);
                    Items.add(xChildInterface);

                } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                      throw new Exception("Could not get child form list of '"
                                         + ListBoxName + "' : " + e.toString());
                }
            }

         } catch (Exception e) {
            throw new Exception("Could not get list of items from '"
                                         + ListBoxName + "' : " + e.toString());
        }
        Object[]ret = new XInterface[Items.size()];
        for (int i=0;i<Items.size();i++){
            ret[i] = Items.get(i);
        }
        return ret;
     }

     /**
      * Helper method: returns the entry manes of a List-Box
      * @param ListBoxName the name of the listbox
      * @return the listbox entry names
      * @throws java.lang.Exception if something fail
      */

     public String[] getListBoxItems(String ListBoxName)
            throws java.lang.Exception
     {
         ArrayList<String> Items = new ArrayList<String>();
         try {
            XAccessibleContext xListBox = null;
            XAccessibleContext xList = null;

            xListBox =mAT.getAccessibleObjectForRole(mXRoot,
                                         AccessibleRole.COMBO_BOX, ListBoxName);
            if (xListBox == null){
                xListBox =mAT.getAccessibleObjectForRole(mXRoot,
                                             AccessibleRole.PANEL, ListBoxName);
            }

            if (xListBox == null){
                // get the list of TreeListBox
                xList =mAT.getAccessibleObjectForRole(mXRoot,
                                              AccessibleRole.TREE, ListBoxName);

            // all other list boxes have a children of kind of LIST
            } else {

                XAccessible xListBoxAccess = UnoRuntime.queryInterface(XAccessible.class, xListBox);
                // if a List is not pulled to be open all entries are not visiblle, therefore the
                // boolean argument
                xList =mAT.getAccessibleObjectForRole(
                                              xListBoxAccess, AccessibleRole.LIST, true);
            }

            for (int i=0;i<xList.getAccessibleChildCount();i++) {
                try {
                    XAccessible xChild = xList.getAccessibleChild(i);
                    XAccessibleContext xChildCont =
                                                  xChild.getAccessibleContext();
                    XInterface xChildInterface = UnoRuntime.queryInterface(XInterface.class, xChildCont);
                    Items.add(getString(xChildInterface));

                } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                      throw new Exception("Could not get child form list of '"
                                         + ListBoxName + "' : " + e.toString());
                }
            }

         } catch (Exception e) {
            throw new Exception("Could not get list of items from '"
                                         + ListBoxName + "' : " + e.toString());
        }
        String[]ret = new String[Items.size()];
        return Items.toArray(ret);
     }
     /**
      * set to a named nureric filed a given value
      * @param NumericFieldName the name of the nureic field
      * @param cValue the value to set
      * @throws java.lang.Exception if something fail
      */
     public void setNumericFieldValue(String NumericFieldName, String cValue)
        throws java.lang.Exception
     {
         try{
            XInterface xNumericField =mAT.getAccessibleObjectForRole(
                                  mXRoot, AccessibleRole.TEXT, NumericFieldName);
            UnoRuntime.queryInterface(
             XAccessibleEditableText.class, xNumericField);

            setString(xNumericField, cValue);
         } catch (Exception e) {
          throw new Exception("Could not set value '" + cValue +
            "' into NumericField '" + NumericFieldName + "' : " + e.toString());
        }
     }

     /**
      * returns the value of a numeric field
      * @param NumericFieldName the name of the numreic field
      * @throws java.lang.Exception if something fail
      * @return the value of the named numeric filed
      */
     public String getNumericFieldValue(String NumericFieldName)
        throws java.lang.Exception
     {
         try{
            XInterface xNumericField =mAT.getAccessibleObjectForRole(
                                  mXRoot, AccessibleRole.TEXT, NumericFieldName);
            return getString(xNumericField);

         } catch (Exception e) {
          throw new Exception("Could get value from NumericField '"
                                    + NumericFieldName + "' : " + e.toString());
        }
     }

     private String removeCharactersFromCurrencyString(String stringVal)
        throws java.lang.Exception
     {
         try{
            int beginIndex = 0;
            int endIndex = 0;
            // find the first numeric character in stringVal
            for(int i = 0; i < stringVal.length(); i++){
                int numVal = Character.getNumericValue(stringVal.charAt(i));
                // if ascii is a numeric value
                if (numVal != -1){
                    beginIndex = i;
                    break;
                }
            }
            // find the last numeric character in stringVal
            for(int i = stringVal.length()-1; i > 0; i--){
                int numVal = Character.getNumericValue(stringVal.charAt(i));
                if (numVal != -1){
                    endIndex = i+1;
                    break;
                }
            }
            String currencyVal = stringVal.substring(beginIndex, endIndex);

            currencyVal = currencyVal.substring(0, currencyVal.length()-3) +
                          "#" + currencyVal.substring(currencyVal.length()-2);

            currencyVal = utils.replaceAll13(currencyVal, ",", "");
            currencyVal = utils.replaceAll13(currencyVal, "\\.", "");
            currencyVal = utils.replaceAll13(currencyVal, "#", ".");

            return currencyVal;
         } catch (Exception e) {
          throw new Exception("Could get remove characters from currency string '"
                                    + stringVal + "' : " + e.toString());
        }

     }

     /**
      * returns the numeric value of a numeric filed. This is needed ie. for
      * fileds include the moneytary unit.
      * @param NumericFieldName the name of the numeric filed
      * @return the value of the numeric filed
      * @throws java.lang.Exception if something fail
      */
     public Double getNumericFieldNumericValue(String NumericFieldName)
        throws java.lang.Exception
     {
         try{
            Double retValue = null;
            String sValue = getNumericFieldValue(NumericFieldName);
            String sAmount = removeCharactersFromCurrencyString(sValue);
            retValue = retValue.valueOf(sAmount);

            return retValue;

         } catch (Exception e) {
          throw new Exception("Could get numeric value from NumericField '"
                                    + NumericFieldName + "' : " + e.toString());
        }
     }


      /**
       * returns the content of a TextBox
       * @param TextFieldName the name of the textbox
       * @return the value of the text box
       * @throws java.lang.Exception if something fail
       */
     public String getTextBoxText(String TextFieldName)
        throws java.lang.Exception
     {
        String TextFieldText = null;
        try{
            XAccessibleContext xTextField =mAT.getAccessibleObjectForRole(mXRoot,
                                     AccessibleRole.SCROLL_PANE, TextFieldName);
            XAccessible xTextFieldAccess = UnoRuntime.queryInterface(XAccessible.class, xTextField);
            XAccessibleContext xFrame =mAT.getAccessibleObjectForRole(
                                   xTextFieldAccess, AccessibleRole.TEXT_FRAME);
            for (int i=0;i<xFrame.getAccessibleChildCount();i++) {
                try {
                    XAccessible xChild = xFrame.getAccessibleChild(i);
                    XAccessibleContext xChildCont =
                                                  xChild.getAccessibleContext();
                    XInterface xChildInterface = UnoRuntime.queryInterface(XInterface.class, xChildCont);
                    TextFieldText += (getString(xChildInterface));

                } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                    throw new Exception("Could not get child fom TextFrame of '"
                                       + TextFieldName + "' : " + e.toString());
                }
            }
            return TextFieldText;
         } catch (Exception e) {
            throw new Exception("Could not get content fom Textbox '"
                                       + TextFieldName + "' : " + e.toString());
        }
     }

     /**
      * set a value to a named check box
      * @param CheckBoxName the name of the check box
      * @param Value the value to set
      *<ul>
      *    <li>0: not checked </li>
      *    <li>1: checked </li>
      *    <li>2: don't know </li>
      *</ul>
      * @throws java.lang.Exception if something fail
      */
     public void setCheckBoxValue(String CheckBoxName, Integer Value)
            throws java.lang.Exception
     {
         try {
            XInterface xCheckBox =mAT.getAccessibleObjectForRole(mXRoot,
                                     AccessibleRole.CHECK_BOX, CheckBoxName);
            XAccessibleValue xCheckBoxValue = UnoRuntime.queryInterface(XAccessibleValue.class, xCheckBox);
            xCheckBoxValue.setCurrentValue(Value);

         } catch (Exception e) {
            throw new Exception("Could not set value to CheckBox '"
                                       + CheckBoxName + "' : " + e.toString());
        }
     }

     /**
      * returns the value of the named check box
      * @param CheckBoxName the name of the check box
      * @return the value of the check box
      * @throws java.lang.Exception if something fail
      */
    public Integer getCheckBoxValue(String CheckBoxName)
            throws java.lang.Exception
     {
         try {
            XInterface xCheckBox =mAT.getAccessibleObjectForRole(mXRoot,
                                     AccessibleRole.CHECK_BOX, CheckBoxName);
            XAccessibleValue xCheckBoxValue = UnoRuntime.queryInterface(XAccessibleValue.class, xCheckBox);

            return (Integer) xCheckBoxValue.getCurrentValue();
         } catch (Exception e) {
            throw new Exception("Could not set value to CheckBox '"
                                       + CheckBoxName + "' : " + e.toString());
        }
     }

      /**
       * returns the message of a Basic-MessageBox
       * @return the message of a Basic-MessageBox
       * @throws java.lang.Exception if something fail
       */
     public String getMsgBoxText()
        throws java.lang.Exception
     {
        String cMessage = null;
        try{
            XAccessibleContext xMessage =mAT.getAccessibleObjectForRole(mXRoot,
                                     AccessibleRole.LABEL);

            XInterface xMessageInterface = UnoRuntime.queryInterface(XInterface.class, xMessage);
            cMessage += (getString(xMessageInterface));


            return cMessage;
         } catch (Exception e) {
            throw new Exception("Could not get message from Basic-MessageBox: " + e.toString());
        }
     }

    /**
     * fetch the window which is equal to the given <CODE>WindowName</CODE>
     * @return the named window
     * @throws java.lang.Exception if something fail
     */
    public XWindow getTopWindow(String WindowName, boolean debug) throws java.lang.Exception
    {
        XInterface xToolKit = null;
        try {
            xToolKit = (XInterface) mMSF.createInstance("com.sun.star.awt.Toolkit") ;
        } catch (com.sun.star.uno.Exception e) {
          throw new Exception("Could not toolkit: " + e.toString());
        }
        XExtendedToolkit tk = UnoRuntime.queryInterface(XExtendedToolkit.class, xToolKit);

        int count = tk.getTopWindowCount();

        XTopWindow retWindow = null;

        if (debug) System.out.println("getTopWindow ->");

        for (int i=0; i < count ; i++){
            XTopWindow xTopWindow = tk.getTopWindow(i);
            XAccessible xAcc = mAT.getAccessibleObject(xTopWindow);
            String accName = xAcc.getAccessibleContext().getAccessibleName();

            if (debug){
                System.out.println("AccessibleName: " + accName);
            }

            if (WindowName.equals(accName)){
                if (debug) System.out.println("-> found window with name '" + WindowName + "'");
                retWindow = xTopWindow;
            }
        }


        if (debug) {
            if (retWindow == null) System.out.println("could not found window with name '" + WindowName + "'");
            System.out.println("<- getTopWindow ");
        }
        return UnoRuntime.queryInterface(XWindow.class, retWindow);
    }

    public void clickMiddleOfAccessibleObject(short role, String name){

        XAccessibleContext xAcc =mAT.getAccessibleObjectForRole(mXRoot, role, name);
        XAccessibleComponent aComp = UnoRuntime.queryInterface(
                                             XAccessibleComponent.class, xAcc);

        System.out.println(xAcc.getAccessibleRole() + "," +
                    xAcc.getAccessibleName() + "(" +
                    xAcc.getAccessibleDescription() + "):" +
                    utils.getImplName(xAcc));

        if (aComp != null) {
            aComp.getLocationOnScreen();
            String bounds = "(" + aComp.getBounds().X + "," +
                            aComp.getBounds().Y + ")" + " (" +
                            aComp.getBounds().Width + "," +
                            aComp.getBounds().Height + ")";
            System.out.println("The boundary Rectangle is " + bounds);
                try {
                    Robot rob = new Robot();
                    int x = aComp.getLocationOnScreen().X + (aComp.getBounds().Width / 2);
                    int y = aComp.getLocationOnScreen().Y + (aComp.getBounds().Height / 2);
                    System.out.println("try to click mouse button on x/y " + x + "/" + y);
                    rob.mouseMove(x, y);
                    rob.mousePress(InputEvent.BUTTON1_MASK);
                    rob.mouseRelease(InputEvent.BUTTON1_MASK);
                } catch (java.awt.AWTException e) {
                    System.out.println("couldn't press mouse button");
                }

        }
    }

    public void doubleClickMiddleOfAccessibleObject(short role, String name) {
        XAccessibleContext xAcc =mAT.getAccessibleObjectForRole(mXRoot, role, name);
        XAccessibleComponent aComp = UnoRuntime.queryInterface(
                                             XAccessibleComponent.class, xAcc);

        System.out.println(xAcc.getAccessibleRole() + "," +
                    xAcc.getAccessibleName() + "(" +
                    xAcc.getAccessibleDescription() + "):" +
                    utils.getImplName(xAcc));

        if (aComp != null) {
            aComp.getLocationOnScreen();
            String bounds = "(" + aComp.getBounds().X + "," +
                            aComp.getBounds().Y + ")" + " (" +
                            aComp.getBounds().Width + "," +
                            aComp.getBounds().Height + ")";
            System.out.println("The boundary Rectangle is " + bounds);
                try {
                    Robot rob = new Robot();
                    int x = aComp.getLocationOnScreen().X + (aComp.getBounds().Width / 2);
                    int y = aComp.getLocationOnScreen().Y + (aComp.getBounds().Height / 2);
                    System.out.println("try to double click mouse button on x/y " + x + "/" + y);
                    rob.mouseMove(x, y);
                    rob.mousePress(InputEvent.BUTTON1_MASK);
                    rob.mouseRelease(InputEvent.BUTTON1_MASK);
                    utils.shortWait(100);
                    rob.mousePress(InputEvent.BUTTON1_MASK);
                    rob.mouseRelease(InputEvent.BUTTON1_MASK);
                } catch (java.awt.AWTException e) {
                    System.out.println("couldn't press mouse button");
                }

        }
    }

    /**
     * <B>DEPRECATED</B>
     * Since <CODE>AccessibilityTools</CODE> handle parameter <CODE>debugIsActive</CODE>
     * this function does not work anymore.
     * @deprecated Since <CODE>AccessibilityTools</CODE> handle parameter <CODE>debugIsActive</CODE>
     * this function does not work anymore.
     * @param log logWriter
     */
    public void printAccessibleTree(PrintWriter log)
    {
        mAT.printAccessibleTree(log, mXRoot);
    }


    /**
     * Prints the accessible tree to the <CODE>logWriter</CODE> only if <CODE>debugIsActive</CODE>
     * is set to <CODE>true</CODE>
     * @param log logWriter
     * @param debugIsActive prints only if this parameter is set to TRUE
     */
    public void printAccessibleTree(PrintWriter log, boolean debugIsActive) {
        mAT.printAccessibleTree(log, mXRoot, debugIsActive);
    }

}
