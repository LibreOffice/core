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

#ifndef INCLUDED_CPPU_ENVDCP_HXX
#define INCLUDED_CPPU_ENVDCP_HXX

#include "rtl/ustring.hxx"
#include "uno/EnvDcp.h"


namespace cppu
{
namespace EnvDcp
{
/** Get the OBI type part of an environment descriptor.
    (http://wiki.openoffice.org/wiki/Uno/Cpp/Spec/Environment_Descriptor)

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
    (http://wiki.openoffice.org/wiki/Uno/Binary/Spec/Environment_Descriptor)

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
