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

#import <unistd.h>

#import <iostream>
#import <AppKit/AppKit.h>

static void usage()
{
    std::cout << "Usage: pasteboard\n"
                 "          List the types on the pasteboard and in each pasteboard item.\n"
                 "       pasteboard -a\n"
                 "          Output the data for all types to stdout. Note: output will\n"
                 "          in many cases be binary. The different types are separated by a textual header.\n"
                 "       pasteboard -t type\n"
                 "          Output the data for the type in question to stdout. Note: output will\n"
                 "          in many cases be binary.\n";
}

int main(int argc, char** argv)
{
    NSString* requestedType;

    int ch;

    while ((ch = getopt(argc, argv, "at:")) != -1)
    {
        switch (ch)
        {
            case 'a':
                requestedType = @"*";
                break;
            case 't':
                requestedType = [NSString stringWithUTF8String:optarg];
                break;
            case '?':
                usage();
                return 0;
        }
    }

    argc -= optind;
    argv += optind;

    if (argc > 0)
    {
        usage();
        return 1;
    }

    NSPasteboard* pb = [NSPasteboard generalPasteboard];

    if ([requestedType isEqualToString:@"*"])
    {
        NSArray<NSPasteboardType>* types = [pb types];
        for (unsigned i = 0; i < [types count]; i++)
        {
            NSData* data = [pb dataForType:types[i]];
            std::cout << i << ": " << [types[i] UTF8String] << ": " << std::to_string([data length]) << " bytes:\n";
            if (data != nil)
            {
                std::cout.write((const char*)[data bytes], [data length]);
                std::cout << "\n";
            }
        }
        return 0;
    }

    if ([requestedType length] > 0)
    {
        NSData* data = [pb dataForType:requestedType];

        if (data == nil)
            std::cerr << "No data for " << [requestedType UTF8String] << std::endl;
        else
            std::cout.write((const char*)[data bytes], [data length]);

        return 0;
    }

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
