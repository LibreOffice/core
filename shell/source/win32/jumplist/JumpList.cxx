/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <algorithm>
#include <cassert>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <o3tl/char16_t2wchar_t.hxx>
#include <o3tl/runtimetooustring.hxx>
#include <o3tl/safeCoInitUninit.hxx>
#include <osl/file.hxx>
#include <osl/mutex.hxx>
#include <osl/process.h>
#include <sal/log.hxx>
#include <systools/win32/comtools.hxx>

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/system/windows/JumpListItem.hpp>
#include <com/sun/star/system/windows/XJumpList.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/InvalidStateException.hpp>

#include <prewin.h>
#include <Shlobj.h>
#include <propkey.h>
#include <propvarutil.h>
#include <postwin.h>

using namespace comphelper;
using namespace cppu;
using namespace css;
using namespace css::uno;
using namespace css::lang;
using namespace css::system::windows;
using namespace css::util;
using namespace osl;
using namespace sal::systools;

namespace
{
class JumpListImpl : public BaseMutex, public WeakComponentImplHelper<XJumpList, XServiceInfo>
{
    Reference<XComponentContext> m_xContext;
    COMReference<ICustomDestinationList> m_aDestinationList;
    COMReference<IObjectArray> m_aRemoved;
    bool m_isListOpen;

public:
    explicit JumpListImpl(const Reference<XComponentContext>& xContext);

    // XJumpList
    virtual void SAL_CALL beginList(const OUString& sApplication) override;
    virtual void SAL_CALL appendCategory(const OUString& sCategory,
                                         const Sequence<JumpListItem>& aJumpListItems) override;
    virtual void SAL_CALL addTasks(const Sequence<JumpListItem>& aJumpListItems) override;
    virtual void SAL_CALL showRecentFiles() override;
    virtual void SAL_CALL showFrequentFiles() override;
    virtual void SAL_CALL commitList() override;
    virtual void SAL_CALL abortList() override;
    virtual void SAL_CALL deleteList(const OUString& sApplication) override;
    virtual Sequence<JumpListItem> SAL_CALL getRemovedItems(const OUString& sApplication) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};

JumpListImpl::JumpListImpl(const Reference<XComponentContext>& xContext)
    : WeakComponentImplHelper(m_aMutex)
    , m_xContext(xContext)
    , m_aDestinationList(CLSID_DestinationList, nullptr, CLSCTX_INPROC_SERVER)
    , m_isListOpen(false)
{
}

// Determines if the provided IShellLinkItem is listed in the array of items that the user has removed
bool lcl_isItemInArray(COMReference<IShellLinkW> pShellLinkItem,
                       COMReference<IObjectArray> poaRemoved)
{
    UINT nItems;
    ThrowIfFailed(poaRemoved->GetCount(&nItems), "GetCount failed.");

    COMReference<IShellLinkW> pShellLinkItemCompare;
    for (UINT i = 0; i < nItems; i++)
    {
        if (!SUCCEEDED(poaRemoved->GetAt(i, IID_PPV_ARGS(&pShellLinkItemCompare))))
            continue;

        PROPVARIANT propvar;
        COMReference<IPropertyStore> pps(pShellLinkItem, COM_QUERY_THROW);
        ThrowIfFailed(pps->GetValue(PKEY_Title, &propvar), "GetValue failed.");
        OUString title(o3tl::toU(PropVariantToStringWithDefault(propvar, L"")));

        COMReference<IPropertyStore> ppsCompare(pShellLinkItemCompare, COM_QUERY_THROW);
        ThrowIfFailed(ppsCompare->GetValue(PKEY_Title, &propvar), "GetValue failed.");
        OUString titleCompare(o3tl::toU(PropVariantToStringWithDefault(propvar, L"")));
        PropVariantClear(&propvar);

        if (title == titleCompare)
            return true;
    }

    return false;
}
}

void SAL_CALL JumpListImpl::beginList(const OUString& sApplication)
{
    if (m_isListOpen)
        throw InvalidStateException("There is already a list open. Close it with 'commitList'");

    if (sApplication != "Writer" && sApplication != "Calc" && sApplication != "Impress"
        && sApplication != "Draw" && sApplication != "Math" && sApplication != "Base"
        && sApplication != "Startcenter")
    {
        throw IllegalArgumentException(
            "Parameter 'application' must be one of 'Writer', 'Calc', 'Impress', 'Draw', "
            "'Math', 'Base', 'Startcenter'.",
            static_cast<OWeakObject*>(this), 1);
    }
    OUString sApplicationID("TheDocumentFoundation.LibreOffice." + sApplication);

    try
    {
        m_aDestinationList->SetAppID(o3tl::toW(sApplicationID.getStr()));

        UINT min_slots;

        ThrowIfFailed(m_aDestinationList->BeginList(&min_slots, IID_PPV_ARGS(&m_aRemoved)),
                      "BeginList failed");
        m_isListOpen = true;
    }
    catch (const ComError& e)
    {
        SAL_WARN("shell.jumplist", e.what());
    }
}

