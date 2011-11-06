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



#ifndef ADC_DISPLAY_DISPLFCT_HXX
#define ADC_DISPLAY_DISPLFCT_HXX


#include <autodoc/displaying.hxx>


/** Interface for parsing code of a programming language and
    delivering the information into an Autodoc Repository.
**/
class DisplayToolsFactory : public autodoc::DisplayToolsFactory_Ifc
{
  public:
                        DisplayToolsFactory();
    virtual             ~DisplayToolsFactory();

//    virtual DYN autodoc::TextDisplay_FunctionList_Ifc *
//                        Create_TextDisplay_FunctionList() const;

    virtual DYN autodoc::HtmlDisplay_UdkStd *
                        Create_HtmlDisplay_UdkStd() const;
    virtual DYN autodoc::HtmlDisplay_Idl_Ifc *
                        Create_HtmlDisplay_Idl() const;

    virtual const display::CorporateFrame &
                        Create_StdFrame() const;
  private:
    static DYN DisplayToolsFactory *
                        dpTheInstance_;

    friend class autodoc::DisplayToolsFactory_Ifc;
};


#endif

