/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PresenterConfigurationAccess.cxx,v $
 *
 * $Revision: 1.5 $
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
#include "precompiled_sdext.hxx"

#include "PresenterConfigurationAccess.hxx"

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

#define A2S(pString) (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(pString)))

namespace sdext { namespace presenter {

const ::rtl::OUString PresenterConfigurationAccess::msPresenterScreenRootName =
    A2S("/org.openoffice.Office.extension.PresenterScreen/");

PresenterConfigurationAccess::PresenterConfigurationAccess (
    const Reference<XComponentContext>& rxContext,
    const OUString& rsRootName,
    WriteMode eMode)
    : mxRoot(),
      maNode()
{
    try
    {
        Reference<lang::XMultiComponentFactory> xFactory (rxContext->getServiceManager());
        if (xFactory.is())
        {
            Sequence<Any> aCreationArguments(3);
            aCreationArguments[0] = makeAny(beans::PropertyValue(
                A2S("nodepath"),
                0,
                makeAny(rsRootName),
                beans::PropertyState_DIRECT_VALUE));
            aCreationArguments[1] = makeAny(beans::PropertyValue(
                A2S("depth"),
                0,
                makeAny((sal_Int32)-1),
                beans::PropertyState_DIRECT_VALUE));
            aCreationArguments[2] = makeAny(beans::PropertyValue(
                A2S("lazywrite"),
                0,
                makeAny(true),
                beans::PropertyState_DIRECT_VALUE));

            OUString sAccessService;
            if (eMode == READ_ONLY)
                sAccessService = A2S("com.sun.star.configuration.ConfigurationAccess");
            else
                sAccessService = A2S("com.sun.star.configuration.ConfigurationUpdateAccess");

            Reference<lang::XMultiServiceFactory> xProvider (
                xFactory->createInstanceWithContext(
                    A2S("com.sun.star.configuration.ConfigurationProvider"),
                    rxContext),
                UNO_QUERY_THROW);
            mxRoot = xProvider->createInstanceWithArguments(
                sAccessService, aCreationArguments);
            maNode <<= mxRoot;
        }
    }
    catch (Exception& rException)
    {
        OSL_TRACE ("caught exception while opening configuration: %s",
            ::rtl::OUStringToOString(rException.Message,
                RTL_TEXTENCODING_UTF8).getStr());
    }
}




PresenterConfigurationAccess::~PresenterConfigurationAccess (void)
{
}




bool PresenterConfigurationAccess::IsValid (void) const
{
    return mxRoot.is();
}




Any PresenterConfigurationAccess::GetConfigurationNode (const OUString& sPathToNode)
{
    return GetConfigurationNode(
        Reference<container::XHierarchicalNameAccess>(mxRoot, UNO_QUERY),
        sPathToNode);
}




Reference<beans::XPropertySet> PresenterConfigurationAccess::GetNodeProperties (
    const OUString& sPathToNode)
{
    return GetNodeProperties(
        Reference<container::XHierarchicalNameAccess>(mxRoot, UNO_QUERY),
        sPathToNode);
}




bool PresenterConfigurationAccess::GoToChild (const ::rtl::OUString& rsPathToNode)
{
    if ( ! IsValid())
        return false;

    Reference<container::XHierarchicalNameAccess> xNode (maNode, UNO_QUERY);
    if (xNode.is())
    {
        maNode = GetConfigurationNode(
            Reference<container::XHierarchicalNameAccess>(maNode, UNO_QUERY),
            rsPathToNode);
        if (Reference<XInterface>(maNode, UNO_QUERY).is())
            return true;
    }

    mxRoot = NULL;
    return false;
}




bool PresenterConfigurationAccess::GoToChild (const Predicate& rPredicate)
{
    if ( ! IsValid())
        return false;

    maNode = Find(Reference<container::XNameAccess>(maNode,UNO_QUERY), rPredicate);
    if (Reference<XInterface>(maNode, UNO_QUERY).is())
        return true;

    mxRoot = NULL;
    return false;
}




bool PresenterConfigurationAccess::SetProperty (
    const ::rtl::OUString& rsPropertyName,
    const Any& rValue)
{
    Reference<beans::XPropertySet> xProperties (maNode, UNO_QUERY);
    if (xProperties.is())
    {
        xProperties->setPropertyValue(rsPropertyName, rValue);
        return true;
    }
    else
        return false;
}




Any PresenterConfigurationAccess::GetConfigurationNode (
    const css::uno::Reference<css::container::XHierarchicalNameAccess>& rxNode,
    const OUString& sPathToNode)
{
    if (sPathToNode.getLength() == 0)
        return Any(rxNode);

    try
    {
        if (rxNode.is())
        {
            return rxNode->getByHierarchicalName(sPathToNode);
        }
    }
    catch (Exception& rException)
    {
        OSL_TRACE ("caught exception while getting configuration node %s: %s",
            ::rtl::OUStringToOString(sPathToNode, RTL_TEXTENCODING_UTF8).getStr(),
            ::rtl::OUStringToOString(rException.Message, RTL_TEXTENCODING_UTF8).getStr());
    }

    return Any();
}




Reference<beans::XPropertySet> PresenterConfigurationAccess::GetNodeProperties (
    const css::uno::Reference<css::container::XHierarchicalNameAccess>& rxNode,
    const ::rtl::OUString& rsPathToNode)
{
    return Reference<beans::XPropertySet>(GetConfigurationNode(rxNode, rsPathToNode), UNO_QUERY);
}




void PresenterConfigurationAccess::CommitChanges (void)
{
    Reference<util::XChangesBatch> xConfiguration (mxRoot, UNO_QUERY);
    if (xConfiguration.is())
        xConfiguration->commitChanges();
}




Any PresenterConfigurationAccess::GetValue (const rtl::OUString& sKey)
{
    Reference<container::XNameAccess> xAccess (GetConfigurationNode(sKey), UNO_QUERY);
    if (xAccess.is())
    {
        return xAccess->getByName(sKey);
    }
    else
    {
        return Any();
    }
}




void PresenterConfigurationAccess::ForAll (
    const Reference<container::XNameAccess>& rxContainer,
    const ::std::vector<OUString>& rArguments,
    const ItemProcessor& rProcessor)
{
    if (rxContainer.is())
    {
        ::std::vector<Any> aValues(rArguments.size());
        Sequence<OUString> aKeys (rxContainer->getElementNames());
        for (sal_Int32 nItemIndex=0; nItemIndex<aKeys.getLength(); ++nItemIndex)
        {
            bool bHasAllValues (true);
            const OUString& rsKey (aKeys[nItemIndex]);
            Reference<container::XNameAccess> xSetItem (rxContainer->getByName(rsKey), UNO_QUERY);
            Reference<beans::XPropertySet> xSet (xSetItem, UNO_QUERY);
            OSL_ASSERT(xSet.is());
            if (xSetItem.is())
            {
                // Get from the current item of the container the children
                // that match the names in the rArguments list.
                for (sal_uInt32 nValueIndex=0; nValueIndex<aValues.size(); ++nValueIndex)
                {
                    if ( ! xSetItem->hasByName(rArguments[nValueIndex]))
                        bHasAllValues = false;
                    else
                        aValues[nValueIndex] = xSetItem->getByName(rArguments[nValueIndex]);
                }
            }
            else
                bHasAllValues = false;
            if (bHasAllValues)
                rProcessor(rsKey,aValues);
        }
    }
}




void PresenterConfigurationAccess::ForAll (
    const Reference<container::XNameAccess>& rxContainer,
    const PropertySetProcessor& rProcessor)
{
    if (rxContainer.is())
    {
        Sequence<OUString> aKeys (rxContainer->getElementNames());
        for (sal_Int32 nItemIndex=0; nItemIndex<aKeys.getLength(); ++nItemIndex)
        {
            const OUString& rsKey (aKeys[nItemIndex]);
            Reference<beans::XPropertySet> xSet (rxContainer->getByName(rsKey), UNO_QUERY);
            if (xSet.is())
                rProcessor(rsKey, xSet);
        }
    }
}




void PresenterConfigurationAccess::FillList(
    const Reference<container::XNameAccess>& rxContainer,
    const ::rtl::OUString& rsArgument,
    ::std::vector<OUString>& rList)
{
    try
    {
        if (rxContainer.is())
        {
            Sequence<OUString> aKeys (rxContainer->getElementNames());
            rList.resize(aKeys.getLength());
            for (sal_Int32 nItemIndex=0; nItemIndex<aKeys.getLength(); ++nItemIndex)
            {
                Reference<container::XNameAccess> xSetItem (
                    rxContainer->getByName(aKeys[nItemIndex]), UNO_QUERY);
                if (xSetItem.is())
                {
                    xSetItem->getByName(rsArgument) >>= rList[nItemIndex];
                }
            }
        }
    }
    catch (RuntimeException&)
    {}
}




Any PresenterConfigurationAccess::Find (
    const Reference<container::XNameAccess>& rxContainer,
    const Predicate& rPredicate)
{
    if (rxContainer.is())
    {
        Sequence<OUString> aKeys (rxContainer->getElementNames());
        for (sal_Int32 nItemIndex=0; nItemIndex<aKeys.getLength(); ++nItemIndex)
        {
            Reference<beans::XPropertySet> xProperties (
                rxContainer->getByName(aKeys[nItemIndex]),
                UNO_QUERY);
            if (xProperties.is())
                if (rPredicate(aKeys[nItemIndex], xProperties))
                    return Any(xProperties);
        }
    }
    return Any();
}




bool PresenterConfigurationAccess::IsStringPropertyEqual (
    const ::rtl::OUString& rsValue,
    const ::rtl::OUString& rsPropertyName,
    const css::uno::Reference<css::beans::XPropertySet>& rxNode)
{
    OUString sValue;
    if (GetProperty(rxNode, rsPropertyName) >>= sValue)
        return sValue == rsValue;
    else
        return false;
}




Any PresenterConfigurationAccess::GetProperty (
    const Reference<beans::XPropertySet>& rxProperties,
    const OUString& rsKey)
{
    OSL_ASSERT(rxProperties.is());
    if ( ! rxProperties.is())
        return Any();
    try
    {
        Reference<beans::XPropertySetInfo> xInfo (rxProperties->getPropertySetInfo());
        if (xInfo.is())
            if ( ! xInfo->hasPropertyByName(rsKey))
                return Any();
        return rxProperties->getPropertyValue(rsKey);
    }
    catch (beans::UnknownPropertyException&)
    {
    }
    return Any();
}




} } // end of namespace sdext::tools

