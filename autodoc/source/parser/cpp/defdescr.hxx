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

#ifndef ADC_CPP_DEFDESCR_HXX
#define ADC_CPP_DEFDESCR_HXX




namespace cpp
{

/** Describes a C/C++ #define statement. May be a define or a macro, for which
    two cases the two different constructors are to be used.

    This class is used by cpp::PreProcessor.
*/
class DefineDescription
{
  public:
    enum E_DefineType
    {
        type_define,
        type_macro
    };
    typedef StringVector str_vector;

                        DefineDescription(      /// Used for: #define DEFINE xyz
                            const String  &     i_sName,
                            const str_vector &  i_rDefinition );
                        DefineDescription(      /// Used for: #define MACRO(...) abc
                            const String  &     i_sName,
                            const str_vector &  i_rParams,
                            const str_vector &  i_rDefinition );
                        ~DefineDescription();

    /// Only vaild if (eDefineType == type_define) else returns "".
    void                GetDefineText(
                            csv::StreamStr &    o_rText ) const;

    /// Only vaild if (eDefineType == type_macro) else returns "".
    void                GetMacroText(
                            csv::StreamStr &    o_rText,
                            const StringVector &
                                                i_rGivenArguments ) const;

    uintt               ParamCount() const;
    E_DefineType        DefineType() const;

  private:
    // DATA
    String              sName;
    str_vector          aParams;
    str_vector          aDefinition;
    E_DefineType        eDefineType;
};




// IMPLEMENTATION
inline uintt
DefineDescription::ParamCount() const
    { return aParams.size(); }
inline DefineDescription::E_DefineType
DefineDescription::DefineType() const
    { return eDefineType; }




}   // end namespace cpp
#endif
