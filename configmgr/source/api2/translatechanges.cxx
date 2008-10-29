/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: translatechanges.cxx,v $
 * $Revision: 1.13 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

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
        class NodeRef;
        class NodeID;
    }
// ---------------------------------------------------------------------------------------------------

    namespace configapi
    {
// ---------------------------------------------------------------------------------------------------
    //interpreting NodeChanges
// resolve the relative path from a given base to the changed node
bool resolveChangeLocation(configuration::RelativePath& aPath, configuration::NodeChangeLocation const& aChange, rtl::Reference< configuration::Tree > const& aBaseTree, configuration::NodeRef const& aBaseNode)
{
    OSL_ENSURE(aChange.isValidLocation(), "Trying to resolve against change location that wasn't set up properly");

    namespace Path = configuration::Path;

    rtl::Reference< configuration::Tree > aChangeBaseTree = aChange.getBaseTree();

    configuration::AbsolutePath aOuterBasePath      = aBaseTree->getAbsolutePath(aBaseNode);
    configuration::AbsolutePath aChangeBasePath = aChangeBaseTree->getAbsolutePath(aChange.getBaseNode());

    std::vector<configuration::Path::Component>::const_reverse_iterator aChangeIt = aChangeBasePath.begin(),    aChangeEnd = aChangeBasePath.end();
    std::vector<configuration::Path::Component>::const_reverse_iterator aOuterIt  = aOuterBasePath.begin(), aOuterEnd  = aOuterBasePath.end();

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

        aPath = configuration::RelativePath(aRemaining).compose(aChange.getAccessor());
    }
    else if (aOuterIt == aOuterEnd) // exact match outside
    {
        aPath = aChange.getAccessor();
    }
    else //(aChangeIt == aChangeEnd) but outer left
    {
        configuration::RelativePath aAccessor = aChange.getAccessor();
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

            aPath = configuration::RelativePath( aRemaining );
        }
    }

    return (aOuterIt == aOuterEnd); // resolved completely and assigned ??

}

// ---------------------------------------------------------------------------------------------------
// change path and base settings to start from the given base
bool rebaseChange(configuration::NodeChangeLocation& aChange, rtl::Reference< configuration::Tree > const& _aBaseTreeRef)
{
    return rebaseChange(aChange,_aBaseTreeRef,_aBaseTreeRef->getRootNode());
}
bool rebaseChange(configuration::NodeChangeLocation& aChange, rtl::Reference< configuration::Tree > const& _aBaseTreeRef, configuration::NodeRef const& aBaseNode)
{
    OSL_ENSURE(aChange.isValidLocation(), "Trying to rebase change location that wasn't set up properly");

    rtl::Reference< configuration::Tree > aBaseTree(_aBaseTreeRef);

    configuration::RelativePath aNewPath;
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
bool resolveUnoObjects(UnoChange& aUnoChange, configuration::NodeChangeData const& aChange,
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
        rtl::Reference< configuration::Tree > aTree = aChange.isRemoveSetChange()?
            aChange.getOldElementTree():
            aChange.getNewElementTree();

        configuration::NodeRef aNodeRef = aTree->getRootNode();

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
                rtl::Reference< configuration::Tree > aOldTree = aChange.getOldElementTree();

                aNodeRef = aOldTree->getRootNode();
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
bool resolveToUno(configuration::NodeChangeData& aChange, Factory& rFactory)
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

/// fill a change info from a NodeChangeInfo
void fillChange(util::ElementChange& rChange, configuration::NodeChangeInformation const& aInfo, rtl::Reference< configuration::Tree > const& aBaseTree, Factory& rFactory)
{
    fillChange(rChange,aInfo,aBaseTree,aBaseTree->getRootNode(),rFactory);
}
/// fill a change info from a NodeChangeInfo
void fillChange(util::ElementChange& rChange, configuration::NodeChangeInformation const& aInfo, rtl::Reference< configuration::Tree > const& aBaseTree, configuration::NodeRef const& aBaseNode, Factory& rFactory)
{
    configuration::RelativePath aRelativePath;
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
void fillChangeFromResolved(util::ElementChange& rChange, configuration::NodeChangeInformation const& aInfo)
{
    rChange.Accessor        <<= aInfo.location.getAccessor().toString();
    rChange.Element         = aInfo.change.unoData.newValue;
    rChange.ReplacedElement = aInfo.change.unoData.oldValue;
}
// ---------------------------------------------------------------------------------------------------
/// fill a event from a NodeChangeInfo (uno objects are assumed to be resolved already)
bool fillEventDataFromResolved(container::ContainerEvent& rEvent, configuration::NodeChangeInformation const& aInfo)
{
    rEvent.Accessor         <<= aInfo.location.getAccessor().getLocalName().getName();
    rEvent.Element          = aInfo.change.unoData.newValue;
    rEvent.ReplacedElement  = aInfo.change.unoData.oldValue;

    return !aInfo.isEmptyChange();
}
// ---------------------------------------------------------------------------------------------------
/// fill a event from a NodeChangeInfo(uno objects are assumed to be resolved already)
bool fillEventDataFromResolved(beans::PropertyChangeEvent& rEvent, configuration::NodeChangeInformation const& aInfo, bool bMore)
{
    if (!aInfo.isValueChange())
        return false;

    rEvent.PropertyName     = aInfo.location.getAccessor().getLocalName().getName();

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