void SAL_CALL JumpListImpl::appendCategory(const OUString& sCategory,
                                           const Sequence<JumpListItem>& aJumpListItems)
{
    if (!m_isListOpen)
        throw InvalidStateException("No list open. Open it with 'beginList'");

    if (sCategory.isEmpty())
    {
        throw IllegalArgumentException("Parameter 'category' must not be empty",
                                       static_cast<OWeakObject*>(this), 1);
    }

    try
    {
        OUString sofficeURL;
        OUString sofficePath;
        oslProcessError err = osl_getExecutableFile(&sofficeURL.pData);
        FileBase::getSystemPathFromFileURL(sofficeURL, sofficePath);
        if (err != osl_Process_E_None)
        {
            SAL_WARN("shell.jumplist", "osl_getExecutableFile failed");
            return;
        }
        // We need to run soffice.exe, not soffice.bin
        sofficePath = sofficePath.replaceFirst("soffice.bin", "soffice.exe");

        COMReference<IObjectCollection> aCollection(CLSID_EnumerableObjectCollection, nullptr,
                                                    CLSCTX_INPROC_SERVER);

        for (auto const& item : aJumpListItems)
        {
            if (item.name.isEmpty())
                continue;
            try
            {
                COMReference<IShellLinkW> pShellLinkItem(CLSID_ShellLink, nullptr,
                                                         CLSCTX_INPROC_SERVER);

                {
                    COMReference<IPropertyStore> pps(pShellLinkItem, COM_QUERY_THROW);

                    PROPVARIANT propvar;
                    ThrowIfFailed(
                        InitPropVariantFromString(o3tl::toW(item.name.getStr()), &propvar),
                        "InitPropVariantFromString failed.");

                    ThrowIfFailed(pps->SetValue(PKEY_Title, propvar), "SetValue failed.");

                    ThrowIfFailed(pps->Commit(), "Commit failed.");

                    PropVariantClear(&propvar);
                }
                ThrowIfFailed(
                    pShellLinkItem->SetDescription(o3tl::toW(item.description.getStr())),
                    Concat2View("Setting description '" + item.description.toUtf8() + "' failed."));

                ThrowIfFailed(pShellLinkItem->SetPath(o3tl::toW(sofficePath.getStr())),
                              Concat2View("Setting path '" + sofficePath.toUtf8() + "' failed."));

                ThrowIfFailed(
                    pShellLinkItem->SetArguments(o3tl::toW(item.arguments.getStr())),
                    Concat2View("Setting arguments '" + item.arguments.toUtf8() + "' failed."));

                ThrowIfFailed(
                    pShellLinkItem->SetIconLocation(o3tl::toW(item.iconPath.getStr()), 0),
                    Concat2View("Setting icon path '" + item.iconPath.toUtf8() + "' failed."));

                if (lcl_isItemInArray(pShellLinkItem, m_aRemoved))
                {
                    SAL_INFO("shell.jumplist", "Ignoring item '"
                                                   << item.name
                                                   << "' (was removed by user). See output of "
                                                      "XJumpList::getRemovedItems().");
                    continue;
                }
                aCollection->AddObject(pShellLinkItem);
            }
            catch (const ComError& e)
            {
                SAL_WARN("shell.jumplist", e.what());
                continue;
            }
        }

        COMReference<IObjectArray> pObjectArray(aCollection, COM_QUERY_THROW);
        UINT nItems;
        ThrowIfFailed(pObjectArray->GetCount(&nItems), "GetCount failed.");
        if (nItems == 0)
        {
            throw IllegalArgumentException(
                "No valid items given. `jumpListItems` is either empty, or contains only items "
                "which were removed by the user. See `XJumpList::getRemovedItems()`.",
                static_cast<OWeakObject*>(this), 1);
        }

        ThrowIfFailed(
            m_aDestinationList->AppendCategory(o3tl::toW(sCategory.getStr()), pObjectArray),
            "AppendCategory failed.");
    }
    catch (const ComError& e)
    {
        SAL_WARN("shell.jumplist", e.what());
    }
}

