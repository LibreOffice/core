/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <basidesh.hxx>
#include <objectbrowser.hxx>
#include <objectbrowsersearch.hxx>
#include <idedataprovider.hxx>
#include "idetimer.hxx"

#include <bitmaps.hlst>
#include <iderid.hxx>
#include <strings.hrc>

#include <basctl/sbxitem.hxx>
#include <comphelper/processfactory.hxx>
#include <o3tl/string_view.hxx>
#include <rtl/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/event.hxx>
#include <svl/itemset.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/viewfrm.hxx>
#include <vcl/taskpanelist.hxx>
#include <vcl/weld/Builder.hxx>
#include <vcl/weld/MessageDialog.hxx>
#include <vcl/weld/weld.hxx>

#include <com/sun/star/system/XSystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>

namespace basctl
{
namespace
{
// Helper to get an icon resource ID for a given symbol type.
OUString GetIconForSymbol(IdeSymbolKind eKind)
{
    switch (eKind)
    {
        case IdeSymbolKind::ROOT_APPLICATION_LIBS:
            return RID_BMP_INSTALLATION;
        case IdeSymbolKind::ROOT_DOCUMENT_LIBS:
            return RID_BMP_DOCUMENT;
        case IdeSymbolKind::LIBRARY:
            return RID_BMP_MODLIB;
        case IdeSymbolKind::MODULE:
            return RID_BMP_MODULE;
        case IdeSymbolKind::FUNCTION:
        case IdeSymbolKind::SUB:
            return RID_BMP_MACRO;
        case IdeSymbolKind::ROOT_UNO_APIS:
            return u"cmd/sc_configuredialog.png"_ustr;
        case IdeSymbolKind::CLASS_MODULE:
            return u"cmd/sc_insertobject.png"_ustr;
        case IdeSymbolKind::UNO_NAMESPACE:
            return u"cmd/sc_navigator.png"_ustr;
        case IdeSymbolKind::UNO_INTERFACE:
            return u"cmd/sc_insertplugin.png"_ustr;
        case IdeSymbolKind::UNO_SERVICE:
            return u"cmd/sc_insertobjectstarmath.png"_ustr;
        case IdeSymbolKind::UNO_STRUCT:
            return u"cmd/sc_insertframe.png"_ustr;
        case IdeSymbolKind::UNO_ENUM:
            return u"cmd/sc_numberformatmenu.png"_ustr;
        case IdeSymbolKind::UNO_PROPERTY:
            return u"cmd/sc_controlproperties.png"_ustr;
        case IdeSymbolKind::UNO_METHOD:
            return u"cmd/sc_insertformula.png"_ustr;
        case IdeSymbolKind::ENUM_MEMBER:
            return u"cmd/sc_bullet.png"_ustr;
        case IdeSymbolKind::PLACEHOLDER:
            return u"cmd/sc_more.png"_ustr;
        default:
            return u"cmd/sc_insertobject.png"_ustr;
    }
}

// Helper to determine if a symbol is expandable in the tree view.
bool IsExpandable(const IdeSymbolInfo& rSymbol)
{
    switch (rSymbol.eKind)
    {
        case IdeSymbolKind::ROOT_UNO_APIS:
        case IdeSymbolKind::ROOT_APPLICATION_LIBS:
        case IdeSymbolKind::ROOT_DOCUMENT_LIBS:
        case IdeSymbolKind::LIBRARY:
        case IdeSymbolKind::MODULE:
        case IdeSymbolKind::CLASS_MODULE:
        case IdeSymbolKind::UNO_NAMESPACE:
            return true;
        default:
            // This case is for future use when members are nested.
            return !rSymbol.mapMembers.empty();
    }
}

std::shared_ptr<const IdeSymbolInfo>
GetSymbolForIter(const weld::TreeIter& rIter, weld::TreeView& rTree,
                 const std::map<OUString, std::shared_ptr<IdeSymbolInfo>>& rIndex)
{
    const OUString sId = rTree.get_id(rIter);
    if (sId.isEmpty())
    {
        return nullptr;
    }

    auto it = rIndex.find(sId);

    return (it != rIndex.end()) ? it->second : nullptr;
}

bool ShouldShowMembers(const IdeSymbolInfo& rSymbol)
{
    switch (rSymbol.eKind)
    {
        case IdeSymbolKind::UNO_CONSTANTS:
        case IdeSymbolKind::UNO_ENUM:
        case IdeSymbolKind::UNO_EXCEPTION:
        case IdeSymbolKind::UNO_INTERFACE:
        case IdeSymbolKind::UNO_SERVICE:
        case IdeSymbolKind::UNO_STRUCT:
        case IdeSymbolKind::UDT:
        case IdeSymbolKind::MODULE:
        case IdeSymbolKind::CLASS_MODULE:
            return true;
        default:
            return false;
    }
}
OUString GetGroupNameForKind(IdeSymbolKind eKind)
{
    switch (eKind)
    {
        case IdeSymbolKind::UNO_PROPERTY:
        case IdeSymbolKind::PROPERTY_GET:
        case IdeSymbolKind::PROPERTY_LET:
        case IdeSymbolKind::PROPERTY_SET:
            return IDEResId(RID_STR_OB_GROUP_PROPERTIES);
        case IdeSymbolKind::UNO_METHOD:
            return IDEResId(RID_STR_OB_GROUP_METHODS);
        case IdeSymbolKind::UNO_FIELD:
            return IDEResId(RID_STR_OB_GROUP_FIELDS);
        case IdeSymbolKind::ENUM_MEMBER:
            return IDEResId(RID_STR_OB_GROUP_MEMBERS);
        case IdeSymbolKind::SUB:
            return IDEResId(RID_STR_OB_GROUP_PROCEDURES);
        case IdeSymbolKind::FUNCTION:
            return IDEResId(RID_STR_OB_GROUP_FUNCTIONS);
        default:
            return IDEResId(RID_STR_OB_GROUP_OTHER);
    }
}

OUString GetSymbolTypeDescription(basctl::IdeSymbolKind eKind)
{
    using basctl::IdeSymbolKind;
    switch (eKind)
    {
        case IdeSymbolKind::ROOT_UNO_APIS:
            return IDEResId(RID_STR_OB_TYPE_UNO_APIS_ROOT);
        case IdeSymbolKind::ROOT_APPLICATION_LIBS:
            return IDEResId(RID_STR_OB_TYPE_APP_LIBS);
        case IdeSymbolKind::ROOT_DOCUMENT_LIBS:
            return IDEResId(RID_STR_OB_TYPE_DOC_LIBS);
        case IdeSymbolKind::LIBRARY:
            return IDEResId(RID_STR_OB_TYPE_LIBRARY);
        case IdeSymbolKind::MODULE:
            return IDEResId(RID_STR_OB_TYPE_MODULE);
        case IdeSymbolKind::CLASS_MODULE:
            return IDEResId(RID_STR_OB_TYPE_CLASS_MODULE);
        case IdeSymbolKind::UNO_NAMESPACE:
            return IDEResId(RID_STR_OB_TYPE_NAMESPACE);
        case IdeSymbolKind::UNO_INTERFACE:
            return IDEResId(RID_STR_OB_TYPE_INTERFACE);
        case IdeSymbolKind::UNO_SERVICE:
            return IDEResId(RID_STR_OB_TYPE_SERVICE);
        case IdeSymbolKind::UNO_STRUCT:
            return IDEResId(RID_STR_OB_TYPE_STRUCT);
        case IdeSymbolKind::UNO_ENUM:
            return IDEResId(RID_STR_OB_TYPE_ENUM);
        case IdeSymbolKind::UNO_CONSTANTS:
            return IDEResId(RID_STR_OB_TYPE_CONSTANTS);
        case IdeSymbolKind::UNO_EXCEPTION:
            return IDEResId(RID_STR_OB_TYPE_EXCEPTION);
        case IdeSymbolKind::UNO_TYPEDEF:
            return IDEResId(RID_STR_OB_TYPE_TYPEDEF);
        case IdeSymbolKind::UNO_METHOD:
            return IDEResId(RID_STR_OB_TYPE_METHOD);
        case IdeSymbolKind::UNO_PROPERTY:
            return IDEResId(RID_STR_OB_TYPE_PROPERTY);
        case IdeSymbolKind::UNO_FIELD:
            return IDEResId(RID_STR_OB_TYPE_FIELD);
        case IdeSymbolKind::SUB:
            return IDEResId(RID_STR_OB_TYPE_SUB);
        case IdeSymbolKind::FUNCTION:
            return IDEResId(RID_STR_OB_TYPE_FUNCTION);
        case IdeSymbolKind::ENUM_MEMBER:
            return IDEResId(RID_STR_OB_TYPE_ENUM_MEMBER);
        default:
            return IDEResId(RID_STR_OB_TYPE_ITEM);
    }
}

void FormatMethodSignature(rtl::OUStringBuffer& rBuffer, const basctl::IdeSymbolInfo& rSymbol)
{
    rBuffer.append(rSymbol.sName);
    rBuffer.append(u"(");
    if (!rSymbol.aParameters.empty())
    {
        for (size_t i = 0; i < rSymbol.aParameters.size(); ++i)
        {
            const auto& param = rSymbol.aParameters[i];
            if (param.bIsOptional)
            {
                rBuffer.append(IDEResId(RID_STR_OB_OPTIONAL) + u" ");
            }
            if (!param.bIsByVal)
            {
                rBuffer.append(IDEResId(RID_STR_OB_BYREF) + u" ");
            }
            if (param.bIsOut && !param.bIsIn)
            {
                rBuffer.append(IDEResId(RID_STR_OB_OUT_PARAM) + u" ");
            }
            else if (param.bIsOut && param.bIsIn)
            {
                rBuffer.append(IDEResId(RID_STR_OB_INOUT_PARAM) + u" ");
            }

            rBuffer.append(param.sName + IDEResId(RID_STR_OB_AS) + param.sTypeName);
            if (i < rSymbol.aParameters.size() - 1)
            {
                rBuffer.append(u", ");
            }
        }
    }
    rBuffer.append(u")");
    if (!rSymbol.sReturnTypeName.isEmpty() && rSymbol.sReturnTypeName != "Void")
    {
        rBuffer.append(IDEResId(RID_STR_OB_AS) + rSymbol.sReturnTypeName);
    }
}

void FormatPropertySignature(rtl::OUStringBuffer& rBuffer, const basctl::IdeSymbolInfo& rSymbol)
{
    rBuffer.append(rSymbol.sName);
    OUString sType = !rSymbol.sTypeName.isEmpty() ? rSymbol.sTypeName : rSymbol.sReturnTypeName;
    if (!sType.isEmpty())
    {
        rBuffer.append(IDEResId(RID_STR_OB_AS) + sType);
    }
}

OUString AccessModifierToString(IdeAccessModifier eAccess)
{
    switch (eAccess)
    {
        case IdeAccessModifier::PUBLIC:
            return IDEResId(RID_STR_OB_ACCESS_PUBLIC);
        case IdeAccessModifier::PRIVATE:
            return IDEResId(RID_STR_OB_ACCESS_PRIVATE);
        default:
            return OUString();
    }
}

OUString FormatSymbolSignature(const IdeSymbolInfo& rSymbol)
{
    rtl::OUStringBuffer sDescription;

    sDescription.append(u"━━━ " + GetSymbolTypeDescription(rSymbol.eKind) + u" ━━━\n\n📌 ");

    switch (rSymbol.eKind)
    {
        case IdeSymbolKind::UNO_METHOD:
        case IdeSymbolKind::SUB:
        case IdeSymbolKind::FUNCTION:
            FormatMethodSignature(sDescription, rSymbol);
            break;
        case IdeSymbolKind::UNO_PROPERTY:
        case IdeSymbolKind::UNO_FIELD:
            FormatPropertySignature(sDescription, rSymbol);
            break;
        default:
            sDescription.append(rSymbol.sName);
            break;
    }
    sDescription.append(u"\n\n");

    if (rSymbol.eAccessModifier != IdeAccessModifier::NOT_APPLICABLE)
    {
        sDescription.append(u"🔒 " + IDEResId(RID_STR_OB_ACCESS)
                            + AccessModifierToString(rSymbol.eAccessModifier) + u"\n");
    }

    if (!rSymbol.sReturnTypeName.isEmpty() && rSymbol.sReturnTypeName != "Void")
    {
        sDescription.append(u"↩️ " + IDEResId(RID_STR_OB_RETURNS) + rSymbol.sReturnTypeName
                            + u"\n");
    }

    if (!rSymbol.sTypeName.isEmpty() && rSymbol.sReturnTypeName.isEmpty())
    {
        sDescription.append(u"📦 " + IDEResId(RID_STR_OB_TYPE) + rSymbol.sTypeName + u"\n");
    }

    if (!rSymbol.aParameters.empty())
    {
        sDescription.append(u"\n📋 " + IDEResId(RID_STR_OB_PARAMETERS));
        for (const auto& param : rSymbol.aParameters)
        {
            sDescription.append(u"  • " + param.sName + u" : " + param.sTypeName);
            rtl::OUStringBuffer sModifiers;
            if (param.bIsOptional)
            {
                sModifiers.append(IDEResId(RID_STR_OB_OPTIONAL));
            }
            if (param.bIsOut && !param.bIsIn)
            {
                OUString sOut = IDEResId(RID_STR_OB_OUT_PARAM);
                sModifiers.append(sModifiers.getLength() ? (u", "_ustr + sOut) : sOut);
            }
            else if (param.bIsOut && param.bIsIn)
            {
                OUString sInOut = IDEResId(RID_STR_OB_INOUT_PARAM);
                sModifiers.append(sModifiers.getLength() ? (u", "_ustr + sInOut) : sInOut);
            }
            if (!param.bIsByVal)
            {
                OUString sByRef = IDEResId(RID_STR_OB_BYREF);
                sModifiers.append(sModifiers.getLength() ? (u", "_ustr + sByRef) : sByRef);
            }
            if (sModifiers.getLength() > 0)
            {
                sDescription.append(u" [" + sModifiers.makeStringAndClear() + u"]");
            }
            if (param.osDefaultValueExpression.has_value())
            {
                sDescription.append(u" = " + param.osDefaultValueExpression.value());
            }
            sDescription.append(u"\n");
        }
    }

    sDescription.append(
        u"\n━━━ " + OUString::Concat(o3tl::trim(IDEResId(RID_STR_OB_LOCATION_HEADER))) + u" ━━━\n");

    if (!rSymbol.sParentName.isEmpty())
    {
        sDescription.append(u"📂 " + IDEResId(RID_STR_OB_MEMBER_OF) + rSymbol.sParentName + u"\n");
    }
    if (!rSymbol.sOriginLibrary.isEmpty())
    {
        sDescription.append(u"📚 " + IDEResId(RID_STR_OB_LIBRARY) + rSymbol.sOriginLibrary + u"\n");
    }
    if (!rSymbol.sOriginLocation.isEmpty())
    {
        sDescription.append(u"📄 " + IDEResId(RID_STR_OB_DOCUMENT) + rSymbol.sOriginLocation
                            + u"\n");
    }
    if (rSymbol.nSourceLine > 0)
    {
        sDescription.append(u"📍 " + IDEResId(RID_STR_OB_LINE)
                            + OUString::number(rSymbol.nSourceLine) + u"\n");
    }
    if (!rSymbol.sQualifiedName.isEmpty() && rSymbol.sQualifiedName != rSymbol.sName)
    {
        sDescription.append(u"\n━━━ "
                            + OUString::Concat(o3tl::trim(IDEResId(RID_STR_OB_FULLNAME_HEADER)))
                            + u" ━━━\n");
        sDescription.append(rSymbol.sQualifiedName + u"\n");
    }

    return sDescription.makeStringAndClear();
}

OUString FormatContainerSignature(const IdeSymbolInfo& rSymbol,
                                  IdeDataProviderInterface* pDataProvider)
{
    if (!pDataProvider)
    {
        return OUString();
    }

    rtl::OUStringBuffer sInfo;
    sInfo.append(u"━━━ " + GetSymbolTypeDescription(rSymbol.eKind) + u" ━━━\n\n📌 " + rSymbol.sName
                 + u"\n\n");

    if (ShouldShowMembers(rSymbol))
    {
        GroupedSymbolInfoList aMembers = pDataProvider->GetMembers(rSymbol);
        size_t nTotalMembers = 0;
        for (const auto& pair : aMembers)
            nTotalMembers += pair.second.size();

        sInfo.append(IDEResId(RID_STR_OB_CONTENTS_HEADER) + IDEResId(RID_STR_OB_TOTAL_MEMBERS)
                     + OUString::number(static_cast<sal_Int64>(nTotalMembers)) + u"\n\n");

        for (const auto& pair : aMembers)
        {
            sInfo.append(u"  • " + GetGroupNameForKind(pair.first) + u": "
                         + OUString::number(static_cast<sal_Int64>(pair.second.size())) + u"\n");
        }
    }
    else if (IsExpandable(rSymbol))
    {
        auto aChildren = pDataProvider->GetChildNodes(rSymbol);
        sal_Int64 nChildren = static_cast<sal_Int64>(aChildren.size());
        sInfo.append(
            IDEResId(RID_STR_OB_CONTENTS_HEADER)
            + IDEResId(RID_STR_OB_CONTAINS_ITEMS).replaceFirst(u"%1", OUString::number(nChildren)));
    }

    if (!rSymbol.sQualifiedName.isEmpty() && rSymbol.sQualifiedName != rSymbol.sName)
    {
        sInfo.append(IDEResId(RID_STR_OB_FULLNAME_HEADER) + rSymbol.sQualifiedName + u"\n");
    }

    if (!rSymbol.sOriginLibrary.isEmpty() || !rSymbol.sOriginLocation.isEmpty())
    {
        sInfo.append(IDEResId(RID_STR_OB_LOCATION_HEADER));
        if (!rSymbol.sOriginLibrary.isEmpty())
        {
            sInfo.append(IDEResId(RID_STR_OB_LIBRARY) + rSymbol.sOriginLibrary + u"\n");
        }
        if (!rSymbol.sOriginLocation.isEmpty())
        {
            sInfo.append(IDEResId(RID_STR_OB_DOCUMENT) + rSymbol.sOriginLocation + u"\n");
        }
    }

    return sInfo.makeStringAndClear();
}

// Helper to add a symbol entry to a tree view and its corresponding data stores.
void AddEntry(weld::TreeView& rTargetTree, std::vector<std::shared_ptr<IdeSymbolInfo>>& rStore,
              std::map<OUString, std::shared_ptr<IdeSymbolInfo>>& rIndex,
              const weld::TreeIter* pParent, const std::shared_ptr<IdeSymbolInfo>& pSymbol,
              bool bChildrenOnDemand, weld::TreeIter* pRetIter = nullptr)
{
    if (!pSymbol)
        return;

    if (pSymbol->sName.isEmpty())
    {
        SAL_WARN("basctl", "AddEntry - Symbol with empty name. ID: " << pSymbol->sIdentifier);
        return;
    }

    OUString sId = pSymbol->sIdentifier;
    if (pSymbol->eKind == IdeSymbolKind::PLACEHOLDER)
    {
        sId = u"placeholder_for:"_ustr + (pParent ? rTargetTree.get_id(*pParent) : u"root"_ustr);
        pSymbol->sIdentifier = sId;
    }

    if (sId.isEmpty())
    {
        SAL_WARN("basctl", "AddEntry - Symbol with empty ID. Name: " << pSymbol->sName);
        return;
    }

    rStore.push_back(pSymbol);
    rIndex[sId] = pSymbol;

    std::unique_ptr<weld::TreeIter> xLocalIter;
    if (!pRetIter)
    {
        xLocalIter = rTargetTree.make_iterator();
        pRetIter = xLocalIter.get();
    }

    rTargetTree.insert(pParent, -1, &pSymbol->sName, &sId, nullptr, nullptr, bChildrenOnDemand,
                       pRetIter);
    OUString sIconName = GetIconForSymbol(pSymbol->eKind);
    rTargetTree.set_image(*pRetIter, sIconName, -1);
}

OUString BuildDoxygenUrl(const IdeSymbolInfo& rSymbol)
{
    if (rSymbol.sQualifiedName.isEmpty())
        return OUString();

    if (rSymbol.sQualifiedName.startsWith("ooo.vba."))
    {
        SAL_INFO("basctl", "BuildDoxygenUrl: Skipping VBA type '" << rSymbol.sQualifiedName << "'");
        return OUString();
    }
    // Doxygen mangles names by replacing '.' with "_1_1"
    OUString sMangledName;
    OUString sTypePrefix;
    OUString sAnchor;

    switch (rSymbol.eKind)
    {
        // Case 1: Types that have their OWN dedicated .html page
        case IdeSymbolKind::UNO_STRUCT:
            sTypePrefix = u"struct"_ustr;
            sMangledName = rSymbol.sQualifiedName.replaceAll(".", "_1_1");
            break;
        case IdeSymbolKind::UNO_INTERFACE:
            sTypePrefix = u"interface"_ustr;
            sMangledName = rSymbol.sQualifiedName.replaceAll(".", "_1_1");
            break;
        case IdeSymbolKind::UNO_SERVICE:
            sTypePrefix = u"service"_ustr;
            sMangledName = rSymbol.sQualifiedName.replaceAll(".", "_1_1");
            break;
        case IdeSymbolKind::UNO_EXCEPTION:
            sTypePrefix = u"exception"_ustr;
            sMangledName = rSymbol.sQualifiedName.replaceAll(".", "_1_1");
            break;
        case IdeSymbolKind::UNO_NAMESPACE:
            sTypePrefix = u"namespace"_ustr;
            sMangledName = rSymbol.sQualifiedName.replaceAll(".", "_1_1");
            break;

        // Case 2: Types that are documented as ANCHORS on their parent namespace page
        case IdeSymbolKind::UNO_ENUM:
        case IdeSymbolKind::UNO_CONSTANTS:
        case IdeSymbolKind::UNO_TYPEDEF:
        {
            if (rSymbol.sParentName.isEmpty())
            {
                return OUString();
            }
            sTypePrefix = u"namespace"_ustr;
            sMangledName = rSymbol.sParentName.replaceAll(".", "_1_1");
            sAnchor = u"#"_ustr + rSymbol.sName;
            break;
        }
        // This symbol kind does not have a Doxygen page
        default:
            return OUString();
    }

    if (sMangledName.isEmpty() || sTypePrefix.isEmpty())
    {
        return OUString();
    }

    return u"https://api.libreoffice.org/docs/idl/ref/"_ustr + sTypePrefix + sMangledName
           + u".html"_ustr + sAnchor;
}

void ShowDocsError(vcl::Window* pParent, const OUString& sPrimaryText,
                   const OUString& sSecondaryText = OUString())
{
    if (!pParent)
        return;

    std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(
        pParent->GetFrameWeld(), VclMessageType::Warning, VclButtonsType::Ok, sPrimaryText));
    xBox->set_title(IDEResId(RID_STR_OB_DOCS_ERROR_TITLE));

