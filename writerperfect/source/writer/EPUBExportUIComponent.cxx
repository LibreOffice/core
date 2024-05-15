/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "EPUBExportUIComponent.hxx"

#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <comphelper/namedvaluecollection.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <vcl/svapp.hxx>

#include "EPUBExportDialog.hxx"

using namespace com::sun::star;

namespace writerperfect
{
EPUBExportUIComponent::EPUBExportUIComponent(uno::Reference<uno::XComponentContext> xContext)
    : mxContext(std::move(xContext))
{
}

uno::Sequence<beans::PropertyValue> EPUBExportUIComponent::getPropertyValues()
{
    maMediaDescriptor[u"FilterData"_ustr] <<= maFilterData.getAsConstPropertyValueList();
    return maMediaDescriptor.getAsConstPropertyValueList();
}

void EPUBExportUIComponent::setPropertyValues(
    const uno::Sequence<beans::PropertyValue>& rProperties)
{
    maMediaDescriptor.clear();
    maMediaDescriptor << rProperties;
    auto it = maMediaDescriptor.find(u"FilterData"_ustr);
    if (it != maMediaDescriptor.end())
    {
        uno::Sequence<beans::PropertyValue> aFilterData;
        if (it->second >>= aFilterData)
        {
            maFilterData.clear();
            maFilterData << aFilterData;
        }
    }
}

OUString EPUBExportUIComponent::getImplementationName()
{
    return u"com.sun.star.comp.Writer.EPUBExportUIComponent"_ustr;
}

sal_Bool EPUBExportUIComponent::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> EPUBExportUIComponent::getSupportedServiceNames()
{
    uno::Sequence<OUString> aRet = { u"com.sun.star.ui.dialogs.FilterOptionsDialog"_ustr };
    return aRet;
}

void EPUBExportUIComponent::setTitle(const OUString& /*rTitle*/) {}

void SAL_CALL EPUBExportUIComponent::initialize(const uno::Sequence<uno::Any>& rArguments)
{
    ::comphelper::NamedValueCollection aProperties(rArguments);
    if (aProperties.has(u"ParentWindow"_ustr))
        aProperties.get(u"ParentWindow"_ustr) >>= mxDialogParent;
}

sal_Int16 EPUBExportUIComponent::execute()
{
    SolarMutexGuard aGuard;

    EPUBExportDialog aDialog(Application::GetFrameWeld(mxDialogParent), maFilterData, mxContext,
                             mxSourceDocument);
    if (aDialog.run() == RET_OK)
        return ui::dialogs::ExecutableDialogResults::OK;
    return ui::dialogs::ExecutableDialogResults::CANCEL;
}

void SAL_CALL EPUBExportUIComponent::setSourceDocument(
    const css::uno::Reference<css::lang::XComponent>& xDocument)
{
    mxSourceDocument = xDocument;
}

void SAL_CALL EPUBExportUIComponent::setDialogTitle(const OUString& aTitle) { setTitle(aTitle); }

void SAL_CALL EPUBExportUIComponent::startExecuteModal(
    const css::uno::Reference<css::ui::dialogs::XDialogClosedListener>& xListener)
{
    SolarMutexGuard aSolarGuard;

    if (!mxAsyncDialog)
    {
        if (mxSourceDocument.is())
            mxAsyncDialog
                = std::make_shared<EPUBExportDialog>(Application::GetFrameWeld(mxDialogParent),
                                                     maFilterData, mxContext, mxSourceDocument);

        if (!mxAsyncDialog)
            return;
    }

    weld::DialogController::runAsync(mxAsyncDialog, [xListener](sal_Int32 nResponse) {
        css::ui::dialogs::DialogClosedEvent aEvent;
        aEvent.DialogResult = nResponse;
        xListener->dialogClosed(aEvent);
    });
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_Writer_EPUBExportUIComponent_get_implementation(
    uno::XComponentContext* pCtx, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new EPUBExportUIComponent(pCtx));
}

} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
