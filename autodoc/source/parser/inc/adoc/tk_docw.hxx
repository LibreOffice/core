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



#ifndef ADC_ADOC_TK_DOCW_HXX
#define ADC_ADOC_TK_DOCW_HXX

// USED SERVICES
    // BASE CLASSES
#include <adoc/adoc_tok.hxx>
    // COMPONENTS
    // PARAMETERS

namespace adoc {


class Tok_DocWord : public Token
{
  public:
    // Spring and Fall
                        Tok_DocWord(
                            const char *        i_sText )
                                                :   sText(i_sText) {}
    // OPERATIONS
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    // INQUIRY
    virtual const char* Text() const;
    uintt               Length() const          { return sText.length(); }

  private:
    // DATA
    String              sText;
};

class Tok_Whitespace : public Token
{
  public:
    // Spring and Fall
                        Tok_Whitespace(
                            UINT8               i_nSize )
                                                :   nSize(i_nSize) {}
    // OPERATIONS
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    // INQUIRY
    virtual const char* Text() const;
    UINT8               Size() const            { return nSize; }

  private:
    // DATA
    UINT8               nSize;
};

class Tok_LineStart : public Token
{
  public:
    // Spring and Fall
                        Tok_LineStart(
                            UINT8               i_nSize )
                                                :   nSize(i_nSize) {}
    // OPERATIONS
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    // INQUIRY
    virtual const char* Text() const;
    UINT8               Size() const            { return nSize; }

  private:
    // DATA
    UINT8               nSize;
};

class Tok_Eol : public Token
{ public:
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    virtual const char *
                        Text() const;
};

class Tok_EoDocu : public Token
{ public:
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    virtual const char *
                        Text() const;
};


}   // namespace adoc

#endif

