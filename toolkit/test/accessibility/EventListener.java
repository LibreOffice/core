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

import com.sun.star.accessibility.*;

/** Objects of this class (usually one, singleton?) listen to accessible
    events of all objects in all trees.
*/
public class EventListener
{
    public boolean mbVerbose = false;

    public EventListener (AccessibilityTreeModel aTreeModel)
    {
        maTreeModel = aTreeModel;
    }


    private static String objectToString(Object aObject)
    {
        if (aObject == null)
            return null;
        else
            return aObject.toString();
    }



    /** This method handles accessibility objects that are being disposed.
     */
    public void disposing (XAccessibleContext xContext)
    {
        if (mbVerbose)
            System.out.println("disposing " + xContext);
        maTreeModel.removeNode (xContext);
    }

    /** This method is called from accessible objects that broadcast
        modifications of themselves or from their children.  The event is
        processed only, except printing some messages, if the tree is not
        locked.  It should be locked during changes to its internal
        structure like expanding nodes.
    */
    public void notifyEvent (AccessibleEventObject aEvent)
    {
        EventHandler aHandler;

        switch (aEvent.EventId)
        {
            case AccessibleEventId.CHILD:
                aHandler = new ChildEventHandler (aEvent, maTreeModel);
                break;

            case AccessibleEventId.BOUNDRECT_CHANGED:
            case AccessibleEventId.VISIBLE_DATA_CHANGED:
                aHandler = new GeometryEventHandler (aEvent, maTreeModel);
                break;


            case AccessibleEventId.NAME_CHANGED:
            case AccessibleEventId.DESCRIPTION_CHANGED:
            case AccessibleEventId.STATE_CHANGED:
            case AccessibleEventId.SELECTION_CHANGED:
                aHandler = new ContextEventHandler (aEvent, maTreeModel);
                break;

            case AccessibleEventId.TABLE_MODEL_CHANGED:
            case AccessibleEventId.TABLE_CAPTION_CHANGED:
            case AccessibleEventId.TABLE_COLUMN_DESCRIPTION_CHANGED:
            case AccessibleEventId.TABLE_COLUMN_HEADER_CHANGED:
            case AccessibleEventId.TABLE_ROW_DESCRIPTION_CHANGED:
            case AccessibleEventId.TABLE_ROW_HEADER_CHANGED:
            case AccessibleEventId.TABLE_SUMMARY_CHANGED:
                aHandler = new TableEventHandler (aEvent, maTreeModel);
                break;

            case AccessibleEventId.ACTION_CHANGED:
                aHandler = new EventHandler (aEvent, maTreeModel);
                break;

            case AccessibleEventId.HYPERTEXT_CHANGED:
                aHandler = new EventHandler (aEvent, maTreeModel);
                break;

            case AccessibleEventId.ACTIVE_DESCENDANT_CHANGED:
            case AccessibleEventId.CARET_CHANGED:
            case AccessibleEventId.TEXT_CHANGED:
            case AccessibleEventId.VALUE_CHANGED:
                aHandler = new EventHandler (aEvent, maTreeModel);
                break;

            default:
                aHandler = null;
                break;
        }

        if (aHandler == null)
            System.out.println ("    unhandled event");
        else
        {
            if (mbVerbose)
                aHandler.Print (System.out);
            aHandler.Process ();
        }
    }


    private AccessibilityTreeModel maTreeModel;
}
