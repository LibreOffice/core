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



#ifndef ARY_ARY_DISP_HXX
#define ARY_ARY_DISP_HXX
//  KORR_DEPRECATED_3.0


// USED SERVICES
    // BASE CLASSES
#include <cosv/tpl/processor.hxx>
    // COMPONENTS
    // PARAMETERS
#include <ary/types.hxx>
#include <ary/cpp/c_types4cpp.hxx>

namespace ary
{
namespace cpp
{
    class Gate;
}

class Display : public csv::ProcessorIfc
{
  public:
    virtual             ~Display() {}

    // OPERATIONS
    void                StartSlot();
    void                FinishSlot();

    /** This method depends on the result of Get_ReFinder().
        If Get_ReFinder() != 0, then DisplayGate::Find_Re() is called
        and if valid, cpp::CppEntity::Accept( *this ) is called.
        If Get_ReFinder() == 0, simply do_DisplaySlot_Rid() is called
        with just the id as parameter.
    */
    void                DisplaySlot_Rid(
                            ary::Rid            i_nId );
//    void                DisplaySlot_Lid(
//                          ary::Lid            i_nId );
    /** This method depends on the result of Get_ReFinder().
        If Get_ReFinder() != 0, then DisplayGate::Find_Re() is called
        and if valid, cpp::CppEntity::Accept( *this ) is called.
        If Get_ReFinder() == 0, simply do_DisplaySlot_LocalCe() is called
        with just the id as parameter.
    */
    void                DisplaySlot_LocalCe(
                            ary::cpp::Ce_id     i_nId,
                            const String  &     i_sName );
    // INQUIRY
    const cpp::Gate *   Get_ReFinder() const;

  private:
    virtual void        do_StartSlot();
    virtual void        do_FinishSlot();


    virtual void        do_DisplaySlot_Rid(
                            ary::Rid            i_nId );
    virtual void        do_DisplaySlot_LocalCe(
                            ary::cpp::Ce_id     i_nId,
                            const String  &     i_sName );
    virtual const cpp::Gate *
                        inq_Get_ReFinder() const = 0;
};


// IMPLEMENTATION


inline void
Display::StartSlot()
    { do_StartSlot(); }
inline void
Display::FinishSlot()
    { do_FinishSlot(); }
inline const cpp::Gate *
Display::Get_ReFinder() const
    { return inq_Get_ReFinder(); }




} // namespace ary
#endif