    if (!sSecondaryText.isEmpty())
    {
        xBox->set_secondary_text(sSecondaryText);
    }
    xBox->run();
}

void OpenDoxygenDocumentation(vcl::Window* pParent, const IdeSymbolInfo& rSymbol)
{
    OUString sUrl = BuildDoxygenUrl(rSymbol);
    SAL_INFO("basctl", "OpenDoxygenDocumentation: Built URL='" << sUrl << "' for symbol '"
                                                               << rSymbol.sName << "'");
    if (sUrl.isEmpty())
    {
        ShowDocsError(pParent, IDEResId(RID_STR_OB_NO_DOCUMENTATION));
        return;
    }

    try
    {
        css::uno::Reference<css::uno::XComponentContext> xContext
            = comphelper::getProcessComponentContext();
        css::uno::Reference<css::lang::XMultiComponentFactory> xServiceManager
            = xContext->getServiceManager();

        css::uno::Reference<css::system::XSystemShellExecute> xSystemShell(
            xServiceManager->createInstanceWithContext(
                u"com.sun.star.system.SystemShellExecute"_ustr, xContext),
            css::uno::UNO_QUERY_THROW);

        xSystemShell->execute(sUrl, OUString(), css::system::SystemShellExecuteFlags::URIS_ONLY);
    }
    catch (const css::uno::Exception& e)
    {
        SAL_WARN("basctl", "Failed to open Doxygen documentation: " << e.Message);

        OUString sSecondaryMsg = u"URL: "_ustr + sUrl + u"\nError: "_ustr + e.Message;
        ShowDocsError(pParent, IDEResId(RID_STR_OB_BROWSER_LAUNCH_FAILED), sSecondaryMsg);
    }
}

} // anonymous namespace

