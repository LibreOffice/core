/*************************************************************************
 *
 *  $RCSfile: translatechanges.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: jb $ $Date: 2001-02-13 17:15:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "translatechanges.hxx"

#include "noderef.hxx"
#include "nodechange.hxx"
#include "nodechangeinfo.hxx"
#include "apifactory.hxx"

namespace configmgr
{
// ---------------------------------------------------------------------------------------------------
    namespace css = ::com::sun::star;
    namespace uno       = css::uno;
    namespace lang      = css::lang;
    namespace util      = css::util;
    namespace beans     = css::beans;
    namespace container = css::container;
// ---------------------------------------------------------------------------------------------------

    namespace configuration
    {
        class NodeChange;
        class NodeChanges;
        class Tree;
        class NodeRef;
        class NodeID;
    }
// ---------------------------------------------------------------------------------------------------

    namespace configapi
    {
        using configuration::Tree;
        using configuration::Name;
        using configuration::Path;
        using configuration::AbsolutePath;
        using configuration::RelativePath;
        using configuration::NodeRef;
        using configuration::NodeID;
        using configuration::NodeChangeInformation;
        using configuration::NodeChangeData;
        using configuration::NodeChangeLocation;
// ---------------------------------------------------------------------------------------------------
    //interpreting NodeChanges
// resolve the relative path from a given base to the changed node
bool resolveChangeLocation(RelativePath& aPath, NodeChangeLocation const& aChange, Tree const& aBaseTree)
{
    return resolveChangeLocation(aPath,aChange,aBaseTree,aBaseTree.getRootNode());
}
bool resolveChangeLocation(RelativePath& aPath, NodeChangeLocation const& aChange, Tree const& aBaseTree, NodeRef const& aBaseNode)
{
    OSL_ENSURE(aChange.isValidLocation(), "Trying to resolve against change location that wasn't set up properly");

    typedef Path::Iterator Iter;

    Tree aChangeBaseTree = aChange.getBaseTree();

    AbsolutePath aOuterBaseTreePath     = aBaseTree.getContextPath();
    AbsolutePath aChangeBaseTreePath    = aChangeBaseTree.getContextPath();
    RelativePath aOuterBaseNodePath     = aBaseTree.getLocalPath(aBaseNode);
    RelativePath aChangeBaseNodePath    = aChangeBaseTree.getLocalPath(aChange.getBaseNode());

    // First start resolving the context pathes
    Iter aChangeIt = aChangeBaseTreePath.begin(),   aChangeEnd = aChangeBaseTreePath.end();
    Iter aOuterIt  = aOuterBaseTreePath.begin(),    aOuterEnd  = aOuterBaseTreePath.end();

    while (aOuterIt != aOuterEnd && aChangeIt != aChangeEnd)
    {
        if (*aOuterIt != *aChangeIt) return false; // mismatch
        ++aOuterIt;
        ++aChangeIt;
    }

    // Prepend any remaining parts to the respective node pathes
    if (aChangeIt != aChangeEnd)
    {
        aChangeBaseNodePath = RelativePath( Path::Components(aChangeIt,aChangeEnd) ).compose(aChangeBaseNodePath);
    }
    else if (aOuterIt != aOuterEnd)
    {
        aOuterBaseNodePath = RelativePath( Path::Components(aOuterIt,aOuterEnd) ).compose(aOuterBaseNodePath);
    }

    // Continue by resolving the node pathes
    aChangeIt = aChangeBaseNodePath.begin(); aChangeEnd = aChangeBaseNodePath.end();
    aOuterIt  = aOuterBaseNodePath.begin();  aOuterEnd  = aOuterBaseNodePath.end();

    while (aOuterIt != aOuterEnd && aChangeIt != aChangeEnd)
    {
        if (*aOuterIt != *aChangeIt) return false; // mismatch
        ++aOuterIt;
        ++aChangeIt;
    }

    // Next consider the stored accessor
    if (aChangeIt != aChangeEnd) // stepping outward - prepend
    {
        aPath = RelativePath( Path::Components(aChangeIt, aChangeEnd) ).compose(aChange.getAccessor());
    }
    else if (aOuterIt == aOuterEnd) // exact match outside
    {
        aPath = aChange.getAccessor();
    }
    else //(aChangeIt == aChangeEnd) but outer left // shall we support actually going inside the accessor?
    {
        RelativePath aAccessor = aChange.getAccessor();
        aChangeIt   = aAccessor.begin();
        aChangeEnd  = aAccessor.end();

        // resolve the outer path against the change accessor
        while (aOuterIt != aOuterEnd && aChangeIt != aChangeEnd)
        {
            if (*aOuterIt != *aChangeIt) return false; // mismatch
            ++aOuterIt;
            ++aChangeIt;
        }

        if (aOuterIt == aOuterEnd)
        {
            aPath = RelativePath( Path::Components(aChangeIt, aChangeEnd) );
        }
    }

    return (aOuterIt == aOuterEnd); // resolved completely and assigned ??

}

// ---------------------------------------------------------------------------------------------------
// change path and base settings to start from the given base
bool rebaseChange(NodeChangeLocation& aChange, Tree const& aBaseTree)
{
    return rebaseChange(aChange,aBaseTree,aBaseTree.getRootNode());
}
bool rebaseChange(NodeChangeLocation& aChange, Tree const& aBaseTree, NodeRef const& aBaseNode)
{
    OSL_ENSURE(aChange.isValidLocation(), "Trying to rebase change location that wasn't set up properly");

    RelativePath aNewPath;
    if (resolveChangeLocation(aNewPath,aChange,aBaseTree, aBaseNode))
    {
        aChange.setBase( aBaseTree, aBaseNode);
        aChange.setAccessor( aNewPath );
        return true;
    }
    else
        return false;
}
// ---------------------------------------------------------------------------------------------------
// resolve non-uno elements to Uno Objects
bool resolveUnoObjects(UnoChange& aUnoChange, NodeChangeData const& aChange,  Factory& rFactory)
{
    if (aChange.isSetChange())
    {
        uno::Reference<uno::XInterface> aNewUnoObject = rFactory.findUnoElement(aChange.getNewElementNodeID());
        uno::Reference<uno::XInterface> aOldUnoObject = rFactory.findUnoElement(aChange.getOldElementNodeID());

        bool bFound = aNewUnoObject.is() || aOldUnoObject.is();

        aUnoChange.newValue <<= aNewUnoObject;
        aUnoChange.oldValue <<= aOldUnoObject;

        return bFound;
    }
    else if (aChange.isValueChange())
    {
        aUnoChange.newValue = aChange.unoData.newValue;
        aUnoChange.oldValue = aChange.unoData.oldValue;
        return true;
    }
    else
    {
        return false;
    }
}
// ---------------------------------------------------------------------------------------------------
// resolve non-uno elements to Uno Objects inplace
bool resolveToUno(NodeChangeData& aChange, Factory& rFactory)
{
    struct UnoChange aUnoChange;
    if (resolveUnoObjects(aUnoChange,aChange,rFactory))
    {
        aChange.unoData.newValue = aUnoChange.newValue;
        aChange.unoData.oldValue = aUnoChange.oldValue;
        return true;
    }
    else
        return false;
}
// ---------------------------------------------------------------------------------------------------

// building events
/// find the sending api object
void fillEventSource(lang::EventObject& rEvent, Tree const& aTree, NodeRef const& aNode, Factory& rFactory)
{
    rEvent.Source = rFactory.findUnoElement( NodeID(aTree,aNode) );
}
// ---------------------------------------------------------------------------------------------------

/// fill a change info from a NodeChangeInfo
void fillChange(util::ElementChange& rChange, NodeChangeInformation const& aInfo, Tree const& aBaseTree, Factory& rFactory)
{
    fillChange(rChange,aInfo,aBaseTree,aBaseTree.getRootNode(),rFactory);
}
/// fill a change info from a NodeChangeInfo
void fillChange(util::ElementChange& rChange, NodeChangeInformation const& aInfo, Tree const& aBaseTree, NodeRef const& aBaseNode, Factory& rFactory)
{
    RelativePath aRelativePath;
    if (!resolveChangeLocation(aRelativePath, aInfo.location, aBaseTree, aBaseNode))
        OSL_ENSURE(false, "WARNING: Change is not part of the given Tree");

    UnoChange aUnoChange;
    if (!resolveUnoObjects(aUnoChange, aInfo.change, rFactory))
        OSL_ENSURE(false, "WARNING: Cannot find out old/new UNO objects involved in change");

    rChange.Accessor        <<= aRelativePath.toString();
    rChange.Element         = aUnoChange.newValue;
    rChange.ReplacedElement = aUnoChange.oldValue;
}
// ---------------------------------------------------------------------------------------------------
/// fill a change info from a NodeChangeInfo (base,path and uno objects are assumed to be resolved already)
void fillChangeFromResolved(util::ElementChange& rChange, NodeChangeInformation const& aInfo)
{
    rChange.Accessor        <<= aInfo.location.getAccessor().toString();
    rChange.Element         = aInfo.change.unoData.newValue;
    rChange.ReplacedElement = aInfo.change.unoData.oldValue;
}
// ---------------------------------------------------------------------------------------------------

/// fill a event from a NodeChangeInfo
bool fillEventData(container::ContainerEvent& rEvent, NodeChangeInformation const& aInfo, Factory& rFactory)
{
    UnoChange aUnoChange;
    if (!resolveUnoObjects(aUnoChange, aInfo.change, rFactory))
    {
        OSL_ENSURE(false, "WARNING: Cannot find out old/new UNO objects involved in change");
        return false;
    }

    rEvent.Accessor     <<= aInfo.location.getAccessor().getLocalName().toString();
    rEvent.Element          = aUnoChange.newValue;
    rEvent.ReplacedElement  = aUnoChange.oldValue;

    return !aInfo.isEmptyChange();
}
// ---------------------------------------------------------------------------------------------------
/// fill a event from a NodeChangeInfo (uno objects are assumed to be resolved already)
bool fillEventDataFromResolved(container::ContainerEvent& rEvent, NodeChangeInformation const& aInfo)
{
    rEvent.Accessor         <<= aInfo.location.getAccessor().getLocalName().toString();
    rEvent.Element          = aInfo.change.unoData.newValue;
    rEvent.ReplacedElement  = aInfo.change.unoData.oldValue;

    return !aInfo.isEmptyChange();
}
// ---------------------------------------------------------------------------------------------------
/// fill a event from a NodeChangeInfo(uno objects are assumed to be resolved already)
bool fillEventData(beans::PropertyChangeEvent& rEvent, NodeChangeInformation const& aInfo, Factory& rFactory, bool bMore)
{
    if (!aInfo.isValueChange())
        return false;

     UnoChange aUnoChange;
    if (!resolveUnoObjects(aUnoChange, aInfo.change, rFactory))
        OSL_ENSURE(false, "WARNING: Cannot find out old/new UNO objects involved in change");

    rEvent.PropertyName     = aInfo.location.getAccessor().getLocalName().toString();

    rEvent.NewValue         = aUnoChange.newValue;
    rEvent.OldValue         = aUnoChange.oldValue;

    rEvent.PropertyHandle   = -1;
     rEvent.Further         = bMore;

    return !aInfo.isEmptyChange();
}
// ---------------------------------------------------------------------------------------------------
/// fill a event from a NodeChangeInfo(uno objects are assumed to be resolved already)
bool fillEventDataFromResolved(beans::PropertyChangeEvent& rEvent, NodeChangeInformation const& aInfo, bool bMore)
{
    if (!aInfo.isValueChange())
        return false;

    rEvent.PropertyName     = aInfo.location.getAccessor().getLocalName().toString();

    rEvent.NewValue         = aInfo.change.unoData.newValue;
    rEvent.OldValue         = aInfo.change.unoData.oldValue;

    rEvent.PropertyHandle   = -1;
     rEvent.Further         = bMore;

    return !aInfo.isEmptyChange();
}
// ---------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------
    }
// ---------------------------------------------------------------------------------------------------
}

