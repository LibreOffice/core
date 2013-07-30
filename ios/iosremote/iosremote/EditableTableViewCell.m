// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "EditableTableViewCell.h"

@implementation EditableTableViewCell

@synthesize inputLabel = _inputLabel;
@synthesize textField = _textField;

- (id)initWithStyle:(UITableViewCellStyle)style
    reuseIdentifier:(NSString *)identifier
{
    self = [super initWithStyle:style reuseIdentifier:identifier];
    
    if (self == nil)
    {
        return nil;
    }
    CGRect bounds = self.contentView.bounds;
    if (UI_USER_INTERFACE_IDIOM()==UIUserInterfaceIdiomPad)
        bounds.size.width += 160;
    CGRect rect = CGRectInset(bounds, 20.0, 10.0);
    UITextField *textField = [[UITextField alloc] initWithFrame:rect];
    
    //  Set the keyboard's return key label to 'Next'.
    //
    [textField setReturnKeyType:UIReturnKeyNext];
    
    //  Make the clear button appear automatically.
    [textField setClearButtonMode:UITextFieldViewModeWhileEditing];
    [textField setOpaque:YES];
    
    [[self contentView] addSubview:textField];
    [self setTextField:textField];
    
    return self;
}

//  Disable highlighting of currently selected cell.
//
- (void)setSelected:(BOOL)selected
           animated:(BOOL)animated
{
    [super setSelected:selected animated:NO];
    
    [self setSelectionStyle:UITableViewCellSelectionStyleNone];
}

@end
