package org.apache.openoffice.ooxml.parser;

import java.util.Iterator;

/** Iterate over two sources of actions, both given as an Iterable<IAction>
 *  object that can be null.
*/
public class ActionIterator implements Iterable<IAction>
{
    public ActionIterator (
        final Iterable<IAction> aOneStateActions,
        final Iterable<IAction> aAllStateActions)
    {
        maOneStateActions = aOneStateActions;
        maAllStateActions = aAllStateActions;
    }




    @Override public Iterator<IAction> iterator()
    {
        return new Iterator<IAction>()
        {
            Iterator<IAction> maIterator = null;
            int mnPhase = 0;

            @Override
            public boolean hasNext()
            {
                while(true)
                {
                    if (mnPhase == 2)
                        return false;
                    else if (mnPhase == 0)
                    {
                        if (maIterator == null)
                            if (maOneStateActions == null)
                            {
                                mnPhase = 1;
                                continue;
                            }
                            else
                                maIterator = maOneStateActions.iterator();
                        if (maIterator.hasNext())
                            return true;
                        else
                        {
                            maIterator = null;
                            mnPhase = 1;
                        }
                    }
                    else if (mnPhase == 1)
                    {
                        if (maIterator == null)
                            if (maAllStateActions == null)
                            {
                                mnPhase = 2;
                                return false;
                            }
                            else
                                maIterator = maAllStateActions.iterator();
                        if (maIterator.hasNext())
                            return true;
                        else
                        {
                            mnPhase = 2;
                        }
                    }
                }
            }




            @Override
            public IAction next()
            {
                return maIterator.next();
            }




            @Override
            public void remove()
            {
            }
        };
    }




    private final Iterable<IAction> maOneStateActions;
    private final Iterable<IAction> maAllStateActions;
}
