/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Environment.hxx,v $
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

#ifndef INCLUDED_cppu_helper_purpenv_Environment_hxx
#define INCLUDED_cppu_helper_purpenv_Environment_hxx

#include "uno/environment.h"
#include "cppu/Enterable.hxx"


namespace cppu { namespace helper { namespace purpenv {

/** C++ helper for implementing Purpose Environments.
    (http://wiki.services.openoffice.org/wiki/Uno/Cpp/Spec/Purpose_Bridge_Implementation_Helper)

    @since UDK 3.2.7
*/
void Environment_initWithEnterable(uno_Environment * pEnvironment, cppu::Enterable * pEnterable);

}}}

#endif
