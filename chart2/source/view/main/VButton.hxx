/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CHART2_SOURCE_VIEW_MAIN_VBUTTON_HXX
#define INCLUDED_CHART2_SOURCE_VIEW_MAIN_VBUTTON_HXX

#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

namespace chart
{

class VButton final
{
private:
    css::uno::Reference<css::lang::XMultiServiceFactory> m_xShapeFactory;
    css::uno::Reference<css::drawing::XShapes> m_xTarget;
    css::uno::Reference<css::drawing::XShape> m_xShape;
    css::awt::Point m_rPosition;
    OUString m_sLabel;
    OUString m_sCID;

public:
    VButton();

    void init(const css::uno::Reference<css::drawing::XShapes>& xTargetPage,
              const css::uno::Reference<css::lang::XMultiServiceFactory>& xFactory);

    void createShapes(const css::awt::Point& rPosition,
                      const css::awt::Size& rReferenceSize,
                      const css::uno::Reference<css::beans::XPropertySet>& xTextProp);

    void setWidth(sal_Int32 nWidth);
    void setLabel(OUString const & sLabel)
    {
        m_sLabel = sLabel;
    }
    void setCID(OUString const & sCID)
    {
        m_sCID = sCID;
    }
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
