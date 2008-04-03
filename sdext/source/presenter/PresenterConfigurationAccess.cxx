/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterConfigurationAccess.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 15:56:40 $
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

#include "PresenterConfigurationAccess.hxx"

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

namespace sdext { namespace presenter {

PresenterConfigurationAccess::PresenterConfigurationAccess (
    const Reference<XComponentContext>& rxContext,
    const OUString& rsRootName,
    WriteMode eMode)
    : mxRoot()
{
    try
    {
        Reference<lang::XMultiComponentFactory> xFactory (rxContext->getServiceManager());
        if (xFactory.is())
        {
            Sequence<Any> aCreationArguments(3);
            aCreationArguments[0] = makeAny(beans::PropertyValue(
                OUString(
                    RTL_CONSTASCII_USTRINGPARAM("nodepath")),
                0,
                makeAny(rsRootName),
                beans::PropertyState_DIRECT_VALUE));
            aCreationArguments[1] = makeAny(beans::PropertyValue(
                OUString(RTL_CONSTASCII_USTRINGPARAM("depth")),
                0,
                makeAny((sal_Int32)-1),
                beans::PropertyState_DIRECT_VALUE));
            aCreationArguments[2] = makeAny(beans::PropertyValue(
                OUString(RTL_CONSTASCII_USTRINGPARAM("lazywrite")),
                0,
                makeAny(true),
                beans::PropertyState_DIRECT_VALUE));

            OUString sAccessService;
            if (eMode == READ_ONLY)
                sAccessService = OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.configuration.ConfigurationAccess"));
            else
                sAccessService = OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.configuration.ConfigurationUpdateAccess"));

            Reference<lang::XMultiServiceFactory> xProvider (
                xFactory->createInstanceWithContext(
                    OUString::createFromAscii("com.sun.star.configuration.ConfigurationProvider"),
                    rxContext),
                UNO_QUERY_THROW);
            mxRoot = xProvider->createInstanceWithArguments(
                sAccessService, aCreationArguments);
        }
    }
    catch (Exception& rException)
    {
        OSL_TRACE ("caught exception while opening configuration: %s",
            ::rtl::OUStringToOString(rException.Message,
                RTL_TEXTENCODING_UTF8).getStr());
    }
}




Any PresenterConfigurationAccess::GetConfigurationNode (const OUString& sPathToNode)
{
    return GetConfigurationNode(
        Reference<container::XHierarchicalNameAccess>(mxRoot, UNO_QUERY),
        sPathToNode);
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




Reference<container::XNameAccess> PresenterConfigurationAccess::Find (
    const Reference<container::XNameAccess>& rxContainer,
    const Predicate& rPredicate)
{
    if (rxContainer.is())
    {
        Sequence<OUString> aKeys (rxContainer->getElementNames());
        for (sal_Int32 nItemIndex=0; nItemIndex<aKeys.getLength(); ++nItemIndex)
        {
            Reference<container::XNameAccess> xSetItem (
                rxContainer->getByName(aKeys[nItemIndex]), UNO_QUERY);
            if (xSetItem.is())
                if (rPredicate(aKeys[nItemIndex], xSetItem))
                    return xSetItem;
        }
    }
    return NULL;
}




} } // end of namespace sdext::tools

