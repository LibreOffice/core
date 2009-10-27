/*************************************************************************
 *
 *  $RCSfile: TextEditDialog.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: af $ $Date: 2003/06/13 16:30:43 $
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
