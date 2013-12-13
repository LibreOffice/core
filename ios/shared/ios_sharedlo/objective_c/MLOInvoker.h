//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
//  MLOInvoker.h
//  CloudOn_LO_PoC
//

#import <Foundation/Foundation.h>

@protocol MLOInvoker <NSObject>

-(void)willShowLibreOffice;
-(void)didShowLibreOffice;

-(void)willHideLibreOffice;
-(void)didHideLibreOffice;

-(CGRect) bounds;

@end
