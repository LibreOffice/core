/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#pragma once

#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/uno/Reference.h>
#include <rtl/ustring.hxx>
#include <rtl/ref.hxx>
#include <sal/types.h>
#include <svx/unoshape.hxx>

namespace com::sun::star::awt { struct Point; }
namespace com::sun::star::chart2 { class XTitle; }
class SvxShapeText;

namespace chart
{
class Title;

class VTitle final
{
public:
    explicit VTitle( css::uno::Reference< css::chart2::XTitle > xTitle );
    ~VTitle();

    void    init( const rtl::Reference<SvxShapeGroupAnyD>& xTargetPage
             , const OUString& rCID );

    void    createShapes( const css::awt::Point& rPos
                          , const css::awt::Size& rReferenceSize
                          , const css::awt::Size& nTextMaxWidth
                          , bool bYAxisTitle );

    double getRotationAnglePi() const;
    css::awt::Size getUnrotatedSize() const;
    css::awt::Size getFinalSize() const;
    void    changePosition( const css::awt::Point& rPos );
    static bool isVisible(
            const rtl::Reference< ::chart::Title > & xTitle);

private:
    rtl::Reference<SvxShapeGroupAnyD>            m_xTarget;
    css::uno::Reference< css::chart2::XTitle >              m_xTitle;
    rtl::Reference<SvxShapeText>                            m_xShape;
    OUString   m_aCID;

    double      m_fRotationAngleDegree;
    sal_Int32   m_nXPos;
    sal_Int32   m_nYPos;
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
