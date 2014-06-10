package org.apache.openoffice.ooxml.schema.automaton;

import org.apache.openoffice.ooxml.schema.model.complex.Any;

/** Description of optional content that can be skipped when not supported.
 *  Corresponds to the 'any' schema element.
 */
public class SkipData
{
    public SkipData (
        final Any.ProcessContents aProcessContents,
        final String[] aNamespaces)
    {
        maProcessContents = aProcessContents;
        maNamespaces = aNamespaces;
    }




    public SkipData Clone (final State aState)
    {
        return new SkipData(
            maProcessContents,
            maNamespaces);
    }




    final Any.ProcessContents maProcessContents;
    final String[] maNamespaces;
}
