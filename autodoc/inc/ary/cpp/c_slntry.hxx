/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_slntry.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:51:25 $
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

#ifndef ARY_CPP_C_SLNTRY_HXX
#define ARY_CPP_C_SLNTRY_HXX
//  KORR_DEPRECATED_3.0

// USED SERVICES
#include <ary/cpp/c_types4cpp.hxx>
#include <ary/namesort.hxx>


namespace ary
{
namespace cpp
{
    class Namespace;



typedef Namespace * NamespacePtr;
struct Less_NamespacePtr
{
    bool                operator()(             /// @return true if (i_p1->Name() < i_p2->Name()) .
                            const NamespacePtr& i_p1,
                            const NamespacePtr& i_p2 );
};




struct S_Classes_Base
{
    Type_id             nId;
    E_Protection        eProtection;
    E_Virtuality        eVirtuality;
    String              sComment;

                        S_Classes_Base()
                            :   nId(0),
                                eProtection(PROTECT_global),
                                eVirtuality(VIRTUAL_none)
                                // sComment
                                { }
};

struct S_TplParam
{
    String              sName;
    Type_id             nId;

                        S_TplParam(
                            String              i_sName,
                            Type_id             i_nId )
                            :   sName(i_sName), nId(i_nId) {}
    const String  &     Name() const            { return sName; }
};


struct S_LocalCe
{
    String              sLocalName;
    Ce_id               nId;

                        S_LocalCe()             : nId(0) {}
                        S_LocalCe(
                            const String  &     i_sLocalName,
                            Cid                 i_nId )
                                                : sLocalName(i_sLocalName), nId(i_nId) {}
    bool                operator<(
                            const S_LocalCe &   i_rCe ) const
                                                { return LesserName()(sLocalName,i_rCe.sLocalName); }
};

typedef std::vector< S_LocalCe >        List_LocalCe;


typedef std::map<String, NamespacePtr>          Map_NamespacePtr;
typedef std::vector< S_Classes_Base >           List_Bases;
typedef std::vector< S_TplParam >               List_TplParam;



}   // namespace cpp
}   // namespace ary
#endif
