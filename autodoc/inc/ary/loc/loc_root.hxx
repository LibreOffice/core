/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: loc_root.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:18:22 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
