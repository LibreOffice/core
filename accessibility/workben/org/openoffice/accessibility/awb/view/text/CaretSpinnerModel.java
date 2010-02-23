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
