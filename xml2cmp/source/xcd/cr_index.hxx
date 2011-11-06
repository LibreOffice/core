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



#ifndef X2C_CR_INDEX_HXX
#define X2C_CR_INDEX_HXX

#include "../support/sistr.hxx"
#include "../support/heap.hxx"
#include "../support/list.hxx"
#include "../support/syshelp.hxx"


class ModuleDescription;


class Index
{
  public:
                        Index(
                            const char  *       i_sOutputDirectory,
                            const char *        i_sIdlRootPath,
                            const List<Simstr> &
                                                i_rTagList );
                        ~Index();

    void                GatherData(
                            const List<Simstr> &
                                                i_rInputFileList );
    void                WriteOutput(
                            const char *        i_sOuputFile );

    void                InsertSupportedService(
                            const Simstr &      i_sService );
  private:
    void                ReadFile(
                            const char *        i_sFilename );
    void                CreateHtmlFileName(
                            char *              o_sOutputHtml,
                            const ModuleDescription &
                                                i_rModule );
    void                WriteTableFromHeap(
                            std::ostream &      o_rOut,
                            Heap &              i_rHeap,
                            const char *        i_sIndexKey,
                            const char *        i_sIndexReference,
                            E_LinkType          i_eLinkType );
    void                WriteHeap(
                            std::ostream &      o_rOut,
                            Heap &              i_rHeap,
                            E_LinkType          i_eLinkType );

    // DATA
    Heap                aService2Module;
    Heap                aModule2Service;

    Simstr              sOutputDirectory;
    Simstr              sIdlRootPath;

    // Temporary Data
    Simstr              sCurModule;
};




#endif