ObjectBrowser::ObjectBrowser(Shell& rShell, vcl::Window* pParent)
    : basctl::DockingWindow(pParent, u"modules/BasicIDE/ui/objectbrowser.ui"_ustr,
                            u"ObjectBrowser"_ustr)
    , m_pShell(&rShell)
    , m_pDataProvider(std::make_unique<IdeDataProvider>())
    , m_bDisposed(false)
    , m_eInitState(ObjectBrowserInitState::NotInitialized)
    , m_bDataMayBeStale(true)
    , m_pDocNotifier(std::make_unique<DocumentEventNotifier>(*this))
{
    SetText(IDEResId(RID_STR_OBJECT_BROWSER));
    SetBackground(GetSettings().GetStyleSettings().GetWindowColor());
    EnableInput(true, true);
}

ObjectBrowser::~ObjectBrowser() { disposeOnce(); }

void ObjectBrowser::Initialize()
{
    if (m_eInitState != ObjectBrowserInitState::NotInitialized)
        return;

    m_eInitState = ObjectBrowserInitState::Initializing;

    // Handles to all our widgets
    m_xScopeSelector = m_xBuilder->weld_combo_box(u"ScopeSelector"_ustr);
    m_pFilterBox = m_xBuilder->weld_entry(u"FilterBox"_ustr);
    m_xLeftTreeView = m_xBuilder->weld_tree_view(u"LeftTreeView"_ustr);
    m_xRightMembersView = m_xBuilder->weld_tree_view(u"RightMembersView"_ustr);
    m_xDetailPane = m_xBuilder->weld_text_view(u"DetailPane"_ustr);
    m_xStatusLabel = m_xBuilder->weld_label(u"StatusLabel"_ustr);
    m_xRightPaneHeaderLabel = m_xBuilder->weld_label(u"RightPaneHeaderLabel"_ustr);
    m_xBackButton = m_xBuilder->weld_button(u"BackButton"_ustr);
    m_xForwardButton = m_xBuilder->weld_button(u"ForwardButton"_ustr);
    m_xClearSearchButton = m_xBuilder->weld_button(u"ClearSearchButton"_ustr);

    m_pSearchHandler = std::make_unique<ObjectBrowserSearch>(*this);
    m_pSearchHandler->Initialize();

    if (m_xScopeSelector)
    {
        m_xScopeSelector->append(u"ALL_LIBRARIES"_ustr, IDEResId(RID_STR_OB_SCOPE_ALL));
        m_xScopeSelector->append(u"CURRENT_DOCUMENT"_ustr, IDEResId(RID_STR_OB_SCOPE_CURRENT));
        m_xScopeSelector->set_active(0);
        m_xScopeSelector->connect_changed(LINK(this, ObjectBrowser, OnScopeChanged));
    }

    if (m_xLeftTreeView)
    {
        m_xLeftTreeView->connect_selection_changed(LINK(this, ObjectBrowser, OnLeftTreeSelect));
        m_xLeftTreeView->connect_expanding(LINK(this, ObjectBrowser, OnNodeExpand));
    }
    if (m_xRightMembersView)
    {
        m_xRightMembersView->connect_selection_changed(
            LINK(this, ObjectBrowser, OnRightTreeSelect));
        m_xRightMembersView->connect_expanding(LINK(this, ObjectBrowser, OnRightNodeExpand));
        m_xRightMembersView->connect_row_activated(
            LINK(this, ObjectBrowser, OnRightTreeDoubleClick));
    }

    if (m_xDetailPane)
    {
        vcl::Font aFont = m_xDetailPane->get_font();
        aFont.SetFamilyName(u"Monospace"_ustr);
        aFont.SetPitch(PITCH_FIXED);
        m_xDetailPane->set_font(aFont);
    }

    if (m_xBackButton)
        m_xBackButton->set_sensitive(false);
    if (m_xForwardButton)
        m_xForwardButton->set_sensitive(false);

    if (GetParent() && GetParent()->GetSystemWindow())
    {
        GetParent()->GetSystemWindow()->GetTaskPaneList()->AddWindow(this);
    }

    // Start listening for application-wide events like document activation
    StartListening(*SfxGetpApp());
    SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    if (pViewFrame)
    {
        if (SfxObjectShell* pObjShell = pViewFrame->GetObjectShell())
        {
            m_sLastActiveDocumentIdentifier = pObjShell->GetTitle();
            SAL_INFO("basctl", "ObjectBrowser::Initialize: Active document -> '"
                                   << m_sLastActiveDocumentIdentifier << "'");
        }
    }

    m_eInitState = ObjectBrowserInitState::Initialized;
}

