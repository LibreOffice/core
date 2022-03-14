/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <algorithm>
#include <cassert>

#include <osl/diagnose.h>
#include <osl/process.h>
#include <sal/log.hxx>
#include "JumpList.hxx"
#include <osl/file.hxx>
#include <sal/macros.h>
#include <sal/log.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <o3tl/char16_t2wchar_t.hxx>
#include <o3tl/runtimetooustring.hxx>
#include <o3tl/safeCoInitUninit.hxx>

#include <prewin.h>
#include <Shlobj.h>
#include <propkey.h>
#include <propvarutil.h>
#include <systools/win32/comtools.hxx>
#include <postwin.h>

using namespace css;
using namespace css::uno;
using namespace css::system::windows;

JumpListImpl::JumpListImpl(const css::uno::Reference<css::uno::XComponentContext>& xContext)
    : WeakComponentImplHelper<css::system::windows::XJumpList, css::lang::XServiceInfo>(m_aMutex)
    , m_xContext(xContext)
{
}

JumpListImpl::~JumpListImpl() {}

namespace
{
OUString lcl_getAppId(const OUString& rDocumentService)
{
    OUString sApplicationName;

    if (rDocumentService == "com.sun.star.text.TextDocument"
        || rDocumentService == "com.sun.star.text.GlobalDocument"
        || rDocumentService == "com.sun.star.text.WebDocument"
        || rDocumentService == "com.sun.star.xforms.XMLFormDocument")
        sApplicationName = "Writer";
    else if (rDocumentService == "com.sun.star.sheet.SpreadsheetDocument"
             || rDocumentService == "com.sun.star.chart2.ChartDocument")
        sApplicationName = "Calc";
    else if (rDocumentService == "com.sun.star.presentation.PresentationDocument")
        sApplicationName = "Impress";
    else if (rDocumentService == "com.sun.star.drawing.DrawingDocument")
        sApplicationName = "Draw";
    else if (rDocumentService == "com.sun.star.formula.FormulaProperties")
        sApplicationName = "Math";
    else if (rDocumentService == "com.sun.star.sdb.DatabaseDocument"
             || rDocumentService == "com.sun.star.sdb.OfficeDatabaseDocument"
             || rDocumentService == "com.sun.star.sdb.RelationDesign"
             || rDocumentService == "com.sun.star.sdb.QueryDesign"
             || rDocumentService == "com.sun.star.sdb.TableDesign"
             || rDocumentService == "com.sun.star.sdb.DataSourceBrowser")
        sApplicationName = "Base";
    else
    {
        sApplicationName = "Startcenter";
        //SAL_WARN("vcl", "Unknown document service: " << rDocumentService);
        //return "";
    }

    return "TheDocumentFoundation.LibreOffice." + sApplicationName;
}
}

