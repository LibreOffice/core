/* -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#import <stdio.h>

#import <CoreFoundation/CoreFoundation.h>
#import <CoreServices/CoreServices.h>
#import <Foundation/Foundation.h>

#import "GetMetadataForFile.h"

int main(int argc, const char* argv[])
{
    @autoreleasepool
    {
        if (argc != 3)
        {
            fprintf(stderr, "Usage: %s UTI path\n", argv[0]);
            return 1;
        }
        NSMutableDictionary* attributes = [NSMutableDictionary dictionaryWithCapacity:10];
        NSString* contentTypeUTI = [NSString stringWithUTF8String:argv[1]];
        NSString* pathToFile = [NSString stringWithUTF8String:argv[2]];

        GetMetadataForFile(NULL, (__bridge CFMutableDictionaryRef)attributes,
                           (__bridge CFStringRef)contentTypeUTI, (__bridge CFStringRef)pathToFile);
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
