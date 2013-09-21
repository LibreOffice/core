//
//  PopoverView
//  https://github.com/runway20/PopoverView
//
//(MIT Licensed)
//
//Copyright (c) 2012 Runway 20 Inc.
//
//Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma mark Constants - Configure look/feel

// BOX GEOMETRY

//Height/width of the actual arrow
#define kArrowHeight 12.f

//padding within the box for the contentView
#define kBoxPadding 10.f

//control point offset for rounding corners of the main popover box
#define kCPOffset 1.8f

//radius for the rounded corners of the main popover box
#define kBoxRadius 4.f

//Curvature value for the arrow.  Set to 0.f to make it linear.
#define kArrowCurvature 6.f

//Minimum distance from the side of the arrow to the beginning of curvature for the box
#define kArrowHorizontalPadding 5.f

//Alpha value for the shadow behind the PopoverView
#define kShadowAlpha 0.4f

//Blur for the shadow behind the PopoverView
#define kShadowBlur 3.f;

//Box gradient bg alpha
#define kBoxAlpha 0.95f

//Padding along top of screen to allow for any nav/status bars
#define kTopMargin 50.f

//margin along the left and right of the box
#define kHorizontalMargin 10.f

//padding along top of icons/images
#define kImageTopPadding 3.f

//padding along bottom of icons/images
#define kImageBottomPadding 3.f


// DIVIDERS BETWEEN VIEWS

//Bool that turns off/on the dividers
#define kShowDividersBetweenViews YES

//color for the divider fill
#define kDividerColor [UIColor colorWithRed:0.329 green:0.341 blue:0.353 alpha:0.15f]


// BACKGROUND GRADIENT

//bottom color white in gradient bg
#define kGradientBottomColor [UIColor colorWithRed:0.98f green:0.98f blue:0.98f alpha:kBoxAlpha]

//top color white value in gradient bg
#define kGradientTopColor [UIColor colorWithRed:1.f green:1.f blue:1.f alpha:kBoxAlpha]


// TITLE GRADIENT

//bool that turns off/on title gradient
#define kDrawTitleGradient YES

//bottom color white value in title gradient bg
#define kGradientTitleBottomColor [UIColor colorWithRed:0.93f green:0.93f blue:0.93f alpha:kBoxAlpha]

//top color white value in title gradient bg
#define kGradientTitleTopColor [UIColor colorWithRed:1.f green:1.f blue:1.f alpha:kBoxAlpha]


// FONTS

//normal text font
#define kTextFontPhone [UIFont fontWithName:@"HelveticaNeue" size:16.f]

//normal text font
#define kTextFontPad [UIFont fontWithName:@"HelveticaNeue" size:20.f]

#define kSmallTextFontPad [UIFont fontWithName:@"HelveticaNeue" size:16.f]

//normal text color
#define kTextColor [UIColor colorWithRed:0.329 green:0.341 blue:0.353 alpha:1]
// highlighted text color
#define kTextHighlightColor [UIColor colorWithRed:0.098 green:0.102 blue:0.106 alpha:1.000]

//normal text alignment
#define kTextAlignment UITextAlignmentCenter

//normal text alignment
#define kTextAlignmentLeft UITextAlignmentLeft

//title font
#define kTitleFontPhone [UIFont fontWithName:@"HelveticaNeue-Bold" size:16.f]

#define kTitleFontPad [UIFont fontWithName:@"HelveticaNeue-Bold" size:22.f]

//title text color
#define kTitleColor [UIColor colorWithRed:0.329 green:0.341 blue:0.353 alpha:1]


// BORDER

//bool that turns off/on the border
#define kDrawBorder NO

//border color
#define kBorderColor [UIColor blackColor]

//border width
#define kBorderWidth 1.f