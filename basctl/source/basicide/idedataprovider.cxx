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
#include <unordered_set>

#include <basic/basmgr.hxx>
#include <basic/sbmeth.hxx>
#include <basctl/scriptdocument.hxx>
#include <comphelper/processfactory.hxx>
#include <sal/log.hxx>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/reflection/XTypeDescriptionEnumeration.hpp>
#include <com/sun/star/reflection/XTypeDescriptionEnumerationAccess.hpp>
#include <com/sun/star/reflection/theCoreReflection.hpp>
#include <com/sun/star/script/XStorageBasedLibraryContainer.hpp>

namespace basctl
{
using namespace basic;

namespace css = ::com::sun::star;
using namespace css::container;
using namespace css::uno;
using namespace css::lang;
using namespace css::reflection;

namespace
{
OUString helper_getTypeName(const Reference<XIdlClass>& xTypeClass)
{
    if (xTypeClass.is())
    {
        return xTypeClass->getName();
    }
    return u"<UnknownType>"_ustr;
}

OUString GetBasicTypeName(SbxDataType eType)
{
    switch (eType)
    {
        case SbxDataType::SbxSTRING:
            return u"String"_ustr;
        case SbxDataType::SbxINTEGER:
            return u"Integer"_ustr;
        case SbxDataType::SbxLONG:
            return u"Long"_ustr;
        case SbxDataType::SbxULONG:
            return u"ULong"_ustr;
        case SbxDataType::SbxSALINT64:
            return u"Long64"_ustr;
        case SbxDataType::SbxSALUINT64:
            return u"ULong64"_ustr;
        case SbxDataType::SbxDECIMAL:
            return u"Decimal"_ustr;
        case SbxDataType::SbxCHAR:
            return u"Char"_ustr;
        case SbxDataType::SbxBYTE:
            return u"Byte"_ustr;
        case SbxDataType::SbxSINGLE:
            return u"Single"_ustr;
        case SbxDataType::SbxDOUBLE:
            return u"Double"_ustr;
        case SbxDataType::SbxCURRENCY:
            return u"Currency"_ustr;
        case SbxDataType::SbxDATE:
            return u"Date"_ustr;
        case SbxDataType::SbxBOOL:
            return u"Boolean"_ustr;
        case SbxDataType::SbxOBJECT:
            return u"Object"_ustr;
        case SbxDataType::SbxERROR:
            return u"Error"_ustr;
        case SbxDataType::SbxEMPTY:
            return u"Empty"_ustr;
        case SbxDataType::SbxNULL:
            return u"Null"_ustr;
        case SbxDataType::SbxVOID:
            return u"Void"_ustr;
        default:
            return u"Variant"_ustr;
    }
}

void ImplGetMembersOfUnoType(SymbolInfoList& rMembers, const IdeSymbolInfo& rNode,
                             std::unordered_set<OUString>& rVisitedTypes, sal_uInt16 nDepth = 0)
{
    constexpr sal_uInt16 nMaxRecursionDepth(8);

    if (nDepth > nMaxRecursionDepth || rNode.eKind == IdeSymbolKind::UNO_TYPEDEF
        || rNode.eKind == IdeSymbolKind::UNO_TYPE)
    {
        return;
    }

    if (!rNode.sQualifiedName.isEmpty() && !rVisitedTypes.insert(rNode.sQualifiedName).second)
    {
        return;
    }
    try
    {
        Reference<XIdlReflection> xReflection
            = css::reflection::theCoreReflection::get(comphelper::getProcessComponentContext());
        if (!xReflection.is())
        {
            return;
        }

        Reference<XIdlClass> xTypeClass = xReflection->forName(rNode.sQualifiedName);
        if (!xTypeClass.is())
        {
            return;
        }

        if (rNode.eKind == IdeSymbolKind::UNO_SERVICE)
        {
            for (const auto& xInterface : xTypeClass->getInterfaces())
            {
                auto pNode = std::make_shared<IdeSymbolInfo>(
                    xInterface->getName(), IdeSymbolKind::UNO_INTERFACE, rNode.sIdentifier);
                pNode->sQualifiedName = xInterface->getName();
                rMembers.push_back(std::move(pNode));
            }
            return;
        }

        for (const auto& xMethod : xTypeClass->getMethods())
        {
            if (!xMethod.is())
            {
                continue;
            }
            auto pNode = std::make_shared<IdeSymbolInfo>(
                xMethod->getName(), IdeSymbolKind::UNO_METHOD, rNode.sIdentifier);

            if (!rNode.sQualifiedName.isEmpty())
            {
                pNode->sQualifiedName = rNode.sQualifiedName + u"." + xMethod->getName();
                pNode->sParentName = rNode.sQualifiedName;
            }
            pNode->sReturnTypeName = helper_getTypeName(xMethod->getReturnType());
            for (const auto& rParam : xMethod->getParameterInfos())
            {
                IdeParamInfo aParam;
                aParam.sName = rParam.aName;
                aParam.sTypeName = helper_getTypeName(rParam.aType);
                aParam.bIsIn = (rParam.aMode == css::reflection::ParamMode_IN
                                || rParam.aMode == css::reflection::ParamMode_INOUT);
                aParam.bIsOut = (rParam.aMode == css::reflection::ParamMode_OUT
                                 || rParam.aMode == css::reflection::ParamMode_INOUT);
                pNode->aParameters.push_back(std::move(aParam));
            }

            rMembers.push_back(std::move(pNode));
        }

        for (const auto& xField : xTypeClass->getFields())
        {
            if (!xField.is())
            {
                continue;
            }
            IdeSymbolKind eFieldKind = IdeSymbolKind::UNO_PROPERTY;
            if (rNode.eKind == IdeSymbolKind::UNO_STRUCT
                || rNode.eKind == IdeSymbolKind::UNO_EXCEPTION)
            {
                eFieldKind = IdeSymbolKind::UNO_FIELD;
            }
            else if (rNode.eKind == IdeSymbolKind::UNO_ENUM
                     || rNode.eKind == IdeSymbolKind::UNO_CONSTANTS)
            {
                eFieldKind = IdeSymbolKind::ENUM_MEMBER;
            }

            auto pMember
                = std::make_shared<IdeSymbolInfo>(xField->getName(), eFieldKind, rNode.sIdentifier);

            if (!rNode.sQualifiedName.isEmpty())
            {
                pMember->sQualifiedName = rNode.sQualifiedName + u"." + xField->getName();
                pMember->sParentName = rNode.sQualifiedName;
            }
            pMember->sTypeName = helper_getTypeName(xField->getType());

            Reference<XIdlClass> xFieldType = xField->getType();
            if (xFieldType.is())
            {
                TypeClass eTypeClass = xFieldType->getTypeClass();
                if (eTypeClass == TypeClass_STRUCT || eTypeClass == TypeClass_INTERFACE)
                {
                    IdeSymbolInfo tempFieldNodeAsType(
                        xFieldType->getName(), UnoApiHierarchy::typeClassToSymbolKind(eTypeClass),
                        rNode.sIdentifier);
                    tempFieldNodeAsType.sQualifiedName = xFieldType->getName();

                    // Recursively get the nested members
                    SymbolInfoList aNestedMembers;
                    ImplGetMembersOfUnoType(aNestedMembers, tempFieldNodeAsType, rVisitedTypes,
                                            nDepth + 1);

                    for (const auto& pNested : aNestedMembers)
                    {
                        pMember->mapMembers[pNested->sName] = pNested;
                    }
                }
            }

            rMembers.push_back(std::move(pMember));
        }
    }
    catch (const Exception& e)
    {
        SAL_WARN("basctl", "Exception while getting members of UNO type: " << e.Message);
    }
}

void ImplGetMembersOfBasicModule(SymbolInfoList& rMembers, const IdeSymbolInfo& rNode)
{
    ScriptDocument aDoc = rNode.sOriginLocation.isEmpty()
                              ? ScriptDocument::getApplicationScriptDocument()
                              : ScriptDocument::getDocumentWithURLOrCaption(rNode.sOriginLocation);
    if (!aDoc.isAlive())
    {
        return;
    }

    BasicManager* pBasMgr = aDoc.getBasicManager();
    StarBASIC* pLib = pBasMgr ? pBasMgr->GetLib(rNode.sOriginLibrary) : nullptr;
    SbModule* pModule = pLib ? pLib->FindModule(rNode.sName) : nullptr;
    if (pModule)
    {
        if (!pModule->IsCompiled())
        {
            pModule->Compile();
        }
        SbxArray* pMethods = pModule->GetMethods().get();
        for (sal_uInt32 i = 0; i < pMethods->Count(); ++i)
        {
            SbxVariable* pVar = pMethods->Get(i);
            if (auto* pMethod = dynamic_cast<SbMethod*>(pVar))
            {
                if (pMethod->IsHidden())
                {
                    continue;
                }

                IdeSymbolKind eKind = (pMethod->GetType() == SbxDataType::SbxVOID)
                                          ? IdeSymbolKind::SUB
                                          : IdeSymbolKind::FUNCTION;
                auto pMember
                    = std::make_shared<IdeSymbolInfo>(pMethod->GetName(), eKind, rNode.sIdentifier);

                pMember->sOriginLibrary = rNode.sOriginLibrary;
                pMember->sOriginModule = rNode.sName; // The parent node name is the module name
                pMember->sOriginLocation = rNode.sOriginLocation;

                if (!rNode.sOriginLibrary.isEmpty() && !rNode.sName.isEmpty())
                {
                    pMember->sParentName = rNode.sOriginLibrary + u"." + rNode.sName;
                    pMember->sQualifiedName = pMember->sParentName + u"." + pMethod->GetName();
                }

                SbxInfo* pInfo = pMethod->GetInfo();
                if (pInfo)
                {
                    pMember->sReturnTypeName = GetBasicTypeName(pMethod->GetType());
                    sal_uInt32 j = 1;
                    const SbxParamInfo* pParamInfo = pInfo->GetParam(j);
                    while (pParamInfo)
                    {
                        IdeParamInfo aParam;
                        aParam.sName = pParamInfo->aName;
                        aParam.sTypeName = GetBasicTypeName(pParamInfo->eType);
                        aParam.bIsByVal = !(pParamInfo->nFlags & SbxFlagBits::Reference);
                        aParam.bIsOptional
                            = static_cast<bool>(pParamInfo->nFlags & SbxFlagBits::Optional);

                        bool bIsByRef
                            = static_cast<bool>(pParamInfo->nFlags & SbxFlagBits::Reference);
                        aParam.bIsByVal = !bIsByRef;
                        aParam.bIsIn = true;
                        aParam.bIsOut = bIsByRef;

                        pMember->aParameters.push_back(aParam);

                        pParamInfo = pInfo->GetParam(++j);
                    }
                }

                rMembers.push_back(std::move(pMember));
            }
        }
    }
}

void ImplGetChildrenOfBasicLibrary(SymbolInfoList& rChildren, const IdeSymbolInfo& rParent)
{
    ScriptDocument aDoc
        = rParent.sOriginLocation.isEmpty()
              ? ScriptDocument::getApplicationScriptDocument()
              : ScriptDocument::getDocumentWithURLOrCaption(rParent.sOriginLocation);
    if (!aDoc.isAlive())
    {
        return;
    }

    if (rParent.eKind == IdeSymbolKind::ROOT_APPLICATION_LIBS
        || rParent.eKind == IdeSymbolKind::ROOT_DOCUMENT_LIBS)
    {
        for (const auto& rLibName : aDoc.getLibraryNames())
        {
            auto pNode = std::make_shared<IdeSymbolInfo>(rLibName, IdeSymbolKind::LIBRARY,
                                                         rParent.sIdentifier);
            if (aDoc.isDocument())
            {
                pNode->sOriginLocation = aDoc.getDocument()->getURL();
            }
            pNode->sParentName = rParent.sName;
            pNode->sIdentifier = rParent.sIdentifier + u":" + rLibName;
            rChildren.push_back(std::move(pNode));
        }
    }
    else if (rParent.eKind == IdeSymbolKind::LIBRARY)
    {
        BasicManager* pBasMgr = aDoc.getBasicManager();
        if (aDoc.hasLibrary(E_SCRIPTS, rParent.sName))
        {
            StarBASIC* pLib = pBasMgr->GetLib(rParent.sName);
            if (pLib)
            {
                for (const auto& pModule : pLib->GetModules())
                {
                    auto pNode = std::make_shared<IdeSymbolInfo>(
                        pModule->GetName(), IdeSymbolKind::MODULE, rParent.sIdentifier);
                    pNode->sOriginLocation = rParent.sOriginLocation;
                    pNode->sOriginLibrary = rParent.sName;
                    pNode->sParentName = rParent.sName;
                    pNode->sIdentifier = rParent.sIdentifier + u":" + pNode->sName;
                    rChildren.push_back(std::move(pNode));
                }
            }
        }
    }
}

} // anonymous namespace

IdeDataProvider::IdeDataProvider()
    : m_pUnoHierarchy(std::make_unique<UnoApiHierarchy>())
    , m_bInitialized(false)
    , m_eCurrentScope(IdeBrowserScope::ALL_LIBRARIES)
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

