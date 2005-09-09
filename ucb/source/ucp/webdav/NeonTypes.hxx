/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NeonTypes.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 16:13:33 $
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

#ifndef _NEONTYPES_HXX_
#define _NEONTYPES_HXX_

#ifndef NE_SESSION_H
#include <neon/ne_session.h> // for ne_session
#endif
#ifndef NE_UTILS_H
#include <neon/ne_utils.h> // for ne_status
#endif
#ifndef NE_BASIC_H
#include <neon/ne_basic.h> // for ne_server_capabilities
#endif
#ifndef NE_PROPS_H
#include <neon/ne_props.h> // for ne_propname, ne_prop_result_set
#endif

typedef ne_session                  HttpSession;
typedef ne_status                   HttpStatus;
typedef ne_server_capabilities      HttpServerCapabilities;

typedef ne_propname                 NeonPropName;
typedef ne_prop_result_set          NeonPropFindResultSet;

#endif // _NEONTYPES_HXX_
