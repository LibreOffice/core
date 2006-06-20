/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: gen_info.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:07:51 $
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

#ifndef GEN_INFO_HXX
#define GEN_INFO_HXX


#include "simstr.hxx"
#include <string.h>



class List_GenericInfo;


/** Holds generic informations in a simple hierarchical format.
*/
class GenericInfo
{
  public:
    // LIFECFYCLE
                        GenericInfo(
                            const Simstr &      i_sKey,
                            const Simstr &      i_sValue = Simstr::null_(),
                            const Simstr &      i_sComment = Simstr::null_() );
                        GenericInfo(
                            const GenericInfo & i_rInfo );
                        ~GenericInfo();

    // OPERATORS
    GenericInfo &       operator=(
                            const GenericInfo & i_rInfo );
    bool                operator<(
                            const GenericInfo & i_rInfo ) const
#ifdef UNX
                                                { return strcasecmp(sKey.str(),i_rInfo.sKey.str()) < 0; }
#else
                                                { return stricmp(sKey.str(),i_rInfo.sKey.str()) < 0; }
#endif
    // INFO
    const Simstr &      Key() const             { return sKey; }
    const Simstr &      Value() const           { return sValue; }
    const Simstr &      Comment() const         { return sComment; }
    bool                HasSubList() const      { return dpSubList != 0; }

    const List_GenericInfo &
                        SubList() const         { return HasSubList() ? *dpSubList : CreateMyList(); }
    // ACCESS
    List_GenericInfo &
                        SubList()               { return HasSubList() ? *dpSubList : CreateMyList(); }

  private:
    /// @precond dpSubList == 0 .
    List_GenericInfo &  CreateMyList() const;

    // DATA
    Simstr              sKey;
    Simstr              sValue;
    Simstr              sComment;
    List_GenericInfo *  dpSubList;      /// Owned by this object.
};


#endif

