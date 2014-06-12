package org.apache.openoffice.ooxml.parser;

import javax.xml.stream.Location;

/** Interface for actions that are bound to states and triggered by XML events.
 */
public interface IAction
{
    /** Callback for a single XML event.
     *  @param eTrigger
     *      Equivalent to the XML event type.
     *  @param aContext
     *      The context of the element that was just entered (element start),
     *      is about to be left (element end) or is currently active (all other
     *      events).
     * @param sText
     *      Contains text for ActionTrigger.Text.  Is null for all other
     *      triggers.
     * @param aLocation
     *      The location in the source file that triggered the XML event.
     */
    void Run (
        final ActionTrigger eTrigger,
        final ElementContext aContext,
        final String sText,
        final Location aLocation);
}
