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
    using namespace sal::systools;

    try
    {
        COMReference<ICustomDestinationList> aDestinationList(CLSID_DestinationList, nullptr,
                                                              CLSCTX_INPROC_SERVER);

        OUString sApplicationID(lcl_getAppId(sDocumentService));
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
            SAL_WARN("vcl", "osl_getExecutableFile failed");
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

                ThrowIfFailed(pShellLinkItem->SetDescription(o3tl::toW(item.name.getStr())),
                              OString("Setting description '" + item.name.toUtf8() + "' failed."));

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
                SAL_WARN("vcl", e.what());
                continue;
            }
        }

        COMReference<IObjectArray> pObjectArray(aCollection, COM_QUERY_THROW);
        ThrowIfFailed(aDestinationList->AppendCategory(o3tl::toW(sCategory.getStr()), pObjectArray),
                      "AppendCategory failed.");

        ThrowIfFailed(aDestinationList->CommitList(), "CommitList failed.");
    }
    catch (const ComError& e)
    {
        SAL_WARN("vcl", e.what());
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
