// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOViewController.h"
#import "MLOTestingTileSubviewControllerProtocol.h"

// The size of the actual pixel tile
static const CGFloat CONTEXT_WIDTH_DEFAULT = 450;
static const CGFloat CONTEXT_HEIGHT_DEFAULT = 450;

// In our "decatwips"
static const CGFloat TILE_POS_X_DEFAULT = 0;
static const CGFloat TILE_POS_Y_DEFAULT = 0;

// "Tile" size here means the decatwip size of the part of the document
// rendered into the pixel tile
static const CGFloat TILE_WIDTH_DEFAULT = 500;
static const CGFloat TILE_HEIGHT_DEFAULT = 500;

@interface MLOTestingTileParametersViewController : MLOViewController<MLOTestingTileSubviewControllerProtocol>
@property CGFloat contextWidth, contextHeight, tilePosX, tilePosY, tileWidth, tileHeight;
-(void)renderTile;
@end
