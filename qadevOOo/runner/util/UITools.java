/*************************************************************************
 *
 *  $RCSfile: UITools.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-10-06 12:42:25 $
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

package util;

import com.sun.star.awt.XWindow;
import com.sun.star.uno.UnoRuntime;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.frame.XModel;
import com.sun.star.uno.XInterface;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleAction;
import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleEditableText;
import com.sun.star.accessibility.XAccessibleSelection;
import com.sun.star.accessibility.XAccessibleText;
import com.sun.star.accessibility.XAccessibleValue;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import java.io.PrintWriter;
import java.util.Vector;
import util.AccessibilityTools;

import util.DesktopTools;

public class UITools {

    private static final AccessibilityTools mAT = new AccessibilityTools();
    private final XAccessible mXRoot;
    private final XMultiServiceFactory mMSF;

    public UITools(XMultiServiceFactory msf, XModel xModel)
    {
        mMSF = msf;
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
        XAccessibleText oText = (XAccessibleText)
                         UnoRuntime.queryInterface(XAccessibleText.class, xInt);
        return oText.getText();
    }

    private static void setString(XInterface xInt, String cText)
    {
        XAccessibleEditableText oText = (XAccessibleEditableText)
                 UnoRuntime.queryInterface(XAccessibleEditableText.class, xInt);

        oText.setText(cText);
    }

    private static Object getValue(XInterface xInt)
    {
         XAccessibleValue oValue = (XAccessibleValue)
                        UnoRuntime.queryInterface(XAccessibleValue.class, xInt);
         return oValue.getCurrentValue();
    }


    private static XAccessible makeRoot(XWindow xWindow)
    {
        return mAT.getAccessibleObject(xWindow);
    }

    public XAccessible getRoot()
    {
        return mXRoot;
    }

    /**
     *Helper mathod: set a text into AccessibleEdit field
     */
    public void setTextEditFiledText(String textfiledName, String stringToSet)
                        throws java.lang.Exception
    {
        XInterface oTextField = mAT.getAccessibleObjectForRole(mXRoot,
                                            AccessibleRole.TEXT, textfiledName);
        setString(oTextField, stringToSet);
    }

    /**
     * Helper method: gets button via accessibility and 'click' it</code>
     */

     public void clickButton(String buttonName) throws java.lang.Exception
     {
//      mAT.printAccessibleTree(new PrintWriter(System.out), mXRoot);

        XAccessibleContext oButton =mAT.getAccessibleObjectForRole
                                (mXRoot, AccessibleRole.PUSH_BUTTON, buttonName);
        if (oButton == null){
            throw new Exception("Could not get button '" + buttonName + "'");
        }
        XAccessibleAction oAction = (XAccessibleAction)
                    UnoRuntime.queryInterface(XAccessibleAction.class, oButton);

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
                XAccessibleAction oAction = (XAccessibleAction)
                            UnoRuntime.queryInterface(XAccessibleAction.class, oButton);
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
      *returns the value of named radio button
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

     public XInterface getGraphic(String GraphicName) throws java.lang.Exception
     {
        return mAT.getAccessibleObjectForRole(mXRoot, AccessibleRole.GRAPHIC,
                                             GraphicName);
     }


     public void setRadioButtonValue(String buttonName, int iValue)
            throws java.lang.Exception
     {
        try {
            XInterface xRB =mAT.getAccessibleObjectForRole(mXRoot, AccessibleRole.RADIO_BUTTON, buttonName);
            if(xRB == null)
                System.out.println("AccessibleObjectForRole couldn't be found for " + buttonName);
            XAccessibleValue oValue = (XAccessibleValue)
                         UnoRuntime.queryInterface(XAccessibleValue.class, xRB);
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
      *select an item in nanmed listbox
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
            XAccessible xListBoxAccess = (XAccessible)
                         UnoRuntime.queryInterface(XAccessible.class, xListBox);
            XAccessibleContext xList =mAT.getAccessibleObjectForRole(
                                          xListBoxAccess, AccessibleRole.LIST);
            XAccessibleSelection xListSelect = (XAccessibleSelection)
                   UnoRuntime.queryInterface(XAccessibleSelection.class, xList);

            xListSelect.selectAccessibleChild(nChildIndex);

        } catch (Exception e) {
          throw new Exception("Could not select item '" +nChildIndex+
                        "' in listbox '" + ListBoxName + "' : " + e.toString());
        }
     }

    /**
     * Helper method: returns the entries of a List-Box
     */

     public String[] getListBoxItems(String ListBoxName)
            throws java.lang.Exception
     {
        Vector Items = new Vector();
         try {
            XAccessibleContext xListBox = null;

            xListBox =mAT.getAccessibleObjectForRole(mXRoot,
                                         AccessibleRole.COMBO_BOX, ListBoxName);
            if (xListBox == null){
                xListBox =mAT.getAccessibleObjectForRole(mXRoot,
                                             AccessibleRole.PANEL, ListBoxName);
            }
            XAccessible xListBoxAccess = (XAccessible)
                         UnoRuntime.queryInterface(XAccessible.class, xListBox);
            XAccessibleContext xList =mAT.getAccessibleObjectForRole(
                                          xListBoxAccess, AccessibleRole.LIST);
            for (int i=0;i<xList.getAccessibleChildCount();i++) {
                try {
                    XAccessible xChild = xList.getAccessibleChild(i);
                    XAccessibleContext xChildCont =
                                                  xChild.getAccessibleContext();
                    XInterface xChildInterface = (XInterface)
                        UnoRuntime.queryInterface(XInterface.class, xChildCont);
                    Items.add(getString(xChildInterface));

                } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                      throw new Exception("Could not get child form list of '"
                                         + ListBoxName + "' : " + e.toString());
                }
            }

         } catch (Exception e) {
          throw new Exception("Could not get item list from '"
                                         + ListBoxName + "' : " + e.toString());
        }
        String[]ret = new String[Items.size()];
        return (String[])Items.toArray(ret);
     }
     /**
      *set some Text into an Text Field
      */
     public void setNumericFieldValue(String NumericFieldName, String cValue)
        throws java.lang.Exception
     {
         try{
            XInterface xNumericField =mAT.getAccessibleObjectForRole(
                                  mXRoot, AccessibleRole.TEXT, NumericFieldName);
            //util.dbg.printInterfaces(xNumericField);
            XAccessibleEditableText oValue = (XAccessibleEditableText)
                                         UnoRuntime.queryInterface(
                                         XAccessibleEditableText.class, xNumericField);

            setString(xNumericField, cValue);
         } catch (Exception e) {
          throw new Exception("Could not set value '" + cValue +
            "' into NumericField '" + NumericFieldName + "' : " + e.toString());
        }
     }

     /**
      *returns the value of a numeric field
      */
     public String getNumericFieldValue(String NumericFieldName)
        throws java.lang.Exception
     {
         try{
            XInterface xNumericField =mAT.getAccessibleObjectForRole(
                                  mXRoot, AccessibleRole.TEXT, NumericFieldName);
            return (String) getString(xNumericField);

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
            boolean startFound = false;
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

            currencyVal = currencyVal.replaceAll(",", "");
            currencyVal = currencyVal.replaceAll("\\.", "");
            currencyVal = currencyVal.replace('#', '.');

            return currencyVal;
         } catch (Exception e) {
          throw new Exception("Could get remove characters from currency string '"
                                    + stringVal + "' : " + e.toString());
        }

     }

     /**
      *returns the numeric value of a numeric filed. This is needed ie. for
      *fileds include the moneytary unit.
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
      * returns the content of an TextBox
      */
     public String getTextBoxText(String TextFieldName)
        throws java.lang.Exception
     {
        String TextFieldText = null;
        try{
            XAccessibleContext xTextField =mAT.getAccessibleObjectForRole(mXRoot,
                                     AccessibleRole.SCROLL_PANE, TextFieldName);
            XAccessible xTextFieldAccess = (XAccessible)
                       UnoRuntime.queryInterface(XAccessible.class, xTextField);
            XAccessibleContext xFrame =mAT.getAccessibleObjectForRole(
                                   xTextFieldAccess, AccessibleRole.TEXT_FRAME);
            for (int i=0;i<xFrame.getAccessibleChildCount();i++) {
                try {
                    XAccessible xChild = xFrame.getAccessibleChild(i);
                    XAccessibleContext xChildCont =
                                                  xChild.getAccessibleContext();
                    XInterface xChildInterface = (XInterface)
                        UnoRuntime.queryInterface(XInterface.class, xChildCont);
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
      * set the value to the named check box
      */
     public void setCheckBoxValue(String CheckBoxName, Integer Value)
            throws java.lang.Exception
     {
         try {
            XInterface xCheckBox =mAT.getAccessibleObjectForRole(mXRoot,
                                     AccessibleRole.CHECK_BOX, CheckBoxName);
            XAccessibleValue xCheckBoxValue = (XAccessibleValue)
                   UnoRuntime.queryInterface(XAccessibleValue.class, xCheckBox);
            xCheckBoxValue.setCurrentValue(Value);

         } catch (Exception e) {
            throw new Exception("Could not set value to CheckBox '"
                                       + CheckBoxName + "' : " + e.toString());
        }
     }

   /**
    * returns the value of the named check box
    */
    public Integer getCheckBoxValue(String CheckBoxName)
            throws java.lang.Exception
     {
         try {
            XInterface xCheckBox =mAT.getAccessibleObjectForRole(mXRoot,
                                     AccessibleRole.CHECK_BOX, CheckBoxName);
            XAccessibleValue xCheckBoxValue = (XAccessibleValue)
                   UnoRuntime.queryInterface(XAccessibleValue.class, xCheckBox);

            return (Integer) xCheckBoxValue.getCurrentValue();
         } catch (Exception e) {
            throw new Exception("Could not set value to CheckBox '"
                                       + CheckBoxName + "' : " + e.toString());
        }
     }
    public XWindow getActiveTopWindow() throws java.lang.Exception
    {
        XInterface xToolKit = null;
        try {
            xToolKit = (XInterface) mMSF.createInstance("com.sun.star.awt.Toolkit") ;
        } catch (com.sun.star.uno.Exception e) {
          throw new Exception("Could not toolkit: " + e.toString());
        }
        XExtendedToolkit tk = (XExtendedToolkit)
            UnoRuntime.queryInterface(XExtendedToolkit.class, xToolKit);
        Object atw = tk.getActiveTopWindow();
        return (XWindow) UnoRuntime.queryInterface(XWindow.class, atw);
    }

    public void printAccessibleTree(PrintWriter log)
    {
        mAT.printAccessibleTree(log, mXRoot);
    }
}
