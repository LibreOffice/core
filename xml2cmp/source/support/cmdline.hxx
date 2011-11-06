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



#ifndef X2C_CMDLINE_HXX
#define X2C_CMDLINE_HXX

#include "sistr.hxx"
#include "list.hxx"


class CommandLine
{
  public:
                        CommandLine(
                            int                 i_argc,
                            char *              i_argv[] );
                        ~CommandLine();

    bool                IsOk() const            { return bIsOk; }
    const char *        ErrorText() const;

    const char *        XmlSrcFile() const      { return sXmlSourceFile.str(); }
    const char *        FuncFile() const        { return sFuncFile.str(); }
    const char *        HtmlFile() const        { return sHtmlFile.str(); }
    const char *        TypeInfoFile() const    { return sTypeInfoFile.str(); }

    bool                IsIndexCommand() const  { return sIndexFile.l() > 0; }
    const char *        XmlSrcDirectory() const { return sXmlSourceDirectory.str(); }
    const char *        IndexOutputFile() const { return sIndexFile.str(); }
    const char *        OutputDirectory() const { return sOutputDirectory.str(); }
    const List<Simstr> &
                        IndexedTags() const     { return aTagsInIndex; }

    const char *        IdlRootPath() const     { return sIdlRootPath.str(); }

  private:
    void                ParseIndexCommand(
                            int                 argc,
                            char *              argv[] );
    void                ParseSingleFileCommand(
                            int                 argc,
                            char *              argv[] );
    Simstr              sXmlSourceFile;
    Simstr              sFuncFile;
    Simstr              sHtmlFile;
    Simstr              sTypeInfoFile;

    Simstr              sXmlSourceDirectory;
    Simstr              sIndexFile;
    Simstr              sOutputDirectory;
    List<Simstr>        aTagsInIndex;

    Simstr              sIdlRootPath;

    bool                bIsOk;
};




#endif

