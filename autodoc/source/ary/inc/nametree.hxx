/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: nametree.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:59:35 $
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

#ifndef ARY_NAMETREE_HXX
#define ARY_NAMETREE_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include <ary/ids.hxx>
#include "instlist.hxx"
#include "namesort.hxx"
    // PARAMETERS


namespace ary
{


class NameTree
{
  public:
#if 0 // Test new comparison
    struct Less_Name
    {
        bool                operator()(
                                const udmstri &     i_r1,
                                const udmstri &     i_r2 ) const;
    };
#endif // 0
    typedef std::map<  udmstri,
                       InstanceList,
                       CompareCeNames >         Map_Name2Inst;

    typedef Map_Name2Inst::const_iterator       const_iterator;
    typedef Map_Name2Inst::iterator             iterator;


                        NameTree();
                        ~NameTree();

    const InstanceList &
                        operator[](
                            const udmstri &     i_rName ) const;

    void                insert(
                            const udmstri &     i_rName,
                            ary::Rid            i_nId );

    const_iterator      find(
                            const udmstri &     i_rName );
    const_iterator      lower_bound(
                            const udmstri &     i_rName ) const;
    const_iterator      begin() const;
    const_iterator      end() const;

    iterator            begin();
    iterator            end();

  private:
    // DATA
    Map_Name2Inst       aNames;
};



// IMPLEMENTATION


}   // namespace ary


#endif