void SAL_CALL JumpListImpl::addTasks(const Sequence<JumpListItem>& aJumpListItems)
{
    if (!m_isListOpen)
        throw InvalidStateException("No list open. Open it with 'beginList'");

    try
    {
        OUString sofficeURL;
        OUString sofficePath;
        oslProcessError err = osl_getExecutableFile(&sofficeURL.pData);
        FileBase::getSystemPathFromFileURL(sofficeURL, sofficePath);
        if (err != osl_Process_E_None)
        {
            SAL_WARN("shell.jumplist", "osl_getExecutableFile failed");
            return;
        }
        // We need to run soffice.exe, not soffice.bin
        sofficePath = sofficePath.replaceFirst("soffice.bin", "soffice.exe");

        COMReference<IObjectCollection> aCollection(CLSID_EnumerableObjectCollection, nullptr,
                                                    CLSCTX_INPROC_SERVER);

        for (auto const& item : aJumpListItems)
        {
            if (item.name.isEmpty())
                continue;
            try
            {
                COMReference<IShellLinkW> pShellLinkItem(CLSID_ShellLink, nullptr,
                                                         CLSCTX_INPROC_SERVER);

                {
                    COMReference<IPropertyStore> pps(pShellLinkItem, COM_QUERY_THROW);

                    PROPVARIANT propvar;
                    ThrowIfFailed(
                        InitPropVariantFromString(o3tl::toW(item.name.getStr()), &propvar),
                        "InitPropVariantFromString failed.");

                    ThrowIfFailed(pps->SetValue(PKEY_Title, propvar), "SetValue failed.");

                    ThrowIfFailed(pps->Commit(), "Commit failed.");

                    PropVariantClear(&propvar);
                }
                ThrowIfFailed(
                    pShellLinkItem->SetDescription(o3tl::toW(item.description.getStr())),
                    Concat2View("Setting description '" + item.description.toUtf8() + "' failed."));

                ThrowIfFailed(pShellLinkItem->SetPath(o3tl::toW(sofficePath.getStr())),
                              Concat2View("Setting path '" + sofficePath.toUtf8() + "' failed."));

                ThrowIfFailed(
                    pShellLinkItem->SetArguments(o3tl::toW(item.arguments.getStr())),
                    Concat2View("Setting arguments '" + item.arguments.toUtf8() + "' failed."));

                ThrowIfFailed(
                    pShellLinkItem->SetIconLocation(o3tl::toW(item.iconPath.getStr()), 0),
                    Concat2View("Setting icon path '" + item.iconPath.toUtf8() + "' failed."));

                aCollection->AddObject(pShellLinkItem);
            }
            catch (const ComError& e)
            {
                SAL_WARN("shell.jumplist", e.what());
                continue;
            }
        }

        COMReference<IObjectArray> pObjectArray(aCollection, COM_QUERY_THROW);
        UINT nItems;
        ThrowIfFailed(pObjectArray->GetCount(&nItems), "GetCount failed.");
        if (nItems == 0)
        {
            throw IllegalArgumentException("No valid items given. `jumpListItems` is empty.",
                                           static_cast<OWeakObject*>(this), 1);
        }

        ThrowIfFailed(m_aDestinationList->AddUserTasks(pObjectArray), "AddUserTasks failed.");
    }
    catch (const ComError& e)
    {
        SAL_WARN("shell.jumplist", e.what());
    }
}

void SAL_CALL JumpListImpl::showRecentFiles()
{
    if (!m_isListOpen)
        throw InvalidStateException("No list open. Open it with 'beginList'");

    try
    {
        ThrowIfFailed(m_aDestinationList->AppendKnownCategory(KDC_RECENT),
                      "AppendKnownCategory(KDC_RECENT) failed.");
    }
    catch (const ComError& e)
    {
        SAL_WARN("shell.jumplist", e.what());
    }
}

void SAL_CALL JumpListImpl::showFrequentFiles()
{
    if (!m_isListOpen)
        throw InvalidStateException("No list open. Open it with 'beginList'");

    try
    {
        ThrowIfFailed(m_aDestinationList->AppendKnownCategory(KDC_FREQUENT),
                      "AppendKnownCategory(KDC_FREQUENT) failed.");
    }
    catch (const ComError& e)
    {
        SAL_WARN("shell.jumplist", e.what());
    }
}

void SAL_CALL JumpListImpl::commitList()
{
    if (!m_isListOpen)
        throw InvalidStateException("No list open. Open it with 'beginList'");

    try
    {
        ThrowIfFailed(m_aDestinationList->CommitList(), "CommitList failed.");
        m_isListOpen = false;
    }
    catch (const ComError& e)
    {
        SAL_WARN("shell.jumplist", e.what());
    }
}

void SAL_CALL JumpListImpl::abortList()
{
    if (!m_isListOpen)
        throw InvalidStateException("No list open.");

    try
    {
        ThrowIfFailed(m_aDestinationList->AbortList(), "AbortList failed.");
        m_isListOpen = false;
    }
    catch (const ComError& e)
    {
        SAL_WARN("shell.jumplist", e.what());
    }
}

