/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optmemory.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 12:22:48 $
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
#ifndef _OFA_OPTMEMORY_HXX
#define _OFA_OPTMEMORY_HXX

#ifndef _SV_LSTBOX_HXX //autogen wg. ListBox
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_GROUP_HXX //autogen wg. FixedLine
#include <vcl/group.hxx>
#endif
#ifndef _SV_FIELD_HXX //autogen wg. NumericField
#include <vcl/field.hxx>
#endif
#ifndef _SV_FIXED_HXX //autogen wg. FixedText
#include <vcl/fixed.hxx>
#endif
#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _SVX_LANGBOX_HXX
#include <langbox.hxx>
#endif
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

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};


#endif
