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



#ifndef ADC_CPP_TOKPROCT_HXX
#define ADC_CPP_TOKPROCT_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS


class ParseEnvironment;

/** is a parent class for classes, which take part in parsing tokens semantically.
    It provides some types for them.
*/
class TokenProcessing_Types
{
  public:
    enum E_TokenDone
    {
        not_done = 0,
        done = 1
    };

    enum E_EnvStackAction
    {
        stay,           // same parse environment
        push,           // push sub environment
        pop_success,    // return to parent environment, parsing was successful
        pop_failure     // return to parent environment, but an error occurred.
    };

    struct TokenProcessing_Result
    {
        E_TokenDone         eDone;
        E_EnvStackAction    eStackAction;
        ParseEnvironment *  pEnv2Push;

                            TokenProcessing_Result()
                                                : eDone(not_done), eStackAction(stay), pEnv2Push(0) {}
        void                Reset()             { eDone = not_done; eStackAction = stay; pEnv2Push = 0; }
    };

    enum E_ParseResult
    {
        res_error,
        res_complete,
        res_predeclaration
    };
};



#endif

