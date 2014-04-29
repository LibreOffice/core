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



#ifndef ARY_DOC_D_PARAMETER_HXX
#define ARY_DOC_D_PARAMETER_HXX

// USED SERVICES
    // BASE CLASSES
#include <ary/doc/d_node.hxx>

namespace ary
{
namespace doc
{


/** Documentation unit with Parameter.
*/
template <class T>
class Parametrized : public Node
{
  public:
    // LIFECYCLE
    explicit            Parametrized(
                            nodetype::id        i_id,
                            T                   i_Parameter );
    virtual             ~Parametrized();

    // INQUIRY
    const HyperText &   Doc() const;
    const T &           Parameter() const;

    // ACCESS
    HyperText &         Doc();
    void                Set_Parameter(
                            const T &           i_param );
  private:
    // Interface csv::ConstProcessorClient:
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;
    // DATA
    HyperText           aDoc;
    T                   aParameter;
};




// IMPLEMENTATION
template <class T>
Parametrized<T>::Parametrized( nodetype::id     i_id,
                               T                i_Parameter )
    :   Node(i_id),
        aDoc(),
        aParameter(i_Parameter)
{
}

template <class T>
Parametrized<T>::~Parametrized()
{
}

template <class T>
const HyperText &
Parametrized<T>::Doc() const
{
    return aDoc;
}

template <class T>
const T &
Parametrized<T>::Parameter() const
{
    return aParameter;
}

template <class T>
HyperText &
Parametrized<T>::Doc()
{
    return aDoc;
}

template <class T>
inline void
Parametrized<T>::Set_Parameter(const T & i_param)
{
    aParameter = i_param;
}




}   // namespace doc
}   // namespace ary
#endif
