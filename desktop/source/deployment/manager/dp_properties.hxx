/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dp_manager.h,v $
 * $Revision: 1.17 $
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

#if ! defined INCLUDED_DP_PROPERTIES_HXX
#define INCLUDED_DP_PROPERTIES_HXX



#include "com/sun/star/beans/NamedValue.hpp"
#include "com/sun/star/ucb/XCommandEnvironment.hpp"
#include "boost/optional.hpp"


namespace css = ::com::sun::star;

namespace dp_manager {



/**

 */
class ExtensionProperties
{
protected:
    ::rtl::OUString m_propFileUrl;
    const css::uno::Reference<css::ucb::XCommandEnvironment> m_xCmdEnv;
    ::boost::optional< ::rtl::OUString> m_prop_suppress_license;
public:

    virtual ~ExtensionProperties() {};
    ExtensionProperties(::rtl::OUString const & urlExtension,
                        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv);

    ExtensionProperties(::rtl::OUString const & urlExtension,
                        css::uno::Sequence<css::beans::NamedValue> const & properties,
                        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv);

    void write();

    bool isSuppressedLicense();
};
}




#endif

