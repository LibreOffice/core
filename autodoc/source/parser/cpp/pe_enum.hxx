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

#ifndef ADC_CPP_PE_ENUM_HXX
#define ADC_CPP_PE_ENUM_HXX

// USED SERVICES
    // BASE CLASSES
#include "cpp_pe.hxx"
    // COMPONENTS
#include <semantic/callf.hxx>
#include <semantic/sub_peu.hxx>

namespace cpp {


class PE_EnumValue;

class PE_Enum : public cpp::Cpp_PE
{
  public:
    enum E_State
    {
        expectName,         /// after "enum"
        gotName,            /// after name, before : or {
        bodyStd,            /// after {
        afterBlock,         /// after ending }
        size_of_states
    };

    enum E_KindOfResult
    {
        is_declaration,             // normal
        is_implicit_declaration,    // like in: enum Abc { rot, gelb, blau } aAbc;
        is_qualified_typename       // like in: enum Abc * fx();

    };
                        PE_Enum(
                            Cpp_PE *            i_pParent );
                        ~PE_Enum();

    virtual void        Call_Handler(
                            const cpp::Token &  i_rTok );

    E_KindOfResult      Result_KindOf() const;
    const String  &     Result_LocalName() const;
    const String  &     Result_FirstNameSegment() const;

  private:
    typedef SubPe< PE_Enum, PE_EnumValue >      SP_EnumValue;
    typedef SubPeUse< PE_Enum, PE_EnumValue>    SPU_EnumValue;

    void                Setup_StatusFunctions();
    virtual void        InitData();
    virtual void        TransferData();
    void                Hdl_SyntaxError( const char *);

    void                On_expectName_Identifier( const char * );
    void                On_expectName_SwBracket_Left( const char * );

    void                On_gotName_SwBracket_Left( const char * );
    void                On_gotName_Return2Type( const char * );

    void                On_bodyStd_Identifier( const char * );
    void                On_bodyStd_SwBracket_Right( const char * );

    void                On_afterBlock_Semicolon( const char * );
    void                On_afterBlock_Return2Type( const char * );

    // DATA
    Dyn< PeStatusArray<PE_Enum> >
                        pStati;
    Dyn<SP_EnumValue>   pSpValue;
    Dyn<SPU_EnumValue>  pSpuValue;

    String              sLocalName;
    ary::cpp::Enum *    pCurObject;

    E_KindOfResult      eResult_KindOf;
};



// IMPLEMENTATION
inline PE_Enum::E_KindOfResult
PE_Enum::Result_KindOf() const
{
    return eResult_KindOf;
}

inline const String  &
PE_Enum::Result_LocalName() const
{
    return sLocalName;
}

inline const String  &
PE_Enum::Result_FirstNameSegment() const
{
    return sLocalName;
}


}   // namespace cpp


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
