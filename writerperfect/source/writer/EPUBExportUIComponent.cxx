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

#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <vcl/svapp.hxx>
#include <vcl/vclptr.hxx>

#include "EPUBExportDialog.hxx"

using namespace com::sun::star;

namespace writerperfect
{

EPUBExportUIComponent::EPUBExportUIComponent(const uno::Reference<uno::XComponentContext> &xContext)
    : mxContext(xContext)
{
}

uno::Sequence<beans::PropertyValue> EPUBExportUIComponent::getPropertyValues()
{
    maMediaDescriptor["FilterData"] <<= maFilterData.getAsConstPropertyValueList();
    return maMediaDescriptor.getAsConstPropertyValueList();
}

void EPUBExportUIComponent::setPropertyValues(const uno::Sequence<beans::PropertyValue> &rProperties)
{
    maMediaDescriptor.clear();
    maMediaDescriptor << rProperties;
    auto it = maMediaDescriptor.find("FilterData");
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
    return OUString("com.sun.star.comp.Writer.EPUBExportUIComponent");
}

sal_Bool EPUBExportUIComponent::supportsService(const OUString &rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> EPUBExportUIComponent::getSupportedServiceNames()
{
    uno::Sequence<OUString> aRet =
    {
        OUString("com.sun.star.ui.dialogs.FilterOptionsDialog")
    };
    return aRet;
}

void EPUBExportUIComponent::setTitle(const OUString &/*rTitle*/)
{
}

sal_Int16 EPUBExportUIComponent::execute()
{
    SolarMutexGuard aGuard;

    ScopedVclPtrInstance<EPUBExportDialog> pDialog(Application::GetDefDialogParent(), maFilterData, mxContext);
    if (pDialog->Execute() == RET_OK)
        return ui::dialogs::ExecutableDialogResults::OK;
    return ui::dialogs::ExecutableDialogResults::CANCEL;
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface * com_sun_star_comp_Writer_EPUBExportUIComponent_get_implementation(uno::XComponentContext *pCtx, uno::Sequence<uno::Any> const &)
{
    return cppu::acquire(new EPUBExportUIComponent(pCtx));
}

} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
