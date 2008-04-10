/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: idl_internalgate.hxx,v $
 * $Revision: 1.3 $
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

#ifndef ARY_IDL_INTERNALGATE_HXX
#define ARY_IDL_INTERNALGATE_HXX

// BASE CLASSES
#include <ary/idl/i_gate.hxx>

namespace ary
{
    class RepositoryCenter;
}




namespace ary
{
namespace idl
{


/** Provides access to the ->idl::RepositoryPartition as far as is needed
    by the ->RepositoryCenter.
*/
class InternalGate : public ::ary::idl::Gate
{
  public:
    virtual             ~InternalGate() {}

    static DYN InternalGate &
                        Create_Partition_(
                            RepositoryCenter &  i_center );
};




}   //  namespace idl
}   //  namespace ary
#endif
