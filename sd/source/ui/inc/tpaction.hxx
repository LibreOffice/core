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


#ifndef _SD_TPACTION_HXX
#define _SD_TPACTION_HXX

#include <com/sun/star/presentation/ClickAction.hpp>
#include <com/sun/star/presentation/AnimationEffect.hpp>
#include <svtools/filedlg.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
#include <svx/dlgctrl.hxx>
#include <sfx2/tabdlg.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/tabdlg.hxx>
#include "sdtreelb.hxx"
#include "anmdef.hxx"

#include <vector>

namespace sd {
    class View;
}
class SdDrawDocument;

/*************************************************************************
|*
|* Effekte-SingleTab-Dialog
|*
\************************************************************************/
class SdActionDlg : public SfxSingleTabDialog
{
private:
    const SfxItemSet&   rOutAttrs;

public:

            SdActionDlg( Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView );
            ~SdActionDlg() {};
};

/*************************************************************************
|*
|* Interaktions-Tab-Page
|*
\************************************************************************/

class SdTPAction : public SfxTabPage
{
private:
    FixedText               aFtAction;              // always visible
    ListBox                 aLbAction;

    FixedText               aFtTree;                // jump destination controls
    SdPageObjsTLB           aLbTree;
    SdPageObjsTLB           aLbTreeDocument;
    ListBox                 aLbOLEAction;

    FixedLine               aFlSeparator;
    Edit                    aEdtSound;
    Edit                    aEdtBookmark;
    Edit                    aEdtDocument;
    Edit                    aEdtProgram;
    Edit                    aEdtMacro;
    PushButton              aBtnSearch;
    PushButton              aBtnSeek;

    const SfxItemSet&       rOutAttrs;
    const ::sd::View*       mpView;
    SdDrawDocument*         mpDoc;
    XColorTable*            pColTab;

    sal_Bool                    bTreeUpdated;
    List*                   pCurrentActions;
    String                  aLastFile;
    ::std::vector< long >   aVerbVector;

    //------------------------------------

    DECL_LINK( ClickSearchHdl, void * );
    DECL_LINK( ClickActionHdl, void * );
    DECL_LINK( SelectTreeHdl, void * );
    DECL_LINK( CheckFileHdl, void * );

    void                    UpdateTree();
    virtual void            OpenFileDialog();
    ::com::sun::star::presentation::ClickAction     GetActualClickAction();
    void                    SetActualClickAction( ::com::sun::star::presentation::ClickAction eCA );
    void                    SetActualAnimationEffect( ::com::sun::star::presentation::AnimationEffect eAE );
    void                    SetEditText( String const & rStr );
    String                  GetEditText( sal_Bool bURL = sal_False );
    sal_uInt16                  GetClickActionSdResId( ::com::sun::star::presentation::ClickAction eCA );
    sal_uInt16                  GetAnimationEffectSdResId( ::com::sun::star::presentation::AnimationEffect eAE );

public:
            SdTPAction( Window* pParent, const SfxItemSet& rInAttrs );
            ~SdTPAction();

    static  SfxTabPage* Create( Window*, const SfxItemSet& );

    virtual sal_Bool FillItemSet( SfxItemSet& );
    virtual void Reset( const SfxItemSet & );

    virtual void ActivatePage( const SfxItemSet& rSet );
    virtual int  DeactivatePage( SfxItemSet* pSet );

    void    Construct();

    void    SetView( const ::sd::View* pSdView );

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;
};

#endif      // _SD_TPACTION_HXX

