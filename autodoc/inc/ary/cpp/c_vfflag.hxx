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

#ifndef ARY_CPP_C_VFFLAG_HXX
#define ARY_CPP_C_VFFLAG_HXX

// USED SERVICES


namespace ary
{
namespace cpp
{


/** Properties of C++ variables.
*/
struct VariableFlags
{
  public:
    enum E_Flags
    {
        f_static_local 		= 0x0001,
        f_static_member 	= 0x0002,
        f_extern 			= 0x0004,
        f_mutable			= 0x0008
    };

                        VariableFlags(
                            UINT16              i_nFlags = 0 )
                                                :   nFlags(i_nFlags) {}

    void                Reset()                 { nFlags = 0; }

    void 				SetStaticLocal()		{ nFlags |= f_static_local; }
    void 				SetStaticMember()		{ nFlags |= f_static_member; }
    void 				SetExtern() 	  		{ nFlags |= f_extern; }
    void 				SetMutable() 			{ nFlags |= f_mutable; }

    bool 				IsStaticLocal()	const	{ return (nFlags & f_static_local) != 0; }
    bool 				IsStaticMember() const	{ return (nFlags & f_static_member) != 0; }
    bool 				IsExtern() const  		{ return (nFlags & f_extern) != 0; }
    bool 				IsMutable() const		{ return (nFlags & f_mutable) != 0; }

  private:
    UINT16              nFlags;
};


/** Properties of C++ functions.
*/
struct FunctionFlags
{
  public:
    enum E_Flags
    {
        f_static_local 		= 0x0001,
        f_static_member 	= 0x0002,
        f_extern 			= 0x0004,
        f_externC 			= 0x0008,
        f_mutable			= 0x0010,
        f_inline 	        = 0x0100,
        f_register		 	= 0x0200,
        f_explicit			= 0x0400
    };

                        FunctionFlags(
                            UINT16              i_nFlags = 0 )
                                                :   nFlags(i_nFlags) {}

    bool                operator==(
                            const FunctionFlags &
                                                i_ff ) const
                                                { return nFlags == i_ff.nFlags; }
    bool                operator!=(
                            const FunctionFlags &
                                                i_ff ) const
                                                { return NOT operator==(i_ff); }

    void                Reset()                 { nFlags = 0; }

    void 				SetStaticLocal()		{ nFlags |= f_static_local; }
    void 				SetStaticMember()		{ nFlags |= f_static_member; }
    void 				SetExtern() 	  		{ nFlags |= f_extern; }
    void 				SetExternC() 	  		{ nFlags |= f_externC; }
    void 				SetMutable() 			{ nFlags |= f_mutable; }
    void 				SetInline() 	  	    { nFlags |= f_inline; }
    void 				SetRegister() 			{ nFlags |= f_register; }
    void 				SetExplicit() 		    { nFlags |= f_explicit; }

    bool 				IsStaticLocal()	const	{ return (nFlags & f_static_local) != 0; }
    bool 				IsStaticMember() const	{ return (nFlags & f_static_member) != 0; }
    bool 				IsExtern() const  		{ return (nFlags & f_extern) != 0; }
    bool 				IsExternC() const  		{ return (nFlags & f_externC) != 0; }
    bool 				IsMutable() const		{ return (nFlags & f_mutable) != 0; }
    bool 				IsInline() const  	    { return (nFlags & f_inline) != 0; }
    bool 				IsRegister() const		{ return (nFlags & f_register) != 0; }
    bool                IsExplicit() const      { return (nFlags & f_explicit) != 0; }

  private:
    UINT16              nFlags;
};


/** A C++ function parameter.
*/
struct S_Parameter
{
    String              sName;
    String              sSizeExpression;
    String              sInitExpression;
    Type_id             nType;

                        S_Parameter()           : nType(0) {}
                        ~S_Parameter()          {}
    void                Empty()                 { nType = Type_id(0);
                                                  sName.clear();
                                                  sSizeExpression.clear();
                                                  sInitExpression.clear(); }
};




}   // namespace cpp
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
