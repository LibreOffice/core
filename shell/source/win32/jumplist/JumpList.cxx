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

#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/compbase.hxx>

#include <o3tl/char16_t2wchar_t.hxx>
#include <o3tl/runtimetooustring.hxx>
#include <o3tl/safeCoInitUninit.hxx>
#include <osl/file.hxx>
#include <osl/mutex.hxx>
#include <osl/process.h>
#include <sal/log.hxx>

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/system/windows/JumpListItem.hpp>
#include <com/sun/star/system/windows/XJumpList.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <prewin.h>
#include <Shlobj.h>
#include <propkey.h>
#include <propvarutil.h>
#include <systools/win32/comtools.hxx>
#include <postwin.h>

using namespace css;
using namespace css::uno;
using namespace css::lang;
using namespace css::system::windows;
using namespace sal::systools;

class JumpListImpl
    : public cppu::WeakComponentImplHelper<css::system::windows::XJumpList, css::lang::XServiceInfo>
{
    css::uno::Reference<css::uno::XComponentContext> m_xContext;
    osl::Mutex m_aMutex;

public:
    explicit JumpListImpl(const css::uno::Reference<css::uno::XComponentContext>& xContext);
    ~JumpListImpl();

    // XJumpList
    virtual void SAL_CALL
    appendCategory(const OUString& sCategory,
                   const css::uno::Sequence<css::system::windows::JumpListItem>& aJumpListItems,
                   const OUString& sDocumentService) override;
    virtual css::uno::Sequence<css::system::windows::JumpListItem>
        SAL_CALL getRemovedItems(const OUString& sDocumentService) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};

JumpListImpl::JumpListImpl(const css::uno::Reference<css::uno::XComponentContext>& xContext)
    : WeakComponentImplHelper<css::system::windows::XJumpList, css::lang::XServiceInfo>(m_aMutex)
    , m_xContext(xContext)
{
}

JumpListImpl::~JumpListImpl() {}

void SAL_CALL JumpListImpl::appendCategory(const OUString& sCategory,
                                           const Sequence<JumpListItem>& aJumpListItems,
                                           const OUString& sApplication)
{
    if (sCategory.isEmpty())
    {
        throw IllegalArgumentException("Parameter 'category' must not be empty",
                                       static_cast<cppu::OWeakObject*>(this), 1);
    }
    if (sApplication != "Writer" && sApplication != "Calc" && sApplication != "Impress"
        && sApplication != "Draw" && sApplication != "Math" && sApplication != "Base"
        && sApplication != "Startcenter")
    {
        throw IllegalArgumentException(
            "Parameter 'application' must be one of 'Writer', 'Calc', 'Impress', 'Draw', "
            "'Math', 'Base', 'Startcenter'.",
            static_cast<cppu::OWeakObject*>(this), 1);
    }
    OUString sApplicationID("TheDocumentFoundation.LibreOffice." + sApplication);

    try
    {
        COMReference<ICustomDestinationList> aDestinationList(CLSID_DestinationList, nullptr,
                                                              CLSCTX_INPROC_SERVER);

        aDestinationList->SetAppID(o3tl::toW(sApplicationID.getStr()));

        UINT min_slots;
        COMReference<IObjectArray> removed;
        ThrowIfFailed(aDestinationList->BeginList(&min_slots, IID_PPV_ARGS(&removed)),
                      "BeginList failed");

        OUString sofficeURL;
        OUString sofficePath;
        oslProcessError err = osl_getExecutableFile(&sofficeURL.pData);
        osl::FileBase::getSystemPathFromFileURL(sofficeURL, sofficePath);
        if (err != osl_Process_E_None)
        {
            SAL_WARN("shell.jumplist", "osl_getExecutableFile failed");
            return;
        }
        // We need to run soffice.exe, not soffice.bin
        sofficePath = sofficePath.replaceFirst("soffice.bin", "soffice.exe");

        COMReference<IObjectCollection> aCollection(CLSID_EnumerableObjectCollection, nullptr,
                                                    CLSCTX_INPROC_SERVER);

        for (auto item : aJumpListItems)
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
                    OString("Setting description '" + item.description.toUtf8() + "' failed."));

                ThrowIfFailed(pShellLinkItem->SetPath(o3tl::toW(sofficePath.getStr())),
                              OString("Setting path '" + sofficePath.toUtf8() + "' failed."));

                ThrowIfFailed(
                    pShellLinkItem->SetArguments(o3tl::toW(item.arguments.getStr())),
                    OString("Setting arguments '" + item.arguments.toUtf8() + "' failed."));

                ThrowIfFailed(
                    pShellLinkItem->SetIconLocation(o3tl::toW(item.iconPath.getStr()), 0),
                    OString("Setting icon path '" + item.iconPath.toUtf8() + "' failed."));

                aCollection->AddObject(pShellLinkItem);
            }
            catch (const ComError& e)
            {
                SAL_WARN("shell.jumplist", e.what());
                continue;
            }
        }

        COMReference<IObjectArray> pObjectArray(aCollection, COM_QUERY_THROW);
        ThrowIfFailed(
            aDestinationList->AppendCategory(o3tl::toW(sCategory.getStr()), pObjectArray),
            "AppendCategory failed. You are not allowed to immediately re-insert entries which "
            "were removed by the user. Please see the output of `getRemovedItems`.");

        ThrowIfFailed(aDestinationList->CommitList(), "CommitList failed.");
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
            static_cast<cppu::OWeakObject*>(this), 1);
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
                      "BeginList failed");

        UINT removed_count;
        if (SUCCEEDED(removed->GetCount(&removed_count) && (removed_count > 0)))
        {
            css::system::windows::JumpListItem item;
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

    return comphelper::containerToSequence(removedItems);
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

css::uno::Sequence<OUString> SAL_CALL JumpListImpl::getSupportedServiceNames()
{
    return { "com.sun.star.system.windows.JumpList" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
shell_JumpListExec_get_implementation(css::uno::XComponentContext* context,
                                      css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new JumpListImpl(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
