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



#ifndef ADC_TOKEN_HXX
#define ADC_TOKEN_HXX

// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETRS


class TokenDealer;

/**
*/
class TextToken
{
  public:
    typedef TextToken * (*F_CRTOK)(const char*);

    // LIFECYCLE
    virtual             ~TextToken() {}


    // INQUIRY
    virtual const char* Text() const = 0;

    virtual void        DealOut(
                            ::TokenDealer &     o_rDealer ) = 0;
};

class Tok_Eof : public TextToken
{
    virtual void        DealOut(                // Implemented in tokdeal.cxx
                            TokenDealer &       o_rDealer );
    virtual const char* Text() const;
};

#endif


