/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: loc_dir.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:23:07 $
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

#ifndef ARY_LOC_LOC_DIR_HXX
#define ARY_LOC_LOC_DIR_HXX

// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include <ary/ids.hxx>
    // PARAMETERS


namespace ary
{
namespace loc
{


class Directory
{
  public:
    typedef std::map< udmstri, Lid >   Map_Children;

    // LIFECYCLE
    virtual             ~Directory() {}


    // OPERATIONS
    void                Add_ChildDir(
                            const udmstri &     i_sName,
                            Lid                 i_nId );
    void                Add_File(
                            const udmstri &     i_sName,
                            Lid                 i_nId );
    //INQUIRY
    Lid                 Id() const;
    const Map_Children &
                        ChildDirs() const;

    const Map_Children &
                        Files() const;
    // ACCESS
    Map_Children &      ChildDirs();
    Map_Children &      Files();

  protected:
                        Directory(
                            Lid                 i_nId );
  private:
    Map_Children        aChildDirs;
    Map_Children        aFiles;
    Lid                 nId;
};

class SubDirectory : public Directory
{
  public:
                        SubDirectory(
                            Lid                 i_nId,
                            const udmstri &     i_sName,
                            Lid                 i_nParentDirectory );
    //INQUIRY
    const udmstri &     Name() const;

  private:
    udmstri             sName;
    Lid                 nParentDirectory;
};


// IMPLEMENTATION

inline Lid
Directory::Id() const
    { return nId; }
inline const Directory::Map_Children &
Directory::ChildDirs() const
    { return aChildDirs; }
inline const Directory::Map_Children &
Directory::Files() const
    { return aFiles; }
inline Directory::Map_Children &
Directory::ChildDirs()
    { return aChildDirs; }
inline Directory::Map_Children &
Directory::Files()
    { return aFiles; }
inline const udmstri &
SubDirectory::Name() const
    { return sName; }


} // namespace loc
} // namespace ary

#endif

