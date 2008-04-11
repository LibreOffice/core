/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: EnvDcp.hxx,v $
 * $Revision: 1.3 $
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

#ifndef INCLUDED_cppu_EnvDcp_hxx
#define INCLUDED_cppu_EnvDcp_hxx

#include "rtl/ustring.hxx"
#include "uno/EnvDcp.h"


namespace cppu
{
namespace EnvDcp
{
/** Get the OBI type part of an environment descriptor.
    (http://wiki.services.openoffice.org/wiki/Uno/Cpp/Spec/Environment_Descriptor)

    @param  rEnvDcp        the Environment Descriptor
    @return                the OBI type
    @since UDK 3.2.7
*/
inline rtl::OUString getTypeName(rtl::OUString const & rEnvDcp)
{
    rtl::OUString typeName;

    uno_EnvDcp_getTypeName(rEnvDcp.pData, &typeName.pData);

    return typeName;
}

/** Get the purpose part of an environment descriptor.
    (http://wiki.services.openoffice.org/wiki/Uno/Binary/Spec/Environment_Descriptor)

    @param  rEnvDcp        the Environment Descriptor
    @return                the purpose
    @since UDK 3.2.7
*/
inline rtl::OUString getPurpose(rtl::OUString const & rEnvDcp)
{
    rtl::OUString purpose;

    uno_EnvDcp_getPurpose(rEnvDcp.pData, &purpose.pData);

    return purpose;
}

}
}


#endif
