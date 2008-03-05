/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AquaFilePickerDelegate.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:33:39 $
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

#ifndef _AQUAFILEPICKERDELEGATE_HXX_
#define _AQUAFILEPICKERDELEGATE_HXX_

#include <premac.h>
#include <Cocoa/Cocoa.h>
#include <postmac.h>

class SalAquaFilePicker;
class FilterHelper;

@interface AquaFilePickerDelegate : NSObject
{
    SalAquaFilePicker* filePicker;
    FilterHelper* filterHelper;
}

- (id)initWithFilePicker:(SalAquaFilePicker*)fPicker;

- (void)setFilterHelper:(FilterHelper*)filterHelper;

- (MacOSBOOL)panel:(id)sender shouldShowFilename:(NSString *)filename;

- (void)panelSelectionDidChange:(id)sender;
- (void)panel:(id)sender directoryDidChange:(NSString *)path;

- (void)filterSelectedAtIndex:(id)sender;

@end

#endif
