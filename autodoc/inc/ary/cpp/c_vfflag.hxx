/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_vfflag.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:53:01 $
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
        f_static_local      = 0x0001,
        f_static_member     = 0x0002,
        f_extern            = 0x0004,
        f_mutable           = 0x0008
    };

                        VariableFlags(
                            UINT16              i_nFlags = 0 )
                                                :   nFlags(i_nFlags) {}

    void                Reset()                 { nFlags = 0; }

    void                SetStaticLocal()        { nFlags |= f_static_local; }
    void                SetStaticMember()       { nFlags |= f_static_member; }
    void                SetExtern()             { nFlags |= f_extern; }
    void                SetMutable()            { nFlags |= f_mutable; }

    bool                IsStaticLocal() const   { return (nFlags & f_static_local) != 0; }
    bool                IsStaticMember() const  { return (nFlags & f_static_member) != 0; }
    bool                IsExtern() const        { return (nFlags & f_extern) != 0; }
    bool                IsMutable() const       { return (nFlags & f_mutable) != 0; }

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
        f_static_local      = 0x0001,
        f_static_member     = 0x0002,
        f_extern            = 0x0004,
        f_externC           = 0x0008,
        f_mutable           = 0x0010,
        f_inline            = 0x0100,
        f_register          = 0x0200,
        f_explicit          = 0x0400
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

    void                SetStaticLocal()        { nFlags |= f_static_local; }
    void                SetStaticMember()       { nFlags |= f_static_member; }
    void                SetExtern()             { nFlags |= f_extern; }
    void                SetExternC()            { nFlags |= f_externC; }
    void                SetMutable()            { nFlags |= f_mutable; }
    void                SetInline()             { nFlags |= f_inline; }
    void                SetRegister()           { nFlags |= f_register; }
    void                SetExplicit()           { nFlags |= f_explicit; }

    bool                IsStaticLocal() const   { return (nFlags & f_static_local) != 0; }
    bool                IsStaticMember() const  { return (nFlags & f_static_member) != 0; }
    bool                IsExtern() const        { return (nFlags & f_extern) != 0; }
    bool                IsExternC() const       { return (nFlags & f_externC) != 0; }
    bool                IsMutable() const       { return (nFlags & f_mutable) != 0; }
    bool                IsInline() const        { return (nFlags & f_inline) != 0; }
    bool                IsRegister() const      { return (nFlags & f_register) != 0; }
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
