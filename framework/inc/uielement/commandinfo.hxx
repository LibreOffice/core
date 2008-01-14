/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: commandinfo.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 17:23:44 $
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

