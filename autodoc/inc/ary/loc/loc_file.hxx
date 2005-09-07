/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: loc_file.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:23:23 $
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

#ifndef ARY_LOC_LOC_FILE_HXX
#define ARY_LOC_LOC_FILE_HXX

// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include <ary/ids.hxx>
    // PARAMETERS


namespace ary
{
namespace loc
{


class SourceCodeFile
{
  public:
                        SourceCodeFile(
                            Lid                 i_nId,
                            Lid                 i_nRoot,
                            Lid                 i_nDirectory,
                            const udmstri &     i_sName );
    //INQUIRY
    Lid                 Id() const;
    Lid                 Root() const;
    const udmstri &     Name() const;

  private:
    Lid                 nId;
    Lid                 nRoot;
    Lid                 nDirectory;
    udmstri             sName;
};

// IMPLEMENTATION

inline Lid
SourceCodeFile::Id() const
    { return nId; }
inline Lid
SourceCodeFile::Root() const
    { return nRoot; }
inline const udmstri &
SourceCodeFile::Name() const
    { return sName; }


} // namespace loc
} // namespace ary

#endif

