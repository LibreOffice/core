/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ca_def.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:29:21 $
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

#ifndef ARY_CPP_CA_DEF_HXX
#define ARY_CPP_CA_DEF_HXX

// USED SERVICES
    // BASE CLASSES
#include <ary/cpp/cp_def.hxx>
    // OTHER
#include "cs_def.hxx"



namespace ary
{
namespace cpp
{
    class Def_Storage;
    class RepositoryPartition;
}
}





namespace ary
{
namespace cpp
{




class DefAdmin : public DefPilot
{
  public:
    // LIFECYCLE
                        DefAdmin(
                            RepositoryPartition &
                                                io_myReposyPartition );
                        ~DefAdmin();

    // INQUIRY
    const Def_Storage & Storage() const;

    // INHERITED
    // Interface DefPilot:
    virtual Define &    Store_Define(
                            const InputContext& i_rContext,
                            const String  &     i_sName,
                            const StringVector &
                                                i_rDefinition );
    virtual Macro &     Store_Macro(
                            const InputContext& i_rContext,
                            const String  &     i_sName,
                            const StringVector &
                                                i_rParams,
                            const StringVector &
                                                i_rDefinition );
    virtual const DefineEntity &
                        Find_Def(
                            De_id               i_id ) const;
    virtual DefsResultList
                        AllDefines() const;
    virtual DefsResultList
                        AllMacros() const;

  private:
    // DATA
    Def_Storage     aStorage;
    RepositoryPartition *
                    pCppRepositoryPartition;
};




// IMPLEMENTATION
inline const Def_Storage &
DefAdmin::Storage() const
{
    return aStorage;
}



}   // namespace cpp
}   // namespace ary
#endif
