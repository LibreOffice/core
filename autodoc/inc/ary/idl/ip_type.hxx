/*************************************************************************
 *
 *  $RCSfile: ip_type.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:11:03 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef ARY_IDL_IP_TYPE_HXX
#define ARY_IDL_IP_TYPE_HXX


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <ary/idl/i_language.hxx>


namespace ary
{

class QualifiedName;

namespace idl
{

class Type;
class ExplicitNameRoom;

class TypePilot
{
  public:
    // LIFECYCLE
    virtual             TypePilot::~TypePilot() {}

    // OPERATIONS
    const Type &        CheckIn_Type(
                            QualifiedName &     i_rFullName,
                            uintt               i_nSequenceCount,
                            Ce_id               i_nModuleOfOccurrence );
    // INQUIRY
    const Type &        Find_Type(
                            Type_id             i_nType ) const;
    String              Search_LocalNameOf(
                            Type_id             i_nType ) const;
    Ce_id               Search_CeRelatedTo(
                            Type_id             i_nType ) const;
    const ExplicitNameRoom &
                        Find_XNameRoom(
                            Type_id             i_nType ) const;
    bool                IsBuiltInOrRelated(
                            const Type &        i_rType ) const;
  private:
    // Locals
    virtual const Type &
                        do_CheckIn_Type(
                            QualifiedName &     i_rFullName,
                            uintt               i_nSequenceCount,
                            Ce_id               i_nModuleOfOccurrence ) = 0;
    virtual const Type &
                        inq_Find_Type(
                            Type_id             i_nType ) const = 0;
    virtual String      inq_Search_LocalNameOf(
                            Type_id             i_nType ) const = 0;
    virtual Ce_id       inq_Search_CeRelatedTo(
                            Type_id             i_nType ) const = 0;
    virtual const ExplicitNameRoom &
                        inq_Find_XNameRoom(
                            Type_id             i_nType ) const = 0;
    virtual bool        inq_IsBuiltInOrRelated(
                            const Type &        i_rType ) const = 0;
};




// IMPLEMENTATION

inline const Type &
TypePilot::CheckIn_Type( QualifiedName &     i_rFullName,
                         uintt               i_nSequenceCount,
                         Ce_id               i_nModuleOfOccurrence )
    { return do_CheckIn_Type(i_rFullName, i_nSequenceCount, i_nModuleOfOccurrence); }

inline const Type &
TypePilot::Find_Type( Type_id i_nType ) const
    { return inq_Find_Type(i_nType); }

inline String
TypePilot::Search_LocalNameOf( Type_id i_nType ) const
    { return inq_Search_LocalNameOf(i_nType); }

inline Ce_id
TypePilot::Search_CeRelatedTo( Type_id i_nType ) const
    { return inq_Search_CeRelatedTo(i_nType); }

inline const ExplicitNameRoom &
TypePilot::Find_XNameRoom( Type_id i_nType ) const
    { return inq_Find_XNameRoom(i_nType); }

inline bool
TypePilot::IsBuiltInOrRelated( const Type & i_rType ) const
    { return inq_IsBuiltInOrRelated( i_rType ); }


}   // namespace idl
}   // namespace ary



#endif
