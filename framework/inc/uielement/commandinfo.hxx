/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: commandinfo.hxx,v $
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

#ifndef __FRAMEWORK_UIELEMENT_COMMANDINFO_HXX_
#define __FRAMEWORK_UIELEMENT_COMMANDINFO_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <macros/generic.hxx>
#include <stdtypes.h>

//_________________________________________________________________________________________________________________
// other includes
//_________________________________________________________________________________________________________________

#include <tools/solar.h>

namespace framework
{

struct CommandInfo
{
    CommandInfo() : nId( 0 ),
                    nImageInfo( 0 ),
                    bMirrored( false ),
                    bRotated( false ) {}

    USHORT                  nId;
    ::std::vector< USHORT > aIds;
    sal_Int16               nImageInfo;
    sal_Bool                bMirrored : 1,
                            bRotated  : 1;
};

typedef BaseHash< CommandInfo > CommandToInfoMap;

} // namespace framework

#endif // __FRAMEWORK_UIELEMENT_COMMANDINFO_HXX_

