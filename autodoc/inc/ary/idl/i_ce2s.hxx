/*************************************************************************
 *
 *  $RCSfile: i_ce2s.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-11-01 17:11:08 $
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

#ifndef ARY_IDL_I_CE2S_HXX
#define ARY_IDL_I_CE2S_HXX


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <ary/idl/i_language.hxx>


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
                            RCid                i_nCeClass );
    // OPERATIONS
    void                Add_Link2DescriptionInManual(
                            const String &      i_link )
                                                { aDescriptionsInManual.push_back(i_link); }
    void                Add_Link2RefInManual(
                            const String &      i_link )
                                                { aRefsInManual.push_back(i_link); }
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
