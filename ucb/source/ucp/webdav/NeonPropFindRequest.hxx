/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NeonPropFindRequest.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:36:54 $
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

#ifndef _NEONPROPFINDREQUEST_HXX_
#define _NEONPROPFINDREQUEST_HXX_

#include <vector>

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _NEONTYPES_HXX_
#include "NeonTypes.hxx"
#endif
#ifndef _DAVTYPES_HXX_
#include "DAVTypes.hxx"
#endif
#ifndef _DAVRESOURCE_HXX_
#include "DAVResource.hxx"
#endif

namespace webdav_ucp
{

class NeonPropFindRequest
{
public:
    // named / allprop
    NeonPropFindRequest( HttpSession* inSession,
                         const char*  inPath,
                         const Depth  inDepth,
                         const std::vector< ::rtl::OUString > & inPropNames,
                         std::vector< DAVResource > & ioResources,
                         int & nError );
    // propnames
    NeonPropFindRequest( HttpSession* inSession,
                         const char*  inPath,
                         const Depth  inDepth,
                         std::vector< DAVResourceInfo > & ioResInfo,
                         int & nError );

    ~NeonPropFindRequest();
};

} // namespace webdav_ucp

#endif // _NEONPROPFINDREQUEST_HXX_
