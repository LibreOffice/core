// -*- Mode: Objective-C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOScrollerData.h"
#import "MLOMainViewController_Friend.h"
#import "MLOGestureEngine_Impl.h"
#import "MLOScrollerGridViewController.h"
#import "MLOScrollerTooltip.h"
#import "MLOGestureLimiter.h"
#import "mlo_uno.h"
#import "mlo_utils.h"

@interface MLOScrollerData ()

@property int left, top, zoom, visibleLeft, visibleTop, visibleRight, visibleBottom, zoomType, isBooklet;
@property float totalPages, totalHeightInLogic, frameWidth, frameHeight, scrollerX, scrollerTotalHeight,tooltipPageHeight,tooltipX,tooltipMaxY;
@property NSMutableString *mutableString;
@property CGRect maxCanvas,contentCanvas,visibleCanvas;
@property MLOMainViewController * mainViewController;

@end

static const float
    SCROLLER_WIDTH=10,
    SCROLLER_PADDING = 0,

    GRID_TO_TOOLTIP_DISTANCE = 2.0f,

    TOOLTIP_HEIGHT=60,
    TOOLTIP_WIDTH=100,

    FLOAT_RESET=-1.0f;

static const int INT_RESET= -1;

@implementation MLOScrollerData


-(id)initWithMainViewController:(MLOMainViewController *) mainViewController{
    self=  [super init];
    if(self){
        self.mutableString = [NSMutableString stringWithString:@""];
        self.mainViewController = mainViewController;
        [self reset];
    }
    return self;
}

-(void) showLibreOffice{
}

-(void)hideLibreOffice{
    [self reset];
}

-(void)reset{
    _tooltipMaxY= _tooltipPageHeight = _tooltipX = _totalPages = _totalHeightInLogic = _frameWidth= _frameHeight= _scrollerX= _scrollerTotalHeight = FLOAT_RESET;
    _left = _top = _zoom = _visibleLeft = _visibleTop = _visibleRight = _visibleBottom = _zoomType = _isBooklet = INT_RESET;
    
}

-(void) updateViewData{
    
    mlo_fetch_view_data(_mutableString);
    
    sscanf([_mutableString UTF8String], "%d;%d;%d;%d;%d;%d;%d;%d;%d;",
           &_left,
           &_top,
           &_zoom,
           &_visibleLeft,
           &_visibleTop,
           &_visibleRight,
           &_visibleBottom,
           &_zoomType,
           &_isBooklet);
    
    if(LOG_GET_VIEW_DATA){
        NSLog(@"left=%d top=%d zoom=%d vLeft=%d vTop=%d vRight=%d vBottom=%d zoomType=%d isBooklet=%d",
              _left,
              _top,
              _zoom,
              _visibleLeft,
              _visibleTop,
              _visibleRight,
              _visibleBottom,
              _zoomType,
              _isBooklet);
    }
}

-(BOOL)canCalculateDocumentSizeInLogic{
    return (_totalHeightInLogic != FLOAT_RESET);
}

-(void) onRotateWithGrid:(MLOScrollerGridViewController *) grid{
    static const float SCROLLLER_X_OFFSET =SCROLLER_PADDING + SCROLLER_WIDTH ,
        DOUBLE_SCROLLER_PADDING =2*SCROLLER_PADDING,
        GRID_X_OFFSET= MLO_SCROLLER_GRID_WIDTH,
        TOOLTIP_X_OFFSET= GRID_X_OFFSET + GRID_TO_TOOLTIP_DISTANCE+ TOOLTIP_WIDTH;
    
    CGRect canvasFrame = _mainViewController.canvas.frame;
    
    _frameHeight = canvasFrame.size.height;
    
    _frameWidth = canvasFrame.size.width;
    
    _scrollerX = _frameWidth - SCROLLLER_X_OFFSET;
    
    _tooltipX = _frameWidth - TOOLTIP_X_OFFSET;
    
    _tooltipMaxY = _frameHeight - TOOLTIP_HEIGHT;
    
    _scrollerTotalHeight =_frameHeight -DOUBLE_SCROLLER_PADDING;
    
    _tooltipPageHeight = _frameHeight/_totalPages;
    
    [grid onRotate:_frameWidth - GRID_X_OFFSET];
}

-(NSInteger) getTotalPages{
    
    NSInteger newTotalPages= mlo_get_pages(YES);
    
    if(newTotalPages!= _totalPages){
                
        _totalPages = newTotalPages;
        _totalHeightInLogic = _totalPages* PORTRAIT_PAGE_HEIGHT_IN_LOGIC_WITH_SEPARATOR;
        
        _tooltipPageHeight = _frameHeight/_totalPages;
        
        _mainViewController.gestureEngine.limiter.documentSizeInLogic = CGSizeMake(CANVAS_WIDTH_IN_LOGIC,
                                                                                   _totalHeightInLogic);
    }
    
    return newTotalPages;
}


-(CGRect) getShiftedScrollerFrame:(CGFloat) pixelDeltaX{
    pixelDeltaX *= PIXEL_TO_LOGIC_RATIO / [_mainViewController getZoom] * 100;
    CGFloat newTop = _visibleTop + pixelDeltaX;
    CGFloat newBottom = _visibleBottom + pixelDeltaX;

    BOOL isOverTop = newTop < 0.0f;
    BOOL isBelowBotton = newBottom > _totalHeightInLogic;

    if(!isOverTop && !isBelowBotton){
        _visibleTop = newTop;
        _visibleBottom = newBottom;
    }else{
        CGFloat scrollerHeightInLogic = _visibleBottom- _visibleTop;
        if(isOverTop){
            _visibleTop = 0.0f;
            _visibleBottom =scrollerHeightInLogic;
        }else{
            _visibleBottom = _totalHeightInLogic;
            _visibleTop = _totalHeightInLogic - scrollerHeightInLogic;
        }
    }

    return [self getNewScrollerFrame];
}
-(CGRect)getNewScrollerFrame{
    
    [self updateViewData];
    
    CGFloat topRatio = max(_visibleTop / _totalHeightInLogic,0.0);
    CGFloat heightRatio = min((_visibleBottom / _totalHeightInLogic) - topRatio,1.0);
    
    return CGRectMake(_scrollerX,
                      SCROLLER_PADDING +topRatio*_scrollerTotalHeight,
                      SCROLLER_WIDTH,
                      heightRatio*_scrollerTotalHeight);
}
-(NSInteger) getCurrentPage{
        
    return min(
               max(
                   ceilf(
                        _visibleTop
                        / PORTRAIT_PAGE_HEIGHT_IN_LOGIC_WITH_SEPARATOR
                         ),
                   1),
               _totalPages);
}

-(void) updateTooltip:(MLOScrollerTooltip *) tooltip withGrid:(MLOScrollerGridViewController *) grid{
    if(mlo_is_document_open() && _visibleTop>=0){
        
        NSInteger currentPage = [self getCurrentPage];

        if(tooltip){
        
            [tooltip updateWithFrame:CGRectMake(_tooltipX,
                                                min(_tooltipPageHeight * (currentPage -1),_tooltipMaxY),
                                                TOOLTIP_WIDTH,
                                                TOOLTIP_HEIGHT) inPage:currentPage];
        }
        [grid onCurrentPageChanged:currentPage];
    }
}


@end
