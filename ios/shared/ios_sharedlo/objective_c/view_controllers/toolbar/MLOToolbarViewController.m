// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOToolbarViewController.h"
#import "MLOMainViewController.h"
#import "MLOToolbarButton.h"
#import "MLOFinder.h"
#import "MLOToolbarViewController_Impl.h"
#import "MLOMainViewController_Friend.h"
#import "MLOResourceImage.h"
#import "MLOTopbarViewController.h"
#import "MLOKeyboardManager.h"

@interface MLOToolbarViewController ()
@property NSArray * buttons;
@property MLOToolbarButton * expandButton;
@property MLOFinder * finder;
@property CGFloat previousHeightOffset;
@property BOOL tappable;
@end

@implementation MLOToolbarViewController

-(id)initWithMainViewController:(MLOMainViewController *) mainViewController{
    self = [self init];
    if (self) {
        self.mainViewController = mainViewController;
        self.buttons=@[[self createExpandButton],
                       [self createFindButton],
                       [self createEditButton]//,
                       //[self createPrintButton],
                       //[self createSaveButton]
                       ];
        self.tappable = NO;
        
        _previousHeightOffset=-1.0f;

    }
    return self;
}

-(void)addToMainViewController{
    
    for (MLOToolbarButton * button in _buttons) {
        
        [button addToToolbarControler:self];
        
        [_mainViewController.canvas addSubview:button];
    }
    [_finder addToMainViewController:_mainViewController];
}

-(void)showLibreOffice{
    
    for (MLOToolbarButton * button in _buttons) {
        [button showLibreOffice];
    }
}

-(void)hideAllButtonsBut:(MLOToolbarButton *) showButton{
    for (MLOToolbarButton * button in _buttons) {
        if(![button isEqual:showButton]){
            [button onOtherButtonTapped];
        }
    }
}
-(BOOL)isTappable{
    return _tappable;
}
-(void)expandDidToggle{
    [_expandButton switchImage];
}

-(MLOToolbarButton *)createExpandButton{
    self.expandButton=
    [MLOToolbarButton
     buttonWithImage:   [MLOResourceImage expand]
     onTap:             ^{   [self.mainViewController toggleExpand]; }
     tapRelease:        AUTOMATIC];

    _expandButton.alternateImage =[MLOResourceImage shrink];

    return _expandButton;
}
-(void) hideLibreOffice{
    for (MLOToolbarButton * button in _buttons) {
        [button hideLibreOffice];
    }
}

-(MLOToolbarButton *)createFindButton{
    MLOToolbarButton* findButton=
    [MLOToolbarButton
     buttonWithImage:   [MLOResourceImage find]
     onTap:             ^{  [self.finder show];  }
     tapRelease:          RETAP_OR_OTHER_TAPPED
     onTapRelease:      ^{  [self.finder hide]; }];

    self.finder = [[MLOFinder alloc] initWithToolbarButton:findButton];
    
    return findButton;
}

-(void)onRotate{
}

-(MLOToolbarButton *)createEditButton{
    return [MLOToolbarButton
            buttonWithImage:    [MLOResourceImage edit]
            onTap:              ^{  self.tappable =YES; }
            tapRelease:         RETAP_ONLY
            onTapRelease:       ^{  self.tappable = NO;
                                    [self.mainViewController.keyboard hide];
            }];
}

-(MLOToolbarButton *)createPrintButton{
    return [MLOToolbarButton
                buttonWithImage:    [MLOResourceImage print]
                onTap:              MLO_TOOLBAR_BUTTON_STUB_CALLBACK
                tapRelease:         RETAP_OR_OTHER_TAPPED];
}

-(MLOToolbarButton *)createSaveButton{
    return [MLOToolbarButton
                buttonWithImage:    [MLOResourceImage save]
                onTap:              MLO_TOOLBAR_BUTTON_STUB_CALLBACK
                tapRelease:         RETAP_OR_OTHER_TAPPED];
}

@end
