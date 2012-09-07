/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

import java.util.Vector;

import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.accessibility.*;

/** Handle all the events send from accessibility objects.  The events
    denoting new or removed top windows are handled as well.

    It does not implement any listener interface as does the
    EventListenerProxy class because it is interested only in a sub set of
    the event types.
*/
public class EventHandler
{
    public EventHandler ()
    {
        mnTopWindowCount = 0;
        maListenerProxy = new EventListenerProxy (this);
        maConnectionTask = new ConnectionTask (maListenerProxy);
        maObjectDisplays = new Vector<IAccessibleObjectDisplay> ();
    }

    public synchronized void addObjectDisplay (IAccessibleObjectDisplay aDisplay)
    {
        maObjectDisplays.add (aDisplay);
    }


    public void finalize ()
    {
        // When it is running then cancel the timer that tries to connect to
        // the Office.
        if (maConnectionTask != null)
            maConnectionTask.cancel();
    }



    public void disposing (com.sun.star.lang.EventObject aEvent)
    {
        // Ignored: We are not holding references to accessibility objects.
    }




    /**  This method is called back when a new top level window has been opened.
    */
    public void windowOpened (XAccessible xAccessible)
    {
        if (xAccessible != null)
        {
            // Update the counter of currently open top level windows
            // observed by this object.
            mnTopWindowCount += 1;

            XAccessibleContext xContext = xAccessible.getAccessibleContext();
            if (xContext != null)
            {
                MessageArea.println ("new top level window has accessible name "
                    + xContext.getAccessibleName());

                // Register at all accessible objects of the new window.
                new RegistrationThread (
                    maListenerProxy,
                    xContext,
                    true,
                    true);
            }
            else
                MessageArea.println ("new top level window is not accessible.");
        }
        else
            MessageArea.println ("new top level window is not accessible.");
    }




    public void windowClosed (XAccessible xAccessible)
    {
        mnTopWindowCount -= 1;
        MessageArea.println ("window closed, " + mnTopWindowCount + " still open");
        if (mnTopWindowCount == 0)
        {
            // This was the last window.  Wait for a new connection.
            MessageArea.println ("lost connection to office");
            new ConnectionTask (maListenerProxy);
        }
        if (xAccessible != null)
            new RegistrationThread (
                maListenerProxy,
                xAccessible.getAccessibleContext(),
                false,
                true);
    }




    /** Print a message that the given object just received the focus.  Call
        all accessible object diplays and tell them to update.
    */
    private synchronized void focusGained (XAccessibleContext xContext)
    {
        if (xContext != null)
        {
            MessageArea.println ("focusGained: " + xContext.getAccessibleName()
                + " with role "
                + NameProvider.getRoleName (xContext.getAccessibleRole()));

            // Tell the object displays to update their views.
            for (int i=0; i<maObjectDisplays.size(); i++)
            {
                IAccessibleObjectDisplay aDisplay =
                    maObjectDisplays.get(i);
                if (aDisplay != null)
                    aDisplay.setAccessibleObject (xContext);
            }

            // Remember the currently focused object.
            mxFocusedObject = xContext;
        }
        else
            MessageArea.println ("focusGained: null");
    }




    /** Print a message that the given object just lost the focus.  Call
        all accessible object diplays and tell them to update.
    */
    private synchronized void focusLost (XAccessibleContext xContext)
    {
        if (xContext != null)
        {
            MessageArea.println ("focusLost: "
                + xContext.getAccessibleName()
                + " with role "
                + NameProvider.getRoleName (xContext.getAccessibleRole()));

            // Tell the object displays to update their views.
            for (int i=0; i<maObjectDisplays.size(); i++)
            {
                IAccessibleObjectDisplay aDisplay =
                    maObjectDisplays.get(i);
                if (aDisplay != null)
                    aDisplay.setAccessibleObject (null);
            }
            mxFocusedObject = null;
        }
        else
            MessageArea.println ("focusLost: null");
    }




    /** Handle a change of the caret position.  Ignore this on all objects
        but the one currently focused.
    */
    private void handleCaretEvent (XAccessibleContext xContext,
        long nOldPosition, long nNewPosition)
    {
        if (xContext == mxFocusedObject)
            MessageArea.println ("caret moved from " + nOldPosition + " to " + nNewPosition);
    }




    /** Print a message that a state has been changed.
        @param xContext
            The accessible context of the object whose state has changed.
        @param nOldState
            When not zero then this value describes a state that has been reset.
        @param nNewValue
            When not zero then this value describes a state that has been set.
    */
    private void handleStateChange (XAccessibleContext xContext, short nOldState, short nNewState)
    {
        // Determine which state has changed and what is its new value.
        short nState;
        boolean aNewValue;
        if (nOldState >= 0)
        {
            nState = nOldState;
            aNewValue = false;
        }
        else
        {
            nState = nNewState;
            aNewValue = true;
        }

        // Print a message about the changed state.
        MessageArea.print ("setting state " + NameProvider.getStateName(nState)
            + " to " + aNewValue);
        if (xContext != null)
        {
            MessageArea.println (" at " + xContext.getAccessibleName() + " with role "
                + NameProvider.getRoleName(xContext.getAccessibleRole()));
        }
        else
            MessageArea.println (" at null");

        // Further handling of some states
        switch (nState)
        {
            case AccessibleStateType.FOCUSED:
                if (aNewValue)
                    focusGained (xContext);
                else
                    focusLost (xContext);
        }
    }




