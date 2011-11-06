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



#ifndef ARY_CPP_ICPRIVOW_HXX
#define ARY_CPP_ICPRIVOW_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/cpp/inpcontx.hxx>
    // COMPONENTS
    // PARAMETERS

namespace cpp
{



typedef ary::cpp::Ce_id     Cid;


class Owner_Namespace : public ary::cpp::InputContext::Owner
{
  public:
                        Owner_Namespace();
    void                SetAnotherNamespace(
                            ary::cpp::Namespace &
                                                io_rScope );
    virtual bool        HasClass(
                                const String  &     i_sLocalName );
  private:
    virtual void        do_Add_Class(
                            const String  &     i_sLocalName,
                            Cid                 i_nId );
    virtual void        do_Add_Enum(
                            const String  &     i_sLocalName,
                            Cid                 i_nId );
    virtual void        do_Add_Typedef(
                            const String  &     i_sLocalName,
                            Cid                 i_nId );
    virtual void        do_Add_Operation(
                                const String  &     i_sLocalName,
                                Cid                 i_nId,
                                bool                 );
    virtual void        do_Add_Variable(
                                const String  &     i_sLocalName,
                                Cid                 i_nId,
                                bool                i_bIsConst,
                                bool                i_bIsStatic );
    virtual Cid         inq_CeId() const;

    // DATA
    ary::cpp::Namespace *
                        pScope;
};

class Owner_Class : public ary::cpp::InputContext::Owner
{
  public:
                        Owner_Class();
    void                SetAnotherClass(
                            ary::cpp::Class &   io_rScope );

    /** @attention Only a dummy for use at ary::cpp::Gate!
        Will work nerver!
    */
    virtual bool        HasClass(
                                const String  &     i_sLocalName );
  private:
    virtual void        do_Add_Class(
                            const String  &     i_sLocalName,
                            Cid                 i_nId );
    virtual void        do_Add_Enum(
                            const String  &     i_sLocalName,
                            Cid                 i_nId );
    virtual void        do_Add_Typedef(
                            const String  &     i_sLocalName,
                            Cid                 i_nId );
    virtual void        do_Add_Operation(
                                const String  &     i_sLocalName,
                                Cid                 i_nId,
                                bool                i_bIsStaticMember );
    virtual void        do_Add_Variable(
                                const String  &     i_sLocalName,
                                Cid                 i_nId,
                                bool                i_bIsConst,
                                bool                i_bIsStatic );
    virtual Cid         inq_CeId() const;

    // DATA
    ary::cpp::Class *   pScope;
};




}   // namespace cpp
#endif
