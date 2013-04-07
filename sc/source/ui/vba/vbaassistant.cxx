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
#include <sfx2/app.hxx>
#include <svtools/helpopt.hxx>

#include <ooo/vba/office/MsoAnimationType.hpp>

#include"vbaassistant.hxx"


using namespace com::sun::star;
using namespace ooo::vba;

using namespace ooo::vba::office::MsoAnimationType;

ScVbaAssistant::ScVbaAssistant( const uno::Reference< XHelperInterface > xParent, const uno::Reference< uno::XComponentContext > xContext ): ScVbaAssistantImpl_BASE( xParent, xContext ),
m_sName( RTL_CONSTASCII_USTRINGPARAM( "Clippit" ) )
{
    m_bIsVisible = false;
    m_nPointsLeft = 795;
    m_nPointsTop = 248;
    m_nAnimation = msoAnimationIdle;
}

ScVbaAssistant::~ScVbaAssistant()
{
}

sal_Bool SAL_CALL ScVbaAssistant::getVisible() throw (uno::RuntimeException)
{
    return m_bIsVisible;
}

void SAL_CALL ScVbaAssistant::setVisible( sal_Bool bVisible ) throw (uno::RuntimeException)
{
    m_bIsVisible = bVisible;
}

sal_Bool SAL_CALL ScVbaAssistant::getOn() throw (uno::RuntimeException)
{
    if( SvtHelpOptions().IsHelpAgentAutoStartMode() )
        return sal_True;
    else
        return false;
}

void SAL_CALL ScVbaAssistant::setOn( sal_Bool bOn ) throw (uno::RuntimeException)
{
    SvtHelpOptions().SetHelpAgentAutoStartMode( bOn );
    setVisible( bOn );
}


::sal_Int32 SAL_CALL
ScVbaAssistant::getTop() throw (css::uno::RuntimeException)
{
    return m_nPointsTop;
}
void SAL_CALL
ScVbaAssistant::setTop( ::sal_Int32 _top ) throw (css::uno::RuntimeException)
{
    m_nPointsTop = _top;
}
::sal_Int32 SAL_CALL
ScVbaAssistant::getLeft() throw (css::uno::RuntimeException)
{
    return m_nPointsLeft;
}
void SAL_CALL
ScVbaAssistant::setLeft( ::sal_Int32 _left ) throw (css::uno::RuntimeException)
{
    m_nPointsLeft = _left;
}
::sal_Int32 SAL_CALL
ScVbaAssistant::getAnimation() throw (css::uno::RuntimeException)
{
    return m_nAnimation;
}
void SAL_CALL
ScVbaAssistant::setAnimation( ::sal_Int32 _animation ) throw (css::uno::RuntimeException)
{
    m_nAnimation = _animation;
}

OUString SAL_CALL
ScVbaAssistant::Name(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
{
    return m_sName;
}

OUString
ScVbaAssistant::getServiceImplName()
{
    return OUString("ScVbaAssistant");
}

uno::Sequence< OUString >
ScVbaAssistant::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = OUString("ooo.vba.Assistant" );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
