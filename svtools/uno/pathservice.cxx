/*************************************************************************
 *
 *  $RCSfile: pathservice.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2005-04-13 11:51:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
        reinterpret_cast< cppu::OWeakObject* >(new PathService()));
}

// -----------------------------------------------------------------------
