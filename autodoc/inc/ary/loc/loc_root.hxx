/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
