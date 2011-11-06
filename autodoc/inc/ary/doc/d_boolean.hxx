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



#ifndef ARY_DOC_D_BOOLEAN_HXX
#define ARY_DOC_D_BOOLEAN_HXX

// BASE CLASSES
#include <ary/doc/d_node.hxx>

// USED SERVICES




namespace ary
{
namespace doc
{


/** Repesents a boolean documentation item like "optional" or "not optional".
*/
class Boolean : public Node
{
  public:
    // LIFECYCLE
    explicit            Boolean(
                            nodetype::id        i_type );
    virtual             ~Boolean();

    // OPERATIONS
    void                Set(
                            bool                i_b );
    // INQUIRY
    bool                IsTrue() const;

  private:
    // Interface csv::ConstProcessorClient:
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;
    // DATA
    bool                b;
};




// IMPLEMENTATION
inline
Boolean::Boolean(nodetype::id i_type)
    :   Node(i_type),
        b(false)
{
}

inline bool
Boolean::IsTrue() const
{
    return b;
}

inline void
Boolean::Set( bool i_b )
{
    b = i_b;
}



}   // namespace doc
}   // namespace ary
#endif
