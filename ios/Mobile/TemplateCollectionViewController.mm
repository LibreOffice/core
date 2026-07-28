// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*-
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#define LIBO_INTERNAL_ONLY
#import <COKit/COKitInit.h>
#import <COKit/COKit.hxx>

#import "ios.h"
#import "AppDelegate.h"
#import "L10n.h"
#import "TemplateCollectionViewController.h"
#import "TemplateSectionHeaderView.h"

#import "svtools/strings.hrc"

static NSString *mapTemplateExtensionToActual(NSString *templateName) {
    NSString *baseName = [templateName stringByDeletingPathExtension];
    NSString *extension = [templateName substringFromIndex:baseName.length];

    if ([extension isEqualToString:@".ott"])
        return [baseName stringByAppendingString:@".odt"];
    else if ([extension isEqualToString:@".ots"])
        return [baseName stringByAppendingString:@".ods"];
    else if ([extension isEqualToString:@".otp"])
        return [baseName stringByAppendingString:@".odp"];
    else
        assert(false);
}

@implementation TemplateCollectionViewController

- (void)viewDidLoad {

    // Partial fix for issue #1962 Dismiss view by tapping outside of the view
    // Setting modalInPresentation to YES will ignore all events outside of
    // the view so set self.modalInPresentation to NO.
    self.modalInPresentation = NO;

    static NSString *downloadedTemplates = [[NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) objectAtIndex:0] stringByAppendingString:@"/downloadedTemplates/"];

    // Scan for available templates. First downloaded ones.
    NSDirectoryEnumerator<NSString *> *e = [[NSFileManager defaultManager] enumeratorAtPath:downloadedTemplates];

    templates[0] = [@[] mutableCopy];
    templates[1] = [@[] mutableCopy];
    templates[2] = [@[] mutableCopy];

    NSString *subPath;
    while ((subPath = [e nextObject]) != nil) {
        NSString *path = [downloadedTemplates stringByAppendingString:subPath];
        if ([[path pathExtension] isEqualToString:@"ott"]) {
            [templates[0] addObject:[NSURL fileURLWithPath:path]];
        } else if ([[path pathExtension] isEqualToString:@"ots"]) {
            [templates[1] addObject:[NSURL fileURLWithPath:path]];
        } else if ([[path pathExtension] isEqualToString:@"otp"]) {
            [templates[2] addObject:[NSURL fileURLWithPath:path]];
        }
    }

    if ([templates[0] count] == 0)
        templates[0] = [[[NSBundle mainBundle] URLsForResourcesWithExtension:@".ott" subdirectory:@"Templates"] mutableCopy];
    if ([templates[1] count] == 0)
        templates[1] = [[[NSBundle mainBundle] URLsForResourcesWithExtension:@".ots" subdirectory:@"Templates"] mutableCopy];
    if ([templates[2] count] == 0)
        templates[2] = [[[NSBundle mainBundle] URLsForResourcesWithExtension:@".otp" subdirectory:@"Templates"] mutableCopy];
}

- (void)viewDidDisappear:(BOOL)animated {
    // Partial fix for issue #1962 Invoke import handler when view is dismissed
    // If the import handler has not already been invoked, invoke it or else
    // -[DocumentBrowserViewController
    // documentBrowser:didRequestDocumentCreationWithHandler:] will never be
    // hcalled again.
    if (self.importHandler) {
        self.importHandler(nil, UIDocumentBrowserImportModeNone);
        self.importHandler = nil;
    }
}

- (NSInteger)numberOfSectionsInCollectionView:(UICollectionView *)collectionView {
    // Three sections: Document, Spreadsheet, and Presentation
    return 3;
}

- (NSInteger)collectionView:(UICollectionView *)collectionView numberOfItemsInSection:(NSInteger)section {
    assert(section >= 0 && section <= 2);
    return templates[section].count;
}

