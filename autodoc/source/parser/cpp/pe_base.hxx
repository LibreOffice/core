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




#ifndef ADC_CPP_PE_BASE_HXX
#define ADC_CPP_PE_BASE_HXX

// BASE CLASSES
#include "cpp_pe.hxx"
// USED SERVICES
#include <semantic/callf.hxx>
#include <semantic/sub_peu.hxx>
#include <ary/cpp/c_types4cpp.hxx>
#include <ary/cpp/c_slntry.hxx>



namespace cpp
{

class PE_Type;

class PE_Base : public Cpp_PE
{
  public:
    enum E_State
    {
        startOfNext,
        inName,
        size_of_states
    };

    typedef ary::cpp::List_Bases        BaseList;
    typedef ary::cpp::S_Classes_Base    Base;

                        PE_Base(
                            Cpp_PE *            i_pParent );
                        ~PE_Base();

    const BaseList &    Result_BaseIds() const;

    virtual void        Call_Handler(
                            const cpp::Token &  i_rTok );

  private:
    typedef SubPe< PE_Base, PE_Type >           SP_Type;
    typedef SubPeUse< PE_Base, PE_Type>         SPU_BaseName;

    void                Setup_StatusFunctions();
    virtual void        InitData();
    virtual void        TransferData();
    void                Hdl_SyntaxError( const char *);

    void                SpReturn_BaseName();

    void                On_startOfNext_Identifier(const char *);
    void                On_startOfNext_public(const char *);
    void                On_startOfNext_protected(const char *);
    void                On_startOfNext_private(const char *);
    void                On_startOfNext_virtual(const char *);
    void                On_startOfNext_DoubleColon(const char *);

    void                On_inName_Identifier(const char *);
    void                On_inName_virtual(const char *);
    void                On_inName_SwBracket_Left(const char *);
    void                On_inName_DoubleColon(const char *);
    void                On_inName_Comma(const char *);

    Base &              CurObject();

    // DATA
    Dyn< PeStatusArray<PE_Base> >
                        pStati;

    Dyn<SP_Type>        pSpType;            /// till "{" incl.
    Dyn<SPU_BaseName>   pSpuBaseName;

    BaseList            aBaseIds;
};



// IMPLEMENTATION

inline const PE_Base::BaseList &
PE_Base::Result_BaseIds() const
    { return aBaseIds; }


inline PE_Base::Base &
PE_Base::CurObject()
    { return aBaseIds.back(); }





}   // namespace cpp
#endif
