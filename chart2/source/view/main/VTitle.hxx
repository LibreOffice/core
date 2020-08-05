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
#include <sal/types.h>

namespace com::sun::star::awt { struct Point; }
namespace com::sun::star::chart2 { class XTitle; }
namespace com::sun::star::drawing { class XShape; }
namespace com::sun::star::drawing { class XShapes; }
namespace com::sun::star::lang { class XMultiServiceFactory; }

namespace chart
{

class VTitle final
{
public:
    explicit VTitle( const css::uno::Reference< css::chart2::XTitle > & xTitle );
    ~VTitle();

    void    init( const css::uno::Reference< css::drawing::XShapes >& xTargetPage
             , const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory
             , const OUString& rCID );

    void    createShapes( const css::awt::Point& rPos
                          , const css::awt::Size& rReferenceSize
                          , const css::awt::Size& nTextMaxWidth
                          , bool bYAxisTitle );

    double getRotationAnglePi() const;
    css::awt::Size getUnrotatedSize() const;
    css::awt::Size getFinalSize() const;
    void    changePosition( const css::awt::Point& rPos );

private:
    css::uno::Reference< css::drawing::XShapes >            m_xTarget;
    css::uno::Reference< css::lang::XMultiServiceFactory>   m_xShapeFactory;
    css::uno::Reference< css::chart2::XTitle >              m_xTitle;
    css::uno::Reference< css::drawing::XShape >             m_xShape;
    OUString   m_aCID;

    double      m_fRotationAngleDegree;
    sal_Int32   m_nXPos;
    sal_Int32   m_nYPos;
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
