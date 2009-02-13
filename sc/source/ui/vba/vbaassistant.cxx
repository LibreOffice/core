/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include <sfx2/app.hxx>
#include <svtools/helpopt.hxx>

#include <ooo/vba/office/MsoAnimationType.hpp>

#include"vbaassistant.hxx"


using namespace com::sun::star;
using namespace ooo::vba;

using namespace ooo::vba::office::MsoAnimationType;

ScVbaAssistant::ScVbaAssistant( const uno::Reference< XHelperInterface > xParent, const uno::Reference< uno::XComponentContext > xContext ): ScVbaAssistantImpl_BASE( xParent, xContext )
{
    m_bIsVisible = sal_False;
    m_nPointsLeft = 795;
    m_nPointsTop = 248;
    m_sName = rtl::OUString::createFromAscii( "Clippit" );
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
        return sal_False;
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
