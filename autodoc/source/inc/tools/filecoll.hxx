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



#ifndef ADC_FILECOLL_HXX
#define ADC_FILECOLL_HXX

// USED SERVICES
    // BASE CLASSES
#include <autodoc/filecoli.hxx>
    // COMPONENTS
    // PARAMETERS



class FileCollector : public autodoc::FileCollector_Ifc
{
  public:
    // LIFECYCLE
                        FileCollector(
                            uintt               i_nRoughNrOfFiles = 0 );

    // OPERATIONS
    virtual uintt       AddFilesFrom(
                            const char *        i_sRootDir,
                            const char *        i_sFilter,
                            E_SearchMode        i_eSearchMode );
    virtual uintt       AddFile(
                            const char *        i_sFilePath );
    virtual void        EraseAll();

    // INQUIRY
    virtual const_iterator
                        Begin() const;
    virtual const_iterator
                        End() const;
    virtual uintt       Size() const;

  private:
    // DATA
    StringVector        aFoundFiles;
};


#endif

