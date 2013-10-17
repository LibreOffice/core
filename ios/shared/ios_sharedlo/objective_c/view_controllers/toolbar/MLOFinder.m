// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOFinder.h"
#import "MLOToolbarButton.h"
#import "MLOButton.h"
#import "MLOMainViewController_Friend.h"
#import "MLOGestureEngine_Impl.h"
#import "MLORenderManager.h"
#import "MLOSubView.h"
#import "MLOResourceImage.h"
#import "mlo_uno.h"

@interface MLOFinder ()
@property MLOSubView * blackBox;
@property UILabel * label;
@property UITextField * textField;
@property NSString * lastSearched;
@property NSDate * findTime;
@property MLOButton * leftButton,* rightButton;
@property NSInteger selectionCount,selectedIndex;
@property MLOMainViewController * mainViewController;
@end

static const NSTimeInterval

    NAVIGATION_BUTTON_FADE_DURATION = 1.0f,
    FIND_DELAY = 1.0,
    FIND_INVOCATION=FIND_DELAY+0.01;

static const CGFloat
    FADE_DURATION = 0.5f,
    FADE_IN_TARGET = 1.0f,
    FADE_OUT_TARGET = 0.0f,

    TEXT_AREA_WIDTH = 150.0f,
    SPACING_FROM_FIND_TOOLBAR_BUTTON=50.0f,
    BLACK_BOX_CORNER_RADIUS=15.0f,

    BLACK_BOX_INNER_PADDING_X = 10.0f,
    BLACK_BOX_INNER_PADDING_Y = 10.0f,
    TEXT_AREA_TO_NAVIAGATION_BUTTONS_SPACING = 5.0f,

    LABEL_HEIGHT = 18.0f,
    LABEL_TO_INPUT_FIELD_PADDING=8.0f,

    INPUT_FIELD_HEIGHT = 20.0f,

    BLACK_BOX_INNER_HEIGHT =LABEL_HEIGHT + LABEL_TO_INPUT_FIELD_PADDING + INPUT_FIELD_HEIGHT,
    BLACK_BOX_HEIGHT=2*BLACK_BOX_INNER_PADDING_Y + BLACK_BOX_INNER_HEIGHT;

@implementation MLOFinder

-(id)initWithToolbarButton:(MLOToolbarButton *) findButton{
    
    self = [super init];
    if(self){

        self.selectedIndex = -1;
        self.selectionCount = -1;
        
        MLOResourceImage * leftButtonImage = [MLOResourceImage left];
        MLOResourceImage * rightButtonImage =[MLOResourceImage right];
        
        CGFloat leftButtonWidth =leftButtonImage.image.size.width;
        CGFloat rightButtonWidth = rightButtonImage.image.size.width;

        CGRect findButtonFrame = findButton.frame;

        CGFloat xOrigin = findButtonFrame.origin.x + findButtonFrame.size.width + SPACING_FROM_FIND_TOOLBAR_BUTTON;
        CGFloat yOrigin = findButtonFrame.origin.y;
        
        CGFloat leftButtonXOrigin = xOrigin + BLACK_BOX_INNER_PADDING_X;

        CGFloat textAreaXOrigin = leftButtonXOrigin +leftButtonWidth + TEXT_AREA_TO_NAVIAGATION_BUTTONS_SPACING;
        CGFloat textAreaYOrigin = yOrigin +BLACK_BOX_INNER_PADDING_Y;

        CGFloat rightButtonXOrigin = textAreaXOrigin + TEXT_AREA_WIDTH + TEXT_AREA_TO_NAVIAGATION_BUTTONS_SPACING;
        
        [self addBlackBoxXOrigin:   xOrigin
                         yOrigin:   yOrigin
                           width:   leftButtonWidth +
                                    rightButtonWidth +
                                    (TEXT_AREA_TO_NAVIAGATION_BUTTONS_SPACING + BLACK_BOX_INNER_PADDING_X)*2 +
                                    TEXT_AREA_WIDTH ];


        [self addLabelXOrigin:textAreaXOrigin yOrigin:textAreaYOrigin];
        [self addTextFieldXOrigin:textAreaXOrigin yOrigin:textAreaYOrigin];

        self.leftButton =
        [self addButtonXOrigin:leftButtonXOrigin
                       yOrigin:textAreaYOrigin
                         image:leftButtonImage
                      selector:@selector(onTapLeft)
                          name:@"left (previous)"];

        self.rightButton =
        [self addButtonXOrigin:rightButtonXOrigin
                       yOrigin:textAreaYOrigin
                         image:rightButtonImage
                      selector:@selector(onTapRight)
                          name:@"right (next)"];

        self.lastSearched = @"";
        self.findTime = [NSDate date];
    }
    return self;
}
-(void) onTapLeft{
    [self onTap:MARK_PREVIOUS];
}
-(void) onTapRight{
    [self onTap:MARK_NEXT];
}

-(void) onTap:(MLOFindSelectionType) type{
    self.selectedIndex = mlo_find(_lastSearched, type);
    [self updateLabel];
    
}

