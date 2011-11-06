/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
