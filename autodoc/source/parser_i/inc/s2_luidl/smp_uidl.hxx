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



#ifndef ADC_SMP_HXX
#define ADC_SMP_HXX



// USED SERVICES
    // BASE CLASSES
#include <s2_luidl/tok_recv.hxx>
#include <s2_dsapi/tok_recv.hxx>
    // COMPONENTS
    // PARAMETERS

namespace csi
{
namespace uidl
{



/** is an implementation class for ParseEnvironment
*/
class SemanticParser : public csi::uidl::Token_Receiver,
                       public csi::dsapi::Token_Receiver
{
  public:
  typedef std::deque< DYN TextToken * > TokenQueue;

                        ~SemanticParser();


    void                Receive(
                            DYN csi::uidl::Token &
                                                let_drToken );
    void                Receive(
                            DYN csi::dsapi::Token &
                                                let_drToken );

  private:
    // DATA
    TokenQueue          aTokenQueue;


};



// IMPLEMENTATION


}   // namespace uidl
}   // namespace csi

#endif


