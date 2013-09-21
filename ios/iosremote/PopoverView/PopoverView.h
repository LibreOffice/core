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

#import <UIKit/UIKit.h>
#import "PopoverViewCompatibility.h"


/**************** Support both ARC and non-ARC ********************/

#ifndef SUPPORT_ARC
#define SUPPORT_ARC

#if __has_feature(objc_arc_weak)                //objc_arc_weak
#define WEAK weak
#define __WEAK __weak
#define STRONG strong

#define AUTORELEASE self
#define RELEASE self
#define RETAIN self
#define CFTYPECAST(exp) (__bridge exp)
#define TYPECAST(exp) (__bridge_transfer exp)
#define CFRELEASE(exp) CFRelease(exp)
#define DEALLOC self

#elif __has_feature(objc_arc)                   //objc_arc
#define WEAK unsafe_unretained
#define __WEAK __unsafe_unretained
#define STRONG strong

#define AUTORELEASE self
#define RELEASE self
#define RETAIN self
#define CFTYPECAST(exp) (__bridge exp)
#define TYPECAST(exp) (__bridge_transfer exp)
#define CFRELEASE(exp) CFRelease(exp)
#define DEALLOC self

#else                                           //none
#define WEAK assign
#define __WEAK
#define STRONG retain

#define AUTORELEASE autorelease
#define RELEASE release
#define RETAIN retain
#define CFTYPECAST(exp) (exp)
#define TYPECAST(exp) (exp)
#define CFRELEASE(exp) CFRelease(exp)
#define DEALLOC dealloc

#endif
#endif

/******************************************************************/


@class PopoverView;

@protocol PopoverViewDelegate <NSObject>

@optional

//Delegate receives this call as soon as the item has been selected
- (void)popoverView:(PopoverView *)popoverView didSelectItemAtIndex:(NSInteger)index;

//Delegate receives this call once the popover has begun the dismissal animation
- (void)popoverViewDidDismiss:(PopoverView *)popoverView;

@end

@interface PopoverView : UIView {
    CGRect boxFrame;
    CGSize contentSize;
    CGPoint arrowPoint;

    BOOL above;

    __WEAK id<PopoverViewDelegate> delegate;

    UIView *parentView;

    UIView *topView;

    NSArray *subviewsArray;

    NSArray *dividerRects;

    UIView *contentView;

    UIView *titleView;

    UIActivityIndicatorView *activityIndicator;

    //Instance variable that can change at runtime
    BOOL showDividerRects;
}

@property (nonatomic, STRONG) UIView *titleView;

@property (nonatomic, STRONG) UIView *contentView;

@property (nonatomic, STRONG) NSArray *subviewsArray;

@property (nonatomic, WEAK) id<PopoverViewDelegate> delegate;

#pragma mark - Class Static Showing Methods

//These are the main static methods you can use to display the popover.
//Simply call [PopoverView show...] with your arguments, and the popover will be generated, added to the view stack, and notify you when it's done.

+ (PopoverView *)showPopoverAtPoint:(CGPoint)point inView:(UIView *)view withText:(NSString *)text delegate:(id<PopoverViewDelegate>)delegate;

+ (PopoverView *)showPopoverAtPoint:(CGPoint)point inView:(UIView *)view withTitle:(NSString *)title withText:(NSString *)text delegate:(id<PopoverViewDelegate>)delegate;

+ (PopoverView *)showPopoverAtPoint:(CGPoint)point inView:(UIView *)view withViewArray:(NSArray *)viewArray delegate:(id<PopoverViewDelegate>)delegate;

+ (PopoverView *)showPopoverAtPoint:(CGPoint)point inView:(UIView *)view withTitle:(NSString *)title withViewArray:(NSArray *)viewArray delegate:(id<PopoverViewDelegate>)delegate;

+ (PopoverView *)showPopoverAtPoint:(CGPoint)point inView:(UIView *)view withStringArray:(NSArray *)stringArray delegate:(id<PopoverViewDelegate>)delegate;

+ (PopoverView *)showPopoverAtPoint:(CGPoint)point inView:(UIView *)view withTitle:(NSString *)title withStringArray:(NSArray *)stringArray delegate:(id<PopoverViewDelegate>)delegate;

