/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_ce2s.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:05:49 $
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

#ifndef ARY_IDL_I_CE2S_HXX
#define ARY_IDL_I_CE2S_HXX


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <ary/idl/i_types4idl.hxx>


namespace ary
{

namespace idl
{



/** Abstract base for all secondary productions of code entities
*/
class Ce_2s
{
  public:
    // LIFECYCLE
    virtual             ~Ce_2s();

    static DYN Ce_2s *  Create_(
                            ClassId             i_nCeClass );
    // OPERATIONS
    void                Add_Link2DescriptionInManual(
                            const String &      i_link,
                            const String &      i_linkUI )
                                                { aDescriptionsInManual.push_back(i_link); aDescriptionsInManual.push_back(i_linkUI); }
    void                Add_Link2RefInManual(
                            const String &      i_link,
                            const String &      i_linkUI )
                                                { aRefsInManual.push_back(i_link); aRefsInManual.push_back(i_linkUI); }
    std::vector<Ce_id> &
                        Access_List(
                            int                 i_indexOfList );
    // INQUIRY
    const StringVector &
                        Links2DescriptionInManual() const
                                                { return aDescriptionsInManual; }
    const StringVector &
                        Links2RefsInManual() const
                                                { return aRefsInManual; }
    int                 CountXrefLists() const  { return aXrefLists.size(); }
    const std::vector<Ce_id> &
                        List(
                            int                 i_indexOfList ) const;
  private:
    typedef DYN std::vector<Ce_id> * ListPtr;

    // DATA
    StringVector        aDescriptionsInManual;
    StringVector        aRefsInManual;
    std::vector<ListPtr>
                        aXrefLists;
};


}   // namespace idl
}   // namespace ary


#endif
