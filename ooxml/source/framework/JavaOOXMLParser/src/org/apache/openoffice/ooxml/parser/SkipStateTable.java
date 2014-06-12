package org.apache.openoffice.ooxml.parser;

import java.util.HashSet;
import java.util.Set;

/** Table of all skip states.
 *
 *  A skip state corresponds to the 'any' element in the schemas.
 *  It means that the content of the element is specified by an extension of the
 *  schema which may or may not be known at parse time.
 *  At the moment the whole element is skipped, i.e. ignored.
 *
 */
public class SkipStateTable
{
    public SkipStateTable (final Iterable<String[]> aData)
    {
        maSkipStates = new HashSet<>();

        for (final String[] aLine : aData)
        {
            // Create new transition.
            final int nStateId = Integer.parseInt(aLine[1]);

            maSkipStates.add(nStateId);
        }
    }




    public boolean Contains (final int nStateId)
    {
        return maSkipStates.contains(nStateId);
    }




    public int GetSkipStateCount ()
    {
        return maSkipStates.size();
    }




    private final Set<Integer> maSkipStates;
}
