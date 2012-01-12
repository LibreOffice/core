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
#include "parsefct.hxx"


// NOT FULLY DECLARED SERVICES
#include <cpp/prs_cpp.hxx>
#include <adoc/prs_adoc.hxx>
#include <tools/filecoll.hxx>


DYN ParseToolsFactory * ParseToolsFactory::dpTheInstance_ = 0;


namespace autodoc
{

ParseToolsFactory_Ifc &
ParseToolsFactory_Ifc::GetIt_()
{
    if ( ParseToolsFactory::dpTheInstance_ == 0 )
        ParseToolsFactory::dpTheInstance_ = new ParseToolsFactory;
    return *ParseToolsFactory::dpTheInstance_;
}

}   // namespace autodoc


ParseToolsFactory::ParseToolsFactory()
{
}

ParseToolsFactory::~ParseToolsFactory()
{
}

DYN autodoc::CodeParser_Ifc *
ParseToolsFactory::Create_Parser_Cplusplus() const
{
    return new cpp::Cpluplus_Parser;
}

DYN autodoc::DocumentationParser_Ifc *
ParseToolsFactory::Create_DocuParser_AutodocStyle() const
{
    return new adoc::DocuParser_AutodocStyle;
}

DYN autodoc::FileCollector_Ifc *
ParseToolsFactory::Create_FileCollector( uintt i_nEstimatedNrOfFiles ) const
{
    return new FileCollector(i_nEstimatedNrOfFiles);
}


