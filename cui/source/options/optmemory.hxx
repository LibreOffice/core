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
