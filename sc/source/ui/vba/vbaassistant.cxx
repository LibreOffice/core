/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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

::rtl::OUString SAL_CALL
ScVbaAssistant::Name(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
{
    return m_sName;
}

rtl::OUString&
ScVbaAssistant::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaAssistant") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaAssistant::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.Assistant" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
