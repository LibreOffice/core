// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*-

/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// List the contents of the macOS pasteboard

// Build with: clang++ -Wall -o pasteboard vcl/workben/pasteboard.mm -framework AppKit

#import <iostream>
#import <AppKit/AppKit.h>

int main(int argc, char** argv)
{
    NSPasteboard* pb = [NSPasteboard generalPasteboard];

    {
        NSArray<NSPasteboardType>* types = [pb types];
        std::cout << "Types directly on pasteboard:\n";
        for (unsigned i = 0; i < [types count]; i++)
        {
            std::cout << "  " << i << ": " << [types[i] UTF8String] << "\n";
        }
    }

    NSArray<NSPasteboardItem*>* items = [pb pasteboardItems];
    std::cout << "New-style items on pasteboard:\n";

    for (unsigned i = 0; i < [items count]; i++)
    {
        std::cout << "  Item " << i << ", types:\n";
        NSArray<NSPasteboardType>* types = [items[i] types];
        for (unsigned j = 0; j < [types count]; j++)
        {
            std::cout << "    " << j << ": " << [types[j] UTF8String];

            if ([types[j] isEqualToString:(NSString*)kUTTypePlainText] ||
                [types[j] isEqualToString:(NSString*)kUTTypeUTF8PlainText] ||
                [types[j] isEqualToString:(NSString*)kUTTypeText] ||
                [types[j] isEqualToString:(NSString*)kUTTypeHTML] ||
                [types[j] isEqualToString:(NSString*)kUTTypeRTF] ||
                [types[j] isEqualToString:(NSString*)kUTTypeUTF16ExternalPlainText])
            {
                NSString* string = [items[i] stringForType:NSPasteboardTypeString];
                if ([string length] > 500)
                    string = [[string substringToIndex:501] stringByAppendingString:@"..."];
                std::cout << ": '" << [string UTF8String] << "'";
            }
            std::cout << "\n";
        }
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