void ObjectBrowser::dispose()
{
    if (m_bDisposed)
    {
        return;
    }
    m_bDisposed = true;

    m_eInitState = ObjectBrowserInitState::Disposed;

    if (GetParent() && GetParent()->GetSystemWindow())
    {
        if (auto* pTaskPaneList = GetParent()->GetSystemWindow()->GetTaskPaneList())
            pTaskPaneList->RemoveWindow(this);
    }

    // Stop listening to SFX events
    EndListening(*SfxGetpApp());

    // Disconnect all signals
    if (m_xScopeSelector)
        m_xScopeSelector->connect_changed(Link<weld::ComboBox&, void>());
    if (m_xLeftTreeView)
    {
        m_xLeftTreeView->connect_selection_changed(Link<weld::TreeView&, void>());
        m_xLeftTreeView->connect_expanding(Link<const weld::TreeIter&, bool>());
    }
    if (m_xRightMembersView)
    {
        m_xRightMembersView->connect_selection_changed(Link<weld::TreeView&, void>());
    }

    m_pDocNotifier->dispose();
    m_pDocNotifier.reset();
    m_pSearchHandler.reset();
    m_pDataProvider.reset();

    // Destroy widgets
    m_xScopeSelector.reset();
    m_pFilterBox.reset();
    m_xLeftTreeView.reset();
    m_xRightMembersView.reset();
    m_xDetailPane.reset();
    m_xStatusLabel.reset();
    m_xRightPaneHeaderLabel.reset();
    m_xBackButton.reset();
    m_xForwardButton.reset();
    m_xClearSearchButton.reset();

    DockingWindow::dispose();
}

