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

#ifndef ADC_DISPLAY_ARYATTRS_HXX
#define ADC_DISPLAY_ARYATTRS_HXX

// USED SERVICES
#include <ary/cpp/c_types4cpp.hxx>
#include <ary/doc/d_docu.hxx>
#include <ary/doc/d_oldcppdocu.hxx>

namespace ary
{
     namespace cpp
    {
        class CodeEntity;
        class Class;
        class DisplayGate;
        class Function;
        class Namespace;
    }
}




const char *        Get_ClassTypeKey(
                        const ary::cpp::Class & i_rClass );
const char *        Get_TypeKey(
                        const ary::cpp::CodeEntity &
                                                i_rCe );
bool                Ce_IsInternal(
                        const ary::cpp::CodeEntity &
                                                i_rCe );
const char *        SyntaxText_PreName(
                        const ary::cpp::Function &
                                                i_rFunction,
                        const ary::cpp::Gate &  i_rAryGate );
const char *        SyntaxText_PostName(
                        const ary::cpp::Function &
                                                i_rFunction,
                        const ary::cpp::Gate &  i_rAryGate );

bool                Get_TypeText(
                        const char * &          o_rPreName,
                        const char * &          o_rName,
                        const char * &          o_rPostName,
                        ary::cpp::Type_id       i_nTypeid,
                        const ary::cpp::Gate &  i_rAryGate );


inline const ary::doc::OldCppDocu *
Get_CppDocu(const ary::doc::Documentation & i_doc)
{
    return dynamic_cast< const ary::doc::OldCppDocu* >(i_doc.Data());
}


class FunctionParam_Iterator
{
  public:
                    FunctionParam_Iterator();
                    ~FunctionParam_Iterator();

                    operator bool() const;
    FunctionParam_Iterator &
                    operator++();

    void            Assign(
                        const ary::cpp::Function &
                                                i_rFunction );

    ary::cpp::Type_id
                    CurType() const;
    const String  & CurName() const;

    bool            IsFunctionConst() const;
    bool            IsFunctionVolatile() const;

  private:
    typedef std::vector<ary::cpp::Type_id>::const_iterator  Type_Iterator;
    typedef StringVector::const_iterator                    Name_Iterator;

    bool                IsValid() const;

    // Forbidden
    FunctionParam_Iterator &
                    operator++(int);
    // DATA
    Type_Iterator       itTypes;
    Type_Iterator       itTypes_end;
    Name_Iterator       itNames_andMore;        /// Name, init-value.
    Name_Iterator       itNames_andMore_end;

    ary::cpp::E_ConVol  eConVol;
};




// IMPLEMENTATION
inline
FunctionParam_Iterator::operator bool() const
    { return IsValid(); }

inline bool
FunctionParam_Iterator::IsValid() const
{
    // By C'tor and Assign(), it is assured, that
    //   both iterators are valid, if one is valid.
    return itTypes != itTypes_end;
}

inline ary::cpp::Type_id
FunctionParam_Iterator::CurType() const
    { return IsValid() ? *itTypes : ary::cpp::Type_id(0); }
inline const String  &
FunctionParam_Iterator::CurName() const
    { return IsValid() ? *itNames_andMore : String::Null_(); }
inline bool
FunctionParam_Iterator::IsFunctionConst() const
    { return (eConVol & ary::cpp::CONVOL_const) != 0; }
inline bool
FunctionParam_Iterator::IsFunctionVolatile() const
    { return (eConVol & ary::cpp::CONVOL_volatile) != 0; }




#endif
