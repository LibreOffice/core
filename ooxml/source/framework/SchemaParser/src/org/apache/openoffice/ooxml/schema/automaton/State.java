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

package org.apache.openoffice.ooxml.schema.automaton;

import java.util.Vector;

import org.apache.openoffice.ooxml.schema.model.base.INode;
import org.apache.openoffice.ooxml.schema.model.base.QualifiedName;

/** Each complex type is represented by a State object (primary state).
 *  For a validating parser additional states are created for sequences, choices, etc. (secondary states).
 *  Secondary states have the same basename as primary states and suffixes to make their names unique.
 *  Full names of states contain both the basename and the suffix.
 */
public class State
    implements Comparable<State>
{
    /** Create a new state from a basename and an optional suffix.
     *
     *  Don't call this constructor directly.  Use methods in StateContext instead.
     *  They ensure that states are unique per context.
     */
    State (
        final QualifiedName aBasename,
        final String sSuffix)
    {
        maBasename = aBasename;
        msSuffix = sSuffix;
        msFullname = GetStateName(aBasename, msSuffix);
        maTransitions = new Vector<>();
        maEpsilonTransitions = new Vector<>();
        maSkipData = new Vector<>();
        mbIsAccepting = false;
        maTextType = null;
    }




    State Clone (final StateContext aContext)
    {
        return aContext.GetOrCreateState(maBasename, msSuffix);
    }




    static String GetStateName (
        final QualifiedName aBasename,
        final String sSuffix)
    {
        if (sSuffix == null)
            return aBasename.GetStateName();
        else
            return aBasename.GetStateName()+"_"+sSuffix;
    }




    public String GetFullname ()
    {
        return msFullname;
    }




    public QualifiedName GetBasename ()
    {
        return maBasename;
    }




    /** Return a qualified name that contains the suffix.
     *  This is typically only used for sorting type names.
     */
    public QualifiedName GetQualifiedName ()
    {
        return new QualifiedName(
            maBasename.GetNamespacePrefix(),
            maBasename.GetNamespaceURI(),
            msSuffix != null
                ? maBasename.GetLocalPart() + "_" + msSuffix
                : maBasename.GetLocalPart());
    }




    public String GetSuffix ()
    {
        return msSuffix;
    }




    public void AddTransition (final Transition aTransition)
    {
        assert(this == aTransition.GetStartState());
        maTransitions.add(aTransition);
    }




    public Iterable<Transition> GetTransitions()
    {
        return maTransitions;
    }




    public int GetTransitionCount ()
    {
        return maTransitions.size();
    }




    public void AddEpsilonTransition (final EpsilonTransition aTransition)
    {
        assert(this == aTransition.GetStartState());
        maEpsilonTransitions.add(aTransition);
    }




    public Iterable<EpsilonTransition> GetEpsilonTransitions()
    {
        return maEpsilonTransitions;
    }




    public void AddSkipData (final SkipData aSkipData)
    {
        maSkipData.add(aSkipData);
    }




    public Iterable<SkipData> GetSkipData ()
    {
        return maSkipData;
    }




    public void SetIsAccepting ()
    {
        mbIsAccepting = true;
    }




    public boolean IsAccepting ()
    {
        return mbIsAccepting;
    }




    /** The basename is the primary sort key.  The suffix is the secondary key.
     */
    @Override
    public int compareTo (final State aOther)
    {
        int nResult = maBasename.compareTo(aOther.maBasename);
        if (nResult == 0)
        {
            if (msSuffix==null && aOther.msSuffix==null)
                nResult = 0;
            else if (msSuffix!=null && aOther.msSuffix!=null)
                nResult = msSuffix.compareTo(aOther.msSuffix);
            else if (msSuffix==null)
                nResult = -1;
            else
                nResult = +1;
        }
        return nResult;
    }




    public void SetTextType (final INode aTextType)
    {
        assert(maTextType==null);
        maTextType = aTextType;
    }




    public INode GetTextType ()
    {
        return maTextType;
    }




    public void CopyFrom (final State aOther)
    {
        if (aOther.IsAccepting())
            SetIsAccepting();
        for (final SkipData aSkipData : aOther.GetSkipData())
            AddSkipData(aSkipData.Clone(this));
        SetTextType(aOther.GetTextType());
    }




    @Override
    public String toString ()
    {
        return msFullname;
    }




    private final QualifiedName maBasename;
    private final String msSuffix;
    private final String msFullname;
    private final Vector<Transition> maTransitions;
    private final Vector<EpsilonTransition> maEpsilonTransitions;
    private final Vector<SkipData> maSkipData;
    private boolean mbIsAccepting;
    private INode maTextType;
}
