/*

 Copyright (c) 2013 Joan Lluch <joan.lluch@sweetwilliamsl.com>

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is furnished
 to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.

 Early code inspired on a similar class by Philip Kluz (Philip.Kluz@zuui.org)

*/


#import <UIKit/UIKit.h>
#import "stopWatch.h"

@class SWRevealViewController;
@protocol SWRevealViewControllerDelegate;

#pragma mark - SWRevealViewController Class

// Enum values for setFrontViewPosition:animated:
typedef enum
{
    FrontViewPositionLeftSideMostRemoved,
    FrontViewPositionLeftSideMost,
    FrontViewPositionLeftSide,

    // Left position, rear view is hidden behind front controller
    FrontViewPositionLeft,

    // Right possition, front view is presented right-offseted by rearViewRevealWidth
    FrontViewPositionRight,

    // Right most possition, front view is presented right-offseted by rearViewRevealWidth+rearViewRevealOverdraw
    FrontViewPositionRightMost,

    // Front controller is removed from view. Animated transitioning from this state will cause the same
    // effect than animating from FrontViewPositionRightMost. Use this instead of FrontViewPositionRightMost when
    // you intent to remove the front controller view to be removed from the view hierarchy.
    FrontViewPositionRightMostRemoved,

} FrontViewPosition;


@interface SWRevealViewController : UIViewController <StopWatchDelegate>

// Object instance init and rear view setting
- (id)initWithRearViewController:(UIViewController *)rearViewController frontViewController:(UIViewController *)frontViewController;

// Rear view controller, can be nil if not used
@property (strong, nonatomic) UIViewController *rearViewController;

// Optional right view controller, can be nil if not used
@property (strong, nonatomic) UIViewController *rightViewController;

// Front view controller, can be nil on initialization but must be supplied by the time its view is loaded
@property (strong, nonatomic) UIViewController *frontViewController;

// Sets the frontViewController using a default set of chained animations consisting on moving the
// presented frontViewController to the top most right, replacing it, and moving it back to the left position
- (void)setFrontViewController:(UIViewController *)frontViewController animated:(BOOL)animated;

// Front view position, use this to set a particular position state on the controller
// On initialization it is set to FrontViewPositionLeft
@property (assign, nonatomic) FrontViewPosition frontViewPosition;

// Chained animation of the frontViewController position. You can call it several times in a row to achieve
// any set of animations you wish. Animations will be chained and performed one after the other.
- (void)setFrontViewPosition:(FrontViewPosition)frontViewPosition animated:(BOOL)animated;

// Toogles the current state of the front controller between Left or Right and fully visible
// Use setFrontViewPosition to set a particular position
- (void)revealToggleAnimated:(BOOL)animated;
- (void)rightRevealToggleAnimated:(BOOL)animated;

// The following methods are meant to be directly connected to the action method of a button
// to perform user triggered postion change of the controller views. This is ussually added to a
// button on top left or right of the frontViewController
- (void)revealToggle:(id)sender;
- (void)rightRevealToggle:(id)sender;

// The following method will provide a panGestureRecognizer suitable to be added to any view on the frontController
// in order to perform usual drag and swipe gestures on the frontViewController to reveal the rear views. This
// is usually added on the top bar of a front controller.
- (UIPanGestureRecognizer*)panGestureRecognizer;

// The following properties are provided for further customization, they are set to default values on initialization,
// you should not generally have to set them

// Defines how much of the rear view is shown, default is 260.
@property (assign, nonatomic) CGFloat rearViewRevealWidth;
@property (assign, nonatomic) CGFloat rightViewRevealWidth;

// Defines how much of an overdraw can occur when dragging further than 'rearViewRevealWidth', default is 60.
@property (assign, nonatomic) CGFloat rearViewRevealOverdraw;
@property (assign, nonatomic) CGFloat rightViewRevealOverdraw;

// If YES (the default) the controller will bounce to the Left position when dragging further than 'rearViewRevealWidth'
@property (assign, nonatomic) BOOL bounceBackOnOverdraw;
@property (assign, nonatomic) BOOL bounceBackOnLeftOverdraw;

// If YES (default is NO) the controller will allow permanent dragging up to the rightMostPosition
@property (assign, nonatomic) BOOL stableDragOnOverdraw;
@property (assign, nonatomic) BOOL stableDragOnLeftOverdraw;

// Velocity required for the controller to toggle its state based on a swipe movement, default is 300
@property (assign, nonatomic) CGFloat quickFlickVelocity;

// Default duration for the revealToggle animation, default is 0.25
@property (assign, nonatomic) NSTimeInterval toggleAnimationDuration;

// Defines the radius of the front view's shadow, default is 2.5f
@property (assign, nonatomic) CGFloat frontViewShadowRadius;

// Defines the radius of the front view's shadow offset default is {0.0f,2.5f}
@property (assign, nonatomic) CGSize frontViewShadowOffset;

// The class properly handles all the relevant calls to appearance methods on the contained controllers.
// Moreover you can assign a delegate to let the class inform you on positions and animation activity.

// Delegate
@property (weak, nonatomic) id<SWRevealViewControllerDelegate> delegate;

@end

#pragma mark - SWRevealViewControllerDelegate Protocol

@protocol SWRevealViewControllerDelegate<NSObject>

@optional

- (void)revealController:(SWRevealViewController *)revealController willMoveToPosition:(FrontViewPosition)position;
- (void)revealController:(SWRevealViewController *)revealController didMoveToPosition:(FrontViewPosition)position;

- (void)revealController:(SWRevealViewController *)revealController animateToPosition:(FrontViewPosition)position;

- (void)revealControllerPanGestureBegan:(SWRevealViewController *)revealController;
- (void)revealControllerPanGestureEnded:(SWRevealViewController *)revealController;

@end


#pragma mark - UIViewController(SWRevealViewController) Category

// We add a category of UIViewController to let childViewControllers easily access their parent SWRevealViewController
@interface UIViewController(SWRevealViewController)

- (SWRevealViewController*)revealViewController;

@end


// This will allow the class to be defined on a storyboard
#pragma mark - SWRevealViewControllerSegue

@interface SWRevealViewControllerSegue : UIStoryboardSegue

@property (strong) void(^performBlock)( SWRevealViewControllerSegue* segue, UIViewController* svc, UIViewController* dvc );

@end


