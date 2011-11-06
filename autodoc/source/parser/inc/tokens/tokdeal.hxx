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



#ifndef ADC_TOKDEAL_HXX
#define ADC_TOKDEAL_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS

namespace cpp
{
    class Distributor;
}


class TokenDealer

{
  public:
    virtual             ~TokenDealer() {}

    virtual void        Deal_Eol() = 0;
    virtual void        Deal_Eof() = 0;
    virtual cpp::Distributor *
                        AsDistributor() = 0;
};


#if 0
class TokenDealer

{
  public:
    virtual void        Deal_IdlCode(
                            idl::Token &        let_drToken );
    virtual void        Deal_UdkDocu(
                            udoc::Token &       let_drToken );
    virtual void        Deal_JavaCode(
                            java::Token &       let_drToken );
    virtual void        Deal_SBasicCode(
                            sbasic::Token &     let_drToken );
};

#endif // 0



#endif