bool ObjectBrowser::Close()
{
    Show(false);
    m_pShell->GetViewFrame().GetBindings().Invalidate(SID_BASICIDE_OBJECT_BROWSER);
    return false;
}

void ObjectBrowser::Show(bool bVisible)
{
    DockingWindow::Show(bVisible);
    if (!bVisible)
    {
        return;
    }

    if (m_eInitState == ObjectBrowserInitState::NotInitialized)
    {
        Initialize();
    }

    if (m_pDataProvider)
    {
        if (!m_pDataProvider->IsInitialized())
        {
            ShowLoadingState();
            IdeTimer aTotalInitTimer("ObjectBrowser::FullInitialization");
            weld::WaitObject aWait(GetFrameWeld());
            m_pDataProvider->Initialize();
            m_bDataMayBeStale = true;

            RefreshUI();
            m_bDataMayBeStale = false;

            if (!m_bFirstLoadComplete)
            {
                m_bFirstLoadComplete = true;
                double fElapsedSeconds = aTotalInitTimer.getElapsedTimeMs() / 1000.0;
                OUString sStatus = IDEResId(RID_STR_OB_READY_LOADED)
                                       .replaceFirst(u"%1", OUString::number(fElapsedSeconds));
                m_xStatusLabel->set_label(sStatus);
            }
        }
        else if (m_bDataMayBeStale)
        {
            RefreshUI();
            m_bDataMayBeStale = false;
        }
    }
}

void ObjectBrowser::RefreshUI(bool /*bForceKeepUno*/)
{
    IdeTimer aTimer(u"ObjectBrowser::RefreshUI"_ustr);

    if (m_bPerformingAction)
    {
        SAL_INFO("basctl", "ObjectBrowser::RefreshUI: Blocked because an action is in progress.");
        return;
    }

    if (!m_pDataProvider || !m_pDataProvider->IsInitialized())
    {
        ShowLoadingState();
        return;
    }

    m_xLeftTreeView->freeze();
    m_xRightMembersView->freeze();
    ClearLeftTreeView();
    ClearRightTreeView();

    static_cast<IdeDataProvider*>(m_pDataProvider.get())->RefreshDocumentNodes();

    if (m_xRightPaneHeaderLabel)
    {
        m_xRightPaneHeaderLabel->set_label(IDEResId(RID_STR_OB_GROUP_MEMBERS));
    }

    UpdateDetailsPane(nullptr, false);

    // Get the filtered list of nodes based on the current scope
    SymbolInfoList aTopLevelNodes = m_pDataProvider->GetTopLevelNodes();
    for (const auto& pSymbol : aTopLevelNodes)
    {
        if (pSymbol)
        {
            AddEntry(*m_xLeftTreeView, m_aLeftTreeSymbolStore, m_aLeftTreeSymbolIndex, nullptr,
                     pSymbol, IsExpandable(*pSymbol));
        }
    }

    m_xLeftTreeView->thaw();
    m_xRightMembersView->thaw();

    if (!m_bFirstLoadComplete && m_xStatusLabel)
        m_xStatusLabel->set_label(IDEResId(RID_STR_OB_READY));
}

