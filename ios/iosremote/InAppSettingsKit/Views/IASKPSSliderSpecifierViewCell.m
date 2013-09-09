//
//  IASKPSSliderSpecifierViewCell.m
//  http://www.inappsettingskit.com
//
//  Copyright (c) 2009-2010:
//  Luc Vandal, Edovia Inc., http://www.edovia.com
//  Ortwin Gentz, FutureTap GmbH, http://www.futuretap.com
//  All rights reserved.
//
//  It is appreciated but not required that you give credit to Luc Vandal and Ortwin Gentz,
//  as the original authors of this code. You can give credit in a blog post, a tweet or on
//  a info page of your app. Also, the original authors appreciate letting them know if you use this code.
//
//  This code is licensed under the BSD license that is available at: http://www.opensource.org/licenses/bsd-license.php
//

#import "IASKPSSliderSpecifierViewCell.h"
#import "IASKSlider.h"
#import "IASKSettingsReader.h"

@implementation IASKPSSliderSpecifierViewCell

@synthesize slider=_slider,
            minImage=_minImage,
            maxImage=_maxImage;

- (void)layoutSubviews {
    [super layoutSubviews];
	CGRect  sliderBounds    = _slider.bounds;
    CGPoint sliderCenter    = _slider.center;
    const double superViewWidth = _slider.superview.frame.size.width;

    sliderCenter.x = superViewWidth / 2;
    sliderBounds.size.width = superViewWidth - kIASKSliderNoImagesPadding * 2;
	_minImage.hidden = YES;
	_maxImage.hidden = YES;

	// Check if there are min and max images. If so, change the layout accordingly.
	if (_minImage.image && _maxImage.image) {
		// Both images
		_minImage.hidden = NO;
		_maxImage.hidden = NO;
        sliderBounds.size.width  = superViewWidth - kIASKSliderImagesPadding * 2;
	}
	else if (_minImage.image) {
		// Min image
		_minImage.hidden = NO;
		sliderCenter.x    += (kIASKSliderImagesPadding - kIASKSliderNoImagesPadding) / 2;
		sliderBounds.size.width  = superViewWidth - kIASKSliderNoImagesPadding - kIASKSliderImagesPadding;
	}
	else if (_maxImage.image) {
		// Max image
		_maxImage.hidden = NO;
		sliderCenter.x    -= (kIASKSliderImagesPadding - kIASKSliderNoImagesPadding) / 2;
		sliderBounds.size.width  = superViewWidth - kIASKSliderNoImagesPadding - kIASKSliderImagesPadding;
	}

	_slider.bounds = sliderBounds;
    _slider.center = sliderCenter;
}

- (void)dealloc {
	_minImage.image = nil;
	_maxImage.image = nil;
}

- (void)prepareForReuse {
	_minImage.image = nil;
	_maxImage.image = nil;
}
@end