void SAL_CALL JumpListImpl::appendCategory(const OUString& sCategory,
                                           const Sequence<JumpListItem>& aJumpListItems,
                                           const OUString& sDocumentService)
{
    sal::systools::COMReference<ICustomDestinationList> aDestinationList;
    HRESULT result = CoCreateInstance(CLSID_DestinationList, NULL, CLSCTX_INPROC_SERVER,
                                      IID_PPV_ARGS(&aDestinationList));

    if (FAILED(result))
    {
        SAL_WARN("vcl", "Creating DestinationList failed.");
        return;
    }

    OUString sApplicationID(lcl_getAppId(sDocumentService));
    aDestinationList->SetAppID(o3tl::toW(sApplicationID.getStr()));

    UINT min_slots;
    sal::systools::COMReference<IObjectArray> removed;
    result = aDestinationList->BeginList(&min_slots, IID_PPV_ARGS(&removed));
    if (FAILED(result))
    {
        SAL_WARN("vcl", "BeginList failed");
        return;
    }

    OUString sofficeURL;
    OUString sofficePath;
    oslProcessError err = osl_getExecutableFile(&sofficeURL.pData);
    osl::FileBase::getSystemPathFromFileURL(sofficeURL, sofficePath);
    if (err != osl_Process_E_None)
    {
        SAL_WARN("vcl", "osl_getExecutableFile failed");
        return;
    }
    // We need to run soffice.exe, not soffice.bin
    sofficePath = sofficePath.replaceFirst("soffice.bin", "soffice.exe");

    sal::systools::COMReference<IObjectCollection> aCollection;
    aCollection.CoCreateInstance(CLSID_EnumerableObjectCollection, NULL, CLSCTX_INPROC_SERVER);

    for (auto item : aJumpListItems)
    {
        if (item.name.isEmpty())
            continue;
        sal::systools::COMReference<IShellLinkW> pShellLinkItem;
        result = pShellLinkItem.CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER);
        if (FAILED(result))
        {
            SAL_WARN("vcl", "Creating item '" << item.name << "' failed.");
            continue;
        }

        sal::systools::COMReference<IPropertyStore> pps(pShellLinkItem,
                                                        sal::systools::COM_QUERY_THROW);
        PROPVARIANT propvar;
        result = InitPropVariantFromString(o3tl::toW(item.name.getStr()), &propvar);
        if (FAILED(result))
        {
            SAL_WARN("vcl", "InitPropVariantFromString failed.");
            continue;
        }

        result = pps->SetValue(PKEY_Title, propvar);
        if (FAILED(result))
        {
            SAL_WARN("vcl", "SetValue failed.");
            continue;
        }

        result = pps->Commit();
        if (FAILED(result))
        {
            SAL_WARN("vcl", "Commit failed.");
            continue;
        }

        PropVariantClear(&propvar);
        pps->Release();

        result = pShellLinkItem->SetDescription(o3tl::toW(item.name.getStr()));
        if (FAILED(result))
        {
            SAL_WARN("vcl", "Setting description '" << item.name << "' failed.");
            continue;
        }

        result = pShellLinkItem->SetPath(o3tl::toW(sofficePath.getStr()));
        if (FAILED(result))
        {
            SAL_WARN("vcl", "Setting path '" << sofficePath << "' failed.");
            continue;
        }

        result = pShellLinkItem->SetArguments(o3tl::toW(item.arguments.getStr()));
        if (FAILED(result))
        {
            SAL_WARN("vcl", "Setting arguments '" << item.arguments << "' failed.");
            continue;
        }

        result = pShellLinkItem->SetIconLocation(o3tl::toW(item.iconPath.getStr()), 0);
        if (FAILED(result))
        {
            SAL_WARN("vcl", "Setting icon path '" << item.iconPath << "' failed.");
            continue;
        }
        aCollection->AddObject(pShellLinkItem);
    }

    sal::systools::COMReference<IObjectArray> pObjectArray(aCollection,
                                                           sal::systools::COM_QUERY_THROW);
    result = aDestinationList->AppendCategory(o3tl::toW(sCategory.getStr()), pObjectArray);
    if (FAILED(result))
    {
        SAL_WARN("vcl", "AppendCategory failed.");
        return;
    }

    result = aDestinationList->CommitList();
    if (FAILED(result))
    {
        SAL_WARN("vcl", "CommitList failed.");
        return;
    }
}

Sequence<JumpListItem> SAL_CALL JumpListImpl::getRemovedItems()
{
    /*ICustomDestinationList* aDestinationList;
    HRESULT result = CoCreateInstance(CLSID_DestinationList, NULL, CLSCTX_INPROC_SERVER,
                                      IID_PPV_ARGS(&aDestinationList));

    if (FAILED(result))
    {
        SAL_WARN("vcl", "Creating DestinationList failed.");
        return Sequence<JumpListItem>();
    }*/

    //OUString sApplicationID(lcl_getAppId(sDocumentService));
    //aDestinationList->SetAppID(o3tl::toW(sApplicationID.getStr()));

    //aDestinationList->GetRemovedDestinations(IID_IObjectArray, pObjectList);

    return Sequence<JumpListItem>();
}

// XServiceInfo

OUString SAL_CALL JumpListImpl::getImplementationName()
{
    return "com.sun.star.system.Windows.JumpListImpl";
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
