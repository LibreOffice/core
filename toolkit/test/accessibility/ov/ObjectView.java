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

package ov;

import javax.swing.JPanel;

import com.sun.star.accessibility.XAccessibleContext;

/** This is the base class for all object views that can be placed inside an
    object view container.

    <p>When provided with a new accessible object the container will call
    the Create method to create a new instance when certain conditions are
    met.  It then calls SetObject to pass the object to the instance.
    Finally it calls Update.</p>

    <p>The SetObject and Update methods may be called for a new object
    without calling Create first.  In this way an existing instance is
    recycled.</p>
*/
abstract public class ObjectView
    extends JPanel
{
    public ObjectView (ObjectViewContainer aContainer)
    {
        maContainer = aContainer;
        mxContext = null;
    }

    /** Call this when you want the object to be destroyed.  Release all
        resources when called.
    */
    public void Destroy ()
    {
    }

    /** Tell the view to display information for a new accessible object.
        @param xContext
            The given object may be null.  A typical behaviour in this case
            would be to display a blank area.  But is also possible to show
            information about the last object.
    */
    public void SetObject (XAccessibleContext xContext)
    {
        mxContext = xContext;
        Update ();
    }


    /** This is a request of a repaint with the current state of the current
        object.  The current object may or may not be the same as the one
        when Update() was called the last time.
    */
    public void Update ()
    {
    }


    /** Return a string that is used as a title of an enclosing frame.
    */
    abstract public String GetTitle ();

    /// Reference to the current object to display information about.
    protected XAccessibleContext mxContext;

    protected ObjectViewContainer maContainer;
}