void ObjectBrowser::Notify(SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    if (m_bDisposed)
    {
        return;
    }

    const SfxEventHint* pEventHint = dynamic_cast<const SfxEventHint*>(&rHint);
    if (!pEventHint || pEventHint->GetEventId() != SfxEventHintId::ActivateDoc)
    {
        return;
    }

    rtl::Reference<SfxObjectShell> pObjShell = pEventHint->GetObjShell();
    if (!pObjShell)
    {
        return;
    }

    OUString sServiceName = pObjShell->GetFactory().GetDocumentServiceName();

    // Filter out BasicIDE and HTML/Web documents (Doxygen docs)
    if (sServiceName.endsWith(u"BasicIDE") || sServiceName.endsWith(u"WebDocument"))
    {
        SAL_INFO("basctl", "ObjectBrowser::Notify: Ignoring BasicIDE & WebDocument activation");
        return;
    }

    OUString sNewDocTitle = pObjShell->GetTitle();
    if (sNewDocTitle != m_sLastActiveDocumentIdentifier)
    {
        m_sLastActiveDocumentIdentifier = sNewDocTitle;
        SAL_INFO("basctl", "ObjectBrowser::Notify: Document activated -> '" << sNewDocTitle << "'");

        // If the user is in "Current Document" mode, a focus change
        // should trigger an immediate refresh to reflect the new context.
        if (IsVisible() && m_xScopeSelector
            && m_xScopeSelector->get_active_id() == "CURRENT_DOCUMENT")
        {
            SAL_INFO("basctl",
                     "ObjectBrowser::Notify: In CURRENT_DOCUMENT scope, updating scope to '"
                         << sNewDocTitle << "' and refreshing UI.");

            // Update scope to newly activated document
            m_pDataProvider->SetScope(IdeBrowserScope::CURRENT_DOCUMENT, sNewDocTitle);
            ScheduleRefresh();
        }
    }
}

void ObjectBrowser::ShowLoadingState()
{
    if (m_xLeftTreeView)
    {
        m_xLeftTreeView->freeze();
        ClearLeftTreeView();
        auto pLoadingNode = std::make_shared<IdeSymbolInfo>(u"[Initializing...]",
                                                            IdeSymbolKind::PLACEHOLDER, u"");
        AddEntry(*m_xLeftTreeView, m_aLeftTreeSymbolStore, m_aLeftTreeSymbolIndex, nullptr,
                 pLoadingNode, false);
        m_xLeftTreeView->thaw();
    }
    if (m_xStatusLabel)
    {
        m_xStatusLabel->set_label(u"Initializing Object Browser..."_ustr);
    }
}

void ObjectBrowser::ClearLeftTreeView()
{
    if (m_xLeftTreeView)
        m_xLeftTreeView->clear();
    m_aLeftTreeSymbolStore.clear();
    m_aLeftTreeSymbolIndex.clear();
}

void ObjectBrowser::ClearRightTreeView()
{
    if (m_xRightMembersView)
        m_xRightMembersView->clear();
    m_aRightTreeSymbolStore.clear();
    m_aRightTreeSymbolIndex.clear();
}

void ObjectBrowser::ScheduleRefresh()
{
    if (m_bPerformingAction)
    {
        SAL_INFO("basctl",
                 "ScheduleRefresh: Blocked refresh request because an action is in progress.");
        return;
    }

    m_bDataMayBeStale = true;

    if (IsVisible())
    {
        RefreshUI();
    }
}

void ObjectBrowser::UpdateDetailsPane(const IdeSymbolInfo* pSymbol, bool bIsContainer)
{
    if (!m_xDetailPane)
    {
        return;
    }

    if (pSymbol && pSymbol->eKind != IdeSymbolKind::PLACEHOLDER && pSymbol->bSelectable)
    {
        if (bIsContainer)
        {
            m_xDetailPane->set_text(FormatContainerSignature(*pSymbol, m_pDataProvider.get()));
        }
        else
        {
            m_xDetailPane->set_text(FormatSymbolSignature(*pSymbol));
        }
    }
    else
    {
        m_xDetailPane->set_text(u""_ustr);
    }
}

void ObjectBrowser::UpdateStatusBar(const IdeSymbolInfo* pLeftSymbol,
                                    const IdeSymbolInfo* pRightSymbol)
{
    if (!m_xStatusLabel)
    {
        return;
    }
    OUString sStatusText;

    if (pRightSymbol)
    {
        if (pRightSymbol->eKind == IdeSymbolKind::PLACEHOLDER)
        {
            sStatusText = pRightSymbol->sName;
        }
        else
        {
            sStatusText = GetSymbolTypeDescription(pRightSymbol->eKind);
        }
    }
    else if (pLeftSymbol)
    {
        if (ShouldShowMembers(*pLeftSymbol))
        {
            GroupedSymbolInfoList aMembers = m_pDataProvider->GetMembers(*pLeftSymbol);
            size_t nTotalMembers = 0;
            for (const auto& pair : aMembers)
                nTotalMembers += pair.second.size();
            sStatusText
                = IDEResId(RID_STR_OB_MEMBERS_COUNT)
                      .replaceFirst(u"%1", OUString::number(static_cast<sal_Int64>(nTotalMembers)));
        }
        else if (IsExpandable(*pLeftSymbol))
        {
            auto aChildren = m_pDataProvider->GetChildNodes(*pLeftSymbol);
            sStatusText
                = IDEResId(RID_STR_OB_ITEMS_COUNT)
                      .replaceFirst(u"%1",
                                    OUString::number(static_cast<sal_Int64>(aChildren.size())));
        }
        else
        {
            sStatusText = GetSymbolTypeDescription(pLeftSymbol->eKind);
        }
    }
    else if (m_bFirstLoadComplete)
    {
        sStatusText = m_xStatusLabel->get_label();
    }
    else
    {
        sStatusText = IDEResId(RID_STR_OB_READY);
    }
    m_xStatusLabel->set_label(sStatusText);
}

void ObjectBrowser::NavigateToMacroSource(const IdeSymbolInfo& rSymbol)
{
    SAL_INFO("basctl", "NavigateToMacroSource: Entry - Library='"
                           << rSymbol.sOriginLibrary << "', Module='" << rSymbol.sOriginModule
                           << "', Method='" << rSymbol.sName << "'");

    if (!m_pShell || rSymbol.sOriginLibrary.isEmpty() || rSymbol.sOriginModule.isEmpty())
    {
        SAL_WARN("basctl", "NavigateToMacroSource: Invalid parameters.");
        return;
    }

    ScriptDocument aDoc
        = rSymbol.sOriginLocation.isEmpty()
              ? ScriptDocument::getApplicationScriptDocument()
              : ScriptDocument::getDocumentWithURLOrCaption(rSymbol.sOriginLocation);

    if (aDoc.isAlive())
    {
        SAL_INFO("basctl",
                 "NavigateToMacroSource: Document is alive, dispatching SID_BASICIDE_SHOWSBX.");

        // Pass the method name so it navigates to the exact line
        SbxItem aSbxItem(SID_BASICIDE_ARG_SBX, std::move(aDoc), rSymbol.sOriginLibrary,
                         rSymbol.sOriginModule, rSymbol.sName, basctl::SBX_TYPE_METHOD);

        SfxViewFrame& rViewFrame = m_pShell->GetViewFrame();
        if (SfxDispatcher* pDispatcher = rViewFrame.GetDispatcher())
        {
            std::ignore = pDispatcher->ExecuteList(SID_BASICIDE_SHOWSBX, SfxCallMode::SYNCHRON,
                                                   { &aSbxItem });
            SAL_INFO("basctl", "NavigateToMacroSource: Dispatched successfully.");
        }
        else
        {
            SAL_WARN("basctl", "NavigateToMacroSource: Could not get dispatcher.");
        }
    }
    else
    {
        SAL_WARN("basctl", "NavigateToMacroSource: ScriptDocument is not alive.");
    }
}