    m_aAllTopLevelNodes.clear();
    m_aAllTopLevelNodes.push_back(
        std::make_shared<IdeSymbolInfo>(u"UNO APIs", IdeSymbolKind::ROOT_UNO_APIS, u"root"));
    m_aAllTopLevelNodes.push_back(std::make_shared<IdeSymbolInfo>(
        u"Application Macros", IdeSymbolKind::ROOT_APPLICATION_LIBS, u"root"));

    AddDocumentNodesWithModules();

    m_bInitialized = true;
    SAL_INFO("basctl", "Synchronous data provider initialization complete.");
}

void IdeDataProvider::Reset()
{
    SAL_INFO("basctl", "IdeDataProvider: Resetting state.");
    m_bInitialized = false;
    m_aAllTopLevelNodes.clear();
    m_aMembersCache.clear();

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

void IdeDataProvider::AddDocumentNodesWithModules()
{
    for (const auto& rDoc : ScriptDocument::getAllScriptDocuments(ScriptDocument::DocumentsSorted))
    {
        if (rDoc.isAlive())
        {
            bool bHasModules = false;
            BasicManager* pBasMgr = rDoc.getBasicManager();
            if (pBasMgr)
            {
                sal_uInt16 nLibCount = pBasMgr->GetLibCount();
                for (sal_uInt16 i = 0; i < nLibCount; ++i)
                {
                    StarBASIC* pLib = pBasMgr->GetLib(i);
                    if (pLib && !pLib->GetModules().empty())
                    {
                        bHasModules = true;
                        break;
                    }
                }
            }

            if (bHasModules)
            {
                auto pDocNode = std::make_shared<IdeSymbolInfo>(
                    rDoc.getTitle(), IdeSymbolKind::ROOT_DOCUMENT_LIBS, u"root");
                if (rDoc.isDocument() && rDoc.getDocument().is())
                {
                    pDocNode->sOriginLocation = rDoc.getDocument()->getURL();
                }
                m_aAllTopLevelNodes.push_back(std::move(pDocNode));
            }
        }
    }
}

void IdeDataProvider::RefreshDocumentNodes()
{
    if (!m_bInitialized)
    {
        return;
    }

    // Remove old document nodes
    m_aAllTopLevelNodes.erase(std::remove_if(m_aAllTopLevelNodes.begin(), m_aAllTopLevelNodes.end(),
                                             [](const auto& pNode) {
                                                 return pNode->eKind
                                                        == IdeSymbolKind::ROOT_DOCUMENT_LIBS;
                                             }),
                              m_aAllTopLevelNodes.end());

    // Re-add current document nodes
    AddDocumentNodesWithModules();
}

SymbolInfoList IdeDataProvider::GetTopLevelNodes()
{
    if (!m_bInitialized)
    {
        SymbolInfoList aNodes;
        auto pLoadingNode = std::make_shared<IdeSymbolInfo>(u"[Initializing...]",
                                                            IdeSymbolKind::PLACEHOLDER, u"");
        pLoadingNode->bSelectable = false;
        aNodes.push_back(std::move(pLoadingNode));
        return aNodes;
    }

    SymbolInfoList aFilteredNodes;
    for (const auto& pNode : m_aAllTopLevelNodes)
    {
        if (pNode->eKind == IdeSymbolKind::ROOT_UNO_APIS
            || pNode->eKind == IdeSymbolKind::ROOT_APPLICATION_LIBS)
        {
            aFilteredNodes.push_back(pNode);
        }
        else if (pNode->eKind == IdeSymbolKind::ROOT_DOCUMENT_LIBS)
        {
            if (m_eCurrentScope == IdeBrowserScope::ALL_LIBRARIES)
            {
                aFilteredNodes.push_back(pNode);
            }
            else // CURRENT_DOCUMENT scope
            {
                // m_sCurrentDocumentURL now holds either the URL or the Title
                bool bMatchesByURL = !m_sCurrentDocumentURL.isEmpty()
                                     && (pNode->sOriginLocation == m_sCurrentDocumentURL);
                bool bMatchesByTitle = !m_sCurrentDocumentURL.isEmpty()
                                       && pNode->sOriginLocation.isEmpty()
                                       && (pNode->sName == m_sCurrentDocumentURL);

                if (bMatchesByURL || bMatchesByTitle)
                {
                    aFilteredNodes.push_back(pNode);
                }
            }
        }
    }
    return aFilteredNodes;
}

GroupedSymbolInfoList IdeDataProvider::GetMembers(const IdeSymbolInfo& rNode)
{
    auto it = m_aMembersCache.find(rNode.sIdentifier);
    if (it != m_aMembersCache.end())
    {
        return it->second;
    }

    GroupedSymbolInfoList aGroupedMembers;
    SymbolInfoList aFlatMembers;

    switch (rNode.eKind)
    {
        case IdeSymbolKind::UNO_CONSTANTS:
        case IdeSymbolKind::UNO_ENUM:
        case IdeSymbolKind::UNO_EXCEPTION:
        case IdeSymbolKind::UNO_INTERFACE:
        case IdeSymbolKind::UNO_SERVICE:
        case IdeSymbolKind::UNO_STRUCT:
        {
            std::unordered_set<OUString> aVisitedTypes;
            ImplGetMembersOfUnoType(aFlatMembers, rNode, aVisitedTypes);
            break;
        }
        case IdeSymbolKind::MODULE:
            ImplGetMembersOfBasicModule(aFlatMembers, rNode);
            break;
        default:
            break;
    }

    for (const auto& pMemberInfo : aFlatMembers)
    {
        aGroupedMembers[pMemberInfo->eKind].push_back(pMemberInfo);
    }

    m_aMembersCache[rNode.sIdentifier] = aGroupedMembers;

    return aGroupedMembers;
}

SymbolInfoList IdeDataProvider::GetChildNodes(const IdeSymbolInfo& rParent)
{
    if (!m_bInitialized)
    {
        return {};
    }

    SymbolInfoList aChildren;
    switch (rParent.eKind)
    {
        case IdeSymbolKind::ROOT_UNO_APIS:
            aChildren = m_pUnoHierarchy->m_hierarchyCache[OUString()];
            break;
        case IdeSymbolKind::UNO_NAMESPACE:
            aChildren = m_pUnoHierarchy->m_hierarchyCache[rParent.sQualifiedName];
            break;
        case IdeSymbolKind::ROOT_APPLICATION_LIBS:
        case IdeSymbolKind::ROOT_DOCUMENT_LIBS:
        case IdeSymbolKind::LIBRARY:
            ImplGetChildrenOfBasicLibrary(aChildren, rParent);
            break;
        default:
            break;
    }

    return aChildren;
}

void IdeDataProvider::SetScope(IdeBrowserScope eScope, const OUString& rCurrentDocumentURL)
{
    m_eCurrentScope = eScope;
    m_sCurrentDocumentURL = rCurrentDocumentURL;
    SAL_INFO("basctl", "SetScope: Scope changed to "
                           << (eScope == IdeBrowserScope::ALL_LIBRARIES ? "ALL_LIBRARIES"
                                                                        : "CURRENT_DOCUMENT")
                           << " for doc URL: '" << rCurrentDocumentURL << "'");
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
