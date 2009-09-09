/*************************************************************************
 *
 *  $RCSfile: CaretSpinnerModel.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obr $ $Date: 2008/05/14 13:21:37 $
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

import java.lang.Integer;
import java.util.Vector;
import javax.swing.SpinnerModel;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import com.sun.star.accessibility.XAccessibleText;
import com.sun.star.lang.IndexOutOfBoundsException;


/** A simple model for JSpinner objects that clips the spinner values to valid
    text indices.
*/
public class CaretSpinnerModel
    implements SpinnerModel
{
    public CaretSpinnerModel (XAccessibleText xText)
    {
        mxText = xText;
        maListeners = new Vector ();
    }

    public void addChangeListener (ChangeListener aListener)
    {
        if (aListener != null)
            maListeners.add (aListener);
    }

    public void removeChangeListener (ChangeListener aListener)
    {
        maListeners.removeElement (aListener);
    }

    public Object getNextValue ()
    {
        if (mxText != null)
        {
            int nPosition = mxText.getCaretPosition();
            if (nPosition+1 <= mxText.getCharacterCount())
                return new Integer (nPosition+1);
        }
        return null;
    }

    public Object getPreviousValue ()
    {
        if (mxText != null)
        {
            int nPosition = mxText.getCaretPosition();
            if (nPosition > 0)
                return new Integer (nPosition-1);
        }
        return null;
    }

    public Object getValue ()
    {
        if (mxText != null)
            return new Integer (mxText.getCaretPosition());
        else
            return null;
    }

    public void setValue (Object aValue)
    {
        if (mxText != null)
            if (aValue instanceof Integer)
            {
                try
                {
                    if( ((Integer)aValue).intValue() != mxText.getCaretPosition() )
                      mxText.setCaretPosition (((Integer)aValue).intValue());
                }
                catch (IndexOutOfBoundsException aException)
                {
                }
            }
    }

    /** Call this method when the caret position has changes so that the model
        can inform its listeners about it.
    */
    public void Update ()
    {
        ChangeEvent aEvent = new ChangeEvent (this);
        for (int i=0; i<maListeners.size(); i++)
            ((ChangeListener)maListeners.elementAt(i)).stateChanged (aEvent);
    }

    private XAccessibleText mxText;
    private Integer maValue;
    private Vector maListeners;
}
