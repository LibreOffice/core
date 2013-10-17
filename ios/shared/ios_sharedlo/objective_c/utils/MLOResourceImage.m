// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOResourceImage.h"

@interface MLOResourceImage ()

@property NSString * name;
@property UIImage * image;
@end


@implementation MLOResourceImage

+(MLOResourceImage *)imageWithName:(NSString *) name{
    return [MLOResourceImage imageWithName:name size:NORMAL];
}

+(MLOResourceImage *)imageWithName:(NSString *) name  size:(MLOResourceImageSize) size{
    MLOResourceImage * image = [MLOResourceImage new];
    if(image){
        image.name = name;
        image.image = [MLOResourceImage getButtonImage:name size:size];
    }
    return image;
}

+(UIImage *) getButtonImage:(NSString *) name size:(MLOResourceImageSize) size{
    return [MLOResourceImage getImage: [@"Button" stringByAppendingString:name] size:size];
}

+(UIImage *) getImage:(NSString *) midfix size:(MLOResourceImageSize) size{
    return [UIImage imageNamed: [MLOResourceImage getImageName:midfix size:size]];
}

+(NSString *) getImageName:(NSString *) midfix size:(MLOResourceImageSize) size{
    return [NSString stringWithFormat: [MLOResourceImage formatForSize:size],midfix];
}
+(NSString *) getImageName:(NSString *) midfix type:(MLOResourceImageType) type size:(MLOResourceImageSize) size{
    return [MLOResourceImage getImageName:  [midfix stringByAppendingString:[MLOResourceImage midfixSuffixForType:type]]
                                     size:  size];
}
+(UIImage *) getImage:(NSString *) midfix type:(MLOResourceImageType) type size:(MLOResourceImageSize) size{
    return [UIImage imageNamed: [MLOResourceImage getImageName:midfix type: type size:size]];
}

+(NSString *) midfixSuffixForType:(MLOResourceImageType) type{
    switch (type) {
        case IMAGE: return @"";
        case MASK: return @"Mask";
    }
}


+(NSString *) formatForSize:(MLOResourceImageSize) size{
    switch (size) {
        case NORMAL: return @"MLO%@.png";
        case RETINA: return @"MLO%@@2x.png";
    }
}

+(UIImage *) loLogo{
    return [self getImage:@"LibreOfficeLogo" size:NORMAL];
}

+(MLOResourceImage *) backWithSize:(MLOResourceImageSize) size{
    return [MLOResourceImage imageWithName:@"Back" size:size];
}
+(MLOResourceImage *) shrink{
    return [MLOResourceImage imageWithName:@"Shrink"];
}
+(MLOResourceImage *) expand{
    return [MLOResourceImage imageWithName:@"Expand"];
}
+(MLOResourceImage *) edit{
    return [MLOResourceImage imageWithName:@"Edit"];
}
+(MLOResourceImage *) find{
    return [MLOResourceImage imageWithName:@"Find"];
}
+(MLOResourceImage *) print{
    return [MLOResourceImage imageWithName:@"Print"];
}
+(MLOResourceImage *) save{
    return [MLOResourceImage imageWithName:@"Save"];
}
+(MLOResourceImage *) left{
    return [MLOResourceImage imageWithName:@"Left"];
}
+(MLOResourceImage *) right{
    return [MLOResourceImage imageWithName:@"Right"];
}
+(MLOResourceImage *) selectionHandle{
    return [MLOResourceImage imageWithName:@"SelectionHandle"];
}
+(NSString *) magnifierName:(MLOResourceImageType) type{
    return [MLOResourceImage getImageName:@"Magnifier" type:type size:RETINA];
}
@end
