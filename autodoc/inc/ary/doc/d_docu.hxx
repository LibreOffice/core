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



#ifndef ARY_DOC_D_DOCU_HXX
#define ARY_DOC_D_DOCU_HXX

// BASE CLASSES
#include <cosv/tpl/processor.hxx>

// USED SERVICES
#include <ary/doc/d_node.hxx>




namespace ary
{
namespace doc
{


/** Represents a documentation which is assigned to an Autodoc
    repository entity.
*/
class Documentation : public csv::ConstProcessorClient
{
  public:
                        Documentation();
                        ~Documentation();
    // OPERATIONS
    void                Clear();

    // INQUIRY
    const Node *        Data() const;

    // ACCESS
    Node *              Data();
    void                Set_Data(
                            ary::doc::Node &    i_data );

  private:
    // Interface csv::ConstProcessorClient:
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;
    // DATA
    Dyn<Node>           pData;
};




// IMPLEMENTATION
inline void
Documentation::Clear()
{
    pData = 0;
}

inline const Node *
Documentation::Data() const
{
    return pData.Ptr();
}

inline Node *
Documentation::Data()
{
    return pData.Ptr();
}

inline void
Documentation::Set_Data(ary::doc::Node & i_data)
{
    pData = &i_data;
}




}   // namespace doc
}   // namespace ary
#endif
