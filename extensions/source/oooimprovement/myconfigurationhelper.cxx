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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#include "myconfigurationhelper.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <rtl/ustrbuf.hxx>
#include <vector>


namespace css = ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::std::vector;


namespace
{
    static const Sequence<Any> sequenceFromVector(const vector<Any>& vec)
    {
        Sequence<Any> result(vec.size());
        for(size_t idx = 0; idx < vec.size(); ++idx)
            result[idx] = vec[idx];
        return result;
    };

    static const OUString noSuchElement(const OUString& path)
    {
        OUStringBuffer buf(256);
        buf.appendAscii("The requested path \"");
        buf.append(path);
        buf.appendAscii("\" does not exists.");
        return buf.makeStringAndClear();
    };
}

namespace oooimprovement
{
    Reference<XInterface> MyConfigurationHelper::openConfig(
        const Reference<XMultiServiceFactory> xSMGR,
        const OUString& sPackage,
        sal_Int32 eMode)
    {
        Reference<XMultiServiceFactory> xConfigProvider(
            xSMGR->createInstance(OUString::createFromAscii("com.sun.star.configuration.ConfigurationProvider")),
            UNO_QUERY_THROW);

        vector<Any> lParams;
        css::beans::PropertyValue aParam;

        // set root path
        aParam.Name = OUString::createFromAscii("nodepath");
        aParam.Value <<= sPackage;
        lParams.push_back(makeAny(aParam));

        // enable all locales mode
        if ((eMode & MyConfigurationHelper::E_ALL_LOCALES)==MyConfigurationHelper::E_ALL_LOCALES)
        {
            aParam.Name = OUString::createFromAscii("locale");
            aParam.Value <<= OUString::createFromAscii("*");
            lParams.push_back(makeAny(aParam));
        }

        // enable lazy writing
        sal_Bool bLazy = ((eMode & MyConfigurationHelper::E_LAZY_WRITE)==MyConfigurationHelper::E_LAZY_WRITE);
        aParam.Name = OUString::createFromAscii("lazywrite");
        aParam.Value = makeAny(bLazy);
        lParams.push_back(makeAny(aParam));

        // open it
        Reference<XInterface> xCFG;

        sal_Bool bReadOnly = ((eMode & MyConfigurationHelper::E_READONLY)==MyConfigurationHelper::E_READONLY);
        if (bReadOnly)
            xCFG = xConfigProvider->createInstanceWithArguments(
                OUString::createFromAscii("com.sun.star.configuration.ConfigurationAccess"),
                sequenceFromVector(lParams));
        else
            xCFG = xConfigProvider->createInstanceWithArguments(
                OUString::createFromAscii("com.sun.star.configuration.ConfigurationUpdateAccess"),
                sequenceFromVector(lParams));
        return xCFG;
    }

    Any MyConfigurationHelper::readRelativeKey(
        const Reference<XInterface> xCFG,
        const OUString& sRelPath,
        const OUString& sKey)
    {
        Reference<css::container::XHierarchicalNameAccess> xAccess(xCFG, UNO_QUERY_THROW);

        Reference<css::beans::XPropertySet> xProps;
        xAccess->getByHierarchicalName(sRelPath) >>= xProps;
        if (!xProps.is())
            throw css::container::NoSuchElementException(
                noSuchElement(sRelPath),
                Reference<XInterface>());
        return xProps->getPropertyValue(sKey);
    }

    void MyConfigurationHelper::writeRelativeKey(
        const Reference<XInterface> xCFG,
        const OUString& sRelPath,
        const OUString& sKey,
        const Any& aValue)
    {
        Reference<css::container::XHierarchicalNameAccess> xAccess(xCFG, UNO_QUERY_THROW);

        Reference<css::beans::XPropertySet> xProps;
        xAccess->getByHierarchicalName(sRelPath) >>= xProps;
        if (!xProps.is())
            throw css::container::NoSuchElementException(
                noSuchElement(sRelPath),
                Reference<XInterface>());
        xProps->setPropertyValue(sKey, aValue);
    }

    Any MyConfigurationHelper::readDirectKey(
        const Reference<XMultiServiceFactory> xSMGR,
        const OUString& sPackage,
        const OUString& sRelPath,
        const OUString& sKey,
        sal_Int32 eMode)
    {
        Reference<XInterface> xCFG = MyConfigurationHelper::openConfig(xSMGR, sPackage, eMode);
        return MyConfigurationHelper::readRelativeKey(xCFG, sRelPath, sKey);
    }

    void MyConfigurationHelper::writeDirectKey(
        const Reference<XMultiServiceFactory> xSMGR,
        const OUString& sPackage,
        const OUString& sRelPath,
        const OUString& sKey,
        const Any& aValue,
        sal_Int32 eMode)
    {
        Reference<XInterface> xCFG = MyConfigurationHelper::openConfig(xSMGR, sPackage, eMode);
        MyConfigurationHelper::writeRelativeKey(xCFG, sRelPath, sKey, aValue);
        MyConfigurationHelper::flush(xCFG);
    }

    void MyConfigurationHelper::flush(const Reference<XInterface>& xCFG)
    {
        Reference<css::util::XChangesBatch> xBatch(xCFG, UNO_QUERY_THROW);
        xBatch->commitChanges();
    }
}
