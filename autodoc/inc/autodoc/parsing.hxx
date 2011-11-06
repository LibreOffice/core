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



#ifndef AUTODOC_PARSING_HXX
#define AUTODOC_PARSING_HXX



namespace autodoc
{

class CodeParser_Ifc;
class DocumentationParser_Ifc;
class FileCollector_Ifc;


/** Interface for parsing code of a programming language and
    delivering the information into an Autodoc Repository.
**/
class ParseToolsFactory_Ifc
{
  public:
    virtual             ~ParseToolsFactory_Ifc() {}
    static ParseToolsFactory_Ifc &
                        GetIt_();

    virtual DYN autodoc::CodeParser_Ifc *
                        Create_Parser_Cplusplus() const = 0;

//  virtual DYN autodoc::CodeParser_Ifc *
//                      CreateParser_UnoIDL() const = 0;
//  virtual DYN autodoc::CodeParser_Ifc *
//                      CreateParser_Java() const = 0;
//  virtual DYN autodoc::CodeParser_Ifc *
//                      CreateParser_StoredProcedures() const = 0;

    virtual DYN autodoc::DocumentationParser_Ifc *
                        Create_DocuParser_AutodocStyle() const = 0;

//  virtual DYN autodoc::DocumentationParser_Ifc *
//                      CreateDocuParser_StarOfficeAPIStyle() const = 0;

    virtual DYN autodoc::FileCollector_Ifc *
                        Create_FileCollector(
                            uintt               i_nEstimatedNrOfFiles ) const = 0;  /// Should be somewhat larger than the maximal estimated number of files.
};


} // namespace autodoc



#endif

