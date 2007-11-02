/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_reposypart.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:46:42 $
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

#ifndef ARY_IDL_I_REPOSYPART_HXX
#define ARY_IDL_I_REPOSYPART_HXX



// USED SERVICES
    // BASE CLASSES
#include <idl_internalgate.hxx>
    // OTHER


namespace ary
{
namespace idl
{
    class CeAdmin;
    class TypeAdmin;
    class NameLookup;
}
}





namespace ary
{
namespace idl
{



/** The idl part of the Autodoc repository.
*/
class RepositoryPartition : public InternalGate
{
  public:
    // LIFECYCLE
                        RepositoryPartition(
                            RepositoryCenter &  i_repository );
                        ~RepositoryPartition();
    // INHERITED
        // Interface Gate:
    virtual void        Calculate_AllSecondaryInformation(
                            const String &      i_devman_reffilepath );
//                            const ::autodoc::Options &
//                                                i_options );
    virtual const CePilot &
                        Ces() const;
    virtual const TypePilot &
                        Types() const;
    virtual CePilot &   Ces();
    virtual TypePilot & Types();

  private:
    // DATA
    RepositoryCenter *  pCenter;

    Dyn<CeAdmin>        pCes;
    Dyn<TypeAdmin>      pTypes;
    Dyn<NameLookup>     pNamesDictionary;
};




}   //  namespace idl
}   //  namespace ary
#endif
