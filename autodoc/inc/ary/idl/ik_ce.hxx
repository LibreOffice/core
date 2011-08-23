/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef ARY_IDL_IK_CE_HXX
#define ARY_IDL_IK_CE_HXX
//  KORR_DEPRECATED_3.0



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <ary/idl/i_types4idl.hxx>
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
