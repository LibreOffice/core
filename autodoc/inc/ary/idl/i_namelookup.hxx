/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_namelookup.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:09:03 $
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

#ifndef ARY_IDL_I_NAMELOOKUP_HXX
#define ARY_IDL_I_NAMELOOKUP_HXX

// BASE CLASSES
#include <ary/idl/i_types4idl.hxx>

// USED SERVICES
#include <ary/stdconstiter.hxx>
#include <ary/itrange.hxx>
#include <vector>
#include <map>




namespace ary
{
namespace idl
{


/** This class finds all occurrences in the current language of a
    name in the repository.
*/
class NameLookup
{
  public:
    struct NameProperties
    {
                            NameProperties()
                                :   nId(0),
                                    nClass(0),
                                    nOwner(0) {}
                            NameProperties(
                                Ce_id               i_id,
                                ClassId             i_class,
                                Ce_id               i_owner )
                                :   nId(i_id),
                                    nClass(i_class),
                                    nOwner(i_owner) {}
        Ce_id               nId;
        ClassId             nClass;
        Ce_id               nOwner;
    };

    /// Map from Name to NameProperties.
    typedef std::multimap<String, NameProperties>   Map_Names;

    // LIFECYCLE
                        NameLookup();
                        ~NameLookup();
    // OPERATIONS
    void                Add_Name(
                            const String &      i_name,
                            Ce_id               i_id,
                            ClassId             i_class,
                            Ce_id               i_owner );
    // INQUIRY
    /**
    */
    bool                Has_Name(
                            const String &      i_name,
                            ClassId             i_class,
                            Ce_id               i_owner ) const;
    void                Get_Names(
                            Dyn_StdConstIterator<Map_Names::value_type> &
                                                o_rResult,
                            const String &      i_name ) const;
  private:
    // DATA
    Map_Names           aNames;
};




}   // namespace idl
}   // namespace ary
#endif
