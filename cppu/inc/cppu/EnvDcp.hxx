/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: EnvDcp.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-09 13:32:45 $
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
