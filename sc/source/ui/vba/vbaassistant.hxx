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
#ifndef SC_VBA_ASSISTANT_HXX
#define SC_VBA_ASSISTANT_HXX

#include <cppuhelper/implbase1.hxx>
#include <ooo/vba/XAssistant.hpp>

#include <sfx2/sfxhelp.hxx>

#include "vbahelper.hxx"
#include "vbahelperinterface.hxx"

typedef ::cppu::WeakImplHelper1< ov::XAssistant > Assistant;
typedef InheritedHelperInterfaceImpl< Assistant > ScVbaAssistantImpl_BASE;

class ScVbaAssistant : public ScVbaAssistantImpl_BASE
{
private:
    sal_Bool        m_bIsVisible;
    sal_Int32       m_nPointsLeft;
    sal_Int32       m_nPointsTop;
    rtl::OUString   m_sName;
    sal_Int32       m_nAnimation;
public:
    ScVbaAssistant( const css::uno::Reference< ov::XHelperInterface > xParent, const css::uno::Reference< css::uno::XComponentContext > xContext );
    virtual ~ScVbaAssistant();
    // XAssistant
    virtual sal_Bool SAL_CALL getOn() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setOn( sal_Bool _on ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getVisible() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setVisible( sal_Bool _visible ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getTop() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setTop( ::sal_Int32 _top ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getLeft() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setLeft( ::sal_Int32 _left ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getAnimation() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setAnimation( ::sal_Int32 _animation ) throw (css::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL Name(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif//SC_VBA_ASSISTANT_HXX
