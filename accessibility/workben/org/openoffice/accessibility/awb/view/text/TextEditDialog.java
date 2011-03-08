/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
