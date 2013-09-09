//
//  IASKPSTextFieldSpecifierViewCell.m
//  http://www.inappsettingskit.com
//
//  Copyright (c) 2009-2010:
//  Luc Vandal, Edovia Inc., http://www.edovia.com
//  Ortwin Gentz, FutureTap GmbH, http://www.futuretap.com
//  All rights reserved.
//
//  It is appreciated but not required that you give credit to Luc Vandal and Ortwin Gentz,
//  as the original authors of this code. You can give credit in a blog post, a tweet or on
//  a info page of your app. Also, the original authors appreciate letting them know if you use this code.
//
//  This code is licensed under the BSD license that is available at: http://www.opensource.org/licenses/bsd-license.php
//

#import "IASKPSTextFieldSpecifierViewCell.h"
#import "IASKTextField.h"
#import "IASKSettingsReader.h"

@implementation IASKPSTextFieldSpecifierViewCell

@synthesize label=_label,
            textField=_textField;

- (void)layoutSubviews {
    [super layoutSubviews];
	CGSize labelSize = [_label sizeThatFits:CGSizeZero];
	labelSize.width = MIN(labelSize.width, _label.bounds.size.width);

	CGRect textFieldFrame = _textField.frame;
	textFieldFrame.origin.x = _label.frame.origin.x + MAX(kIASKMinLabelWidth, labelSize.width) + kIASKSpacing;
	if (!_label.text.length)
		textFieldFrame.origin.x = _label.frame.origin.x;
	textFieldFrame.size.width = _textField.superview.frame.size.width - textFieldFrame.origin.x - _label.frame.origin.x;
	_textField.frame = textFieldFrame;
}

- (void)setSelected:(BOOL)selected animated:(BOOL)animated {

    [super setSelected:selected animated:animated];

    // Configure the view for the selected state
}




@end