    /** Handle a child event that describes the creation of removal of a
        single child.
    */
    private void handleChildEvent (
        XAccessibleContext aOldChild,
        XAccessibleContext aNewChild)
    {
        if (aOldChild != null)
            // Remove event listener from the child and all of its descendants.
            new RegistrationThread (maListenerProxy, aOldChild, false, false);
        else if (aNewChild != null)
            // Add event listener to the new child and all of its descendants.
            new RegistrationThread (maListenerProxy, aNewChild, true, false);
    }




    /** Handle the change of some visible data of an object.
    */
    private void handleVisibleDataEvent (XAccessibleContext xContext)
    {
        // The given object may affect the visible appearance of the focused
        // object even when the two are not identical when the given object
        // is an ancestor of the focused object.
        // In order to not check this we simply call an update on the
        // focused object.
        if (mxFocusedObject != null)
            for (int i=0; i<maObjectDisplays.size(); i++)
            {
                IAccessibleObjectDisplay aDisplay =
                    maObjectDisplays.get(i);
                if (aDisplay != null)
                    aDisplay.updateAccessibleObject (mxFocusedObject);
            }
    }




    /** Print some information about an event that is not handled by any
        more specialized handler.
    */
    private void handleGenericEvent (
        int nEventId,
        Object aSource,
        Object aOldValue,
        Object aNewValue)
    {
        // Print event to message area.
        MessageArea.print ("received event "
            + NameProvider.getEventName (nEventId) + " from ");
        XAccessibleContext xContext = objectToContext (aSource);
        if (xContext != null)
            MessageArea.print (xContext.getAccessibleName());
        else
            MessageArea.print ("null");
        MessageArea.println (" / "
            + NameProvider.getRoleName(xContext.getAccessibleRole()));
    }



    /** This is the main method for handling accessibility events.  It is
        assumed that it is not called directly from the Office but from a
        listener proxy that runs in a separate thread so that calls back to
        the Office do not result in dead-locks.
    */
    public void notifyEvent (com.sun.star.accessibility.AccessibleEventObject aEvent)
    {
        try // Guard against disposed objects.
        {
            switch (aEvent.EventId)
            {
                case AccessibleEventId.CHILD:
                    handleChildEvent (
                        objectToContext (aEvent.OldValue),
                        objectToContext (aEvent.NewValue));
                    break;

                case AccessibleEventId.STATE_CHANGED:
                {
                    short nOldState = -1;
                    short nNewState = -1;
                    try
                    {
                        if (AnyConverter.isShort (aEvent.NewValue))
                            nNewState = AnyConverter.toShort (aEvent.NewValue);
                        if (AnyConverter.isShort (aEvent.OldValue))
                            nOldState = AnyConverter.toShort (aEvent.OldValue);
                    }
                    catch (com.sun.star.lang.IllegalArgumentException e)
                    {}
                    handleStateChange (
                        objectToContext (aEvent.Source),
                        nOldState,
                        nNewState);
                }
                break;

                case AccessibleEventId.VISIBLE_DATA_CHANGED:
                case AccessibleEventId.BOUNDRECT_CHANGED:
                    handleVisibleDataEvent (objectToContext (aEvent.Source));
                    break;

                case AccessibleEventId.CARET_CHANGED:
                    try
                    {
                        handleCaretEvent (
                            objectToContext (aEvent.Source),
                            AnyConverter.toLong(aEvent.OldValue),
                            AnyConverter.toLong(aEvent.NewValue));
                    }
                    catch (com.sun.star.lang.IllegalArgumentException e)
                    {}
                    break;

                default:
                    handleGenericEvent (aEvent.EventId,
                        aEvent.Source, aEvent.OldValue, aEvent.NewValue);
                    break;
            }
        }
        catch (com.sun.star.lang.DisposedException e)
        {}
    }




    /** Convert the given object into an accessible context.  The object is
        interpreted as UNO Any and may contain either an XAccessible or
        XAccessibleContext reference.
        @return
            The returned value is null when the given object can not be
            converted to an XAccessibleContext reference.
    */
    private XAccessibleContext objectToContext (Object aObject)
    {
        XAccessibleContext xContext = null;
        XAccessible xAccessible = null;
        try
        {
            xAccessible = (XAccessible)AnyConverter.toObject(
                new Type(XAccessible.class), aObject);
        }
        catch (com.sun.star.lang.IllegalArgumentException e)
        {}
        if (xAccessible != null)
            xContext = xAccessible.getAccessibleContext();
        else
            try
            {
                xContext = (XAccessibleContext)AnyConverter.toObject(
                    new Type(XAccessibleContext.class), aObject);
            }
            catch (com.sun.star.lang.IllegalArgumentException e)
            {}
        return xContext;
    }




    /** The proxy that runs in a seperate thread and allows to call back to
        the Office without running into dead-locks.
    */
    private EventListenerProxy maListenerProxy;

    /** The currently focused object.  A value of null means that no object
        has the focus.
    */
    private XAccessibleContext mxFocusedObject;

    /** Keep track of the currently open top windows to start a registration
        loop when the last window (and the Office) is closed.
    */
    private long mnTopWindowCount;

    /** A list of objects that can display accessible objects in specific
        ways such as showing a graphical representation or some textual
        descriptions.
    */
    private Vector<IAccessibleObjectDisplay> maObjectDisplays;

    /** The timer task that attempts in regular intervals to connect to a
        running Office application.
    */
    private ConnectionTask maConnectionTask;
}
