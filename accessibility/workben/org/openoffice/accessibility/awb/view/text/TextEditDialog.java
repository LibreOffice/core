/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



package org.openoffice.accessibility.awb.view.text;

import javax.swing.text.JTextComponent;

import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleEditableText;
import com.sun.star.lang.IndexOutOfBoundsException;
import com.sun.star.uno.UnoRuntime;


class TextEditDialog
    extends TextActionDialog
{
    public TextEditDialog (
        XAccessibleContext xContext,
        String sExplanation,
        String sTitle )
    {
        super (xContext, sExplanation, sTitle);
    }

    protected void Layout()
    {
        super.Layout();
        maText.setEditable (true);
    }


    /** edit the text */
    boolean EditableTextAction (XAccessibleEditableText xText)
    {
        return UpdateText (xText, maText.getText());
    }


    /** update the text */
    boolean UpdateText (XAccessibleEditableText xText, String sNew)
    {
        boolean bResult = false;

        String sOld = xText.getText();

        // false alarm? Early out if no change was done!
        if ( ! sOld.equals (sNew))
        {

            // Get the minimum length of both strings.
            int nMinLength = sOld.length();
            if (sNew.length() < nMinLength)
                nMinLength = sNew.length();

            // Count equal characters from front and end.
            int nFront = 0;
            while ((nFront < nMinLength) &&
                (sNew.charAt(nFront) == sOld.charAt(nFront)))
                nFront++;
            int nBack = 0;
            while ((nBack < nMinLength) &&
                (sNew.charAt(sNew.length()-nBack-1) ==
                 sOld.charAt(sOld.length()-nBack-1)    ))
                nBack++;
            if (nFront + nBack > nMinLength)
                nBack = nMinLength - nFront;

            // so... the first nFront and the last nBack characters are the
            // same. Change the others!
            String sDel = sOld.substring (nFront, sOld.length() - nBack);
            String sIns = sNew.substring (nFront, sNew.length() - nBack);

            System.out.println ("edit text: " +
                sOld.substring(0, nFront) +
                " [ " + sDel + " -> " + sIns + " ] " +
                sOld.substring(sOld.length() - nBack));

            try
            {
                // edit the text, and use
                // (set|insert|delete|replace)Text as needed
                if( nFront+nBack == 0 )
                    bResult = xText.setText( sIns );
                else if( sDel.length() == 0 )
                    bResult = xText.insertText( sIns, nFront );
                else if( sIns.length() == 0 )
                    bResult = xText.deleteText( nFront, sOld.length()-nBack );
                else
                    bResult = xText.replaceText(nFront, sOld.length()-nBack,sIns);
            }
            catch( IndexOutOfBoundsException aException)
            {
            }
        }

        return bResult;
    }
}
