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



#ifndef ARY_LOC_PROJECT_HXX
#define ARY_LOC_PROJECT_HXX

// BASE CLASSES
#include <ary/loc/loc_le.hxx>
// USED SERVICES
#include <cosv/ploc.hxx>
#include <ary/loc/loc_dir.hxx>




namespace ary
{
namespace loc
{


/** Represents a root directory for source files.
*/
class Root : public LocationEntity
{
  public:
    enum E_ClassId { class_id = 7000 };

    explicit            Root(
                            const csv::ploc::Path &
                                                i_rRootDirectoryPath );
    void                Assign_Directory(
                            Le_id               i_assignedDirectory );
    virtual             ~Root();

    // INQUIRY
    const csv::ploc::Path &
                        Path() const;
    Le_id               MyDir() const;

  private:
    // Interface csv::ConstProcessorClient:
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;
    // Interface ary::Object:
    virtual ClassId     get_AryClass() const;

    // Interface LocationEntity:
    virtual const String &
                        inq_LocalName() const;
    virtual Le_id       inq_ParentDirectory() const;

    // DATA
    csv::ploc::Path     aPath;
    String              sPathAsString;
    Le_id               aMyDirectory;
};




// IMPLEMENTATION
inline void
Root::Assign_Directory(Le_id i_assignedDirectory)
{
    aMyDirectory = i_assignedDirectory;
}

inline const csv::ploc::Path &
Root::Path() const
{
    return aPath;
}

inline Le_id
Root::MyDir() const
{
    return aMyDirectory;
}




} // namespace loc
} // namespace ary
#endif
