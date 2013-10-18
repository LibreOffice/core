// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOAppRoleFactory.h"
#import "MLOAppRoleLoApp.h"
#import "MLOAppRoleTileTester.h"
#import "MLOMainViewController.h"

@implementation MLOAppRoleFactory

+(MLOAppRoleBase *) getInstanceWithMainViewController:(MLOMainViewController *) mainViewController{
    MLOAppRoleBase * role;

    NSLog(@"Creating app role: %@",MLOAppRoleString(APP_ROLE));

    switch (APP_ROLE) {
        case LO_APP:
        {
            role = [MLOAppRoleLoApp new];
            break;
        }
        case TILE_TESTER:
        {
            role=  [MLOAppRoleTileTester new];
            break;
        }
    }
    role.mainViewController = mainViewController;
    return role;
}

@end
