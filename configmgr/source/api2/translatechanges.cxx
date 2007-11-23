/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: translatechanges.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:09:27 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include "translatechanges.hxx"

#ifndef CONFIGMGR_CONFIGNODE_HXX_
#include "noderef.hxx"
#endif
#ifndef CONFIGMGR_CONFIGCHANGE_HXX_
#include "nodechange.hxx"
#endif
#ifndef CONFIGMGR_CONFIGCHANGEINFO_HXX_
#include "nodechangeinfo.hxx"
#endif
#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "apifactory.hxx"
#endif

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
        using configuration::TreeRef;
        using configuration::Name;
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

    namespace Path = configuration::Path;

    typedef Path::Iterator Iter;

    Tree aChangeBaseTree = aChange.getBaseTree();

    AbsolutePath aOuterBasePath     = aBaseTree.getAbsolutePath(aBaseNode);
    AbsolutePath aChangeBasePath    = aChangeBaseTree.getAbsolutePath(aChange.getBaseNode());

    Iter aChangeIt = aChangeBasePath.begin(),   aChangeEnd = aChangeBasePath.end();
    Iter aOuterIt  = aOuterBasePath.begin(),    aOuterEnd  = aOuterBasePath.end();

    // First by resolve the base node pathes
    while (aOuterIt != aOuterEnd && aChangeIt != aChangeEnd)
    {
        if ( ! Path::matches(*aOuterIt,*aChangeIt) ) return false; // mismatch
        ++aOuterIt;
        ++aChangeIt;
    }

    // Next consider the stored accessor
    if (aChangeIt != aChangeEnd) // stepping outward - prepend
    {
        Path::Rep aRemaining(aChangeIt, aChangeEnd);

        aPath = RelativePath(aRemaining).compose(aChange.getAccessor());
    }
    else if (aOuterIt == aOuterEnd) // exact match outside
    {
        aPath = aChange.getAccessor();
    }
    else //(aChangeIt == aChangeEnd) but outer left
    {
        RelativePath aAccessor = aChange.getAccessor();
        aChangeIt   = aAccessor.begin();
        aChangeEnd  = aAccessor.end();

        // resolve the outer path against the change accessor
        while (aOuterIt != aOuterEnd && aChangeIt != aChangeEnd)
        {
            if ( ! Path::matches(*aOuterIt,*aChangeIt) ) return false; // mismatch
            ++aOuterIt;
            ++aChangeIt;
        }

        if (aOuterIt == aOuterEnd)
        {
            Path::Rep aRemaining(aChangeIt, aChangeEnd);

            aPath = RelativePath( aRemaining );
        }
    }

    return (aOuterIt == aOuterEnd); // resolved completely and assigned ??

}

// ---------------------------------------------------------------------------------------------------
// change path and base settings to start from the given base
bool rebaseChange(NodeChangeLocation& aChange, TreeRef const& _aBaseTreeRef)
{
    return rebaseChange(aChange,_aBaseTreeRef,_aBaseTreeRef.getRootNode());
}
bool rebaseChange(NodeChangeLocation& aChange, TreeRef const& _aBaseTreeRef, NodeRef const& aBaseNode)
{
    OSL_ENSURE(aChange.isValidLocation(), "Trying to rebase change location that wasn't set up properly");

    Tree aBaseTree(_aBaseTreeRef);

    RelativePath aNewPath;
    if (resolveChangeLocation(aNewPath,aChange,aBaseTree,aBaseNode))
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
bool resolveUnoObjects(UnoChange& aUnoChange, NodeChangeData const& aChange,
                       Factory& rFactory)
{
    if (aChange.isSetChange())
    {
        //Check we have ElementTree
        if ((aChange.element.newValue == NULL) &&
            (aChange.element.oldValue == NULL))
        {
            if( ( aChange.unoData.newValue.getValue()!=NULL) ||
               ( aChange.unoData.newValue.getValue()!=NULL))
            {
                return true;
            }
            else return false;
        }

        //Check if complex or simple type
        Tree aTree = aChange.isRemoveSetChange()?
            aChange.getOldElementTree():
            aChange.getNewElementTree();

        NodeRef aNodeRef = aTree.getRootNode();

        if (configuration::isStructuralNode(aTree, aNodeRef))
        {
            uno::Reference<uno::XInterface> aNewUnoObject = rFactory.findUnoElement(aChange.getNewElementNodeID());
            uno::Reference<uno::XInterface> aOldUnoObject = rFactory.findUnoElement(aChange.getOldElementNodeID());

            bool bFound = aNewUnoObject.is() || aOldUnoObject.is();
            aUnoChange.newValue <<= aNewUnoObject;
            aUnoChange.oldValue <<= aOldUnoObject;
            return bFound;
        }
        else
        {
            aUnoChange.newValue = configuration::getSimpleElementValue(aTree, aNodeRef);

            if (aChange.isReplaceSetChange() )
            {
                Tree aOldTree = aChange.getOldElementTree();

                aNodeRef = aOldTree.getRootNode();
                OSL_ENSURE(!configuration::isStructuralNode(aOldTree, aNodeRef), "resolveUnoObject types mismatch");
                aUnoChange.oldValue =  configuration::getSimpleElementValue(aOldTree, aNodeRef);
            }
            bool bFound = aUnoChange.newValue.hasValue() || aUnoChange.oldValue.hasValue();
            return bFound;
        }
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
    if (resolveUnoObjects(aUnoChange,aChange, rFactory))
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

    rEvent.Accessor     <<= aInfo.location.getAccessor().getLocalName().getName().toString();
    rEvent.Element          = aUnoChange.newValue;
    rEvent.ReplacedElement  = aUnoChange.oldValue;

    return !aInfo.isEmptyChange();
}
// ---------------------------------------------------------------------------------------------------
/// fill a event from a NodeChangeInfo (uno objects are assumed to be resolved already)
bool fillEventDataFromResolved(container::ContainerEvent& rEvent, NodeChangeInformation const& aInfo)
{
    rEvent.Accessor         <<= aInfo.location.getAccessor().getLocalName().getName().toString();
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

    rEvent.PropertyName     = aInfo.location.getAccessor().getLocalName().getName().toString();

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

    rEvent.PropertyName     = aInfo.location.getAccessor().getLocalName().getName().toString();

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

