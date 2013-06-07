//
//  Base64.h
//  CryptTest
//
//  Created by Kiichi Takeuchi on 4/20/10.
//  Copyright 2010 ObjectGraph LLC. All rights reserved.
// 
// Original Source Code is donated by Cyrus
// Public Domain License
// http://www.cocoadev.com/index.pl?BaseSixtyFour

#import <Foundation/Foundation.h>


@interface Base64 : NSObject {

}

+ (void) initialize;

+ (NSString*) encode:(const uint8_t*) input length:(NSInteger) length;

+ (NSString*) encode:(NSData*) rawBytes;

+ (NSData*) decode:(const char*) string length:(NSInteger) inputLength;

+ (NSData*) decode:(NSString*) string;

@end