- (UICollectionViewCell *)collectionView:(UICollectionView *)collectionView cellForItemAtIndexPath:(NSIndexPath *)indexPath {
    assert(indexPath.length == 2);
    assert([indexPath indexAtPosition:0] <= 2);
    assert([indexPath indexAtPosition:1] < templates[[indexPath indexAtPosition:0]].count);

    UICollectionViewCell *cell = [collectionView dequeueReusableCellWithReuseIdentifier:@"Cell" forIndexPath:indexPath];

    UIImageView *image = (UIImageView *)[cell viewWithTag:1];
    UILabel *title = (UILabel *)[cell viewWithTag:2];

    NSString *templateThumbnail = [[templates[[indexPath indexAtPosition:0]][[indexPath indexAtPosition:1]] path] stringByAppendingString:@".png"];
    UIImage *thumbnail;
    if ([NSFileManager.defaultManager fileExistsAtPath:templateThumbnail])
        thumbnail = [UIImage imageWithContentsOfFile:templateThumbnail];
    else
        thumbnail = [UIImage imageNamed:@"AppIcon"];

    image.image = thumbnail;

    NSString *fileName = [templates[[indexPath indexAtPosition:0]][[indexPath indexAtPosition:1]] lastPathComponent];

    title.text = [fileName stringByDeletingPathExtension];

    return cell;
}

- (CGSize)collectionView:(UICollectionView *)collectionView layout:(UICollectionViewLayout*)collectionViewLayout sizeForItemAtIndexPath:(NSIndexPath *)indexPath {
    return CGSizeMake(150, 150);
}

- (UICollectionReusableView *)collectionView:(UICollectionView *)collectionView viewForSupplementaryElementOfKind:(NSString *)kind atIndexPath:(NSIndexPath *)indexPath {
    assert(kind == UICollectionElementKindSectionHeader);

    assert(indexPath.length == 2);
    assert([indexPath indexAtPosition:1] == 0);

    NSUInteger index = [indexPath indexAtPosition:0];
    assert(index <= 2);

    TemplateSectionHeaderView *header = [collectionView dequeueReusableSupplementaryViewOfKind:UICollectionElementKindSectionHeader withReuseIdentifier:@"SectionHeaderView" forIndexPath:indexPath];

    char *translatedHeader;

    if (index == 0)
        translatedHeader = _(STR_DESCRIPTION_FACTORY_WRITER, "svt");
    else if (index == 1)
        translatedHeader = _(STR_DESCRIPTION_FACTORY_CALC, "svt");
    else if (index == 2)
        translatedHeader = _(STR_DESCRIPTION_FACTORY_IMPRESS, "svt");
    else
        abort();

    header.title.text = [NSString stringWithUTF8String:translatedHeader];

    free(translatedHeader);

    return header;
}

- (BOOL)collectionView:(UICollectionView *)collectionView shouldSelectItemAtIndexPath:(NSIndexPath *)indexPath {
    NSURL *selectedTemplate = templates[[indexPath indexAtPosition:0]][[indexPath indexAtPosition:1]];

    NSURL *cacheDirectory = [NSFileManager.defaultManager URLsForDirectory:NSCachesDirectory inDomains:NSUserDomainMask][0];
    NSURL *newURL = [cacheDirectory URLByAppendingPathComponent:mapTemplateExtensionToActual(selectedTemplate.lastPathComponent)
                                                    isDirectory:NO];
    // Load the template into LibreOffice core, save as the corresponding document type (with the
    // same basename), and then proceed to edit that.

    COKitDocument *doc = lo_kit->pClass->documentLoad(lo_kit, [[selectedTemplate absoluteString] UTF8String]);
    doc->pClass->saveAs(doc, [[newURL absoluteString] UTF8String], nullptr, nullptr);
    doc->pClass->destroy(doc);

    // Partial fix for issue #1962 Set import handler to nil after use
    if (self.importHandler) {
        self.importHandler(newURL, UIDocumentBrowserImportModeMove);
        self.importHandler = nil;
    }

    [self dismissViewControllerAnimated:YES completion:nil];

    return YES;
}

- (void)cancel {
    // Partial fix for issue #1962 Set import handler to nil after use
    if (self.importHandler) {
        self.importHandler(nil, UIDocumentBrowserImportModeNone);
        self.importHandler = nil;
    }

    [self dismissViewControllerAnimated:YES completion:nil];
}

@end

// vim:set shiftwidth=4 softtabstop=4 expandtab:
