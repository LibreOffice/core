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



#ifndef CGM_MAIN_HXX
#define CGM_MAIN_HXX

#include "cgm.hxx"

// ---------------------------------------------------------------

#define CGM_LEVEL1                      0x0001  // VERSION 1 METAFILE
#define CGM_LEVEL2                      0x0002
#define CGM_LEVEL3                      0x0003
#define CGM_GDSF_ONLY                   0x00F0
#define CGM_UNKNOWN_LEVEL               0x00F1
#define CGM_UNKNOWN_COMMAND             0x00F2
#define CGM_DESCRIPTION                 0x00F3

#define CGM_EXTENDED_PRIMITIVES_SET     0x0100  // INDICATES PRIMITIVES WHICH ARE AVAILABLE IN
                                                // LEVEL 1 BUT ARE NOT DEFINED IN ISO 7942(GKS)
#define CGM_DRAWING_PLUS_CONTROL_SET    0x0200  // INDICATES THAT THIS IS AN ADDITIONAL LEVEL 1
                                                // ELEMENT
#define ComOut( Level, Description ) if ( mpCommentOut ) ImplComment( Level, Description );

#define BMCOL( _col ) BitmapColor( (sal_Int8)(_col >> 16 ), (sal_Int8)( _col >> 8 ), (sal_Int8)_col )
#define ALIGN2( _nElementSize ) { _nElementSize = ( _nElementSize + 1 ) & ~1; }
#define ALIGN4( _nElementSize ) { _nElementSize = ( _nElementSize + 3 ) & ~3; }
#define ALIGN8( _nElementSize ) { _nElementSize = ( _nElementSize + 7 ) & ~7; }

#include <vcl/salbtype.hxx>
#include <tools/stream.hxx>
#include <tools/list.hxx>
#include "bundles.hxx"
#include "bitmap.hxx"
#include "elements.hxx"


#endif

