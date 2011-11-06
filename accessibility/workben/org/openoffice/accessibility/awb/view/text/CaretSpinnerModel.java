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
