/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ik_ce.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 11:48:55 $
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

#ifndef ARY_IDL_IK_CE_HXX
#define ARY_IDL_IK_CE_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <ary/idl/i_language.hxx>
#include <ary/stdconstiter.hxx>

namespace ary
{
namespace info
{
     class Text;
}
namespace idl
{


namespace ifc_ce
{


typedef ::ary::Dyn_StdConstIterator<Ce_id>      Dyn_CeIterator;
typedef ::ary::Dyn_StdConstIterator<Type_id>    Dyn_TypeIterator;
typedef ::ary::info::Text                       DocText;



struct attr
{
    static Ce_id        CeId(
                            const CodeEntity &  i_ce );
    static const String &
                        LocalName(
                            const CodeEntity &  i_ce );
    static Ce_id        NameRoom(
                            const CodeEntity &  i_ce );
    static Rid          Owner(
                            const CodeEntity &  i_ce );
    static E_SightLevel SightLevel(
                            const CodeEntity &  i_ce );
    static bool         Search_Member(
                            const CodeEntity &  ,
                            const String &      )
                            { return true; }    // KORR_FUTURE
};

struct xref
{
};

struct doc
{
    static const DocText &
                        ShortInfo(          /// @return a short description of the CodeEntity
                            const CodeEntity &  i_ce );

    static const DocText &
                        TagAuthor(
                            const CodeEntity &  i_ce );
    static const DocText &
                        TagExample(
                            const CodeEntity &  i_ce );
    static const DocText &
                        TagDescr(
                            const CodeEntity &  i_ce );
    static const DocText &
                        TagGuarantees(
                            const CodeEntity &  i_ce );
    static const DocText &
                        TagKey(
                            const CodeEntity &  i_ce );
    static const DocText &
                        TagMissing(
                            const CodeEntity &  i_ce );
    static const DocText &
                        TagSee(
                            const CodeEntity &  i_ce );
    static const DocText &
                        TagShort(
                            const CodeEntity &  i_ce );
    static const DocText &
                        TagVersion(
                            const CodeEntity &  i_ce );

    void                Get_UnkownTags(
                            Dyn_CeIterator &    o_result,
                            const CodeEntity &  i_ce );

    bool                IsDeprecated(
                            const CodeEntity &  i_ce );
    bool                IsIncomplete(
                            const CodeEntity &  i_ce );
    bool                IsInternal(
                            const CodeEntity &  i_ce );
    bool                IsNodoc(
                            const CodeEntity &  i_ce );
    bool                IsOptional(
                            const CodeEntity &  i_ce );
    bool                IsSuspicious(
                            const CodeEntity &  i_ce );

};


}   // namespace ifc_ce


}   // namspace idl
}   // namspace ary

#endif


