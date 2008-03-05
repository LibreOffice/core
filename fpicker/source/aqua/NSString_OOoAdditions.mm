/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NSString_OOoAdditions.mm,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:36:21 $
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

#ifndef _CFSTRINGUTILITIES_HXX_
#include "CFStringUtilities.hxx"
#endif

#include "NSString_OOoAdditions.hxx"

#define CLASS_NAME "NSString"

@implementation NSString (OOoAdditions) 

+ (id) stringWithOUString:(const rtl::OUString&)ouString
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "ouString", ouString);

    NSString *string = [[NSString alloc] initWithOUString:ouString];

    DBG_PRINT_EXIT(CLASS_NAME, __func__, string);
    return [string autorelease];
}

- (id) initWithOUString:(const rtl::OUString&)ouString
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "ouString", ouString);
    if ((self = [super init])) {
        self = [self initWithCharacters:ouString.getStr() length:ouString.getLength()];
        
        DBG_PRINT_EXIT(CLASS_NAME, __func__, self);
        
        return self;
    }
    
    DBG_PRINT_EXIT(CLASS_NAME, __func__, self);
    return nil;
}

- (rtl::OUString) OUString
{
    unsigned int nFileNameLength = [self length];

    UniChar unichars[nFileNameLength+1];

    //'close' the string buffer correctly
    unichars[nFileNameLength] = '\0';

    [self getCharacters:unichars];

    return rtl::OUString(unichars);
}

@end