void ObjectBrowser::PopulateMembersPane(const IdeSymbolInfo& rSymbol)
{
    if (!m_xRightMembersView)
    {
        return;
    }

    m_xRightMembersView->freeze();
    ClearRightTreeView();

    if (m_xRightPaneHeaderLabel)
    {
        m_xRightPaneHeaderLabel->set_label(u"Members of: "_ustr + rSymbol.sName);
    }
    GroupedSymbolInfoList aGroupedMembers = m_pDataProvider->GetMembers(rSymbol);
    std::vector<std::unique_ptr<weld::TreeIter>> aGroupItersToExpand;

    for (const auto& rPair : aGroupedMembers)
    {
        if (rPair.second.empty())
        {
            continue;
        }
        OUString sGroupName = GetGroupNameForKind(rPair.first);
        auto pGroupNode
            = std::make_shared<IdeSymbolInfo>(sGroupName, IdeSymbolKind::PLACEHOLDER, u"");
        pGroupNode->bSelectable = false;
        auto xGroupIter = m_xRightMembersView->make_iterator();
        AddEntry(*m_xRightMembersView, m_aRightTreeSymbolStore, m_aRightTreeSymbolIndex, nullptr,
                 pGroupNode, true, xGroupIter.get());

        for (const auto& pMemberInfo : rPair.second)
        {
            bool bHasNestedMembers = !pMemberInfo->mapMembers.empty();

            AddEntry(*m_xRightMembersView, m_aRightTreeSymbolStore, m_aRightTreeSymbolIndex,
                     xGroupIter.get(), pMemberInfo, bHasNestedMembers);
        }
        aGroupItersToExpand.push_back(std::move(xGroupIter));
    }

    m_xRightMembersView->thaw();

    for (const auto& xGroupIter : aGroupItersToExpand)
    {
        m_xRightMembersView->expand_row(*xGroupIter);
    }
}

// Document Event Handlers
void ObjectBrowser::onDocumentCreated(const ScriptDocument&) { ScheduleRefresh(); }
void ObjectBrowser::onDocumentOpened(const ScriptDocument&) { ScheduleRefresh(); }
void ObjectBrowser::onDocumentSave(const ScriptDocument&) { /* STUB */}
void ObjectBrowser::onDocumentSaveDone(const ScriptDocument&) { ScheduleRefresh(); }
void ObjectBrowser::onDocumentSaveAs(const ScriptDocument&) { /* STUB */}
void ObjectBrowser::onDocumentSaveAsDone(const ScriptDocument&) { ScheduleRefresh(); }
void ObjectBrowser::onDocumentClosed(const ScriptDocument&) { ScheduleRefresh(); }
void ObjectBrowser::onDocumentTitleChanged(const ScriptDocument&) { ScheduleRefresh(); }
void ObjectBrowser::onDocumentModeChanged(const ScriptDocument&) { /* STUB */}

IMPL_LINK(ObjectBrowser, OnLeftTreeSelect, weld::TreeView&, rTree, void)
{
    if (m_bDisposed)
    {
        return;
    }

    std::unique_ptr<weld::TreeIter> xSelectedIter = rTree.get_selected();
    if (!xSelectedIter)
    {
        UpdateStatusBar(nullptr, nullptr);
        UpdateDetailsPane(nullptr, false);
        return;
    }

    auto pSymbol = GetSymbolForIter(*xSelectedIter, rTree, m_aLeftTreeSymbolIndex);
    if (!pSymbol)
    {
        return;
    }

    // A selection in the left pane is always a container
    UpdateDetailsPane(pSymbol.get(), true);
    UpdateStatusBar(pSymbol.get(), nullptr);

    ClearRightTreeView();

    if (ShouldShowMembers(*pSymbol))
    {
        PopulateMembersPane(*pSymbol);
    }
}

IMPL_LINK(ObjectBrowser, OnRightTreeSelect, weld::TreeView&, rTree, void)
{
    if (m_bDisposed)
    {
        return;
    }

    std::unique_ptr<weld::TreeIter> xLeftIter = m_xLeftTreeView->get_selected();
    std::shared_ptr<const IdeSymbolInfo> pLeftSymbol
        = xLeftIter ? GetSymbolForIter(*xLeftIter, *m_xLeftTreeView, m_aLeftTreeSymbolIndex)
                    : nullptr;

    std::unique_ptr<weld::TreeIter> xRightIter = rTree.get_selected();
    if (!xRightIter)
    {
        // Revert to showing container info if right-pane selection is cleared
        UpdateStatusBar(pLeftSymbol.get(), nullptr);
        UpdateDetailsPane(pLeftSymbol.get(), true);
        return;
    }

    auto pRightSymbol = GetSymbolForIter(*xRightIter, rTree, m_aRightTreeSymbolIndex);
    if (!pRightSymbol)
    {
        return;
    }

    // We create a temporary copy to correctly populate the parent name for display
    IdeSymbolInfo tempSymbol = *pRightSymbol;
    if (pLeftSymbol)
    {
        tempSymbol.sParentName = pLeftSymbol->sName;
    }

    UpdateDetailsPane(&tempSymbol, false);
    UpdateStatusBar(pLeftSymbol.get(), pRightSymbol.get());
}

IMPL_LINK(ObjectBrowser, OnRightNodeExpand, const weld::TreeIter&, rParentIter, bool)
{
    if (m_bDisposed)
    {
        return false;
    }

    if (m_xRightMembersView->iter_has_child(rParentIter))
    {
        return true;
    }

    auto pParentSymbol
        = GetSymbolForIter(rParentIter, *m_xRightMembersView, m_aRightTreeSymbolIndex);
    if (!pParentSymbol)
    {
        return false;
    }

    for (const auto& pair : pParentSymbol->mapMembers)
    {
        const auto& pChildSymbol = pair.second;
        if (pChildSymbol)
        {
            // Check if the child is also expandable
            bool bChildHasChildren = !pChildSymbol->mapMembers.empty();
            AddEntry(*m_xRightMembersView, m_aRightTreeSymbolStore, m_aRightTreeSymbolIndex,
                     &rParentIter, pChildSymbol, bChildHasChildren);
        }
    }

    return true;
}

