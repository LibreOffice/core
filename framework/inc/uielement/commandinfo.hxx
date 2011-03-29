/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
                    nWidth( 0 ),
                    nImageInfo( 0 ),
                    bMirrored( false ),
                    bRotated( false ) {}

    sal_uInt16                  nId;
    sal_uInt16                  nWidth;
    ::std::vector< sal_uInt16 > aIds;
    sal_Int16               nImageInfo;
    sal_Bool                bMirrored : 1,
                            bRotated  : 1;
};

typedef BaseHash< CommandInfo > CommandToInfoMap;

} // namespace framework

#endif // __FRAMEWORK_UIELEMENT_COMMANDINFO_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
