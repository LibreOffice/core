/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pathservice.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 15:28:51 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include "pathoptions.hxx"

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif
#ifndef _RTL_USTRING_HXX_
#include "rtl/ustring.hxx"
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONFIGMANAGER_HPP_
#include <com/sun/star/frame/XConfigManager.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

namespace css = com::sun::star;
using rtl::OUString;

// -----------------------------------------------------------------------

class PathService : public ::cppu::WeakImplHelper2< css::frame::XConfigManager, css::lang::XServiceInfo >
{
    SvtPathOptions m_aOptions;

public:
    PathService()
        {}

    virtual OUString SAL_CALL getImplementationName()
        throw(css::uno::RuntimeException)
        {
            return OUString::createFromAscii("com.sun.star.comp.svtools.PathService");
        }

    virtual sal_Bool SAL_CALL supportsService (
        const OUString & rName)
        throw(css::uno::RuntimeException)
        {
            return (rName.compareToAscii("com.sun.star.config.SpecialConfigManager") == 0);
        }

    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw(css::uno::RuntimeException)
        {
            css::uno::Sequence< OUString > aRet(1);
            aRet.getArray()[0] = OUString::createFromAscii("com.sun.star.config.SpecialConfigManager");
            return aRet;
        }

    virtual OUString SAL_CALL substituteVariables (
        const OUString& sText)
        throw(css::uno::RuntimeException)
        {
            return m_aOptions.SubstituteVariable( sText );
        }

    virtual void SAL_CALL addPropertyChangeListener (
        const OUString &, const css::uno::Reference< css::beans::XPropertyChangeListener > &)
        throw(css::uno::RuntimeException)
        {}

    virtual void SAL_CALL removePropertyChangeListener (
        const OUString &, const css::uno::Reference< css::beans::XPropertyChangeListener > &)
        throw(css::uno::RuntimeException)
        {}

    virtual void SAL_CALL flush()
        throw(css::uno::RuntimeException)
        {}
};

// -----------------------------------------------------------------------

css::uno::Reference< css::uno::XInterface > PathService_CreateInstance (
    const css::uno::Reference< css::lang::XMultiServiceFactory > &)
{
    return css::uno::Reference< css::uno::XInterface >(
        static_cast< cppu::OWeakObject* >(new PathService()));
}

// -----------------------------------------------------------------------
