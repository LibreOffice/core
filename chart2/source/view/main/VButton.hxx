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

#include <tools/color.hxx>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/uno/Reference.hxx>

namespace com { namespace sun { namespace star { namespace beans { class XPropertySet; } } } }
namespace com { namespace sun { namespace star { namespace drawing { class XShape; } } } }
namespace com { namespace sun { namespace star { namespace drawing { class XShapes; } } } }
namespace com { namespace sun { namespace star { namespace lang { class XMultiServiceFactory; } } } }

namespace chart
{

class VButton final
{
private:
    css::uno::Reference<css::lang::XMultiServiceFactory> m_xShapeFactory;
    css::uno::Reference<css::drawing::XShapes> m_xTarget;
    css::uno::Reference<css::drawing::XShape> m_xShape;
    OUString m_sLabel;
    OUString m_sCID;
    css::awt::Point m_aPosition;
    css::awt::Size m_aSize;
    bool m_bShowArrow;
    Color m_nArrowColor;
    Color m_nBGColor;

    css::uno::Reference<css::drawing::XShape>
        createTriangle(css::awt::Size aSize);

public:
    VButton();

    void init(const css::uno::Reference<css::drawing::XShapes>& xTargetPage,
              const css::uno::Reference<css::lang::XMultiServiceFactory>& xFactory);

    void createShapes(const css::uno::Reference<css::beans::XPropertySet>& xTextProp);

    void showArrow(bool bShowArrow)
    {
        m_bShowArrow = bShowArrow;
    }
    void setArrowColor(Color nArrowColor)
    {
        m_nArrowColor = nArrowColor;
    }
    void setBGColor(Color nBGColor)
    {
        m_nBGColor = nBGColor;
    }
    void setLabel(OUString const & rLabel)
    {
        m_sLabel = rLabel;
    }
    void setCID(OUString const & rCID)
    {
        m_sCID = rCID;
    }
    void setPosition(css::awt::Point const & rPosition)
    {
        m_aPosition = rPosition;
    }
    css::awt::Size const & getSize() const
    {
        return m_aSize;
    }
    void setSize(css::awt::Size const & rSize)
    {
        m_aSize = rSize;
    }
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
