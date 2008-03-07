/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tpaction.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 16:27:35 $
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


#ifndef _SD_TPACTION_HXX
#define _SD_TPACTION_HXX

#ifndef _COM_SUN_STAR_PRESENTATION_CLICKACTION_HPP_
#include <com/sun/star/presentation/ClickAction.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_ANIMATIONEFFECT_HPP_
#include <com/sun/star/presentation/AnimationEffect.hpp>
#endif

#ifndef _SVT_FILEDLG_HXX //autogen
#include <svtools/filedlg.hxx>
#endif

#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _SVX_DLG_CTRL_HXX //autogen
#include <svx/dlgctrl.hxx>
#endif
#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _BASEDLGS_HXX //autogen
#include <sfx2/basedlgs.hxx>
#endif
#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif
#include "sdtreelb.hxx"
#include "anmdef.hxx"

#include <vector>

class ::sd::View;
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

    BOOL                    bTreeUpdated;
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
    String                  GetEditText( BOOL bURL = FALSE );
    String                  GetMacroName( const String& rPath );
    USHORT                  GetClickActionSdResId( ::com::sun::star::presentation::ClickAction eCA );
    USHORT                  GetAnimationEffectSdResId( ::com::sun::star::presentation::AnimationEffect eAE );

public:
            SdTPAction( Window* pParent, const SfxItemSet& rInAttrs );
            ~SdTPAction();

    static  SfxTabPage* Create( Window*, const SfxItemSet& );
    static  USHORT*    GetRanges();

    virtual BOOL FillItemSet( SfxItemSet& );
    virtual void Reset( const SfxItemSet & );

    virtual void ActivatePage( const SfxItemSet& rSet );
    virtual int  DeactivatePage( SfxItemSet* pSet );

    void    Construct();

    void    SetView( const ::sd::View* pSdView );

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;
};

#endif      // _SD_TPACTION_HXX