void SAL_CALL JumpListImpl::deleteList(const OUString& sApplication)
{
    if (m_isListOpen)
        throw InvalidStateException("You are in a list building session. Close it with "
                                    "'commitList', or abort with 'abortList'");

    if (sApplication != "Writer" && sApplication != "Calc" && sApplication != "Impress"
        && sApplication != "Draw" && sApplication != "Math" && sApplication != "Base"
        && sApplication != "Startcenter")
    {
        throw IllegalArgumentException(
            "Parameter 'application' must be one of 'Writer', 'Calc', 'Impress', 'Draw', "
            "'Math', 'Base', 'Startcenter'.",
            static_cast<OWeakObject*>(this), 1);
    }
    OUString sApplicationID("TheDocumentFoundation.LibreOffice." + sApplication);

    try
    {
        COMReference<ICustomDestinationList> aDestinationList(CLSID_DestinationList, nullptr,
                                                              CLSCTX_INPROC_SERVER);
        aDestinationList->DeleteList(o3tl::toW(sApplicationID.getStr()));
    }
    catch (const ComError& e)
    {
        SAL_WARN("shell.jumplist", e.what());
    }
}

Sequence<JumpListItem> SAL_CALL JumpListImpl::getRemovedItems(const OUString& sApplication)
{
    if (sApplication != "Writer" && sApplication != "Calc" && sApplication != "Impress"
        && sApplication != "Draw" && sApplication != "Math" && sApplication != "Base"
        && sApplication != "Startcenter")
    {
        throw IllegalArgumentException(
            "Parameter 'application' must be one of 'Writer', 'Calc', 'Impress', 'Draw', "
            "'Math', 'Base', 'Startcenter'.",
            static_cast<OWeakObject*>(this), 1);
    }
    OUString sApplicationID("TheDocumentFoundation.LibreOffice." + sApplication);

    std::vector<JumpListItem> removedItems;
    try
    {
        COMReference<ICustomDestinationList> aDestinationList(CLSID_DestinationList, nullptr,
                                                              CLSCTX_INPROC_SERVER);

        aDestinationList->SetAppID(o3tl::toW(sApplicationID.getStr()));

        COMReference<IObjectArray> removed;
        ThrowIfFailed(aDestinationList->GetRemovedDestinations(IID_PPV_ARGS(&removed)),
                      "GetRemovedDestinations failed");

        UINT removed_count;
        if (SUCCEEDED(removed->GetCount(&removed_count) && (removed_count > 0)))
        {
            JumpListItem item;
            COMReference<IShellLinkW> pShellLinkItem;
            for (UINT i = 0; i < removed_count; ++i)
            {
                if (SUCCEEDED(removed->GetAt(i, IID_PPV_ARGS(&pShellLinkItem))))
                {
                    COMReference<IPropertyStore> propertyStore(pShellLinkItem, COM_QUERY_THROW);
                    PROPVARIANT propvar;
                    ThrowIfFailed(propertyStore->GetValue(PKEY_Title, &propvar),
                                  "GetValue failed.");
                    item.name = o3tl::toU(PropVariantToStringWithDefault(propvar, L""));

                    ThrowIfFailed(propertyStore->GetValue(PKEY_Link_Arguments, &propvar),
                                  "GetValue failed.");
                    item.arguments = o3tl::toU(PropVariantToStringWithDefault(propvar, L""));
                    PropVariantClear(&propvar);

                    wchar_t itemDesc[MAX_PATH];
                    ThrowIfFailed(pShellLinkItem->GetDescription(
                                      itemDesc, std::extent<decltype(itemDesc)>::value),
                                  "GetDescription failed.");
                    item.description = o3tl::toU(itemDesc);

                    wchar_t path[MAX_PATH];
                    int icon_index;
                    ThrowIfFailed(pShellLinkItem->GetIconLocation(
                                      path, std::extent<decltype(path)>::value, &icon_index),
                                  "GetIconLocation failed.");
                    item.iconPath = o3tl::toU(path);

                    removedItems.emplace_back(item);
                }
            }
        }
    }
    catch (const ComError& e)
    {
        SAL_WARN("shell.jumplist", e.what());
    }

    return containerToSequence(removedItems);
}

// XServiceInfo

OUString SAL_CALL JumpListImpl::getImplementationName()
{
    return "com.sun.star.system.windows.JumpListImpl";
}

sal_Bool SAL_CALL JumpListImpl::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence<OUString> SAL_CALL JumpListImpl::getSupportedServiceNames()
{
    return { "com.sun.star.system.windows.JumpList" };
}

extern "C" SAL_DLLPUBLIC_EXPORT XInterface*
shell_JumpListExec_get_implementation(XComponentContext* context, Sequence<Any> const&)
{
    return acquire(new JumpListImpl(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
