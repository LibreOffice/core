/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/uno/Any.hxx>

#ifndef _SALAQUAFILEPICKER_HXX_
#include "SalAquaFilePicker.hxx"
#endif

#ifndef _FILTERHELPER_HXX_
#include "FilterHelper.hxx"
#endif

#include "AquaFilePickerDelegate.hxx"

@implementation AquaFilePickerDelegate

- (id)initWithFilePicker:(SalAquaFilePicker*)fPicker
{
    if ((self = [super init])) {
        filePicker = fPicker;
        filterHelper = NULL;
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
    if( filterHelper == NULL )
        return true;
    if( filename == nil )
        return false;
    return filterHelper->filenameMatchesFilter(filename);
}

- (void)panelSelectionDidChange:(id)sender
{
    if (filePicker != NULL) {
        ::com::sun::star::ui::dialogs::FilePickerEvent evt;
        filePicker->fileSelectionChanged(evt);
    }
}

- (void)panel:(id)sender directoryDidChange:(NSString *)path
{
    if (filePicker != NULL) {
        ::com::sun::star::ui::dialogs::FilePickerEvent evt;
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
    
    if (filterHelper == NULL) {
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
    
    filePicker->setValue(::com::sun::star::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION, 0, aValue);
}

@end
