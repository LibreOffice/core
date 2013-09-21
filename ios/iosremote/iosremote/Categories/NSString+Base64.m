/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#import "NSString+Base64.h"

@implementation NSData(Base64)

+ (id) dataWithBase64String:(NSString *)base64Encoding
{
	if ([base64Encoding length] % 4 != 0)
		return nil;
	
	NSString *plist = [NSString stringWithFormat:@"<?xml version=\"1.0\" encoding=\"UTF-8\"?><plist version=\"1.0\"><data>%@</data></plist>", base64Encoding];
	return [NSPropertyListSerialization propertyListWithData:[plist dataUsingEncoding:NSASCIIStringEncoding] options:0 format:NULL error:NULL];
}


@end
