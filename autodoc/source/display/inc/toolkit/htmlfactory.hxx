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



#ifndef ADC_DISPLAY_HTMLFACTORY_HXX
#define ADC_DISPLAY_HTMLFACTORY_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include "outputstack.hxx"
    // PARAMETERS
#include <udm/xml/xmlitem.hxx>
#include <udm/html/htmlitem.hxx>

namespace Xml = ::csi::xml;
namespace Html = ::csi::html;

/** @resp
    Base class for HTML page creators (factories) for code entites or
    similar items.
*/
template <class ENV>
class HtmlFactory
{
  public:
    // INQUIRY
    ENV &               Env() const             { return *pEnv; }
    Xml::Element &      CurOut() const          { return aDestination.Out(); }

    // ACCESS
    OutputStack &       Out() const             { return aDestination; }

  protected:
                        HtmlFactory(
                            ENV &               io_rEnv,
                            Xml::Element *      o_pOut = 0 )
                            :   pEnv(&io_rEnv)  { if (o_pOut != 0) aDestination.Enter(*o_pOut); }
                        ~HtmlFactory() {}
  private:
    // DATA
    ENV *               pEnv;
    mutable OutputStack aDestination;
};


/** @resp
    Base class for HTML paragraph creators, which are to be put into
    a parent HTML element.
*/
class HtmlMaker
{
  public:

    // INQUIRY
    Xml::Element &      CurOut() const          { return *pOut; }

  protected:
                        HtmlMaker(
                            Xml::Element &      o_rOut )
                            :   pOut(&o_rOut)   {}
  private:
    // DATA
    Xml::Element *      pOut;
};




// IMPLEMENTATION




#endif


