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

#include <precomp.h>
#include <ary/cpp/c_builtintype.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/cpp/c_type.hxx>


namespace ary
{
namespace cpp
{




//**********************        Type        **************************//
Rid
Type::inq_RelatedCe() const
{
     return 0;
}


//**********************        BuiltInType        **************************//

BuiltInType::BuiltInType( const String  &		i_sName,
                          E_TypeSpecialisation	i_eSpecialisation )
    :	sName( i_sName ),
        eSpecialisation( i_eSpecialisation )
{
}

String
BuiltInType::SpecializedName_( const char *		    i_sName,
                               E_TypeSpecialisation i_eTypeSpecialisation )
{
    StreamLock
        aStrLock(60);
    StreamStr &
        ret = aStrLock();

    switch ( i_eTypeSpecialisation )
    {
        case TYSP_unsigned:
                    ret << "u_";
                    break;
        case TYSP_signed:
                    if (strcmp(i_sName,"char") == 0)
                        ret << "s_";
                    break;
        default:
                    ;

    }	// end switch

    ret << i_sName;
    return String(ret.c_str());
}

void
BuiltInType::do_Accept(csv::ProcessorIfc & io_processor) const
{
    csv::CheckedCall(io_processor,*this);
}

ary::ClassId
BuiltInType::get_AryClass() const
{
    return class_id;
}

bool
BuiltInType::inq_IsConst() const
{
    return false;
}

void
BuiltInType::inq_Get_Text( StreamStr &      ,               // o_rPreName
                           StreamStr &      o_rName,
                           StreamStr &      ,               // o_rPostName
                           const Gate &     ) const         // i_rGate
{
    switch (eSpecialisation)
    {
        case TYSP_unsigned:	o_rName << "unsigned "; break;
        case TYSP_signed:   o_rName << "signed ";   break;

        default:            // Does nothing.
                            ;
    }
    o_rName << sName;
}




}   // namespace cpp
}   // namespace ary

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
