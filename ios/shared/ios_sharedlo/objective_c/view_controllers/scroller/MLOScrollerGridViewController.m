// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOScrollerGridViewController.h"
#import "MLOMainViewController.h"

static const CGFloat GRID_LINE_WIDTH=1.5f,
    GRID_ACTUAL_WIDTH = MLO_SCROLLER_GRID_WIDTH- GRID_LINE_WIDTH,
    UNSELECTED_ALPHA=0.05f,
    SELECTED_ALPHA=0.15f;

@interface MLOScrollerGridViewController ()
@property MLOMainViewController * mainViewController;
@property NSInteger pageCount,currentPage;
@property CGFloat height,x,gridCellHeight;
@property NSMutableArray * pages;
@end

@implementation MLOScrollerGridViewController


-(void)hide{
    
    [self resetMembers];
    
    for(UIView * cell in _pages){
        [cell removeFromSuperview];
        cell.alpha=0.0f;
    }
    
    [_pages removeAllObjects];

}

-(void)resetMembers{
    _pageCount=-1;
    _currentPage=0;
    _height=-1.0f;
    _x=-1.0f;
    _gridCellHeight=-1.0;
}

-(id) initWithMainViewController:(MLOMainViewController *) mainViewController{
    self = [super init];
    if (self) {
        self.mainViewController = mainViewController;
        [self resetMembers];
        self.pages = [NSMutableArray new];
    }
    return self;
}

-(void)onRotate:(CGFloat) x{
    _height =_mainViewController.canvas.frame.size.height;
    _x=x;
    [self reshape];
}

-(UIView *) getNewCell{
    UIView * cell =[UIView new];
    
    cell.backgroundColor = [UIColor blackColor];
    cell.alpha = UNSELECTED_ALPHA;
    
    [_mainViewController.canvas addSubview:cell];

    return cell;
}

-(void)onPageCountChanged:(NSInteger) newPageCount{
    if(_pageCount!=newPageCount){
        _pageCount = newPageCount;
        
        NSInteger currentSize = [_pages count];
    
        NSInteger delta = abs(currentSize-newPageCount);
        
        if(delta>0){
            BOOL isAdd = currentSize< newPageCount;
            
            for (NSInteger i=0; i<delta; i++) {
            
               if(isAdd){
                                    
                   [_pages addObject:[self getNewCell]];
                   
               }else{
                   [_pages removeLastObject];
               }
           }
            
           [self reshape];
            
            if(!isAdd){
                if(_currentPage > newPageCount){
                    [self onCurrentPageChanged:newPageCount];
                }
            }
        }
    }
}

-(void)reshape{
    if((_x>=0) &&(_pageCount>0)){
        _gridCellHeight= (_height - GRID_LINE_WIDTH*(_pageCount +1 ))/_pageCount;
 
        CGFloat y = GRID_LINE_WIDTH;
        
        for (UIView * cell in _pages){
        
            cell.frame = CGRectMake(_x+GRID_LINE_WIDTH, y, GRID_ACTUAL_WIDTH, _gridCellHeight);
        
            y+=_gridCellHeight +GRID_LINE_WIDTH;
        }
    }
}

-(void) onCurrentPageChanged:(NSInteger) newCurrentPage{
    if(_currentPage!=newCurrentPage){
        if( (_currentPage > 0) && ([_pages count] >= _currentPage)){
            ((UIView *) [_pages objectAtIndex:_currentPage-1]).alpha = UNSELECTED_ALPHA;
        }
        if( (newCurrentPage > 0) && ([_pages count] >= newCurrentPage)){
            ((UIView *) [_pages objectAtIndex:newCurrentPage -1]).alpha = SELECTED_ALPHA;
        }
        _currentPage  = newCurrentPage;
    }
    
}

@end
