// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "HorizontalSlideCell.h"

@implementation HorizontalSlideCell

@synthesize thumbnail = _thumbnail;
@synthesize numberLabel = _numberLabel;

- (NSString *)reuseIdentifier
{
    return @"HorizontalTableSlideCell";
}

- (id)initWithStyle:(UITableViewCellStyle)style reuseIdentifier:(NSString *)reuseIdentifier
{
    self = [super initWithStyle:style reuseIdentifier:reuseIdentifier];
    if (self) {
        // Initialization code
    }
    return self;
}

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    
    self.thumbnail = [[UIImageView alloc] initWithFrame:CGRectMake(kArticleCellHorizontalInnerPadding, kArticleCellVerticalInnerPadding, kCellWidth - kArticleCellHorizontalInnerPadding * 2, kCellHeight - kArticleCellVerticalInnerPadding * 2)];
    self.thumbnail.opaque = YES;
    
    [self.contentView addSubview:self.thumbnail];
    
    self.numberLabel = [[UILabel alloc] initWithFrame:CGRectMake(self.thumbnail.frame.size.width * 0.8, self.thumbnail.frame.size.height * 0.8, self.thumbnail.frame.size.width * 0.2, self.thumbnail.frame.size.height * 0.2)];
    self.numberLabel.opaque = YES;
	self.numberLabel.backgroundColor = kHorizontalTableCellHighlightedBackgroundColor;
    self.numberLabel.textColor = [UIColor whiteColor];
    self.numberLabel.font = [UIFont boldSystemFontOfSize:11];
    self.numberLabel.textAlignment = UITextAlignmentCenter;
    self.numberLabel.numberOfLines = 1;
    [self.thumbnail addSubview:self.numberLabel];
    
    self.backgroundColor = [UIColor colorWithRed:0 green:0.40784314 blue:0.21568627 alpha:1.0];
    self.selectedBackgroundView = [[UIView alloc] initWithFrame:self.thumbnail.frame];
    self.selectedBackgroundView.backgroundColor = kHorizontalTableSelectedBackgroundColor;
    
    self.transform = CGAffineTransformMakeRotation(M_PI * 0.5);
    
    return self;
}

- (void)setSelected:(BOOL)selected animated:(BOOL)animated
{
    [super setSelected:selected animated:animated];

    // Configure the view for the selected state
}

@end
