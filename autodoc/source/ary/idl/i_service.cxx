/*************************************************************************
 *
 *  $RCSfile: i_service.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:16:06 $
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

#include <precomp.h>
#include <ary/idl/i_service.hxx>


// NOT FULLY DECLARED SERVICES
#include <ary/idl/ihost_ce.hxx>
#include <ary/idl/ik_service.hxx>
#include <ary_i/codeinf2.hxx>
#include <sci_impl.hxx>
#include "ipi_2s.hxx"


namespace ary
{
namespace idl
{

Service::Service( const String &      i_sName,
                  Ce_id               i_nOwner )
    :   sName(i_sName),
        nOwner(i_nOwner),
        aIncludedServices(),
        aSupportedInterfaces(),
        aProperties()
{
}

Service::~Service()
{
    for ( RelationList::iterator it = aIncludedServices.begin();
          it != aIncludedServices.end();
          ++it )
    {
        delete (*it).Info();
    }

    for ( RelationList::iterator it = aSupportedInterfaces.begin();
          it != aSupportedInterfaces.end();
          ++it )
    {
        delete (*it).Info();
    }
}

void
Service::Get_SupportedInterfaces( Dyn_StdConstIterator<CommentedRelation> & o_rResult ) const
{
    o_rResult = new SCI_Vector<CommentedRelation>(aSupportedInterfaces);
}

void
Service::Get_IncludedServices( Dyn_StdConstIterator<CommentedRelation> & o_rResult ) const
{
    o_rResult = new SCI_Vector<CommentedRelation>(aIncludedServices);
}

void
Service::do_Visit_CeHost( CeHost & o_rHost ) const
{
    o_rHost.Do_Service(*this);
}

RCid
Service::inq_ClassId() const
{
    return class_id;
}

const String &
Service::inq_LocalName() const
{
    return sName;
}

Ce_id
Service::inq_NameRoom() const
{
    return nOwner;
}

Ce_id
Service::inq_Owner() const
{
    return nOwner;
}

E_SightLevel
Service::inq_SightLevel() const
{
    return sl_File;
}


namespace ifc_service
{

inline const Service &
service_cast( const CodeEntity &  i_ce )
{
    csv_assert( i_ce.ClassId() == Service::class_id );
    return static_cast< const Service& >(i_ce);
}

void
attr::Get_IncludedServices( Dyn_StdConstIterator<CommentedRelation> & o_result,
                            const CodeEntity &                        i_ce )
{
    o_result = new SCI_Vector<CommentedRelation>( service_cast(i_ce).aIncludedServices );
}

void
attr::Get_ExportedInterfaces( Dyn_StdConstIterator<CommentedRelation> & o_result,
                              const CodeEntity &                            i_ce )
{
    o_result = new SCI_Vector<CommentedRelation>( service_cast(i_ce).aSupportedInterfaces );
}

void
attr::Get_Properties( Dyn_CeIterator &    o_result,
                      const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>( service_cast(i_ce).aProperties );
}

void
xref::Get_IncludingServices( Dyn_CeIterator &    o_result,
                             const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(i_ce.Secondaries().List(service_2s_IncludingServices));
}

void
xref::Get_InstantiatingSingletons( Dyn_CeIterator &    o_result,
                                   const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(i_ce.Secondaries().List(service_2s_InstantiatingSingletons));
}


} // namespace ifc_service


}   //  namespace   idl
}   //  namespace   ary

