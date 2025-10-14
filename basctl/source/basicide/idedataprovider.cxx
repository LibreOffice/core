/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <idedataprovider.hxx>
#include "idetimer.hxx"

#include <basic/basmgr.hxx>
#include <basctl/scriptdocument.hxx>
#include <comphelper/processfactory.hxx>
#include <sal/log.hxx>

#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/reflection/XTypeDescriptionEnumeration.hpp>
#include <com/sun/star/reflection/XTypeDescriptionEnumerationAccess.hpp>
#include <com/sun/star/reflection/theCoreReflection.hpp>

namespace basctl
{
using namespace basic;

namespace css = ::com::sun::star;
using namespace css::container;
using namespace css::uno;
using namespace css::lang;
using namespace css::reflection;

IdeDataProvider::IdeDataProvider()
    : m_pUnoHierarchy(std::make_unique<UnoApiHierarchy>())
    , m_bInitialized(false)
{
}

void IdeDataProvider::Initialize()
{
    if (m_bInitialized)
        return;

    IdeTimer aTotalInitTimer(u"IdeDataProvider::Initialize (Synchronous)"_ustr);
    SAL_INFO("basctl", "Starting synchronous data provider initialization...");

    // Perform a full synchronous UNO scan
    SAL_INFO("basctl", "Performing full synchronous UNO scan...");
    performFullUnoScan();

    // Preload all BASIC Libraries
    IdeTimer aBasicTimer(u"IdeDataProvider::Initialize (Basic Library Preload)"_ustr);
    SAL_INFO("basctl", "Starting BASIC library preload...");

    try
    {
        ScriptDocument aAppDoc = ScriptDocument::getApplicationScriptDocument();
        if (aAppDoc.isAlive())
        {
            Reference<css::script::XLibraryContainer> xLibContainer
                = aAppDoc.getLibraryContainer(E_SCRIPTS);
            if (xLibContainer.is())
            {
                for (const OUString& rLibName : xLibContainer->getElementNames())
                {
                    try
                    {
                        if (xLibContainer->hasByName(rLibName)
                            && !xLibContainer->isLibraryLoaded(rLibName))
                        {
                            xLibContainer->loadLibrary(rLibName);
                        }
                    }
                    catch (const Exception& e)
                    {
                        SAL_WARN("basctl", "Exception while preloading application library '"
                                               << rLibName << "': " << e.Message);
                    }
                }
            }
        }
    }
    catch (const Exception& e)
    {
        SAL_WARN("basctl", "Could not retrieve Application library container: " << e.Message);
    }

    // Preload from any open documents
    try
    {
        for (const auto& rDoc :
             ScriptDocument::getAllScriptDocuments(ScriptDocument::DocumentsSorted))
        {
            if (rDoc.isAlive())
            {
                auto xDocLibContainer = rDoc.getLibraryContainer(E_SCRIPTS);
                if (xDocLibContainer.is())
                {
                    for (const OUString& rLibName : xDocLibContainer->getElementNames())
                    {
                        try
                        {
                            if (xDocLibContainer->hasByName(rLibName)
                                && !xDocLibContainer->isLibraryLoaded(rLibName))
                            {
                                xDocLibContainer->loadLibrary(rLibName);
                            }
                        }
                        catch (const Exception& e)
                        {
                            SAL_WARN("basctl", "Exception while preloading document library '"
                                                   << rLibName << "' in document '"
                                                   << rDoc.getTitle() << "': " << e.Message);
                        }
                    }
                }
            }
        }
    }
    catch (const Exception& e)
    {
        SAL_WARN("basctl", "Could not iterate through document libraries: " << e.Message);
    }
    SAL_INFO("basctl", "BASIC library preload finished.");

    m_aTopLevelNodes.clear();
    m_aTopLevelNodes.push_back(
        std::make_shared<IdeSymbolInfo>(u"UNO APIs", IdeSymbolKind::ROOT_UNO_APIS, u"root"));
    m_aTopLevelNodes.push_back(std::make_shared<IdeSymbolInfo>(
        u"Application Macros", IdeSymbolKind::ROOT_APPLICATION_LIBS, u"root"));

    m_bInitialized = true;
    SAL_INFO("basctl", "Synchronous data provider initialization complete.");
}

void IdeDataProvider::Reset()
{
    SAL_INFO("basctl", "IdeDataProvider: Resetting state.");
    m_bInitialized = false;
    m_aTopLevelNodes.clear();

    m_pUnoHierarchy = std::make_unique<UnoApiHierarchy>();
}

void IdeDataProvider::performFullUnoScan()
{
    IdeTimer aScanTimer(u"IdeDataProvider::performFullUnoScan"_ustr);
    sal_Int32 nProcessedCount = 0;

    try
    {
        Reference<XHierarchicalNameAccess> xTypeManager;
        comphelper::getProcessComponentContext()->getValueByName(
            u"/singletons/com.sun.star.reflection.theTypeDescriptionManager"_ustr)
            >>= xTypeManager;

        if (xTypeManager.is())
        {
            Reference<XTypeDescriptionEnumerationAccess> xEnumAccess(xTypeManager, UNO_QUERY_THROW);
            Reference<XEnumeration> xEnum = xEnumAccess->createTypeDescriptionEnumeration(
                u""_ustr, {}, TypeDescriptionSearchDepth_INFINITE);

            while (xEnum.is() && xEnum->hasMoreElements())
            {
                try
                {
                    Reference<XTypeDescription> xTypeDesc;
                    if ((xEnum->nextElement() >>= xTypeDesc) && xTypeDesc.is())
                    {
                        m_pUnoHierarchy->addNode(xTypeDesc->getName(), xTypeDesc->getTypeClass());
                        nProcessedCount++;
                    }
                }
                catch (const Exception& e)
                {
                    SAL_WARN("basctl",
                             "performFullUnoScan: Exception processing a single UNO type: "
                                 << e.Message);
                }
            }
        }
    }
    catch (const Exception& e)
    {
        SAL_WARN("basctl", "Full UNO scan failed with exception: " << e.Message);
    }

    sal_Int64 nFullScanTimeMs = aScanTimer.getElapsedTimeMs();
    SAL_INFO("basctl", "UNO scan completed in " << nFullScanTimeMs << " ms. Found "
                                                << nProcessedCount << " types.");
}

SymbolInfoList IdeDataProvider::GetTopLevelNodes()
{
    if (!m_bInitialized)
    {
        SymbolInfoList aNodes;
        auto pLoadingNode = std::make_shared<IdeSymbolInfo>(u"[Initializing...]",
                                                            IdeSymbolKind::PLACEHOLDER, u"");
        pLoadingNode->bSelectable = false;
        aNodes.push_back(pLoadingNode);
        return aNodes;
    }
    return m_aTopLevelNodes;
}

GroupedSymbolInfoList IdeDataProvider::GetMembers(const IdeSymbolInfo& /*rNode*/) { return {}; }

SymbolInfoList IdeDataProvider::GetChildNodes(const IdeSymbolInfo& rParent)
{
    if (!m_bInitialized)
    {
        return {};
    }

    SymbolInfoList aChildren;
    if (rParent.eKind == IdeSymbolKind::ROOT_UNO_APIS)
    {
        aChildren = m_pUnoHierarchy->m_hierarchyCache[OUString()];
    }
    else if (rParent.eKind == IdeSymbolKind::UNO_NAMESPACE)
    {
        aChildren = m_pUnoHierarchy->m_hierarchyCache[rParent.sQualifiedName];
    }

    return aChildren;
}

void UnoApiHierarchy::addNode(std::u16string_view sQualifiedNameView, TypeClass eTypeClass)
{
    const OUString sQualifiedName(sQualifiedNameView);

    if (sQualifiedName.isEmpty())
        return;

    OUString sParentPath;
    OUString sCurrentPath;
    sal_Int32 nStartIndex = 0;
    sal_Int32 nDotIndex;

    do
    {
        nDotIndex = sQualifiedName.indexOf('.', nStartIndex);
        OUString sPart = (nDotIndex == -1)
                             ? sQualifiedName.copy(nStartIndex)
                             : sQualifiedName.copy(nStartIndex, nDotIndex - nStartIndex);

        if (sPart.isEmpty())
            continue;

        sParentPath = sCurrentPath;
        sCurrentPath = sCurrentPath.isEmpty() ? sPart : sCurrentPath + u"." + sPart;

        // Find the list of children for the current parent
        auto& rChildren = m_hierarchyCache[sParentPath];

        // Avoid adding duplicate nodes
        bool bExists = std::any_of(rChildren.begin(), rChildren.end(),
                                   [&sPart](const auto& pNode) { return pNode->sName == sPart; });

        if (!bExists)
        {
            IdeSymbolKind eKind = (nDotIndex == -1) ? typeClassToSymbolKind(eTypeClass)
                                                    : IdeSymbolKind::UNO_NAMESPACE;

            auto pNewNode
                = std::make_shared<IdeSymbolInfo>(sPart.getStr(), eKind, sParentPath.getStr());
            pNewNode->sQualifiedName = sCurrentPath;
            pNewNode->sParentName = sParentPath;
            pNewNode->sIdentifier = sCurrentPath;
            rChildren.push_back(std::move(pNewNode));
        }

        nStartIndex = nDotIndex + 1;
    } while (nDotIndex != -1);
}

IdeSymbolKind UnoApiHierarchy::typeClassToSymbolKind(TypeClass eTypeClass)
{
    switch (eTypeClass)
    {
        case TypeClass_INTERFACE:
            return IdeSymbolKind::UNO_INTERFACE;
        case TypeClass_STRUCT:
            return IdeSymbolKind::UNO_STRUCT;
        case TypeClass_ENUM:
            return IdeSymbolKind::UNO_ENUM;
        case TypeClass_TYPEDEF:
            return IdeSymbolKind::UNO_TYPEDEF;
        case TypeClass_CONSTANTS:
            return IdeSymbolKind::UNO_CONSTANTS;
        case TypeClass_EXCEPTION:
            return IdeSymbolKind::UNO_EXCEPTION;
        case TypeClass_SERVICE:
            return IdeSymbolKind::UNO_SERVICE;
        case TypeClass_MODULE:
            return IdeSymbolKind::UNO_NAMESPACE;
        case TypeClass_SINGLETON:
            return IdeSymbolKind::UNO_SERVICE; // Treat singletons as services
        default:
            return IdeSymbolKind::UNO_TYPE;
    }
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
