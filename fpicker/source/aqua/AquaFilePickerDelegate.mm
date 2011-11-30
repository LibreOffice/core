/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/


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
