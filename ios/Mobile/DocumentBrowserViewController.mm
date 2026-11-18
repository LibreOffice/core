// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*-
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#import "config.h"

#import "AppDelegate.h"
#import "CODocument.h"
#import "DocumentBrowserViewController.h"
#import "DocumentViewController.h"
#import "TemplateCollectionViewController.h"

@interface DocumentBrowserViewController () <UIDocumentBrowserViewControllerDelegate>

@end

@implementation DocumentBrowserViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    self.delegate = self;
    self.allowsDocumentCreation = YES;
    self.allowsPickingMultipleItems = NO;

    // Update the style of the UIDocumentBrowserViewController
    // self.browserUserInterfaceStyle = UIDocumentBrowserUserInterfaceStyleDark;
    // self.view.tintColor = [UIColor whiteColor];

    // Specify the allowed content types of your application via the Info.plist.

    // Do any additional setup after loading the view, typically from a nib.
}

- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];

#if 0
    NSLog(@"Contents of NSHomeDirectory:");
    auto enumerator = [[NSFileManager defaultManager] enumeratorAtPath:NSHomeDirectory()];
    NSString *file;
    long long total = 0;
    while ((file = [enumerator nextObject])) {
        NSString *suffix = @"";
        if ([enumerator fileAttributes][NSFileType] == NSFileTypeRegular) {
            suffix = [NSString stringWithFormat:@"  %@", [enumerator fileAttributes][NSFileSize]];
            total += [[enumerator fileAttributes][NSFileSize] longLongValue];
        } else if ([enumerator fileAttributes][NSFileType] == NSFileTypeDirectory) {
            suffix = @"/";
        }
        NSLog(@"%@%@%@", [NSString stringWithFormat:@"%*s", (int)[enumerator level] * 2, ""], [file lastPathComponent], suffix);
    }
    NSLog(@"==== Total size of app home directory: %lld", total);
#endif
}

- (void)documentBrowser:(UIDocumentBrowserViewController *)controller didRequestDocumentCreationWithHandler:(void (^)(NSURL * _Nullable, UIDocumentBrowserImportMode))importHandler {
    UIStoryboard *storyBoard = [UIStoryboard storyboardWithName:@"Main" bundle:nil];
    TemplateCollectionViewController *templateCollectionViewController = [storyBoard instantiateViewControllerWithIdentifier:@"TemplateCollectionViewController"];
    [templateCollectionViewController removeFromParentViewController];
    templateCollectionViewController.importHandler = importHandler;

    // Fix issue #1962 Use UINavigationController to add a cancel button
    UINavigationController *navController = [[UINavigationController alloc] initWithRootViewController:templateCollectionViewController];
    templateCollectionViewController.navigationItem.rightBarButtonItem = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemCancel target:templateCollectionViewController action:@selector(cancel)];
    [self.view addSubview:navController.view];
    [self presentViewController:navController animated:YES completion:nil];
}

-(void)documentBrowser:(UIDocumentBrowserViewController *)controller didPickDocumentsAtURLs:(NSArray<NSURL *> *)documentURLs {
    NSURL *sourceURL = documentURLs.firstObject;
    if (!sourceURL) {
        return;
    }

    // Present the Document View Controller for the first document that was picked.
    // If you support picking multiple items, make sure you handle them all.
    [self presentDocumentAtURL:sourceURL];
}

- (void)documentBrowser:(UIDocumentBrowserViewController *)controller didImportDocumentAtURL:(NSURL *)sourceURL toDestinationURL:(NSURL *)destinationURL {
    // Present the Document View Controller for the new newly created document
    [self presentDocumentAtURL:destinationURL];
}

- (void)documentBrowser:(UIDocumentBrowserViewController *)controller failedToImportDocumentAtURL:(NSURL *)documentURL error:(NSError * _Nullable)error {
    // Make sure to handle the failed import appropriately, e.g., by presenting an error message to the user.
}

- (void)presentDocumentAtURL:(NSURL *)documentURL {
    UIStoryboard *storyBoard = [UIStoryboard storyboardWithName:@"Main" bundle:nil];
    DocumentViewController *documentViewController = [storyBoard instantiateViewControllerWithIdentifier:@"DocumentViewController"];
    documentViewController.document = [[CODocument alloc] initWithFileURL:documentURL];
    documentViewController.document->fakeClientFd = -1;
    documentViewController.document->readOnly = false;
    documentViewController.document.viewController = documentViewController;
    [self presentViewController:documentViewController animated:YES completion:nil];
}

@end

// vim:set shiftwidth=4 softtabstop=4 expandtab:
