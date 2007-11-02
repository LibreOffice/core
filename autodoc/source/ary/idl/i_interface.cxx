/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_interface.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:45:13 $
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

#include <precomp.h>
#include <ary/idl/i_interface.hxx>


// NOT FULLY DECLARED SERVICES
#include <cosv/tpl/processor.hxx>
#include <ary/idl/ik_interface.hxx>
#include <ary/doc/d_oldidldocu.hxx>
#include <sci_impl.hxx>
#include "i2s_calculator.hxx"


namespace ary
{
namespace idl
{


class Interface_2s
{
};


Interface::Interface( const String &      i_sName,
                      Ce_id               i_nOwner )
    :   sName(i_sName),
        nOwner(i_nOwner),
        aBases(),
        aFunctions(),
        aAttributes(),
        p2s()
{
}

Interface::~Interface()
{
    for ( RelationList::iterator it = aBases.begin();
          it != aBases.end();
          ++it )
    {
        delete (*it).Info();
    }
}

void
Interface::do_Accept( csv::ProcessorIfc & io_processor ) const
{
    csv::CheckedCall(io_processor, *this);
}

ClassId
Interface::get_AryClass() const
{
    return class_id;
}

const String &
Interface::inq_LocalName() const
{
    return sName;
}

Ce_id
Interface::inq_NameRoom() const
{
    return nOwner;
}

Ce_id
Interface::inq_Owner() const
{
    return nOwner;
}

E_SightLevel
Interface::inq_SightLevel() const
{
    return sl_File;
}


namespace ifc_interface
{

inline const Interface &
interface_cast( const CodeEntity &  i_ce )
{
    csv_assert( i_ce.AryClass() == Interface::class_id );
    return static_cast< const Interface& >(i_ce);
}

void
attr::Get_Bases( Dyn_StdConstIterator<CommentedRelation> &     o_result,
                 const CodeEntity &                             i_ce )
{
    o_result = new SCI_Vector<CommentedRelation>(interface_cast(i_ce).aBases);
}

void
attr::Get_Functions( Dyn_CeIterator &    o_result,
                     const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(interface_cast(i_ce).aFunctions);
}

void
attr::Get_Attributes( Dyn_CeIterator &    o_result,
                      const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(interface_cast(i_ce).aAttributes);
}

void
xref::Get_Derivations( Dyn_CeIterator &    o_result,
                        const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(i_ce.Secondaries().List(interface_2s_Derivations));
}

void
xref::Get_SynonymTypedefs( Dyn_CeIterator &    o_result,
                           const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(i_ce.Secondaries().List(interface_2s_SynonymTypedefs));
}

void
xref::Get_ExportingServices( Dyn_CeIterator &    o_result,
                             const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(i_ce.Secondaries().List(interface_2s_ExportingServices));
}

void
xref::Get_ExportingSingletons( Dyn_CeIterator &    o_result,
                               const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(i_ce.Secondaries().List(interface_2s_ExportingSingletons));
}

void
xref::Get_AsReturns( Dyn_CeIterator &    o_result,
                     const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(i_ce.Secondaries().List(interface_2s_AsReturns));
}

void
xref::Get_AsParameters( Dyn_CeIterator &    o_result,
                        const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(i_ce.Secondaries().List(interface_2s_AsParameters));
}

void
xref::Get_AsDataTypes( Dyn_CeIterator &    o_result,
                       const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(i_ce.Secondaries().List(interface_2s_AsDataTypes));
}




}   // namespace ifc_interface


}   //  namespace   idl
}   //  namespace   ary
