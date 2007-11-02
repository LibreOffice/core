/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_comrela.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:06:06 $
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

#ifndef ARY_IDL_I_COMRELA_HXX
#define ARY_IDL_I_COMRELA_HXX

// USED SERVICES
#include <ary/idl/i_types4idl.hxx>

namespace ary
{
namespace doc
{
    class OldIdlDocu;
}
}




namespace ary
{
namespace idl
{


/** Contains data for an IDL code entity related to another one like a base of
    an interface or of a service or the supported interface of a service.
*/
class CommentedRelation
{
  public:
    // LIFECYCLE

                        CommentedRelation(
                            Type_id             i_nType,
                            doc::OldIdlDocu *   i_pInfo )
                                                :   nType(i_nType),
                                                    pInfo(i_pInfo)
                                                {}
    // INQUIRY
    Type_id             Type() const            { return nType; }
    doc::OldIdlDocu *   Info() const            { return pInfo; }

  private:
    // DATA
    Type_id             nType;
    doc::OldIdlDocu *   pInfo;
};




}   // namespace idl
}   // namespace ary
#endif
