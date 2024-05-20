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

#include <ooo/vba/office/MsoAnimationType.hpp>

#include"vbaassistant.hxx"

using namespace com::sun::star;
using namespace ooo::vba;

using namespace ooo::vba::office::MsoAnimationType;

constexpr OUStringLiteral g_sName = u"Clippit";

ScVbaAssistant::ScVbaAssistant( const uno::Reference< XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext ): ScVbaAssistantImpl_BASE(rParent, rContext)
{
    m_bIsVisible = false;
    m_nPointsLeft = 795;
    m_nPointsTop = 248;
    m_nAnimation = msoAnimationIdle;
}

ScVbaAssistant::~ScVbaAssistant()
{
}

sal_Bool SAL_CALL ScVbaAssistant::getVisible()
{
    return m_bIsVisible;
}

void SAL_CALL ScVbaAssistant::setVisible( sal_Bool bVisible )
{
    m_bIsVisible = bVisible;
}

sal_Bool SAL_CALL ScVbaAssistant::getOn()
{
    return false;
}

void SAL_CALL ScVbaAssistant::setOn( sal_Bool bOn )
{
    setVisible( bOn );
}

::sal_Int32 SAL_CALL
ScVbaAssistant::getTop()
{
    return m_nPointsTop;
}
void SAL_CALL
ScVbaAssistant::setTop( ::sal_Int32 _top )
{
    m_nPointsTop = _top;
}
::sal_Int32 SAL_CALL
ScVbaAssistant::getLeft()
{
    return m_nPointsLeft;
}
void SAL_CALL
ScVbaAssistant::setLeft( ::sal_Int32 _left )
{
    m_nPointsLeft = _left;
}
::sal_Int32 SAL_CALL
ScVbaAssistant::getAnimation()
{
    return m_nAnimation;
}
void SAL_CALL
ScVbaAssistant::setAnimation( ::sal_Int32 _animation )
{
    m_nAnimation = _animation;
}

OUString SAL_CALL
ScVbaAssistant::Name(  )
{
    return g_sName;
}

OUString
ScVbaAssistant::getServiceImplName()
{
    return u"ScVbaAssistant"_ustr;
}

uno::Sequence< OUString >
ScVbaAssistant::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.Assistant"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
