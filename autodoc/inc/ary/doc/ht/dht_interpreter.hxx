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



#ifndef ARY_DHT_INTERPRETER_HXX
#define ARY_DHT_INTERPRETER_HXX




namespace ary
{
namespace doc
{
namespace ht
{
    class Processor;


/** Interface for all interpreters of a ->Component.
*/
class Interpreter
{
  public:
    virtual             ~Interpreter() {}

    void                Accept(
                            Processor &         io_processor,
                            const String &      i_data ) const;
  private:
    virtual void        do_Accept(
                            Processor &         io_processor,
                            const String &      i_data ) const = 0;
};




// IMPLEMENTATION
inline void
Interpreter::Accept( Processor &     io_processor,
                     const String &  i_data ) const
{
    do_Accept(io_processor, i_data);
}




}   // namespace ht
}   // namespace doc
}   // namespace ary
#endif
