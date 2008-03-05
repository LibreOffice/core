/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NSURL_OOoAdditions.mm,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:36:44 $
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
#include "NSString_OOoAdditions.hxx"
#endif

#include "NSURL_OOoAdditions.hxx"

@implementation NSURL (OOoAdditions)
- (rtl::OUString) OUStringForInfo:(InfoType)info
{
    NSAutoreleasePool *pool = [NSAutoreleasePool new];

    NSString *sURLString = nil;

    switch(info) {
        case FULLPATH:
            OSL_TRACE("Extracting the full path of an item");
            sURLString = [self absoluteString];
            [sURLString retain];
            break;
        case FILENAME:
            OSL_TRACE("Extracting the file name of an item");
            NSString *path = [self path];
            if (path == nil) {
                sURLString = @"";
            }
            else {
                sURLString = [path lastPathComponent];
            }
            [sURLString retain];
            break;
        case PATHWITHOUTLASTCOMPONENT:
            OSL_TRACE("Extracting the last but one component of an item's path");
            path = [self absoluteString];
            if (path == nil) {
                sURLString = @"";
            }
            else {
                NSString* lastComponent = [path lastPathComponent];
                unsigned int lastLength = [lastComponent length];
                sURLString = [path substringToIndex:([path length] - lastLength)];
            }
            [sURLString retain];
            break;
        default:
            break;
    }

    rtl::OUString sResult = [sURLString OUString];
    [sURLString release];

    [pool release];

    return sResult;
}
@end
