/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cessentl.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:37:58 $
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

#ifndef ARY_CESSENTL_HXX
#define ARY_CESSENTL_HXX
//  KORR_DEPRECATED_3.0


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <ary/cpp/c_types4cpp.hxx>
#include <ary/loc/loc_types4loc.hxx>



namespace ary
{
namespace cpp
{


class CeEssentials
{   // Non inline functions are implemented in ceworker.cxx .
  public:
    // LIFECYCLE
                        CeEssentials();
                        CeEssentials(
                            const String  &     i_sLocalName,
                            Ce_id               i_nOwner,
                            loc::Le_id          i_nLocation );
                        ~CeEssentials();
    // INQUIRY
    const String  &     LocalName() const;
    Ce_id               Owner() const;
    loc::Le_id          Location() const;

    // ACCESS
  private:
    String              sLocalName;
    Ce_id               nOwner;
    loc::Le_id          nLocation;
};



// IMPLEMENTATION
inline const String     &
CeEssentials::LocalName() const
    { return sLocalName; }
inline Ce_id
CeEssentials::Owner() const
    { return nOwner; }
inline loc::Le_id
CeEssentials::Location() const
    { return nLocation; }




}   // namespace cpp
}   // namespace ary
#endif
