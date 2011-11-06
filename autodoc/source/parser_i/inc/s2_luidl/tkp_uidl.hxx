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



#ifndef ADC_TKP_UIDL_HXX
#define ADC_TKP_UIDL_HXX

// USED SERVICES
    // BASE CLASSES
#include <tokens/tkp2.hxx>
    // COMPONENTS
    // PARAMETRS

class TkpDocuContext;


namespace csi
{
namespace uidl
{



class Token_Receiver;
class Context_UidlCode;


/** This is a TokenParser which is able to parse tokens from
    C++ source code.
*/
class TokenParser_Uidl : public TokenParse2
{
  public:
    // LIFECYCLE
                        TokenParser_Uidl(
                            Token_Receiver &    o_rUidlReceiver,
                            DYN TkpDocuContext &
                                                let_drDocuContext );
    virtual             ~TokenParser_Uidl();

    // OPERATIONS
  private:
    virtual ::TkpContext &
                        CurrentContext();

    virtual void        SetStartContext();
    virtual void        SetCurrentContext(
                            TkpContext &        io_rContext );
    // DATA
    Dyn<Context_UidlCode>
                        pBaseContext;
    ::TkpContext *      pCurContext;
};


}   // namespace uidl
}   // namespace csi

#endif


