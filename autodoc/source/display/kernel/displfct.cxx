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

#include <precomp.h>
#include "displfct.hxx"


// NOT FULLY DECLARED SERVICES
#include <html/chd_udk2.hxx>
#include <idl/hi_display.hxx>
#include <cfrstd.hxx>


DYN DisplayToolsFactory * DisplayToolsFactory::dpTheInstance_ = 0;


namespace autodoc
{

DisplayToolsFactory_Ifc &
DisplayToolsFactory_Ifc::GetIt_()
{
    if ( DisplayToolsFactory::dpTheInstance_ == 0 )
        DisplayToolsFactory::dpTheInstance_ = new DisplayToolsFactory;
    return *DisplayToolsFactory::dpTheInstance_;
}

}   // namespace autodoc


DisplayToolsFactory::DisplayToolsFactory()
{
}

DisplayToolsFactory::~DisplayToolsFactory()
{
}

// DYN autodoc::TextDisplay_FunctionList_Ifc *
// DisplayToolsFactory::Create_TextDisplay_FunctionList() const
// {
//      return new CppTextDisplay_FunctionList;
// }


DYN autodoc::HtmlDisplay_UdkStd *
DisplayToolsFactory::Create_HtmlDisplay_UdkStd() const
{
    return new CppHtmlDisplay_Udk2;
}

DYN autodoc::HtmlDisplay_Idl_Ifc *
DisplayToolsFactory::Create_HtmlDisplay_Idl() const
{
    return new HtmlDisplay_Idl;
}

const display::CorporateFrame &
DisplayToolsFactory::Create_StdFrame() const
{
    static StdFrame aFrame;
    return aFrame;
}