IMPL_LINK(ObjectBrowser, OnRightTreeDoubleClick, weld::TreeView&, rTree, bool)
{
    SAL_INFO("basctl", "OnRightTreeDoubleClick: Handler entered.");

    if (m_bDisposed)
    {
        SAL_WARN("basctl", "OnRightTreeDoubleClick: Browser is disposed, aborting.");
        return false;
    }

    std::unique_ptr<weld::TreeIter> xSelectedIter = rTree.get_selected();
    if (!xSelectedIter)
    {
        SAL_INFO("basctl", "OnRightTreeDoubleClick: No item selected.");
        return false;
    }

    auto pSymbol = GetSymbolForIter(*xSelectedIter, rTree, m_aRightTreeSymbolIndex);
    if (!pSymbol || pSymbol->eKind == IdeSymbolKind::PLACEHOLDER)
    {
        SAL_INFO("basctl",
                 "OnRightTreeDoubleClick: Selected item is a placeholder or has no symbol.");
        return false;
    }

    m_bPerformingAction = true;
    SAL_INFO("basctl", "OnRightTreeDoubleClick: Processing symbol '"
                           << pSymbol->sName << "' of kind " << static_cast<int>(pSymbol->eKind));

    // Symbol is a BASIC Macro
    if (pSymbol->eKind == IdeSymbolKind::SUB || pSymbol->eKind == IdeSymbolKind::FUNCTION)
    {
        SAL_INFO("basctl", "OnRightTreeDoubleClick: Action is NavigateToMacroSource.");
        NavigateToMacroSource(*pSymbol);
        m_bPerformingAction = false;
        return true;
    }

    // The symbol itself is documentable
    OUString sSymbolUrl = BuildDoxygenUrl(*pSymbol);
    if (!sSymbolUrl.isEmpty())
    {
        SAL_INFO("basctl",
                 "OnRightTreeDoubleClick: Symbol is a documentable container. Opening its docs.");
        OpenDoxygenDocumentation(this, *pSymbol);
        m_bPerformingAction = false;
        return true;
    }

    // Find documentable parent in RIGHT tree first
    auto xParentIter = rTree.make_iterator(xSelectedIter.get());
    while (rTree.iter_parent(*xParentIter))
    {
        auto pParentSymbol = GetSymbolForIter(*xParentIter, rTree, m_aRightTreeSymbolIndex);
        if (pParentSymbol && pParentSymbol->eKind != IdeSymbolKind::PLACEHOLDER)
        {
            // If this parent has a qualified name it's documentable
            if (!pParentSymbol->sQualifiedName.isEmpty())
            {
                SAL_INFO("basctl",
                         "OnRightTreeDoubleClick: Action is OpenDoxygenDocumentation for parent '"
                             << pParentSymbol->sName << "' (from right pane).");
                OpenDoxygenDocumentation(this, *pParentSymbol);
                m_bPerformingAction = false;
                return true;
            }
            // Otherwise, keep walking up the tree
        }
    }

    // Find documentable parent in LEFT tree
    if (std::unique_ptr<weld::TreeIter> xLeftTreeParentIter = m_xLeftTreeView->get_selected())
    {
        auto pParentSymbol
            = GetSymbolForIter(*xLeftTreeParentIter, *m_xLeftTreeView, m_aLeftTreeSymbolIndex);
        if (pParentSymbol)
        {
            // Try to build URL for parent
            OUString sParentUrl = BuildDoxygenUrl(*pParentSymbol);

            if (sParentUrl.isEmpty())
            {
                auto xAncestorIter = m_xLeftTreeView->make_iterator(xLeftTreeParentIter.get());
                while (m_xLeftTreeView->iter_parent(*xAncestorIter))
                {
                    auto pAncestorSymbol = GetSymbolForIter(*xAncestorIter, *m_xLeftTreeView,
                                                            m_aLeftTreeSymbolIndex);
                    if (pAncestorSymbol)
                    {
                        if (pAncestorSymbol->sQualifiedName.startsWith("ooo.vba"))
                        {
                            SAL_INFO("basctl", "OnRightTreeDoubleClick: Ancestor is VBA namespace, "
                                               "skipping documentation.");
                            break;
                        }
                        OUString sAncestorUrl = BuildDoxygenUrl(*pAncestorSymbol);
                        if (!sAncestorUrl.isEmpty())
                        {
                            SAL_INFO(
                                "basctl",
                                "OnRightTreeDoubleClick: Parent has no docs, opening ancestor '"
                                    << pAncestorSymbol->sName << "'");
                            OpenDoxygenDocumentation(this, *pAncestorSymbol);
                            m_bPerformingAction = false;
                            return true;
                        }
                    }
                }
                SAL_INFO("basctl", "OnRightTreeDoubleClick: No documentable ancestor found for '"
                                       << pParentSymbol->sName << "'");
            }
            else
            {
                SAL_INFO("basctl",
                         "OnRightTreeDoubleClick: Action is OpenDoxygenDocumentation for parent '"
                             << pParentSymbol->sName << "' (from left pane).");
                OpenDoxygenDocumentation(this, *pParentSymbol);
                m_bPerformingAction = false;
                return true;
            }
        }
    }

    m_bPerformingAction = false;
    SAL_WARN("basctl", "OnRightTreeDoubleClick: No action taken for symbol.");
    ShowDocsError(this, IDEResId(RID_STR_OB_NO_DOCUMENTATION));

    return false;
}

IMPL_LINK(ObjectBrowser, OnNodeExpand, const weld::TreeIter&, rParentIter, bool)
{
    if (m_bDisposed || !m_pDataProvider)
    {
        return false;
    }

    auto pParentSymbol = GetSymbolForIter(rParentIter, *m_xLeftTreeView, m_aLeftTreeSymbolIndex);
    if (!pParentSymbol)
    {
        return false;
    }

    if (m_xLeftTreeView->iter_has_child(rParentIter))
    {
        return true;
    }

    const auto aAllChildren = m_pDataProvider->GetChildNodes(*pParentSymbol);
    if (aAllChildren.empty())
    {
        return true;
    }

    for (const auto& pChildInfo : aAllChildren)
    {
        if (pChildInfo)
        {
            AddEntry(*m_xLeftTreeView, m_aLeftTreeSymbolStore, m_aLeftTreeSymbolIndex, &rParentIter,
                     pChildInfo, IsExpandable(*pChildInfo));
        }
    }

    return true;
}

IMPL_LINK(ObjectBrowser, OnScopeChanged, weld::ComboBox&, rComboBox, void)
{
    if (m_bDisposed || !m_pDataProvider)
        return;

    OUString sSelectedId = rComboBox.get_active_id();
    IdeBrowserScope eScope = (sSelectedId == "ALL_LIBRARIES") ? IdeBrowserScope::ALL_LIBRARIES
                                                              : IdeBrowserScope::CURRENT_DOCUMENT;

    m_pDataProvider->SetScope(eScope, m_sLastActiveDocumentIdentifier);
    RefreshUI();
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