-(void)updateLabel{
    CGFloat navigationAlpha;
    if(_selectionCount>0){
        self.label.text = [NSString stringWithFormat: @"Find (%d of %d)",_selectedIndex + 1,_selectionCount];
        navigationAlpha = 1.0f;

    }else{

        self.label.text = @"Find";
        navigationAlpha = 0.0f;
    }
    
    [self.mainViewController.gestureEngine.renderer renderNow];

    if(_leftButton.alpha != navigationAlpha){
        [UIView animateWithDuration:NAVIGATION_BUTTON_FADE_DURATION animations:^{
            _leftButton.alpha = navigationAlpha;
            _rightButton.alpha = navigationAlpha;

        }];
    }

}

-(MLOButton *) addButtonXOrigin:(CGFloat) xOrigin yOrigin:(CGFloat) yOrigin image:(MLOResourceImage *) image selector:(SEL) selector name:(NSString *) buttonName{
    MLOButton * button = [MLOButton buttonWithImage:image];

    CGFloat height = image.image.size.height;
    
    if(height < BLACK_BOX_INNER_HEIGHT){
        yOrigin += (BLACK_BOX_INNER_HEIGHT - height)/2.0f;
    }

    button.frame = CGRectMake(xOrigin,yOrigin,image.image.size.width,height);
    button.alpha = 0.0f;
    [button addTarget:self action:selector];
    NSLog(@"Created the navigation button %@",buttonName);
    return button;
}

-(void)addBlackBoxXOrigin:(CGFloat) xOrigin yOrigin:(CGFloat)yOrigin width:(CGFloat) width{
    self.blackBox =
    [[MLOSubView alloc]
     initWithFrame: CGRectMake(xOrigin,
                               yOrigin,
                               width,
                               BLACK_BOX_HEIGHT)
     color:         [UIColor blackColor]
     cornerRadius:  BLACK_BOX_CORNER_RADIUS
     alpha:         0.0f];

    NSLog(@"Created finder blackbox");
}

-(void) addLabelXOrigin:(CGFloat) xOrigin yOrigin:(CGFloat) yOrigin{
    self.label = [[UILabel alloc] initWithFrame:CGRectMake(xOrigin,
                                                           yOrigin,
                                                           TEXT_AREA_WIDTH,
                                                           LABEL_HEIGHT)];
    _label.textColor=[UIColor whiteColor];
    _label.backgroundColor = [UIColor clearColor];
    _label.alpha=0.0f;

    [self updateLabel];
}

-(void) addTextFieldXOrigin:(CGFloat) xOrigin yOrigin:(CGFloat) yOrigin{
    self.textField = [[UITextField alloc] initWithFrame:CGRectMake(xOrigin,
                                                                   yOrigin+LABEL_HEIGHT +LABEL_TO_INPUT_FIELD_PADDING,
                                                                   TEXT_AREA_WIDTH,
                                                                   INPUT_FIELD_HEIGHT)];
    _textField.text =@"";
    _textField.textColor = [UIColor blackColor];
    _textField.backgroundColor = [UIColor whiteColor];
    _textField.alpha = 0.0f;
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onTextChanged)
                                                 name:UITextFieldTextDidChangeNotification
                                               object:_textField];
}



-(void)show{
    [self fadeTo:FADE_IN_TARGET];
    [_textField becomeFirstResponder];
}
-(void)hide{
    [self fadeTo:FADE_OUT_TARGET];
    
    [_textField resignFirstResponder];
}

-(void)fadeTo:(CGFloat) alphaTarget{
    if(_blackBox.alpha !=alphaTarget){

        BOOL isFadeNavigationButton = (_selectionCount > 0) || (alphaTarget == 0.0f);
       
        [UIView animateWithDuration:FADE_DURATION animations:^{
             _blackBox.alpha    =   alphaTarget;
            _label.alpha        =   alphaTarget;
            _textField.alpha    =   alphaTarget;

            if(isFadeNavigationButton){
                _leftButton.alpha   =   alphaTarget;
                _rightButton.alpha  =   alphaTarget;
            }
        }];
    }
}


-(void)addToMainViewController:(MLOMainViewController *)mainViewController{
    [mainViewController.canvas addSubview:_blackBox];
    [mainViewController.canvas addSubview:_label];
    [mainViewController.canvas addSubview:_textField];
    [mainViewController.canvas addSubview:_leftButton];
    [mainViewController.canvas addSubview:_rightButton];
    self.mainViewController = mainViewController;
}
- (void)onTextChanged{
    
    self.findTime  = [NSDate dateWithTimeIntervalSinceNow:FIND_DELAY];

    [self performSelector:@selector(invokeFind) withObject:nil afterDelay:FIND_INVOCATION];
    
}

-(void) invokeFind{
    
    if(![[_findTime laterDate:[NSDate date]] isEqualToDate:_findTime]){
        
        NSInteger trimmedLength = [_textField.text  stringByTrimmingCharactersInSet: [NSCharacterSet whitespaceCharacterSet]].length;
        
        if( (_textField.text.length > 0) &&
            (trimmedLength>0) &&
           ![_textField.text isEqualToString:_lastSearched]){
            
            self.lastSearched  = [_textField.text copy];
            
            self.selectionCount = mlo_find(_lastSearched, MARK_FIRST);
            self.selectedIndex = 0;

        }else if(trimmedLength==0){
            
            self.lastSearched =@"";
            self.selectionCount = -1;
            self.selectedIndex = -1;
        }

        [self updateLabel];
    }
}
@end
