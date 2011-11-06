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


#ifndef _WALLITEM_HXX
#define _WALLITEM_HXX

#ifndef SHL_HXX
#include <tools/shl.hxx>
#endif

// -----------------------------------------------------------------------------------------
// Hilfsklasse, um die Download-Funktionalitaet des SvxBrushItems unterhalb
// des SVX zu benutzen. Der Link wird im Konstruktor von SvxDialogDll gesetzt.
#ifndef _SFX_BRUSHITEMLINK_DECLARED
#define _SFX_BRUSHITEMLINK_DECLARED
#include <vcl/graph.hxx>

#ifndef SHL_BRUSHITEM
#define SHL_BRUSHITEM SHL_SHL1
#endif

class Window;
class SfxItemSet;

typedef void* (*CreateSvxBrushTabPage)(Window *pParent, const SfxItemSet &rAttrSet);
typedef sal_uInt16* (*GetSvxBrushTabPageRanges)();

class Graphic;
class String;
class SfxBrushItemLink
{
public:
    virtual Graphic GetGraphic( const String& rLink, const String& rFilter) = 0;
    virtual CreateSvxBrushTabPage GetBackgroundTabpageCreateFunc() = 0;
    virtual GetSvxBrushTabPageRanges GetBackgroundTabpageRanges() = 0;
    static SfxBrushItemLink* Get() { return *(SfxBrushItemLink**)GetAppData(SHL_BRUSHITEM); }
    static void Set( SfxBrushItemLink* pLink );
};
#endif  // _SFX_BRUSHITEMLINK_DECLARED

#endif // _WALLITEM_HXX

