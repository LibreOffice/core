/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AquaFilePickerDelegate.mm,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:33:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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

- (MacOSBOOL)panel:(id)sender shouldShowFilename:(NSString *)filename
{
    if (filterHelper == NULL)
        return true;
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


@end