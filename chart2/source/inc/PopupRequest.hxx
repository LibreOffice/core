/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CHART2_SOURCE_INC_POPUPREQUEST_HXX
#define INCLUDED_CHART2_SOURCE_INC_POPUPREQUEST_HXX

#include "MutexContainer.hxx"
#include <cppuhelper/compbase.hxx>
#include <com/sun/star/chart2/data/XPopupRequest.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

namespace chart
{

namespace impl
{

typedef cppu::WeakComponentImplHelper<css::chart2::data::XPopupRequest> PopupRequest_Base;

}

class PopupRequest : public MutexContainer, public impl::PopupRequest_Base
{
public:
    explicit PopupRequest();
    virtual ~PopupRequest() override;

protected:
    // ____ XRequestCallback ____
    virtual void SAL_CALL addCallback(const css::uno::Reference< ::css::awt::XCallback >& xCallback,
                                      const css::uno::Any& aData) override;

    // ____ WeakComponentImplHelperBase ____
    // is called when dispose() is called at this component
    virtual void SAL_CALL disposing() override;

private:
    css::uno::Reference<css::awt::XCallback> m_xCallback;
};

} //  namespace chart

#endif // INCLUDED_CHART2_SOURCE_INC_POPUPREQUEST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
