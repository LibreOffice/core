/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Environment.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-09 13:33:36 $
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
