/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: NSString_OOoAdditions.hxx,v $
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

#ifndef _NSSTRING_OOOADDITIONS_HXX_
#define _NSSTRING_OOOADDITIONS_HXX_

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>
#include <rtl/ustring.hxx>
// #include <sal/types.h>

//for Cocoa types
@interface NSString (OOoAdditions)
+ (id) stringWithOUString:(const rtl::OUString&)ouString;
- (id) initWithOUString:(const rtl::OUString&)ouString;
- (rtl::OUString) OUString;
@end

#endif // _NSSTRING_OOOADDITIONS_HXX_
