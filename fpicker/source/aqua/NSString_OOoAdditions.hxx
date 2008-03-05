/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NSString_OOoAdditions.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:36:07 $
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

#ifndef _NSSTRING_OOOADDITIONS_HXX_
#define _NSSTRING_OOOADDITIONS_HXX_

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
// #include <sal/types.h>

//for Cocoa types
@interface NSString (OOoAdditions)
+ (id) stringWithOUString:(const rtl::OUString&)ouString;
- (id) initWithOUString:(const rtl::OUString&)ouString;
- (rtl::OUString) OUString;
@end

#endif // _NSSTRING_OOOADDITIONS_HXX_
