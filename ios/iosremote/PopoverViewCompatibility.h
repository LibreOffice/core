//
//  PopoverViewCompatibility.h
//  popover
//
//  Created by alanduncan on 7/22/13.
//  Copyright (c) 2013 Oliver Rickard. All rights reserved.
//

#ifndef popover_PopoverViewCompatibility_h
#define popover_PopoverViewCompatibility_h

#ifdef __IPHONE_6_0

#define UITextAlignmentCenter       NSTextAlignmentCenter
#define UITextAlignmentLeft         NSTextAlignmentLeft
#define UITextAlignmentRight        NSTextAlignmentRight
#define UILineBreakModeTailTruncation   NSLineBreakByTruncatingTail
#define UILineBreakModeMiddleTruncation NSLineBreakByTruncatingMiddle
#define UILineBreakModeWordWrap         NSLineBreakByWordWrapping

#endif

#endif
