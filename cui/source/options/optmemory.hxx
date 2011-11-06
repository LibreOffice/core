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


#ifndef _OFA_OPTMEMORY_HXX
#define _OFA_OPTMEMORY_HXX

#include <vcl/lstbox.hxx>
#include <vcl/group.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/langbox.hxx>
#define FOLDERWEBVIEW_DEFAULTFILE   "folder.so"

// class OfaMiscTabPage --------------------------------------------------

class OfaMemoryOptionsPage : public SfxTabPage
{
    using SfxTabPage::DeactivatePage;
private:
    FixedLine           aUndoBox;
    FixedText           aUndoText;
    NumericField        aUndoEdit;

    FixedLine           aGbGraphicCache;
    FixedText           aFtGraphicCache;
    NumericField        aNfGraphicCache;
    FixedText           aFtGraphicCacheUnit;
    FixedText           aFtGraphicObjectCache;
    NumericField        aNfGraphicObjectCache;
    FixedText           aFtGraphicObjectCacheUnit;
    FixedText           aFtGraphicObjectTime;
    TimeField           aTfGraphicObjectTime;
    FixedText           aFtGraphicObjectTimeUnit;

    FixedLine           aGbOLECache;
    FixedText           aFtOLECache;
    NumericField        aNfOLECache;

    FixedLine           aQuickLaunchFL;
    CheckBox            aQuickLaunchCB;

    DECL_LINK( GraphicCacheConfigHdl, NumericField* );

    inline long         GetNfGraphicCacheVal( void ) const;                 // returns # of Bytes
    inline void         SetNfGraphicCacheVal( long nSizeInBytes );

    long                GetNfGraphicObjectCacheVal( void ) const;           // returns # of Bytes
    void                SetNfGraphicObjectCacheVal( long nSizeInBytes );
    inline void         SetNfGraphicObjectCacheMax( long nSizeInBytes );
    inline void         SetNfGraphicObjectCacheLast( long nSizeInBytes );
protected:
    virtual int         DeactivatePage( SfxItemSet* pSet = NULL );

public:
    OfaMemoryOptionsPage( Window* pParent, const SfxItemSet& rSet );
    ~OfaMemoryOptionsPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};


#endif
