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

- (BOOL)panel:(id)sender shouldShowFilename:(NSString *)filename;
- (void)panelSelectionDidChange:(id)sender;
- (void)panel:(id)sender directoryDidChange:(NSString *)path;

- (void)filterSelectedAtIndex:(id)sender;
- (void)autoextensionChanged:(id)sender;

@end

#endif
