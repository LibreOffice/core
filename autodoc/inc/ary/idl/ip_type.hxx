/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ip_type.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:14:26 $
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

#ifndef ARY_IDL_IP_TYPE_HXX
#define ARY_IDL_IP_TYPE_HXX

// USED SERVICES
#include <ary/idl/i_types4idl.hxx>




namespace ary
{
    class QualifiedName;

namespace idl
{
    class Type;
    class ExplicitNameRoom;


/** Access point to all {->Type}s in IDL.
*/
class TypePilot
{
  public:
    // LIFECYCLE
    virtual             ~TypePilot() {}

    // OPERATIONS
    virtual const Type &
                        CheckIn_Type(
                            QualifiedName &     i_rFullName,
                            uintt               i_nSequenceCount,
                            Ce_id               i_nModuleOfOccurrence,
                            Type_id             i_nTemplateType ) = 0;
    // INQUIRY
    virtual const Type &
                        Find_Type(
                            Type_id             i_nType ) const = 0;
    virtual String      Search_LocalNameOf(
                            Type_id             i_nType ) const = 0;
    virtual Ce_id       Search_CeRelatedTo(
                            Type_id             i_nType ) const = 0;
    virtual const ExplicitNameRoom &
                        Find_XNameRoom(
                            Type_id             i_nType ) const = 0;
    virtual bool        IsBuiltInOrRelated(
                            const Type &        i_rType ) const = 0;
};




}   // namespace idl
}   // namespace ary
#endif
