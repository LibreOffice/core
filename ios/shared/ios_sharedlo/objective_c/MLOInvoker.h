//
//  MLOInvoker.h
//  CloudOn_LO_PoC
//
//  Created by ptyl on 10/3/13.
//  Copyright (c) 2013 CloudOn. All rights reserved.
//

#import <Foundation/Foundation.h>

@protocol MLOInvoker <NSObject>

-(void)willShowLibreOffice;
-(void)didShowLibreOffice;

-(void)willHideLibreOffice;
-(void)didHideLibreOffice;

-(CGRect) bounds;

@end
