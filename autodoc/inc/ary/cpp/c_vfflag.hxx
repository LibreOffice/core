/*************************************************************************
 *
 *  $RCSfile: c_vfflag.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef ARY_CPP_C_VFFLAG_HXX
#define ARY_CPP_C_VFFLAG_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS



namespace ary
{
namespace cpp
{


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

struct S_Parameter
{
    udmstri             sName;
    udmstri             sSizeExpression;
    udmstri             sInitExpression;
    Tid                 nType;

                        S_Parameter()           : nType(0) {}
                        ~S_Parameter()          {}
    void                Empty()                 { nType = 0;
                                                  sName.clear();
                                                  sSizeExpression.clear();
                                                  sInitExpression.clear(); }
};



}   // namespace cpp
}   // namespace ary


#endif

