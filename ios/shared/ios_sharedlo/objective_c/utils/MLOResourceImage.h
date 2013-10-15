// -*- Mode: Objective-C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOObject.h"

typedef enum {NORMAL,RETINA} MLOResourceImageSize;
#define MLOResourceImageSizeString(enum) [@[@"NORMAL",@"RETINA"] objectAtIndex:enum]
typedef enum {IMAGE, MASK} MLOResourceImageType;
#define MLOResourceImageTypeString(enum) [@[@"MASK",@"RETINA"] objectAtIndex:enum]

@interface MLOResourceImage : MLOObject
-(NSString *)name;
-(UIImage *)image;

+(MLOResourceImage *) backWithSize:(MLOResourceImageSize) size;
+(MLOResourceImage *) shrink;
+(MLOResourceImage *) expand;
+(MLOResourceImage *) edit;
+(MLOResourceImage *) find;
+(MLOResourceImage *) print;
+(MLOResourceImage *) save;
+(MLOResourceImage *) left;
+(MLOResourceImage *) right;
+(MLOResourceImage *) selectionHandle;
+(NSString *) magnifierName:(MLOResourceImageType) type;
+(UIImage *) loLogo;
@end
