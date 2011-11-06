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



#ifndef ADC_DISPLAY_HF_DOCENTRY_HXX
#define ADC_DISPLAY_HF_DOCENTRY_HXX



// USED SERVICES
    // BASE CLASSES
#include "htmlfactory.hxx"
    // COMPONENTS
    // PARAMETERS

/** @resp
    Produces a list of <DT>..</DT> and <DD>.
*/
class HF_DocEntryList : public HtmlMaker
{
  public:

                        HF_DocEntryList(
                            Xml::Element &      o_rOut );
    virtual             ~HF_DocEntryList();

    Xml::Element &      Produce_Term(
                            const char *        i_sTerm = 0 );
    Xml::Element &      Produce_NormalTerm(     /// Font will not be bold.
                            const char *        i_sTerm = 0 );
    Xml::Element &      Produce_Definition();
};



#endif
