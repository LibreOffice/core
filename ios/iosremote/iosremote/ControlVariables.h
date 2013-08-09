// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef iosremote_ControlVariables_h
#define iosremote_ControlVariables_h

// Width (or length before rotation) of the table view embedded within another table view's row
#define kTableLength                                768

// Width of the cells of the embedded table view (after rotation, which means it controls the rowHeight property)
#define kCellWidth                                  172
// Height of the cells of the embedded table view (after rotation, which would be the table's width)
#define kCellHeight                                 129

// Padding for the Cell containing the article image and title
#define kArticleCellVerticalInnerPadding            6
#define kArticleCellHorizontalInnerPadding          7

// Padding for the title label in an article's cell
#define kArticleTitleLabelPadding                   10

// Vertical padding for the embedded table view within the row
#define kRowVerticalPadding                         1
// Horizontal padding for the embedded table view within the row
#define kRowHorizontalPadding                       1

// Background color for the horizontal table view (the one embedded inside the rows of our vertical table)
#define kHorizontalTableBackgroundColor             [UIColor blackColor]

// The background color on the horizontal table view for when we select a particular cell
#define kHorizontalTableSelectedBackgroundColor     [UIColor colorWithRed:0.0 green:0.59607843 blue:0.37254902 alpha:1.0]

#define kHorizontalTableCellHighlightedBackgroundColor [UIColor colorWithRed:0 green:0.4745098 blue:0.29019808 alpha:0.9]

#define kTintColor                                  [UIColor colorWithRed:0 green:0.462745098 blue:1 alpha:1.0]

#define kAppTitleFont                               [UIFont fontWithName:@"HelveticaNeue-Medium" size:20.0f]

#define kAppTextFont                               [UIFont fontWithName:@"HelveticaNeue-Medium" size:16.0f]

#define kAppSmallTextFont                          [UIFont fontWithName:@"HelveticaNeue" size:14.0f]

#endif