+ (PopoverView *)showPopoverAtPoint:(CGPoint)point inView:(UIView *)view withStringArray:(NSArray *)stringArray withImageArray:(NSArray *)imageArray delegate:(id<PopoverViewDelegate>)delegate;

+ (PopoverView *)showPopoverAtPoint:(CGPoint)point inView:(UIView *)view withTitle:(NSString *)title withStringArray:(NSArray *)stringArray withImageArray:(NSArray *)imageArray delegate:(id<PopoverViewDelegate>)delegate;

+ (PopoverView *)showPopoverAtPoint:(CGPoint)point inView:(UIView *)view withTitle:(NSString *)title withContentView:(UIView *)cView delegate:(id<PopoverViewDelegate>)delegate;

+ (PopoverView *)showPopoverAtPoint:(CGPoint)point inView:(UIView *)view withContentView:(UIView *)cView delegate:(id<PopoverViewDelegate>)delegate;

#pragma mark - Instance Showing Methods

//Adds/animates in the popover to the top of the view stack with the arrow pointing at the "point"
//within the specified view.  The contentView will be added to the popover, and should have either
//a clear color backgroundColor, or perhaps a rounded corner bg rect (radius 4.f if you're going to
//round).
- (void)showAtPoint:(CGPoint)point inView:(UIView *)view withContentView:(UIView *)contentView;

//Calls above method with a UILabel containing the text you deliver to this method.
- (void)showAtPoint:(CGPoint)point inView:(UIView *)view withText:(NSString *)text;

//Calls top method with an array of UIView objects.  This method will stack these views vertically
//with kBoxPadding padding between each view in the y-direction.
- (void)showAtPoint:(CGPoint)point inView:(UIView *)view withViewArray:(NSArray *)viewArray;

//Does same as above, but adds a title label at top of the popover.
- (void)showAtPoint:(CGPoint)point inView:(UIView *)view withTitle:(NSString *)title withViewArray:(NSArray *)viewArray;

//Calls the viewArray method with an array of UILabels created with the strings
//in stringArray.  All contents of stringArray must be NSStrings.
- (void)showAtPoint:(CGPoint)point inView:(UIView *)view withStringArray:(NSArray *)stringArray;

//This method does same as above, but with a title label at the top of the popover.
- (void)showAtPoint:(CGPoint)point inView:(UIView *)view withTitle:(NSString *)title withStringArray:(NSArray *)stringArray;

//Draws a vertical list of the NSString elements of stringArray with UIImages
//from imageArray placed centered above them.
- (void)showAtPoint:(CGPoint)point inView:(UIView *)view withStringArray:(NSArray *)stringArray withImageArray:(NSArray *)imageArray;

//Does the same as above, but with a title
- (void)showAtPoint:(CGPoint)point inView:(UIView *)view withTitle:(NSString *)title withStringArray:(NSArray *)stringArray withImageArray:(NSArray *)imageArray;

//Lays out the PopoverView at a point once all of the views have already been setup elsewhere
- (void)layoutAtPoint:(CGPoint)point inView:(UIView *)view;

#pragma mark - Other Interaction
//This method animates the rotation of the PopoverView to a new point
- (void)animateRotationToNewPoint:(CGPoint)point inView:(UIView *)view withDuration:(NSTimeInterval)duration;

#pragma mark - Dismissal
//Dismisses the view, and removes it from the view stack.
- (void)dismiss;
- (void)dismiss:(BOOL)animated;

#pragma mark - Activity Indicator Methods

//Shows the activity indicator, and changes the title (if the title is available, and is a UILabel).
- (void)showActivityIndicatorWithMessage:(NSString *)msg;

//Hides the activity indicator, and changes the title (if the title is available) to the msg
- (void)hideActivityIndicatorWithMessage:(NSString *)msg;

#pragma mark - Custom Image Showing

//Animate in, and display the image provided here.
- (void)showImage:(UIImage *)image withMessage:(NSString *)msg;

#pragma mark - Error/Success Methods

//Shows (and animates in) an error X in the contentView
- (void)showError;

//Shows (and animates in) a success checkmark in the contentView
- (void)showSuccess;

@end
