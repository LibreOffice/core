/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: nametree.hxx,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef ARY_NAMETREE_HXX
#define ARY_NAMETREE_HXX

// USED SERVICES
#include <ary/types.hxx>
#include <ary/namesort.hxx>
#include "instlist.hxx"




namespace ary
{


class NameTree
{
  public:
    typedef std::map<  String ,
                       InstanceList,
                       LesserName >             Map_Name2Inst;

    typedef Map_Name2Inst::const_iterator       const_iterator;
    typedef Map_Name2Inst::iterator             iterator;


                        NameTree();
                        ~NameTree();

    const InstanceList &
                        operator[](
                            const String  &     i_rName ) const;

    void                insert(
                            const String  &     i_rName,
                            ary::Rid            i_nId );

    const_iterator      find(
                            const String  &     i_rName );
    const_iterator      lower_bound(
                            const String  &     i_rName ) const;
    const_iterator      begin() const;
    const_iterator      end() const;

    iterator            begin();
    iterator            end();

  private:
    // DATA
    Map_Name2Inst       aNames;
};




}   // namespace ary
#endif
