/*************************************************************************
 *
 *  $RCSfile: aryattrs.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:22 $
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

#ifndef ADC_DISPLAY_ARYATTRS_HXX
#define ADC_DISPLAY_ARYATTRS_HXX



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
        class Class;
        class DisplayGate;
        class Function;
        class Namespace;
    }
    class CodeEntity;
}

const char *        Get_ClassTypeKey(
                        const ary::cpp::Class &
                                            i_rClass );
const char *        Get_TypeKey(
                        const ary::CodeEntity &
                                            i_rCe );
bool                Ce_IsInternal(
                        const ary::CodeEntity &
                                            i_rCe );

const char *        Namespace_DisplayName(
                        const ary::cpp::Namespace &
                                            i_rNsp );

const char *        TypeText(
                        ary::Tid                i_nId,
                        const ary::cpp::DisplayGate &
                                                i_rAryGate );

const char *        SyntaxText_PreName(
                        const ary::cpp::Function &
                                                i_rFunction,
                        const ary::cpp::DisplayGate &
                                                i_rAryGate );
const char *        SyntaxText_PostName(
                        const ary::cpp::Function &
                                                i_rFunction,
                        const ary::cpp::DisplayGate &
                                                i_rAryGate );

bool                Get_TypeText(
                        const char * &          o_rPreName,
                        const char * &          o_rName,
                        const char * &          o_rPostName,
                        ary::Tid                i_nTypeid,
                        const ary::cpp::DisplayGate &
                                                i_rAryGate );



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
                                                i_rFunction,
                        const ary::cpp::DisplayGate &
                                                i_rAryGate );

    ary::Tid        CurType() const;
    const udmstri & CurName() const;

    bool            IsFunctionConst() const;
    bool            IsFunctionVolatile() const;

  private:
    typedef std::vector<ary::Tid>::const_iterator        Type_Iterator;
    typedef StringVector::const_iterator                 Name_Iterator;

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

inline ary::Tid
FunctionParam_Iterator::CurType() const
    { return IsValid() ? *itTypes : 0; }
inline const udmstri &
FunctionParam_Iterator::CurName() const
    { return IsValid() ? *itNames_andMore : udmstri::Null_(); }
inline bool
FunctionParam_Iterator::IsFunctionConst() const
    { return (eConVol & ary::cpp::CONVOL_const) != 0; }
inline bool
FunctionParam_Iterator::IsFunctionVolatile() const
    { return (eConVol & ary::cpp::CONVOL_volatile) != 0; }


#endif


