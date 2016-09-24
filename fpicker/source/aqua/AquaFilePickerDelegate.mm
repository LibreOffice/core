/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/uno/Any.hxx>

#include "SalAquaFilePicker.hxx"
#include "FilterHelper.hxx"
#include "AquaFilePickerDelegate.hxx"

@implementation AquaFilePickerDelegate

- (id)initWithFilePicker:(SalAquaFilePicker*)fPicker
{
    if ((self = [super init])) {
        filePicker = fPicker;
        filterHelper = nullptr;
        return self;
    }
    return nil;
}

- (void)setFilterHelper:(FilterHelper*)helper
{
    filterHelper = helper;
}

#pragma mark NSSavePanel delegate methods

- (BOOL)panel:(id)sender shouldShowFilename:(NSString *)filename
{
    (void)sender;
    if( filterHelper == nullptr )
        return true;
    if( filename == nil )
        return false;
    return filterHelper->filenameMatchesFilter(filename);
}

- (void)panelSelectionDidChange:(id)sender
{
    (void)sender;
    if (filePicker != nullptr) {
        css::ui::dialogs::FilePickerEvent evt;
        filePicker->fileSelectionChanged(evt);
    }
}

- (void)panel:(id)sender directoryDidChange:(NSString *)path
{
    (void)sender;
    (void)path;
    if (filePicker != nullptr) {
        css::ui::dialogs::FilePickerEvent evt;
        filePicker->directoryChanged(evt);
    }
}


#pragma mark UIActions
- (void)filterSelectedAtIndex:(id)sender
{
    if (sender == nil) {
        return;
    }
    
    if ([sender class] != [NSPopUpButton class]) {
        return;
    }
    
    if (filterHelper == nullptr) {
        return;
    }
    
    NSPopUpButton *popup = (NSPopUpButton*)sender;
    unsigned int selectedIndex = [popup indexOfSelectedItem];
    
    filterHelper->SetFilterAtIndex(selectedIndex);
    
    filePicker->filterControlChanged();
}

- (void)autoextensionChanged:(id)sender 
{
    if (sender == nil) {
        return;
    }
    
    if ([sender class] != [NSButton class]) {
        return;
    }
    uno::Any aValue;
    aValue <<= ([((NSButton*)sender) state] == NSOnState);
    
    filePicker->setValue(css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION, 0, aValue);
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
