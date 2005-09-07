/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_slntry.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:01:07 $
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

// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <ary/ids.hxx>
#include <ary/cpp/c_etypes.hxx>


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
    Tid                 nId;
    E_Protection        eProtection;
    E_Virtuality        eVirtuality;
    udmstri             sComment;

                        S_Classes_Base()
                            :   nId(0),
                                eProtection(PROTECT_global),
                                eVirtuality(VIRTUAL_none)
                                // sComment
                                { }
};

struct S_TplParam
{
    udmstri             sName;
    Tid                 nId;

                        S_TplParam(
                            udmstri             i_sName,
                            Tid                 i_nId )
                            :   sName(i_sName), nId(i_nId) {}
    const udmstri &     Name() const            { return sName; }

};



}   // namespace cpp
}   // namespace ary



#endif

