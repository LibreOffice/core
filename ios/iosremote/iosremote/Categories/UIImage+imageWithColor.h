//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// UIImage+imageWithColor.h
//

#import <UIKit/UIKit.h>

@interface UIImage (imageWithColor)

+ (UIImage *) imageWithColor:(UIColor *) color size:(CGSize) size;

@end
