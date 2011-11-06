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



#ifndef ARY_LOC_FILE_HXX
#define ARY_LOC_FILE_HXX

// BASE CLASSES
#include <ary/loc/loc_filebase.hxx>




namespace ary
{
namespace loc
{


/** Represents an unspecified source code file.
*/
class File : public FileBase
{
  public:
    enum E_ClassId { class_id = 7100 };

                        File(
                            const String  &     i_sLocalName,
                            Le_id               i_nParentDirectory );
    virtual             ~File();

  private:
    // Interface csv::ConstProcessorClient:
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;
    // Interface ary::Object:
    virtual ClassId     get_AryClass() const;
};




} // namespace loc
} // namespace ary
#endif
