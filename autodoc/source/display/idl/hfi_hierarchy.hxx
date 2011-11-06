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



#ifndef ADC_DISPLAY_HFI_HIERARCHY_HXX
#define ADC_DISPLAY_HFI_HIERARCHY_HXX

// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <ary/idl/i_comrela.hxx>
#include <ary/idl/i_types4idl.hxx>


namespace csi
{
namespace xml
{
    class Element;
}
}


class HF_IdlInterface;
class HtmlEnvironment_Idl;



/** Represents a node in an pyramidic inheritance hierarchy which shall be
    displayed in text mode.
*/
class HF_IdlBaseNode
{
  public:
    typedef ary::idl::CodeEntity            CE;
    typedef ary::idl::Type                  TYPE;
    typedef ary::idl::Gate                  GATE;
    typedef ary::idl::Ce_id                 Ce_id;
    typedef ary::idl::Type_id               Type_id;

    /** @descr
        The constructor recursively calls further constructors of
        HF_IdlBaseNode for the bases of ->i_rType, if ->i_rType matches to a
        ->CE.
        So it builds up a complete hierarchy tree of all base classes
        of ->i_pEntity.
    */
                        HF_IdlBaseNode(
                            const TYPE &        i_rType,
                            const GATE &        i_rGate,
                            intt                i_nPositionOffset,
                            HF_IdlBaseNode &    io_rDerived );
                        ~HF_IdlBaseNode();

    /** Recursively fills ->o_rPositionList with the instances of base
        classes in the order in which they will be displayed.
    */
    void                FillPositionList(
                            std::vector< const HF_IdlBaseNode* > &
                                                o_rPositionList ) const;

    Type_id             Type() const            { return nType; }
    intt                BaseCount() const       { return nCountBases; }
    intt                Position() const        { return nPosition; }
    int                 Xpos() const            { return 3*Position(); }
    int                 Ypos() const            { return 2*Position(); }
    const HF_IdlBaseNode *        Derived() const         { return pDerived; }

  private:
    typedef std::vector< DYN HF_IdlBaseNode* > BaseList;

    void                GatherBases(
                            const CE &          i_rCe,
                            const GATE &        i_rGate );

    // DATA
    Type_id             nType;
    BaseList            aBases;
    intt                nCountBases;
    intt                nPosition;
    HF_IdlBaseNode *    pDerived;
};

void                Write_BaseHierarchy(
                        csi::xml::Element & o_rOut,
                        HtmlEnvironment_Idl &
                                            i_env,
                        const ary::idl::CodeEntity &
                                            i_rCe );

void                Write_Bases(
                        csi::xml::Element & o_rOut,
                        HtmlEnvironment_Idl &
                                            i_env,
                        const ary::idl::CodeEntity &
                                            i_rCe,
                        std::vector<uintt> &
                                            io_setColumns );

#endif
