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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"

#include <stdio.h>

#ifndef _SV_PRNSETUP_HXX //autogen
#include <svtools/prnsetup.hxx>
#endif
#include <vcl/cmdevt.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/print.hxx>
#include <svl/style.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>
#include <svtools/imagemgr.hxx>
#include <vcl/waitobj.hxx>
#include <tools/urlobj.hxx>
#include <tools/color.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/moduleoptions.hxx>
#include <sot/exchange.hxx>
#include <comphelper/storagehelper.hxx>

#include "helpid.hrc"
#include "docvor.hxx"
#include <sfx2/docfac.hxx>
#include "orgmgr.hxx"
#include <sfx2/doctempl.hxx>
#include <sfx2/templdlg.hxx>
#include "sfxtypes.hxx"
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include "sfx2/sfxresid.hxx"
#include "doc.hrc"
#include <sfx2/sfx.hrc>
#include "docvor.hrc"
#include <sfx2/docfilt.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <svtools/localresaccess.hxx>
#ifndef _SVT_DOC_ADDRESSTEMPLATE_HXX_
#include <svtools/addresstemplate.hxx>
#endif
#include <comphelper/processfactory.hxx>
#define _SVSTDARR_STRINGSDTOR
#include <svl/svstdarr.hxx>

static const char cDelim = ':';
sal_Bool SfxOrganizeListBox_Impl::bDropMoveOk = sal_True;

using namespace ::com::sun::star;

//=========================================================================

class SuspendAccel
{
public:
    Accelerator*    pAccel;

    SuspendAccel( Accelerator* pA )
    {
        pAccel=pA;
        GetpApp()->RemoveAccel( pAccel );
    }
    ~SuspendAccel()
    {
        GetpApp()->InsertAccel( pAccel );
    }
};

//=========================================================================


inline void SfxOrganizeListBox_Impl::SetBitmaps(
    const Image &rOFolder, const Image &rCFolder, const Image &rODoc, const Image &rCDoc,
    const Image &rOFolderHC, const Image &rCFolderHC, const Image &rODocHC, const Image &rCDocHC )
{
    aOpenedFolderBmp = rOFolder;
    aClosedFolderBmp = rCFolder;
    aOpenedDocBmp = rODoc;
    aClosedDocBmp = rCDoc;

    aOpenedFolderBmpHC = rOFolderHC;
    aClosedFolderBmpHC = rCFolderHC;
    aOpenedDocBmpHC = rODocHC;
    aClosedDocBmpHC = rCDocHC;

}

//=========================================================================

#define NO_DROP_ACTION  ((sal_Int8)-1)

class SfxOrganizeDlg_Impl
{
friend class SfxTemplateOrganizeDlg;
friend class SfxOrganizeListBox_Impl;

    SuspendAccel*           pSuspend;
    SfxTemplateOrganizeDlg* pDialog;

    SfxOrganizeListBox_Impl* pFocusBox;
    Printer*                 pPrt;

    // save pointer for asynchronous D&D
    SvLBox*                 pSourceView;
    SvLBoxEntry*            pTargetEntry;
    SfxOrganizeListBox_Impl* pFinishedBox;
    sal_Int8                nDropAction;
    bool                    bExecDropFinished;

    // save some variables for the asynchronous file dialog
    sal_uInt16                  m_nRegion;
    sal_uInt16                  m_nIndex;
    String                  m_sExtension4Save;

    SfxOrganizeListBox_Impl aLeftLb;
    ListBox                 aLeftTypLb;

    SfxOrganizeListBox_Impl aRightLb;
    ListBox                 aRightTypLb;

    OKButton                aOkBtn;
    MenuButton              aEditBtn;
    HelpButton              aHelpBtn;
    PushButton              aAddressTemplateBtn;
    PushButton              aFilesBtn;

    Accelerator             aEditAcc;

    String                  aLastDir;
    SfxOrganizeMgr          aMgr;
    sfx2::FileDialogHelper* pFileDlg;

    SvStringsDtor*          GetAllFactoryURLs_Impl() const;
    sal_Bool                GetServiceName_Impl( String& rFactoryURL, String& rFileURL ) const;
    long                    Dispatch_Impl( sal_uInt16 nId, Menu* _pMenu );
    String                  GetPath_Impl( sal_Bool bOpen, const String& rFileName );
    ::com::sun::star::uno::Sequence< ::rtl::OUString >
                            GetPaths_Impl( const String& rFileName );
    void                    InitBitmaps( void );

    DECL_LINK( GetFocus_Impl, SfxOrganizeListBox_Impl * );
    DECL_LINK( LeftListBoxSelect_Impl, ListBox * );
    DECL_LINK( RightListBoxSelect_Impl, ListBox * );
    DECL_LINK( AccelSelect_Impl, Accelerator * );
    DECL_LINK( MenuSelect_Impl, Menu * );
    DECL_LINK( MenuActivate_Impl, Menu * );
    DECL_LINK( AddFiles_Impl, Button * );
    DECL_LINK( OnAddressTemplateClicked, Button * );

    DECL_LINK( ImportHdl, sfx2::FileDialogHelper* );
    DECL_LINK( ExportHdl, sfx2::FileDialogHelper* );
    DECL_LINK( AddFilesHdl, sfx2::FileDialogHelper* );

    sal_Bool        DontDelete_Impl( SvLBoxEntry* pEntry );
    void        OkHdl( Button* );

public:
    SfxOrganizeDlg_Impl( SfxTemplateOrganizeDlg* pParent, SfxDocumentTemplates* pTempl );
    ~SfxOrganizeDlg_Impl();
};

//-------------------------------------------------------------------------

SfxOrganizeDlg_Impl::SfxOrganizeDlg_Impl( SfxTemplateOrganizeDlg* pParent,
                                          SfxDocumentTemplates* pTempl ) :

    pSuspend            ( NULL ),
    pDialog             ( pParent ),
    pFocusBox           ( NULL ),
    pPrt                ( NULL ),
    pSourceView         ( NULL ),
    pTargetEntry        ( NULL ),
    pFinishedBox        ( NULL ),
    nDropAction         ( NO_DROP_ACTION ),
    bExecDropFinished   ( true ),

    aLeftLb     ( this, pParent, WB_BORDER | WB_TABSTOP | WB_HSCROLL, SfxOrganizeListBox_Impl::VIEW_TEMPLATES ),
    aLeftTypLb  (  pParent, SfxResId( LB_LEFT_TYP ) ),

    aRightLb    ( this, pParent, WB_BORDER | WB_TABSTOP | WB_HSCROLL, SfxOrganizeListBox_Impl::VIEW_FILES ),
    aRightTypLb ( pParent, SfxResId( LB_RIGHT_TYP ) ),

    aOkBtn              ( pParent, SfxResId( BTN_OK ) ),
    aEditBtn            ( pParent, SfxResId( BTN_EDIT ) ),
    aHelpBtn            ( pParent, SfxResId( BTN_HELP ) ),
    aAddressTemplateBtn ( pParent, SfxResId( BTN_ADDRESSTEMPLATE ) ),
    aFilesBtn           ( pParent, SfxResId( BTN_FILES ) ),

    aEditAcc    ( SfxResId( ACC_EDIT ) ),
    aMgr        ( &aLeftLb, &aRightLb, pTempl ),
    pFileDlg    ( NULL )

{
    // update the SfxDocumentTemplates the manager works with
    if ( aMgr.GetTemplates() )  // should never fail, but who knows ....
    {
        // for this, show a wait cursor (it may take a while)
        Window* pWaitObjectRange = pDialog ? pDialog->GetParent() : NULL;
        if ( !pWaitObjectRange )
            pWaitObjectRange = pDialog;

        WaitObject aWaitCursor( pWaitObjectRange );
        const_cast< SfxDocumentTemplates* >( aMgr.GetTemplates() )->Update( sal_True /* be smart */ );
            // this const_cast is a hack - but the alternative would be to
            // * have a method which returns the templates non-const
            // * use a new SfxDocumentTemplates instance for the update (knowing that they all share the same
            //   implementation class)
            // * always work with an own instance, even if we get only NULL in this ctor
    }

    aLeftLb.SetHelpId( HID_CTL_ORGANIZER_LEFT );
    aRightLb.SetHelpId( HID_CTL_ORGANIZER_RIGHT );

    String aWorkPath = SvtPathOptions().GetWorkPath();
    if ( aWorkPath.Len() )
    {
        INetURLObject aObj( aWorkPath );
        DBG_ASSERT( aObj.GetProtocol() != INET_PROT_NOT_VALID, "Illegal URL !" );
        aObj.setFinalSlash();
        aLastDir = aObj.GetMainURL( INetURLObject::DECODE_TO_IURI );
    }
    else
    {
        // fallback
        String aProgURL = SvtPathOptions().SubstituteVariable( String::CreateFromAscii("$(PROGURL)") );
        INetURLObject aObj( aProgURL );
        DBG_ASSERT( aObj.GetProtocol() != INET_PROT_NOT_VALID, "Illegal URL !" );
        aLastDir = aObj.GetMainURL( INetURLObject::DECODE_TO_IURI );
    }

    InitBitmaps();

    aEditBtn.GetPopupMenu()->SetSelectHdl( LINK( this, SfxOrganizeDlg_Impl, MenuSelect_Impl ) );
    aEditBtn.GetPopupMenu()->SetActivateHdl( LINK( this, SfxOrganizeDlg_Impl, MenuActivate_Impl ) );
    aEditAcc.SetSelectHdl( LINK( this, SfxOrganizeDlg_Impl, AccelSelect_Impl ) );
    GetpApp()->InsertAccel( &aEditAcc );

    aFilesBtn.SetClickHdl(
        LINK(this,SfxOrganizeDlg_Impl, AddFiles_Impl));
    aAddressTemplateBtn.SetClickHdl(
        LINK(this,SfxOrganizeDlg_Impl, OnAddressTemplateClicked));
    aLeftTypLb.SetSelectHdl(
        LINK(this, SfxOrganizeDlg_Impl, LeftListBoxSelect_Impl));
    aRightTypLb.SetSelectHdl(
        LINK(this, SfxOrganizeDlg_Impl, RightListBoxSelect_Impl));
    aLeftLb.SetGetFocusHdl(
        LINK(this, SfxOrganizeDlg_Impl, GetFocus_Impl));
    aRightLb.SetGetFocusHdl(
        LINK(this, SfxOrganizeDlg_Impl, GetFocus_Impl));
    aLeftLb.SetPosSizePixel(pParent->LogicToPixel(Point(3, 6), MAP_APPFONT),
                            pParent->LogicToPixel(Size(94, 132), MAP_APPFONT));
    aRightLb.SetPosSizePixel(pParent->LogicToPixel(Point(103, 6), MAP_APPFONT),
                             pParent->LogicToPixel(Size(94, 132), MAP_APPFONT));

    if ( !SvtModuleOptions().IsModuleInstalled(SvtModuleOptions::E_SDATABASE) )
        aAddressTemplateBtn.Hide();
    Font aFont(aLeftLb.GetFont());
    aFont.SetWeight(WEIGHT_NORMAL);
    aLeftLb.SetFont(aFont);
    aRightLb.SetFont(aFont);
    const long nIndent = aLeftLb.GetIndent() / 2;
    aLeftLb.SetIndent( (short)nIndent );
    aRightLb.SetIndent( (short)nIndent );

    aLeftLb.SetMgr(&aMgr);
    aRightLb.SetMgr(&aMgr);
    aLeftLb.Reset();
    aRightLb.Reset();//SetModel(aLeftLb.GetModel());

    aLeftLb.Show();
    aRightLb.Show();

    aLeftLb.SelectAll( sal_False );
    aRightLb.SelectAll( sal_False );
    aRightLb.GrabFocus();
}

//-------------------------------------------------------------------------

SfxOrganizeDlg_Impl::~SfxOrganizeDlg_Impl()
{
    delete pFileDlg;
}

//-------------------------------------------------------------------------

void SfxOrganizeDlg_Impl::InitBitmaps( void )
{
    Image   aOpenedFolderBmp( SfxResId( IMG_OPENED_FOLDER ) );
    Image   aClosedFolderBmp( SfxResId( IMG_CLOSED_FOLDER ) );
    Image   aOpenedDocBmp( SfxResId( IMG_OPENED_DOC ) );
    Image   aClosedDocBmp( SfxResId( IMG_CLOSED_DOC ) );

    Image   aOpenedFolderBmpHC( SfxResId( IMG_OPENED_FOLDER_HC ) );
    Image   aClosedFolderBmpHC( SfxResId( IMG_CLOSED_FOLDER_HC ) );
    Image   aOpenedDocBmpHC( SfxResId( IMG_OPENED_DOC_HC ) );
    Image   aClosedDocBmpHC( SfxResId( IMG_CLOSED_DOC_HC ) );

    aLeftLb.SetBitmaps( aOpenedFolderBmp, aClosedFolderBmp, aOpenedDocBmp, aClosedDocBmp,
                        aOpenedFolderBmpHC, aClosedFolderBmpHC, aOpenedDocBmpHC, aClosedDocBmpHC );
    aRightLb.SetBitmaps( aOpenedFolderBmp, aClosedFolderBmp, aOpenedDocBmp, aClosedDocBmp,
                        aOpenedFolderBmpHC, aClosedFolderBmpHC, aOpenedDocBmpHC, aClosedDocBmpHC );
}

//=========================================================================

sal_Bool QueryDelete_Impl(Window *pParent,      // Parent der QueryBox
                             sal_uInt16 nId,            // Resource Id
                             const String &rTemplateName)   // Name der zu l"oschenden Vorlage
/*  [Beschreibung]

    "oschabfrage

*/
{
    SfxResId aResId( nId );
    String aEntryText( aResId );
    aEntryText.SearchAndReplaceAscii( "$1", rTemplateName );
    QueryBox aBox( pParent, WB_YES_NO | WB_DEF_NO, aEntryText );
    return RET_NO != aBox.Execute();
}

//-------------------------------------------------------------------------

void ErrorDelete_Impl(Window *pParent, const String &rName, sal_Bool bFolder = sal_False )

/*  [Beschreibung]

    Benutzerinformation, da"s die Vorlage rName nicht gel"oscht werden konnte

*/
{
    if ( bFolder )
    {
        String aText( SfxResId( STR_ERROR_DELETE_TEMPLATE_DIR ) );
        ErrorBox( pParent, WB_OK, aText ).Execute();
    }
    else
    {
        String aText( SfxResId( STR_ERROR_DELETE_TEMPLATE ) );
        aText.SearchAndReplaceAscii( "$1", rName );
        ErrorBox( pParent, WB_OK, aText ).Execute();
    }
}


//=========================================================================

/*  [Beschreibung]

    Implementierungsklasse; Referenzklasse f"ur USHORT-Array

*/

struct ImpPath_Impl
{
    SvUShorts   aUS;
    sal_uInt16      nRef;

    ImpPath_Impl();
    ImpPath_Impl( const ImpPath_Impl& rCopy );
};

//-------------------------------------------------------------------------

ImpPath_Impl::ImpPath_Impl() : aUS(5), nRef(1)
{
}

//-------------------------------------------------------------------------

ImpPath_Impl::ImpPath_Impl( const ImpPath_Impl& rCopy ) :

    aUS ( (sal_uInt8)rCopy.aUS.Count() ),
    nRef( 1 )

{
    const sal_uInt16 nCount = rCopy.aUS.Count();

    for ( sal_uInt16 i = 0; i < nCount; ++i )
        aUS.Insert( rCopy.aUS[i], i );
}

//==========================================================================

/*  [Beschreibung]

    Implementierungsklasse; Darstellung einer Position in der Outline-
    Listbox als sal_uInt16-Array; dieses beschreibt die Position jeweil
    als relative Postion zum "ubergeordneten Eintrag

*/
class Path
{
    ImpPath_Impl *pData;
    void NewImp();
public:
    Path(SvLBox *pBox, SvLBoxEntry *pEntry);
    Path(const Path &rPath):
        pData(rPath.pData)
    {
        ++pData->nRef;
    }
    const Path &operator=(const Path &rPath)
    {
        if(&rPath != this)
        {
            if(!--pData->nRef)
                delete pData;
            pData = rPath.pData;
            pData->nRef++;
        }
        return *this;
    }
    ~Path()
    {
        if(!--pData->nRef)
            delete pData;
    }
    sal_uInt16 Count() const { return pData->aUS.Count(); }
    sal_uInt16 operator[]( sal_uInt16 i ) const
    {
        return i < Count()? pData->aUS[i]: INDEX_IGNORE;
    }
};

//-------------------------------------------------------------------------

Path::Path(SvLBox *pBox, SvLBoxEntry *pEntry) :
    pData(new ImpPath_Impl)
{
    DBG_ASSERT(pEntry != 0, "EntryPtr ist NULL");
    if(!pEntry)
        return;
    SvLBoxEntry *pParent = pBox->GetParent(pEntry);
    do {
        pData->aUS.Insert((sal_uInt16)pBox->GetModel()->GetRelPos(pEntry), 0);
        if(0 == pParent)
            break;
        pEntry = pParent;
        pParent = pBox->GetParent(pEntry);
    } while(1);
}

//-------------------------------------------------------------------------

void Path::NewImp()
{
    if(pData->nRef != 1)
    {
        pData->nRef--;
        pData = new ImpPath_Impl(*pData);
    }
}

//-------------------------------------------------------------------------

SvLBoxEntry *GetIndices_Impl(SvLBox *pBox,
                               SvLBoxEntry *pEntry,
                               sal_uInt16 &rRegion,
                               sal_uInt16 &rOffset)
/*  [Beschreibung]

    Bereich und Position innerhalb eines Bereiches f"ur eine
    Dokumentvorlage wird ermittelt.

    [Parameter]

    SvLBox *pBox            Listbox, an der das Ereignis auftrat
    SvLBoxEntry *pEntry     Eintrag, dessen Position ermittelt werden soll
    sal_uInt16 &rRegion         der Bereich innerhalb der Bereiche der
                            Dokumentvorlagen (Out-Parameter)
    sal_uInt16 &rOffset         die Position innerhalb des Bereiches
                            Dokumentvorlagen (Out-Parameter)

    [Querverweise]

    <class Path>    (unter Umst"anden kann auf diese Funktion zugunsten
                     von Path verzichtet werden.)

*/

{
    if(!pEntry)
    {
        rRegion = rOffset = 0;
        return pEntry;
    }
    if(0 == pBox->GetModel()->GetDepth(pEntry))
    {
        rRegion = (sal_uInt16)pBox->GetModel()->GetRelPos(pEntry);
        rOffset = USHRT_MAX;
        return pEntry;
    }
    SvLBoxEntry *pParent = pBox->GetParent(pEntry);
    rRegion = (sal_uInt16)pBox->GetModel()->GetRelPos(pParent);
    rOffset = (sal_uInt16)pBox->GetModel()->GetRelPos(pEntry);
    return pEntry;
}

//-------------------------------------------------------------------------

sal_Bool SfxOrganizeListBox_Impl::Select( SvLBoxEntry* pEntry, sal_Bool bSelect )
{
    if(!bSelect)
        return SvTreeListBox::Select(pEntry,bSelect);
    sal_uInt16 nLevel = GetDocLevel();
    if(GetModel()->GetDepth(pEntry)+nLevel<3)
        return SvTreeListBox::Select(pEntry,bSelect);

    Path aPath(this, pEntry);

    // it is ok to use the SfxObjectShellRef here since the object that
    // provides it ( GetObjectShell() calls CreateObjectShell() ) has a lock on it
    GetObjectShell(aPath)->TriggerHelpPI(
        aPath[nLevel+1], aPath[nLevel+2], aPath[nLevel+3]);
    return SvTreeListBox::Select(pEntry,bSelect);
}

//-------------------------------------------------------------------------

sal_Bool SfxOrganizeListBox_Impl::MoveOrCopyTemplates(SvLBox *pSourceBox,
                                            SvLBoxEntry *pSource,
                                            SvLBoxEntry* pTarget,
                                            SvLBoxEntry *&pNewParent,
                                            sal_uIntPtr &rIdx,
                                            sal_Bool bCopy)
/*  [Beschreibung]

    Verschieben oder Kopieren von Dokumentvorlagen

    [Parameter]

    SvLBox *pSourceBox          Quell-Listbox, an der das Ereignis auftrat
    SvLBoxEntry *pSource        Quell-Eintrag, der kopiert / verschoben werden soll
    SvLBoxEntry* pTarget        Ziel-Eintrag, auf den verschoben werden soll
    SvLBoxEntry *&pNewParent    der Parent der an der Zielposition erzeugten
                                Eintrags (Out-Parameter)
    sal_uIntPtr &rIdx                 Index des Zieleintrags
    sal_Bool bCopy                  Flag f"ur Kopieren / Verschieben


    [Returnwert]                sal_Bool: Erfolg oder Mi"serfolg

    [Querverweise]

    <SfxOrganizeListBox_Impl::MoveOrCopyContents(SvLBox *pSourceBox,
                                            SvLBoxEntry *pSource,
                                            SvLBoxEntry* pTarget,
                                            SvLBoxEntry *&pNewParent,
                                            sal_uIntPtr &rIdx,
                                            sal_Bool bCopy)>
    <sal_Bool SfxOrganizeListBox_Impl::NotifyMoving(SvLBoxEntry *pTarget,
                                            SvLBoxEntry* pSource,
                                            SvLBoxEntry *&pNewParent,
                                            sal_uIntPtr &rIdx)>
    <sal_Bool SfxOrganizeListBox_Impl::NotifyCopying(SvLBoxEntry *pTarget,
                                            SvLBoxEntry* pSource,
                                            SvLBoxEntry *&pNewParent,
                                            sal_uIntPtr &rIdx)>
*/

{
    sal_Bool bOk = sal_False;

    if(pSource)
    {
        sal_uInt16 nTargetRegion = 0, nTargetIndex = 0;
        GetIndices_Impl(this, pTarget, nTargetRegion, nTargetIndex);

        sal_uInt16 nSourceRegion = 0, nSourceIndex = 0;
        GetIndices_Impl(pSourceBox, pSource, nSourceRegion, nSourceIndex);

        bOk =  bCopy ?
            pMgr->Copy(nTargetRegion, nTargetIndex+1,
                       nSourceRegion, nSourceIndex):
            pMgr->Move(nTargetRegion, nTargetIndex+1,
                       nSourceRegion, nSourceIndex);

        if(bOk)
        {
            if(pSourceBox->GetModel()->GetDepth(pSource) == GetModel()->GetDepth(pTarget))
            {
                pNewParent = GetParent(pTarget);
                rIdx = GetModel()->GetRelPos(pTarget)+1;
            }
            else
            {
                if(nTargetIndex == USHRT_MAX)
                {
                    pNewParent = pTarget;
                    rIdx = 0;
                }
                else
                    SvLBox::NotifyCopying(
                        pTarget, pSource, pNewParent, rIdx);
            }
        }
        else if ( bCopy )
        {
            // the template organizer always tries copy after the move, so no error is required for move case
            String aText( SfxResId( bCopy ? STR_ERROR_COPY_TEMPLATE : STR_ERROR_MOVE_TEMPLATE ) );
            aText.SearchAndReplaceAscii( "$1",
                                         ( (SvTreeListBox *)pSourceBox )->GetEntryText( pSource ) );
            ErrorBox( this, WB_OK, aText ).Execute();
        }
    }
    return bOk;
}

//-------------------------------------------------------------------------

sal_Bool SfxOrganizeListBox_Impl::MoveOrCopyContents(SvLBox *pSourceBox,
                                            SvLBoxEntry *pSource,
                                            SvLBoxEntry* pTarget,
                                            SvLBoxEntry *&pNewParent,
                                            sal_uIntPtr &rIdx,
                                            sal_Bool bCopy)
/*  [Beschreibung]

    Verschieben oder Kopieren von Dokumentinhalten

    [Parameter]

    SvLBox *pSourceBox          Quell-Listbox, an der das Ereignis auftrat
    SvLBoxEntry *pSource        Quell-Eintrag, der kopiert / verschoben werden soll
    SvLBoxEntry* pTarget        Ziel-Eintrag, auf den verschoben werden soll
    SvLBoxEntry *&pNewParent    der Parent der an der Zielposition erzeugten
                                Eintrags (Out-Parameter)
    sal_uIntPtr &rIdx                 Index des Zieleintrags
    sal_Bool bCopy                  Flag f"ur Kopieren / Verschieben


    [Returnwert]                sal_Bool: Erfolg oder Mi"serfolg

    [Querverweise]

    <SfxOrganizeListBox_Impl::MoveOrCopyTemplates(SvLBox *pSourceBox,
                                            SvLBoxEntry *pSource,
                                            SvLBoxEntry* pTarget,
                                            SvLBoxEntry *&pNewParent,
                                            sal_uIntPtr &rIdx,
                                            sal_Bool bCopy)>
    <sal_Bool SfxOrganizeListBox_Impl::NotifyMoving(SvLBoxEntry *pTarget,
                                            SvLBoxEntry* pSource,
                                            SvLBoxEntry *&pNewParent,
                                            sal_uIntPtr &rIdx)>
    <sal_Bool SfxOrganizeListBox_Impl::NotifyCopying(SvLBoxEntry *pTarget,
                                            SvLBoxEntry* pSource,
                                            SvLBoxEntry *&pNewParent,
                                            sal_uIntPtr &rIdx)>
*/

{
    SfxErrorContext aEc( ERRCTX_SFX_MOVEORCOPYCONTENTS, this);
    sal_Bool bOk = sal_False, bKeepExpansion = sal_False;
    sal_Bool bRemovedFromSource = sal_False;
    Path aSource(pSourceBox, pSource);
    Path aTarget(this, pTarget);

    // it is ok to use the SfxObjectShellRef here since the object that
    // provides it ( GetObjectShell() calls CreateObjectShell() ) has a lock on it
    SfxObjectShellRef aSourceDoc = ((SfxOrganizeListBox_Impl *)pSourceBox)->GetObjectShell(aSource);
    SfxObjectShellRef aTargetDoc = GetObjectShell(aTarget);

    const sal_uInt16 nSLevel =
        ((SfxOrganizeListBox_Impl *)pSourceBox)->GetDocLevel();
    const sal_uInt16 nTLevel = GetDocLevel();

    if(aSourceDoc.Is() && aTargetDoc.Is())
    {
        if (aSourceDoc->GetStyleSheetPool())
            aSourceDoc->GetStyleSheetPool()->SetSearchMask(
                SFX_STYLE_FAMILY_ALL, SFXSTYLEBIT_USERDEF | SFXSTYLEBIT_USED);

        if (aTargetDoc->GetStyleSheetPool())
            aTargetDoc->GetStyleSheetPool()->SetSearchMask(
                SFX_STYLE_FAMILY_ALL, SFXSTYLEBIT_USERDEF | SFXSTYLEBIT_USED);
        sal_uInt16 p[3];
        sal_uInt16 nIdxDeleted = INDEX_IGNORE;
        p[0]=aTarget[nTLevel+1];
        p[1]=aTarget[nTLevel+2];
        if(p[1]!=INDEX_IGNORE)p[1]++;
        p[2]=aTarget[nTLevel+3];

        bOk = aTargetDoc->Insert(
            *aSourceDoc, aSource[nSLevel+1],
            aSource[nSLevel+2], aSource[nSLevel+3],
            p[0], p[1], p[2],  nIdxDeleted);
        // Positionskorrektur auswerten
        // a = Dokumentinhalt
        // b = Position Sub-Inhalt 1
        // c = Position Sub-Inhalt 2
        // doppelte Eintraege loeschen
        if(bOk)
        {
            SvLBoxEntry *pParentIter = pTarget;
            // bis auf die DokumentEbene nach oben als
            // allgemeiner Bezugspunkt
            while(GetModel()->GetDepth(pParentIter) != nTLevel)
                pParentIter = GetParent(pParentIter);
            if(pParentIter->HasChildsOnDemand() &&
                !GetModel()->HasChilds(pParentIter))
                RequestingChilds(pParentIter);
            SvLBoxEntry *pChildIter = 0;

            sal_uInt16 i = 0;
            while(i < 2 && p[i+1] != INDEX_IGNORE)
            {
                pChildIter = FirstChild(pParentIter);
                // bis zum Index der aktuellen Ebene
                for(sal_uInt16 j = 0; j < p[i]; ++j)
                    pChildIter = NextSibling(pChildIter);
                // gfs Fuellen bei Items onDemand
                ++i;
                if(p[i+1] != INDEX_IGNORE &&
                   pChildIter->HasChildsOnDemand() &&
                   !GetModel()->HasChilds(pChildIter))
                    RequestingChilds(pChildIter);
                pParentIter = pChildIter;
            }
            rIdx = p[i];
            pNewParent = pParentIter;
            if(!IsExpanded(pNewParent) &&
               pNewParent->HasChildsOnDemand() &&
               !GetModel()->HasChilds(pNewParent))
            {
                bOk = sal_False;
                if(!bCopy)
                    pSourceBox->GetModel()->Remove(pSource);
            }
            // Geloeschte Eintraege entfernen
            // (kann durch Ueberschreiben geschehen)
            if(nIdxDeleted != INDEX_IGNORE)
            {
                pChildIter = FirstChild(pParentIter);
                for(sal_uInt16 j = 0; j < nIdxDeleted; ++j)
                    pChildIter = NextSibling(pChildIter);
                if( pChildIter && pChildIter != pSource )
                {
                    bKeepExpansion = IsExpanded(pParentIter);
                    GetModel()->Remove(pChildIter);
                }
                else
                    bOk = sal_False;
            }
            if(!bCopy && &aSourceDoc != &aTargetDoc)
            {
                //#109566# pool styles that are moved produce
                //an rIdx == INDEX_IGNORE
                //the method has to return true to keep the box content consistent
                bRemovedFromSource = aSourceDoc->Remove(aSource[nSLevel+1],
                                     aSource[nSLevel+2],
                                     aSource[nSLevel+3]);
            }
        }
    }
//  rIdx++;
    return (((rIdx != INDEX_IGNORE)|| bRemovedFromSource) && bOk )
        ? bKeepExpansion? (sal_Bool)2: sal_True: sal_False;
}

//-------------------------------------------------------------------------

sal_Bool SfxOrganizeListBox_Impl::NotifyMoving(SvLBoxEntry *pTarget,
                                        SvLBoxEntry* pSource,
                                        SvLBoxEntry *&pNewParent,
                                        sal_uIntPtr &rIdx)

/*  [Beschreibung]

    Benachrichtigung, da"s ein Eintrag verschoben werden soll
    (SV-Handler)

    [Parameter]

    SvLBoxEntry* pTarget        Ziel-Eintrag, auf den verschoben werden soll
    SvLBoxEntry *pSource        Quell-Eintrag, der verschoben werden soll
    SvLBoxEntry *&pNewParent    der Parent der an der Zielposition erzeugten
                                Eintrags (Out-Parameter)
    sal_uIntPtr &rIdx                 Index des Zieleintrags


    [Returnwert]                sal_Bool: Erfolg oder Mi"serfolg

    [Querverweise]

    <SfxOrganizeListBox_Impl::MoveOrCopyTemplates(SvLBox *pSourceBox,
                                            SvLBoxEntry *pSource,
                                            SvLBoxEntry* pTarget,
                                            SvLBoxEntry *&pNewParent,
                                            sal_uIntPtr &rIdx,
                                            sal_Bool bCopy)>
    <SfxOrganizeListBox_Impl::MoveOrCopyContents(SvLBox *pSourceBox,
                                            SvLBoxEntry *pSource,
                                            SvLBoxEntry* pTarget,
                                            SvLBoxEntry *&pNewParent,
                                            sal_uIntPtr &rIdx,
                                            sal_Bool bCopy)>
    <sal_Bool SfxOrganizeListBox_Impl::NotifyCopying(SvLBoxEntry *pTarget,
                                            SvLBoxEntry* pSource,
                                            SvLBoxEntry *&pNewParent,
                                            sal_uIntPtr &rIdx)>
*/

{
    sal_Bool bOk =  sal_False;
    SvLBox* pSourceBox = GetSourceView();
    if ( !pSourceBox )
        pSourceBox = pDlg->pSourceView;
    DBG_ASSERT( pSourceBox, "no source view" );
    if ( !pTarget )
        pTarget = pDlg->pTargetEntry;

    if ( pSourceBox->GetModel()->GetDepth( pSource ) <= GetDocLevel() &&
                      GetModel()->GetDepth( pTarget ) <= GetDocLevel() )
        bOk = MoveOrCopyTemplates( pSourceBox, pSource, pTarget, pNewParent, rIdx, sal_False );
    else
        bOk = MoveOrCopyContents(pSourceBox, pSource, pTarget, pNewParent, rIdx, sal_False );

    return bOk;
}

//-------------------------------------------------------------------------

sal_Bool SfxOrganizeListBox_Impl::NotifyCopying(SvLBoxEntry *pTarget,
                                        SvLBoxEntry* pSource,
                                        SvLBoxEntry *&pNewParent,
                                        sal_uIntPtr &rIdx)
/*  [Beschreibung]

    Benachrichtigung, da"s ein Eintrag kopiert werden soll
    (SV-Handler)

    [Parameter]

    SvLBoxEntry* pTarget        Ziel-Eintrag, auf den kopiert werden soll
    SvLBoxEntry *pSource        Quell-Eintrag, der kopiert werden soll
    SvLBoxEntry *&pNewParent    der Parent der an der Zielposition erzeugten
                                Eintrags (Out-Parameter)
    sal_uIntPtr &rIdx                 Index des Zieleintrags


    [Returnwert]                sal_Bool: Erfolg oder Mi"serfolg

    [Querverweise]

    <SfxOrganizeListBox_Impl::MoveOrCopyTemplates(SvLBox *pSourceBox,
                                            SvLBoxEntry *pSource,
                                            SvLBoxEntry* pTarget,
                                            SvLBoxEntry *&pNewParent,
                                            sal_uIntPtr &rIdx,
                                            sal_Bool bCopy)>
    <SfxOrganizeListBox_Impl::MoveOrCopyContents(SvLBox *pSourceBox,
                                            SvLBoxEntry *pSource,
                                            SvLBoxEntry* pTarget,
                                            SvLBoxEntry *&pNewParent,
                                            sal_uIntPtr &rIdx,
                                            sal_Bool bCopy)>
    <sal_Bool SfxOrganizeListBox_Impl::NotifyMoving(SvLBoxEntry *pTarget,
                                            SvLBoxEntry* pSource,
                                            SvLBoxEntry *&pNewParent,
                                            sal_uIntPtr &rIdx)>
*/
{
    sal_Bool bOk =  sal_False;
    SvLBox* pSourceBox = GetSourceView();
    if ( !pSourceBox )
        pSourceBox = pDlg->pSourceView;
    DBG_ASSERT( pSourceBox, "no source view" );
    if ( !pTarget )
        pTarget = pDlg->pTargetEntry;
    if ( pSourceBox->GetModel()->GetDepth( pSource ) <= GetDocLevel() &&
                     GetModel()->GetDepth( pTarget ) <= GetDocLevel() )
        bOk = MoveOrCopyTemplates( pSourceBox, pSource, pTarget, pNewParent, rIdx, sal_True );
    else
        bOk = MoveOrCopyContents( pSourceBox, pSource, pTarget, pNewParent, rIdx, sal_True );

    return bOk;
}

//-------------------------------------------------------------------------

sal_Bool SfxOrganizeListBox_Impl::EditingEntry( SvLBoxEntry* pEntry, Selection&  )

/*  [Beschreibung]

    Nachfrage, ob ein Eintrag editierbar ist
    (SV-Handler)

    [Querverweise]
    <SfxOrganizeListBox_Impl::EditedEntry(SvLBoxEntry* pEntry, const String& rText)>
*/

{
    if( VIEW_TEMPLATES == eViewType &&
        GetModel()->GetDepth(pEntry) < 2 )
    {
        pDlg->pSuspend = new SuspendAccel( &pDlg->aEditAcc );
        return sal_True;
    }
    return sal_False;
}

//-------------------------------------------------------------------------

sal_Bool SfxOrganizeListBox_Impl::EditedEntry(SvLBoxEntry* pEntry, const String& rText)

/*  [Beschreibung]

    Der Name eines Eintrags wurde bearbeitet; ist der eingegebene Name
    ein g"ultiger Name ("ange > 0), wird das Model aktualisiert.
    (SV-Handler)

    [Returnwert]

    sal_Bool                sal_True: der Name soll in der Anzeige ge"andert werden
                            sal_False:der Name soll nicht ge"andert werden

    [Querverweise]
    <SfxOrganizeListBox_Impl::EditingEntry(SvLBoxEntry* pEntry, const String& rText)>
*/

{
    DBG_ASSERT(pEntry, "kein Entry selektiert");
    delete pDlg->pSuspend;
    pDlg->pSuspend = NULL;
    SvLBoxEntry* pParent = GetParent(pEntry);
    if( !rText.Len() )
    {
        ErrorBox aBox( this, SfxResId( MSG_ERROR_EMPTY_NAME ) );
        aBox.GrabFocus();
        aBox.Execute();
        return sal_False;
    }
    if ( !IsUniqName_Impl( rText, pParent, pEntry ) )
    {
        ErrorBox aBox( this, SfxResId( MSG_ERROR_UNIQ_NAME ) );
        aBox.GrabFocus();
        aBox.Execute();
        return sal_False;
    }
    sal_uInt16 nRegion = 0, nIndex = 0;
    GetIndices_Impl( this, pEntry, nRegion, nIndex );
    String aOldName;
    if ( USHRT_MAX != nIndex )
        aOldName = pMgr->GetTemplates()->GetName( nRegion, nIndex );
    else
        aOldName = pMgr->GetTemplates()->GetRegionName( nRegion );

    if ( !pMgr->SetName( rText, nRegion, nIndex ) )
    {
        SfxResId aResId( USHRT_MAX != nIndex ? MSG_ERROR_RENAME_TEMPLATE
                                             : MSG_ERROR_RENAME_TEMPLATE_REGION );
        ErrorBox( this, aResId ).Execute();
        return sal_False;
    }
/*
    else
    {
        SfxTemplateOrganizeDlg* pDlg = (SfxTemplateOrganizeDlg*)Window::GetParent();
    }
*/
    return sal_True;
}

//-------------------------------------------------------------------------

DragDropMode SfxOrganizeListBox_Impl::NotifyStartDrag( TransferDataContainer&, SvLBoxEntry* pEntry )
{
    sal_uInt16 nSourceLevel = GetModel()->GetDepth( pEntry );
    if ( VIEW_FILES == GetViewType() )
        ++nSourceLevel;
    if ( nSourceLevel >= 2 )
        bDropMoveOk = sal_False;
    else
        bDropMoveOk = sal_True;

    return GetDragDropMode();
}

//-------------------------------------------------------------------------

sal_Bool SfxOrganizeListBox_Impl::NotifyAcceptDrop( SvLBoxEntry* pEntry )
{
    if(!pEntry)
        return sal_False;
    SvLBox *pSource = GetSourceView();
    SvLBoxEntry *pSourceEntry = pSource->FirstSelected();
    if(pEntry == pSourceEntry)
        return sal_False;
    sal_uInt16 nSourceLevel = pSource->GetModel()->GetDepth(pSourceEntry);
    if(VIEW_FILES == ((SfxOrganizeListBox_Impl *)pSource)->GetViewType())
        ++nSourceLevel;
    sal_uInt16 nTargetLevel = GetModel()->GetDepth(pEntry);
    if(VIEW_FILES == GetViewType())
        ++nTargetLevel;
    Path aSource(pSource, pSourceEntry);
    Path aTarget(this, pEntry);
    const sal_uInt16 SL = ((SfxOrganizeListBox_Impl *)pSource)->GetDocLevel();
    const sal_uInt16 TL = GetDocLevel();

    return( (nSourceLevel == 1 && nTargetLevel == 0 &&
            VIEW_TEMPLATES ==
            ((SfxOrganizeListBox_Impl *)pSource)->GetViewType()) ||
           (nSourceLevel == 1 && nTargetLevel == 1 &&
            VIEW_TEMPLATES ==
            ((SfxOrganizeListBox_Impl *)pSource)->GetViewType() &&
            VIEW_TEMPLATES == GetViewType()) ||
           (nSourceLevel == 3 && nTargetLevel == 1) ||
           (nSourceLevel == 3 && nTargetLevel == 2 &&
            aSource[1+SL] == aTarget[1+TL]) ||
           (nSourceLevel == 3 && nTargetLevel == 3 &&
            aSource[1+SL] == aTarget[1+TL]) ||
           (nSourceLevel == 4 && nTargetLevel == 3 &&
            aSource[1+SL] == aTarget[1+TL] &&
            aSource[2+SL] == aTarget[2+TL]) ||
           (nSourceLevel == 4 && nTargetLevel == 4 &&
            aSource[1+SL] == aTarget[1+TL] &&
            aSource[2+SL] == aTarget[2+TL]));
}

//-------------------------------------------------------------------------

sal_Int8 SfxOrganizeListBox_Impl::AcceptDrop( const AcceptDropEvent& rEvt )
{
    sal_Bool bAccept = ( eViewType == VIEW_FILES && IsDropFormatSupported( SOT_FORMAT_FILE ) );
    if ( bAccept )
        return rEvt.mnAction;
    else
        return SvTreeListBox::AcceptDrop( rEvt );
}

//-------------------------------------------------------------------------

sal_Int8 SfxOrganizeListBox_Impl::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    TransferableDataHelper aHelper( rEvt.maDropEvent.Transferable );
    sal_uInt32 nFormatCount = aHelper.GetFormatCount();
    sal_Bool bSuccess = sal_False;
    for ( sal_uInt32 i = 0; i < nFormatCount; ++i )
    {
        String aFileName;
        SotFormatStringId nId = aHelper.GetFormat(i);

        if ( SOT_FORMAT_FILE == nId && aHelper.GetString( nId, aFileName ) )
        {
            INetURLObject aObj( aFileName, INET_PROT_FILE );
            bSuccess |= pMgr->InsertFile( this, aObj.GetMainURL(INetURLObject::DECODE_TO_IURI) );
        }
    }
    bDropMoveOk = sal_True;
    sal_Int8 nRet = rEvt.mnAction;
    if ( !bSuccess )
    {
        // asynchronous, because of MessBoxes
        pDlg->pSourceView = GetSourceView();
        pDlg->pTargetEntry = pTargetEntry;
        pDlg->pFinishedBox = NULL;
        pDlg->nDropAction = NO_DROP_ACTION;
        PostUserEvent( LINK( this, SfxOrganizeListBox_Impl, OnAsyncExecuteDrop ),
                       new ExecuteDropEvent( rEvt ) );
    }

    return nRet;
}

//-------------------------------------------------------------------------

void SfxOrganizeListBox_Impl::DragFinished( sal_Int8 nDropAction )
{
    if ( pDlg->bExecDropFinished )
    {
        if ( pDlg->nDropAction != NO_DROP_ACTION )
            nDropAction = pDlg->nDropAction;
        SvTreeListBox::DragFinished( nDropAction );
        pDlg->nDropAction = NO_DROP_ACTION;
    }
    else
        pDlg->pFinishedBox = this;
}

//-------------------------------------------------------------------------

inline sal_uInt16 SfxOrganizeListBox_Impl::GetDocLevel() const

/*  [Beschreibung]

    Ermittelt, auf welche Ebene sich Dokumente befinden (unterschiedlich
    in der Dokumentvorlagensicht und der Dokumentensicht)

    [Returnwert]

    sal_uInt16              Die Ebene der Dokumente

*/

{
    return eViewType == VIEW_FILES? 0: 1;
}

//-------------------------------------------------------------------------

SfxObjectShellRef SfxOrganizeListBox_Impl::GetObjectShell(const Path &rPath)

/*  [Beschreibung]

    Zugriff auf die ObjectShell, die dem aktuellen Eintrag zugeordnet
    ist.

    [Parameter]

    const Path &rPath       Beschreibung des aktuellen Eintrags

    [Returnwert]

    SfxObjectShellRef     Referenz auf die ObjectShell

    [Querverweise]

    <class Path>

*/

{
    SfxObjectShellRef aDoc;
    if(eViewType == VIEW_FILES)
        aDoc = pMgr->CreateObjectShell(rPath[0]);
    else
        aDoc = pMgr->CreateObjectShell(rPath[0], rPath[1]);
    return aDoc;
}

//-------------------------------------------------------------------------

void SfxOrganizeListBox_Impl::RequestingChilds( SvLBoxEntry* pEntry )

/*  [Beschreibung]

    Aufforderung, der Childs eines Eintrags einzuf"ugen
    ist.
    (SV-Handler)

    [Parameter]

    SvLBoxEntry* pEntry     der Eintrag, dessen Childs erfragt werden


*/

{
    // wenn keine Childs vorhanden sind, gfs. Childs
    // einfuegen
    BmpColorMode eColorMode = BMP_COLOR_NORMAL;

    if ( GetSettings().GetStyleSettings().GetHighContrastMode() )
        eColorMode = BMP_COLOR_HIGHCONTRAST;


    if ( !GetModel()->HasChilds( pEntry ) )
    {
        WaitObject aWaitCursor( this );

        // Choose the correct mask color dependent from eColorMode. This must be adopted if
        // we change the mask color for normal images, too!
        Color aMaskColor( COL_LIGHTMAGENTA );

        // hier sind alle initial eingefuegt
        SfxErrorContext aEc(ERRCTX_SFX_CREATEOBJSH, pDlg->pDialog);
        if(VIEW_TEMPLATES == GetViewType() && 0 == GetModel()->GetDepth(pEntry))
        {
            sal_uInt16 i = (sal_uInt16)GetModel()->GetRelPos(pEntry);
            const sal_uInt16 nEntryCount = pMgr->GetTemplates()->GetCount(i);
            for(sal_uInt16 j = 0; j < nEntryCount; ++j)
                InsertEntryByBmpType( pMgr->GetTemplates()->GetName( i, j ), BMPTYPE_DOC, pEntry, sal_True );
        }
        else
        {
            const sal_uInt16 nDocLevel = GetDocLevel();
            Path aPath(this, pEntry);

            // it is ok to use the SfxObjectShellRef here since the object that
            // provides it ( GetObjectShell() calls CreateObjectShell() ) has a lock on it
            SfxObjectShellRef aRef = GetObjectShell(aPath);
            if(aRef.Is())
            {
                const sal_uInt16 nCount = aRef->GetContentCount(
                    aPath[nDocLevel+1], aPath[nDocLevel+2]);
                String aText;
                Bitmap aClosedBmp, aOpenedBmp;
                const sal_Bool bCanHaveChilds =
                    aRef->CanHaveChilds(aPath[nDocLevel+1],
                                        aPath[nDocLevel+2]);
                for(sal_uInt16 i = 0; i < nCount; ++i)
                {
                    sal_Bool bDeletable;
                    aRef->GetContent(
                        aText, aClosedBmp, aOpenedBmp, eColorMode, bDeletable,
                        i, aPath[nDocLevel+1], aPath[nDocLevel+2]);

                    // Create image with the correct mask color
                    Image aClosedImage( aClosedBmp, aMaskColor );
                    Image aOpenedImage( aOpenedBmp, aMaskColor );

                    SvLBoxEntry *pNew = SvTreeListBox::InsertEntry(
                        aText, aOpenedImage, aClosedImage,
                        pEntry, bCanHaveChilds);
                    pNew->SetUserData(bDeletable ? &bDeletable : 0);
                }
            }
        }
    }
}

//-------------------------------------------------------------------------

long SfxOrganizeListBox_Impl::ExpandingHdl()

/*  [Beschreibung]

    SV-Handler, der nach dem und vor dem Aufklappen eines Eintrags
    gerufen wird.
    Wird verwendet, um gfs. die ObjectShell wieder zu schlie"sen;
    die Eintr"age mit den Inhalten dieser Shell werden ebenfalls
    entfernt.

*/

{
    if ( !(nImpFlags & SVLBOX_IS_EXPANDING) )
    {
        SvLBoxEntry* pEntry  = GetHdlEntry();
        const sal_uInt16 nLevel = GetModel()->GetDepth(pEntry);
        if((eViewType == VIEW_FILES && nLevel == 0) ||
           (eViewType == VIEW_TEMPLATES && nLevel == 1))
        {
            Path aPath(this, pEntry);
            // Beim Schliessen des Files die ObjectShell freigeben
            if(eViewType == VIEW_FILES && nLevel == 0)
                pMgr->DeleteObjectShell(aPath[0]);
            else
                pMgr->DeleteObjectShell(aPath[0], aPath[1]);
            // alle SubEntries loeschen
            SvLBoxEntry *pToDel = SvLBox::GetEntry(pEntry, 0);
            while(pToDel)
            {
                GetModel()->Remove(pToDel);
                pToDel = SvLBox::GetEntry(pEntry, 0);
            }
        }
    }
    return sal_True;
}

//-------------------------------------------------------------------------

sal_Bool SfxOrganizeListBox_Impl::IsUniqName_Impl(const String &rText,
                                         SvLBoxEntry* pParent, SvLBoxEntry *pEntry) const

/*  [Beschreibung]

    Pr"uft, ob eine Name auf seiner Ebene eindeutig ist.

    [Parameter]

    const String &         Name des zu suchenden Eintrags
    SvLBoxEntry* pSibling  Geschwister (bezeichnet die Ebene)

    [Returnwert]

    sal_Bool                     sal_True, wenn der Name eindeutig ist, sonst sal_False
*/

{
    SvLBoxEntry* pChild = FirstChild(pParent);
    while(pChild)  {
        const String aEntryText(GetEntryText(pChild));
        if(COMPARE_EQUAL == aEntryText.CompareIgnoreCaseToAscii(rText)&&(!pEntry || pEntry!=pChild))
            return sal_False;
        pChild = NextSibling(pChild);
    }
    return sal_True;
}

//-------------------------------------------------------------------------

sal_uInt16 SfxOrganizeListBox_Impl::GetLevelCount_Impl(SvLBoxEntry* pParent) const
{
    SvLBoxEntry* pChild = FirstChild(pParent);
    sal_uInt16 nCount = 0;
    while(pChild)  {
        pChild = NextSibling(pChild);
        ++nCount;
    }
    return nCount;
}

//-------------------------------------------------------------------------

SvLBoxEntry* SfxOrganizeListBox_Impl::InsertEntryByBmpType( const XubString& rText, BMPTYPE eBmpType,
    SvLBoxEntry* pParent, sal_Bool bChildsOnDemand, sal_uIntPtr nPos, void* pUserData )
{
    SvLBoxEntry*    pEntry = NULL;
    const Image*    pExp = NULL;
    const Image*    pCol = NULL;
    const Image*    pExpHC = NULL;
    const Image*    pColHC = NULL;

    switch( eBmpType )
    {
        case BMPTYPE_FOLDER:
            pExp = &aOpenedFolderBmp;
            pCol = &aClosedFolderBmp;
            pExpHC = &aOpenedFolderBmpHC;
            pColHC = &aClosedFolderBmpHC;
            break;
        default:
            DBG_ERROR( "SfxOrganizeListBox_Impl::InsertEntryByBmpType(): something forgotten?!" );

        case BMPTYPE_DOC:
            pExp = &aOpenedDocBmp;
            pCol = &aClosedDocBmp;
            pExpHC = &aOpenedDocBmpHC;
            pColHC = &aClosedDocBmpHC;
            break;
    }

    pEntry = SvTreeListBox::InsertEntry( rText, *pExp, *pCol, pParent, bChildsOnDemand, nPos, pUserData );

    SetExpandedEntryBmp( pEntry, *pExpHC, BMP_COLOR_HIGHCONTRAST );
    SetCollapsedEntryBmp( pEntry, *pColHC, BMP_COLOR_HIGHCONTRAST );

    return pEntry;
}

//-------------------------------------------------------------------------

SfxOrganizeListBox_Impl::SfxOrganizeListBox_Impl
(
    SfxOrganizeDlg_Impl* pArgDlg,
    Window* pParent,
    WinBits nBits,
    DataEnum eType
) :

    SvTreeListBox( pParent, nBits ),

    pMgr        ( NULL ),
    pDlg        ( pArgDlg ),
    eViewType   ( eType )

/*  [Beschreibung]

    Konstruktor SfxOrganizeListBox

*/

{
    SetDragDropMode(
        SV_DRAGDROP_CTRL_MOVE | SV_DRAGDROP_CTRL_COPY |
        SV_DRAGDROP_APP_MOVE  | SV_DRAGDROP_APP_COPY  | SV_DRAGDROP_APP_DROP );
    SetEntryHeight( 16 );
    SetSelectionMode( SINGLE_SELECTION );
    GetModel()->SetSortMode( SortNone );

    EnableContextMenuHandling();
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxOrganizeListBox_Impl, OnAsyncExecuteDrop, ExecuteDropEvent*, pEvent )
{
    DBG_ASSERT( pEvent, "invalid DropEvent" );
    if ( pEvent )
    {
        SvLBox* pSourceView = GetSourceView();
        if ( !pSourceView )
            pSourceView = pDlg->pSourceView;
        pDlg->bExecDropFinished = false;
        // if a template can not be moved it should be copied
        if ( pEvent->mnAction == DND_ACTION_MOVE )
            pEvent->mnAction = DND_ACTION_COPYMOVE;
        pDlg->nDropAction = SvTreeListBox::ExecuteDrop( *pEvent, pSourceView );
        delete pEvent;
        pDlg->pSourceView = NULL;
        pDlg->pTargetEntry = NULL;
        pDlg->bExecDropFinished = true;
        if ( pDlg->pFinishedBox )
        {
            pDlg->pFinishedBox->DragFinished( pDlg->nDropAction );
            pDlg->pFinishedBox = NULL;
        }
    }
    return 0;
}

//-------------------------------------------------------------------------

void SfxOrganizeListBox_Impl::Reset()

/*  [Beschreibung]

    Einf"ugen der Elemente in die ListBox

*/

{
    DBG_ASSERT( pMgr != 0, "kein Manager" );
    // Inhalte l"oschen
    SetUpdateMode(sal_False);
    Clear();
    if ( VIEW_TEMPLATES == eViewType )
    {
        const sal_uInt16 nCount = pMgr->GetTemplates()->GetRegionCount();
        for ( sal_uInt16 i = 0; i < nCount; ++i )
            InsertEntryByBmpType( pMgr->GetTemplates()->GetFullRegionName(i), BMPTYPE_FOLDER, 0, sal_True );
    }
    else
    {
        const SfxObjectList& rList = pMgr->GetObjectList();
        const sal_uInt16 nCount = rList.Count();
        for ( sal_uInt16 i = 0; i < nCount; ++i )
            InsertEntryByBmpType( rList.GetBaseName(i), BMPTYPE_DOC, 0, sal_True );

    }
    SetUpdateMode(sal_True);
    Invalidate();
    Update();
}

//-------------------------------------------------------------------------

const Image &SfxOrganizeListBox_Impl::GetClosedBmp(sal_uInt16 nLevel) const

/*  [Beschreibung]

    Zugriff auf die Bitmap f"ur einen geschlossenen Eintrag
    der jeweiligen Ebene

    [Parameter]

    sal_uInt16 nLevel       Angabe der Ebene, 2 Ebenen sind erlaubt

    [Returnwert]

    const Image &       das Image auf der Ebenen nLevel

*/

{
    sal_Bool            bHC = GetSettings().GetStyleSettings().GetHighContrastMode();
    const Image*    pRet = NULL;

    switch( nLevel )
    {
        default:    DBG_ERROR( "Bitmaps ueberindiziert" );

        case 0:     pRet = bHC? &aClosedFolderBmpHC : &aClosedFolderBmp;        break;
        case 1:     pRet = bHC? &aClosedDocBmpHC : &aClosedDocBmp;              break;
    }

    return *pRet;
}

//-------------------------------------------------------------------------

const Image &SfxOrganizeListBox_Impl::GetOpenedBmp(sal_uInt16 nLevel) const

/*  [Beschreibung]

    Zugriff auf die Bitmap f"ur einen ge"offneten Eintrag
    der jeweiligen Ebene

    [Parameter]

    sal_uInt16 nLevel       Angabe der Ebene, 2 Ebenen sind erlaubt

    [Returnwert]

    const Image &       das Image auf der Ebenen nLevel

*/

{
    sal_Bool         bHC = GetSettings().GetStyleSettings().GetHighContrastMode();
    const Image* pRet = NULL;

    switch( nLevel )
    {
        case 0:
           pRet = bHC ? &aOpenedFolderBmpHC : &aOpenedFolderBmp; break;
        case 1:
           pRet = bHC ? &aOpenedDocBmpHC : &aOpenedDocBmp; break;
        default:
            pRet = bHC ? &aClosedFolderBmpHC : &aClosedFolderBmp; break;
    }

    return *pRet;
}

//-------------------------------------------------------------------------

PopupMenu* SfxOrganizeListBox_Impl::CreateContextMenu()
{
    return new PopupMenu( *( pDlg->aEditBtn.GetPopupMenu() ) );
}

//-------------------------------------------------------------------------

String SfxOrganizeDlg_Impl::GetPath_Impl( sal_Bool bOpen, const String& rFileName )

/*  [Beschreibung]

    Pfad per FileDialog erfragen, f"ur Import / Export von
    Dokumentvorlagen

    [Parameter]

    sal_Bool bOpen                      Flag: "Offnen / Speichern
    const String& rFileName         aktueller Dateiname als Vorschlag

    [R"uckgabewert]                 Dateiname mit Pfad oder Leerstring, wenn
                                    der Benutzer 'Abbrechen' gedr"uckt hat
*/

{
    String aPath;
    m_sExtension4Save = DEFINE_CONST_UNICODE( "vor" );
    sal_Int16 nDialogType = bOpen
        ? com::sun::star::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE
        : com::sun::star::ui::dialogs::TemplateDescription::FILESAVE_SIMPLE;
    if ( pFileDlg )
        delete pFileDlg;
    pFileDlg = new sfx2::FileDialogHelper( nDialogType, 0L );

    // add "All" filter
    pFileDlg->AddFilter( String( SfxResId( STR_SFX_FILTERNAME_ALL ) ),
                         DEFINE_CONST_UNICODE( FILEDIALOG_FILTER_ALL ) );
    // add template filter
    String sFilterName( SfxResId( STR_TEMPLATE_FILTER ) );
    String sFilterExt;
    // add filters of modules which are installed
    SvtModuleOptions aModuleOpt;
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SWRITER ) )
        sFilterExt += DEFINE_CONST_UNICODE( "*.ott;*.stw;*.oth" );
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SCALC ) )
    {
        if ( sFilterExt.Len() > 0 )
            sFilterExt += ';';
        sFilterExt += DEFINE_CONST_UNICODE( "*.ots;*.stc" );
    }
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SIMPRESS ) )
    {
        if ( sFilterExt.Len() > 0 )
            sFilterExt += ';';
        sFilterExt += DEFINE_CONST_UNICODE( "*.otp;*.sti" );
    }
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SDRAW ) )
    {
        if ( sFilterExt.Len() > 0 )
            sFilterExt += ';';
        sFilterExt += DEFINE_CONST_UNICODE( "*.otg;*.std" );
    }
    if ( sFilterExt.Len() > 0 )
        sFilterExt += ';';
    sFilterExt += DEFINE_CONST_UNICODE( "*.vor" );

    sFilterName += DEFINE_CONST_UNICODE( " (" );
    sFilterName += sFilterExt;
    sFilterName += ')';
    pFileDlg->AddFilter( sFilterName, sFilterExt );
    pFileDlg->SetCurrentFilter( sFilterName );

    if ( aLastDir.Len() || rFileName.Len() )
    {
        INetURLObject aObj;
        if ( aLastDir.Len() )
        {
            aObj.SetURL( aLastDir );
            if ( rFileName.Len() )
                aObj.insertName( rFileName );
        }
        else
            aObj.SetURL( rFileName );

        if ( aObj.hasExtension() )
        {
            m_sExtension4Save = aObj.getExtension(
                INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );
            aObj.removeExtension();
        }

        DBG_ASSERT( aObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );
        pFileDlg->SetDisplayDirectory( aObj.GetMainURL( INetURLObject::NO_DECODE ) );
    }

    pFileDlg->StartExecuteModal( LINK( this, SfxOrganizeDlg_Impl, ImportHdl ) );

    return aPath;
}

//-------------------------------------------------------------------------

::com::sun::star::uno::Sequence< ::rtl::OUString >
    SfxOrganizeDlg_Impl::GetPaths_Impl( const String& rFileName )

/*  [Description]

   Query plural paths by FileDialog, for Import / Export from document
   templates

   [Parameter]

   const String& rFileName         The default file name when dialog executes

   [Return value]                 Empty sequence when users have clicked
                                  'Cancel', a sequence just containing one
                                  file name with path when they have
                                  choosed one file or a sequence containing
                                  path and file names without path
*/

{
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aPaths;
    m_sExtension4Save = DEFINE_CONST_UNICODE( "vor" );
    if ( pFileDlg )
            delete pFileDlg;
    pFileDlg = new sfx2::FileDialogHelper(
        com::sun::star::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE, SFXWB_MULTISELECTION );

    // add "All" filter
    pFileDlg->AddFilter( String( SfxResId( STR_SFX_FILTERNAME_ALL ) ),
                         DEFINE_CONST_UNICODE( FILEDIALOG_FILTER_ALL ) );

    // add template filter
    String sFilterName( SfxResId( STR_TEMPLATE_FILTER ) );
    String sFilterExt;
    // add filters of modules which are installed
    SvtModuleOptions aModuleOpt;
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SWRITER ) )
        sFilterExt += DEFINE_CONST_UNICODE( "*.ott;*.stw;*.oth" );
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SCALC ) )
    {
        if ( sFilterExt.Len() > 0 )
            sFilterExt += ';';
        sFilterExt += DEFINE_CONST_UNICODE( "*.ots;*.stc" );
    }
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SIMPRESS ) )
    {
        if ( sFilterExt.Len() > 0 )
            sFilterExt += ';';
        sFilterExt += DEFINE_CONST_UNICODE( "*.otp;*.sti" );
    }
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SDRAW ) )
    {
        if ( sFilterExt.Len() > 0 )
            sFilterExt += ';';
        sFilterExt += DEFINE_CONST_UNICODE( "*.otg;*.std" );
    }
    if ( sFilterExt.Len() > 0 )
        sFilterExt += ';';
    sFilterExt += DEFINE_CONST_UNICODE( "*.vor" );

    sFilterName += DEFINE_CONST_UNICODE( " (" );
    sFilterName += sFilterExt;
    sFilterName += ')';
    pFileDlg->AddFilter( sFilterName, sFilterExt );
    pFileDlg->SetCurrentFilter( sFilterName );

    if ( aLastDir.Len() || rFileName.Len() )
    {
        INetURLObject aObj;
        if ( aLastDir.Len() )
        {
            aObj.SetURL( aLastDir );
            if ( rFileName.Len() )
                aObj.insertName( rFileName );
        }
        else
            aObj.SetURL( rFileName );

        if ( aObj.hasExtension() )
        {
            m_sExtension4Save = aObj.getExtension(
                 INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );
            aObj.removeExtension();
        }

        DBG_ASSERT( aObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );
        pFileDlg->SetDisplayDirectory( aObj.GetMainURL( INetURLObject::NO_DECODE ) );
    }

    pFileDlg->StartExecuteModal( LINK( this, SfxOrganizeDlg_Impl, ExportHdl ) );

    return aPaths;
}

//-------------------------------------------------------------------------

sal_Bool SfxOrganizeDlg_Impl::DontDelete_Impl( SvLBoxEntry* pEntry )
{
    sal_uInt16 nDepth = pFocusBox->GetModel()->GetDepth(pEntry);
    if(SfxOrganizeListBox_Impl::VIEW_FILES ==
       pFocusBox->GetViewType())
        nDepth++;
    if( (nDepth > 2 && !pEntry->GetUserData()) ||
       //Delete ueber GetContent verboten
       nDepth==2 || //Vorlage / Konfigurtionsrubrik nicht loeshcen
       (nDepth==1 && SfxOrganizeListBox_Impl::VIEW_FILES ==
        pFocusBox->GetViewType()) || //Files nicht loeschen
       (0 == nDepth && pFocusBox->GetLevelCount_Impl(0) < 2))
        //Mindestens eine Vorlage behalten
    {
        return sal_True;
    }

    sal_uInt16 nRegion = 0, nIndex = 0;
    GetIndices_Impl( pFocusBox, pEntry, nRegion, nIndex );
    const SfxDocumentTemplates* pTemplates = aMgr.GetTemplates();
    if ( !pTemplates || !pTemplates->HasUserContents( nRegion, nIndex ) )
        return sal_True;

    return sal_False;
}

SvStringsDtor* SfxOrganizeDlg_Impl::GetAllFactoryURLs_Impl( ) const
{
    SvtModuleOptions aModOpt;
    const ::com::sun::star::uno::Sequence < ::rtl::OUString >& aServiceNames = aModOpt.GetAllServiceNames() ;
    SvStringsDtor* pList = new SvStringsDtor;
    sal_Int32 nCount = aServiceNames.getLength();
    for( sal_Int32 i=0; i<nCount; ++i )
    {
        if ( SfxObjectFactory::GetStandardTemplate( aServiceNames[i] ).Len() > 0 )
        {
            SvtModuleOptions::EFactory eFac = SvtModuleOptions::E_WRITER;
            SvtModuleOptions::ClassifyFactoryByName( aServiceNames[i], eFac );
            String* pURL = new String( aModOpt.GetFactoryEmptyDocumentURL( eFac ) );
            pList->Insert( pURL, pList->Count() );
        }
    }

    return pList;
}

sal_Bool SfxOrganizeDlg_Impl::GetServiceName_Impl( String& rName, String& rFileURL ) const
{
    sal_Bool bRet = sal_False;
    const SfxDocumentTemplates* pTemplates = aMgr.GetTemplates();
    SvLBoxEntry* pEntry = pFocusBox ? pFocusBox->FirstSelected() : NULL;
    sal_uInt16 nRegion = 0, nIndex = 0;
    GetIndices_Impl( pFocusBox, pEntry, nRegion, nIndex );
    rFileURL = pTemplates->GetPath( nRegion, nIndex );
    if ( rFileURL.Len() > 0 )
    {
        try
        {
            uno::Reference< embed::XStorage > xStorage = ::comphelper::OStorageHelper::GetStorageFromURL(
                                                    rFileURL,
                                                    embed::ElementModes::READ );
            sal_uIntPtr nFormat = SotStorage::GetFormatID( xStorage );
            const SfxFilter* pFilter =
                SFX_APP()->GetFilterMatcher().GetFilter4ClipBoardId( nFormat );
            if ( pFilter )
            {
                rName = pFilter->GetServiceName();
                bRet = sal_True;
            }
        }
        catch( uno::Exception& )
        {}
    }

    return bRet;
}

long SfxOrganizeDlg_Impl::Dispatch_Impl( sal_uInt16 nId, Menu* _pMenu )

/*  [Beschreibung]

    Verarbeiten der Events aus MenuButton oder Accelerator

    [Parameter]

    sal_uInt16 nId                      ID des Events

    [R"uckgabewert]                 1: Event wurde verarbeitet,
                                    0: Event wurde nicht verarbeitet (SV-Menu)

*/

{
    SuspendAccel aTmp(&aEditAcc);
    SvLBoxEntry *pEntry = pFocusBox? pFocusBox->FirstSelected(): 0;
    sal_Bool bHandled = sal_True;
    switch(nId)
    {
        case ID_NEW:
        {
            if(!pEntry)
                return 1;
            if(pFocusBox->GetViewType() == SfxOrganizeListBox_Impl::VIEW_TEMPLATES)
            {
                if(0 == pFocusBox->GetModel()->GetDepth(pEntry))
                {
                    const String aNoName( SfxResId(STR_NONAME) );
                    SvLBoxEntry* pParent = pFocusBox->GetParent(pEntry);
                    String aName(aNoName);
                    sal_uInt16 n = 1;
                    while(!pFocusBox->IsUniqName_Impl(aName, pParent))
                    {
                        aName = aNoName;
                        aName += String::CreateFromInt32( n++ );
                    }
                    aMgr.InsertDir( pFocusBox, aName,
                            (sal_uInt16)pFocusBox->GetModel()->GetRelPos(pEntry)+1);
                }
            }
            break;
        }

        case ID_DELETE:
        {
            if(!pEntry || DontDelete_Impl(pEntry))
                return 1;
            const sal_uInt16 nDepth = pFocusBox->GetModel()->GetDepth(pEntry);
            if(nDepth < 2)
            {
                if(0 == nDepth && pFocusBox->GetLevelCount_Impl(0) < 2) return 1;
                if(SfxOrganizeListBox_Impl::VIEW_TEMPLATES == pFocusBox->GetViewType())
                {
                    sal_uInt16 nResId = nDepth? STR_DELETE_TEMPLATE :
                                            STR_DELETE_REGION;
                    if( !QueryDelete_Impl(
                        pDialog, nResId, pFocusBox->GetEntryText(pEntry)))
                        return 1;
                    if ( STR_DELETE_REGION == nResId &&
                         pFocusBox->GetChildCount(pEntry))
                    {
                        QueryBox aQBox(pDialog, SfxResId(MSG_REGION_NOTEMPTY));
                        if(RET_NO == aQBox.Execute())
                            return 1;
                    }
                    sal_uInt16 nRegion = 0, nIndex = 0;
                    GetIndices_Impl(pFocusBox, pEntry, nRegion, nIndex);

                    sal_uInt16 nDeleteInd = ( STR_DELETE_REGION == nResId? USHRT_MAX: nIndex );
                    if ( !aMgr.Delete( pFocusBox, nRegion, nDeleteInd ) )
                        ErrorDelete_Impl(
                            pDialog,
                            pFocusBox->GetEntryText(pEntry),
                            ( nDeleteInd == USHRT_MAX && pFocusBox->GetChildCount(pEntry) ) );
                }
            }
            // Inhaltsformen
            else if(nDepth + pFocusBox->GetDocLevel() >= 2)
            {
                if(!QueryDelete_Impl(pDialog, STR_DELETE_TEMPLATE, pFocusBox->GetEntryText(pEntry)))
                    return 1;
                Path aPath(pFocusBox, pEntry);

                // it is ok to use the SfxObjectShellRef here since the object that
                // provides it ( GetObjectShell() calls CreateObjectShell() ) has a lock on it
                SfxObjectShellRef aRef = pFocusBox->GetObjectShell(aPath);
                if(aRef.Is() &&
                    aRef->Remove(aPath[1+pFocusBox->GetDocLevel()],
                                 aPath[2+pFocusBox->GetDocLevel()],
                                 aPath[3+pFocusBox->GetDocLevel()]))
                    pFocusBox->GetModel()->Remove(pEntry);
                else
                    ErrorDelete_Impl(pDialog, pFocusBox->GetEntryText(pEntry), sal_False );
            }
            break;
        }

        case ID_EDIT:
        {
            if(!pEntry)
                return 1;
            sal_uInt16 nRegion = 0, nIndex = 0;
            GetIndices_Impl( pFocusBox, pEntry, nRegion, nIndex );
            const SfxStringItem aName( SID_FILE_NAME, aMgr.GetTemplates()->GetPath( nRegion, nIndex ) );
            const SfxStringItem aLongName( SID_FILE_LONGNAME, pFocusBox->GetEntryText( pEntry ) );
            const SfxStringItem aReferer( SID_REFERER, DEFINE_CONST_UNICODE( "private:user" ) );
            const SfxStringItem aTargetName( SID_TARGETNAME, DEFINE_CONST_UNICODE( "_default" ) );
            const SfxBoolItem   aTemplateIndicator( SID_TEMPLATE, sal_False );

            SFX_APP()->GetAppDispatcher_Impl()->Execute( SID_OPENTEMPLATE, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                                      &aName, &aLongName, &aReferer, &aTargetName, &aTemplateIndicator, 0L );
            pDialog->EndDialog( RET_EDIT_STYLE );
            break;
        }

        case ID_COPY_FROM:
        {
            if ( !pEntry )
                return 1;
            m_nRegion = 0;
            m_nIndex = 0;
            GetIndices_Impl( pFocusBox, pEntry, m_nRegion, m_nIndex );
            GetPaths_Impl( String() );
            break;
        }

        case ID_COPY_TO:
        {
            if ( !pEntry )
                return 1;
            m_nRegion = 0;
            m_nIndex = 0;
            GetIndices_Impl( pFocusBox, pEntry, m_nRegion, m_nIndex );
            GetPath_Impl( sal_False, aMgr.GetTemplates()->GetFileName( m_nRegion, m_nIndex ) );
            break;
        }

        case ID_RESCAN:
            if ( !aMgr.Rescan() )
                ErrorBox( pDialog, SfxResId( MSG_ERROR_RESCAN ) ).Execute();
            if ( SfxOrganizeListBox_Impl::VIEW_TEMPLATES == aLeftLb.GetViewType() )
                aLeftLb.Reset();
            if ( SfxOrganizeListBox_Impl::VIEW_TEMPLATES == aRightLb.GetViewType() )
                aRightLb.Reset();
            break;

        case ID_PRINT:
        {
            if ( !pEntry )
                return 1;
            Path aPath( pFocusBox, pEntry );

            // it is ok to use the SfxObjectShellRef here since the object that
            // provides it ( GetObjectShell() calls CreateObjectShell() ) has a lock on it
            SfxObjectShellRef aRef = pFocusBox->GetObjectShell( aPath );
            if ( aRef.Is() )
            {
                const sal_uInt16 nDocLevel = pFocusBox->GetDocLevel();
                if ( !pPrt )
                    pPrt = new Printer;
                SvLBoxEntry *pDocEntry = pEntry;
                while ( pFocusBox->GetModel()->GetDepth( pDocEntry ) > nDocLevel )
                    pDocEntry = pFocusBox->GetParent( pDocEntry );
                const String aName(pFocusBox->GetEntryText(pDocEntry));
                if ( !aRef->Print( *pPrt, aPath[1+nDocLevel],
                                   aPath[2+nDocLevel], aPath[3+nDocLevel], &aName ) )
                    ErrorBox( pDialog, SfxResId( MSG_PRINT_ERROR ) ).Execute();
            }
            break;
        }

        case ID_PRINTER_SETUP:
        {
            PrinterSetupDialog* pDlg = new PrinterSetupDialog( pDialog );
            if ( !pPrt )
                pPrt = new Printer;
            pDlg->SetPrinter( pPrt );
            pDlg->Execute();
            delete pDlg;
            break;
        }

        case ID_DEFAULT_TEMPLATE:
        {
            String aServiceName, aFileURL;
            if ( GetServiceName_Impl( aServiceName, aFileURL ) )
                SfxObjectFactory::SetStandardTemplate( aServiceName, aFileURL );
            break;
        }

        default:
            bHandled = sal_False;
    }

    if ( !bHandled && ( nId > ID_RESET_DEFAULT_TEMPLATE || nId <= ID_RESET_DEFAULT_TEMPLATE_END ) )
    {
        Menu* pSubMenu = _pMenu ? _pMenu : aEditBtn.GetPopupMenu()->GetPopupMenu( ID_RESET_DEFAULT_TEMPLATE );
        if ( pSubMenu )
        {
            String aServiceName = SfxObjectShell::GetServiceNameFromFactory( pSubMenu->GetItemCommand( nId ) );
            SfxObjectFactory::SetStandardTemplate( aServiceName, String() );
            bHandled = sal_True;
        }
    }

    return bHandled ? 1 : 0;
}

//-------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxOrganizeDlg_Impl, MenuSelect_Impl, Menu *, pMenu )

/*  [Beschreibung]

    SelectHandler des Men"us des Men"ubuttons (SV)

    [Parameter]

    MenuButton *pBtn                der das Event ausl"osende Button

    [R"uckgabewert]                 1: Event wurde verarbeitet,
                                    0: Event wurde nicht verarbeitet (SV-Menu)

*/
{
    return Dispatch_Impl( pMenu->GetCurItemId(), pMenu );
}
IMPL_LINK_INLINE_END( SfxOrganizeDlg_Impl, MenuSelect_Impl, Menu *, pMenu )

//-------------------------------------------------------------------------

IMPL_LINK( SfxOrganizeDlg_Impl, AccelSelect_Impl, Accelerator *, pAccel )

/*  [Beschreibung]

    SelectHandler des Accelerators (SV)

    [Parameter]

    Accelerator *pAccel             der das Event ausl"osende Accelerator

    [R"uckgabewert]                 1: Event wurde verarbeitet,
                                    0: Event wurde nicht verarbeitet (SV)

*/

{
    SvLBoxEntry* pEntry = pFocusBox && pFocusBox->GetSelectionCount() ?
        pFocusBox->FirstSelected() : NULL ;
    return pEntry && ( pAccel->GetCurItemId() == ID_NEW  || !DontDelete_Impl( pEntry ) ) ?
        Dispatch_Impl( pAccel->GetCurItemId(), NULL ) : 0;
}

//-------------------------------------------------------------------------

void SfxOrganizeDlg_Impl::OkHdl(Button *pButton)
{
    if(pFocusBox && pFocusBox->IsEditingActive())
        pFocusBox->EndEditing(sal_False);
    pButton->Click();
}



IMPL_LINK( SfxOrganizeDlg_Impl, MenuActivate_Impl, Menu *, pMenu )

/*  [Beschreibung]

    ActivateHandler des Men"us des Men"ubuttons (SV)

    [Parameter]

    Menu *pMenu                     das das Event ausl"osende Men"u

    [R"uckgabewert]                 1: Event wurde verarbeitet,
                                    0: Event wurde nicht verarbeitet (SV-Menu)

*/
{
    if ( pFocusBox && pFocusBox->IsEditingActive() )
        pFocusBox->EndEditing( sal_False );
    sal_Bool bEnable = ( pFocusBox && pFocusBox->GetSelectionCount() );
    SvLBoxEntry* pEntry = bEnable ? pFocusBox->FirstSelected() : NULL;
    const sal_uInt16 nDepth =
        ( bEnable && pFocusBox->GetSelectionCount() ) ? pFocusBox->GetModel()->GetDepth( pEntry ) : 0;
    const sal_uInt16 nDocLevel = bEnable ? pFocusBox->GetDocLevel() : 0;
    int eVT = pFocusBox ? pFocusBox->GetViewType() : 0;
        // nur Vorlagen anlegen
    pMenu->EnableItem( ID_NEW, bEnable && 0 == nDepth && SfxOrganizeListBox_Impl::VIEW_TEMPLATES == eVT );
    // Vorlagen: Loeschen Ebene 0,1,3ff
    //           ein Bereich mu"s mindestens erhalten bleiben
    // Dateien : Loeschen Ebene > 2

    pMenu->EnableItem( ID_DELETE, bEnable && !DontDelete_Impl( pEntry ) );
    pMenu->EnableItem( ID_EDIT,
                       bEnable && eVT == SfxOrganizeListBox_Impl::VIEW_TEMPLATES && nDepth == nDocLevel
                           && !DontDelete_Impl( pEntry ) );
    pMenu->EnableItem( ID_COPY_FROM,
                       bEnable && eVT == SfxOrganizeListBox_Impl::VIEW_TEMPLATES &&
                       ( nDepth == nDocLevel || nDepth == nDocLevel - 1 ) );
    pMenu->EnableItem( ID_COPY_TO,
                       bEnable && eVT == SfxOrganizeListBox_Impl::VIEW_TEMPLATES &&
                       nDepth == nDocLevel );
    pMenu->EnableItem( ID_RESCAN,
                       SfxOrganizeListBox_Impl::VIEW_TEMPLATES == aRightLb.GetViewType() ||
                       SfxOrganizeListBox_Impl::VIEW_TEMPLATES == aLeftLb.GetViewType() );
    sal_Bool bPrint = bEnable && nDepth > pFocusBox->GetDocLevel();
    if ( bPrint && pPrt )
        bPrint = !pPrt->IsPrinting() && !pPrt->IsJobActive();
    if ( bPrint && bEnable )
    {
        // only styles printable
        Path aPath( pFocusBox, pFocusBox->FirstSelected() );
        sal_uInt16 nIndex = aPath[ nDocLevel + 1 ];
        bPrint = ( nIndex == CONTENT_STYLE );
                }
    pMenu->EnableItem( ID_PRINT, bPrint );

    if ( bEnable && eVT == SfxOrganizeListBox_Impl::VIEW_TEMPLATES && nDepth == nDocLevel )
    {
        String aFactoryURL, aFileURL;
        bEnable = GetServiceName_Impl( aFactoryURL, aFileURL );
    }
    else if ( bEnable )
        bEnable = sal_False;
    pMenu->EnableItem( ID_DEFAULT_TEMPLATE, bEnable );

    bEnable = sal_True;
    SvStringsDtor* pList = GetAllFactoryURLs_Impl();
    sal_uInt16 nCount = pList->Count();
    if ( nCount > 0 )
    {
        PopupMenu* pSubMenu = new PopupMenu;
        sal_uInt16 nItemId = ID_RESET_DEFAULT_TEMPLATE + 1;
        for ( sal_uInt16 i = 0; i < nCount; ++i )
        {
            String aObjFacURL( *pList->GetObject(i) );
            String aTitle = SvFileInformationManager::GetDescription(
                INetURLObject(aObjFacURL) );
            pSubMenu->InsertItem( nItemId, aTitle,
                SvFileInformationManager::GetImage(INetURLObject(aObjFacURL)) );
            pSubMenu->SetItemCommand( nItemId++, aObjFacURL );
            DBG_ASSERT( nItemId <= ID_RESET_DEFAULT_TEMPLATE_END, "menu item id overflow" );
        }
        pMenu->SetPopupMenu( ID_RESET_DEFAULT_TEMPLATE, pSubMenu );
    }
    else
        bEnable = sal_False;

    delete pList;
    pMenu->EnableItem( ID_RESET_DEFAULT_TEMPLATE, bEnable );

    return 1;
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxOrganizeDlg_Impl, GetFocus_Impl, SfxOrganizeListBox_Impl *, pBox )

/*  [Beschreibung]

    GetFocus-Handler, wird aus den Select-Handler der Listboxen
    gerufen.
    Wird verwendet, im die Listbox, die den Focus besitzt sowie
    deren Zustand zu ermitteln.

    [Parameter]

    SfxOrganizeListBox *pBox        die rufende Box

*/

{
    if(pFocusBox && pFocusBox != pBox)
        pFocusBox->SelectAll(sal_False);
    pFocusBox = pBox;
    aFilesBtn.Enable( SfxOrganizeListBox_Impl::VIEW_FILES ==
                      pFocusBox->GetViewType() );
    return 0;
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxOrganizeDlg_Impl, LeftListBoxSelect_Impl, ListBox *, pBox )

/*  [Beschreibung]

    Select-Handler, wird aus den Select-Handler der Listboxen
    gerufen.
    Wenn sich der Modus der Boxen (Dokumentsicht, Dokumentvorlagensicht)
    unterscheiden, werden die Models getrennt; andernfalls zusammengefa"st.

    [Parameter]

    ListBox *pBox               die rufende Box

*/
{
    const SfxOrganizeListBox_Impl::DataEnum
        eViewType = pBox->GetSelectEntryPos() == 0 ?
        SfxOrganizeListBox_Impl::VIEW_TEMPLATES : SfxOrganizeListBox_Impl::VIEW_FILES;
    if(eViewType!= aLeftLb.GetViewType()) {
        aLeftLb.SetViewType(eViewType);
        if(aRightLb.GetViewType() == eViewType)
            aLeftLb.SetModel(aRightLb.GetModel());
        else {
            // Models trennen
            aLeftLb.DisconnectFromModel();
            aLeftLb.Reset();
        }
    }
    GetFocus_Impl(&aLeftLb);
    return 0;
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxOrganizeDlg_Impl, RightListBoxSelect_Impl, ListBox *, pBox )

/*  [Beschreibung]

    Select-Handler, wird aus den Select-Handler der Listboxen
    gerufen.
    Wenn sich der Modus der Boxen (Dokumentsicht, Dokumentvorlagensicht)
    unterscheiden, werden die Models getrennt; andernfalls zusammengefa"st.

    [Parameter]

    ListBox *pBox               die rufende Box

*/
{
    const SfxOrganizeListBox_Impl::DataEnum eViewType =
        pBox->GetSelectEntryPos() == 0 ?
        SfxOrganizeListBox_Impl::VIEW_TEMPLATES : SfxOrganizeListBox_Impl::VIEW_FILES;
    if(eViewType!= aRightLb.GetViewType())
    {
        aRightLb.SetViewType(eViewType);
        if(aLeftLb.GetViewType() == eViewType)
            aRightLb.SetModel(aLeftLb.GetModel());
        else
        {
            // Models trennen
            aRightLb.DisconnectFromModel();
            aRightLb.Reset();
        }
    }
    aRightLb.GrabFocus();
    GetFocus_Impl(&aRightLb);
    return 0;
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxOrganizeDlg_Impl, OnAddressTemplateClicked, Button *, pButton )
{
    (void)pButton; //unused
    svt::AddressBookSourceDialog aDialog(pDialog, ::comphelper::getProcessServiceFactory());
    aDialog.Execute();
    return 0L;
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxOrganizeDlg_Impl, AddFiles_Impl, Button *, pButton )

/*  [Beschreibung]

    Handler des Buttons f"ur das Hinzuf"ugen von Dateien per Dialog.

    [Parameter]

    Button *                der Button, der dieses Events ausgel"ost hat.

*/
{
    (void)pButton; //unused
    if ( pFileDlg )
        delete pFileDlg;
    pFileDlg = new sfx2::FileDialogHelper( WB_OPEN, String() );

    // add config and basic filter
    static String sOpenBracket( DEFINE_CONST_UNICODE( " (" ) );
    static String sCloseBracket( DEFINE_CONST_UNICODE( ")" ) );
    static String sConfigExt( DEFINE_CONST_UNICODE( "*.cfg" ) );
    static String sBasicExt( DEFINE_CONST_UNICODE( "*.sbl" ) );

    String sFilterName( SfxResId( RID_STR_FILTCONFIG ) );
    sFilterName += sOpenBracket;
    sFilterName += sConfigExt;
    sFilterName += sCloseBracket;
    pFileDlg->AddFilter( sFilterName, sConfigExt );

    sFilterName = String( SfxResId( RID_STR_FILTBASIC ) );
    sFilterName += sOpenBracket;
    sFilterName += sBasicExt;
    sFilterName += sCloseBracket;
    pFileDlg->AddFilter( sFilterName, sBasicExt );

    // set "All" filter as current
    pFileDlg->SetCurrentFilter( String( SfxResId( STR_SFX_FILTERNAME_ALL ) ) );

    if ( aLastDir.Len() )
        pFileDlg->SetDisplayDirectory( aLastDir );

    pFileDlg->StartExecuteModal( LINK( this, SfxOrganizeDlg_Impl, AddFilesHdl ) );

    return 0;
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxOrganizeDlg_Impl, ImportHdl, sfx2::FileDialogHelper *, EMPTYARG )
{
    DBG_ASSERT( pFileDlg, "SfxOrganizeDlg_Impl::ImportHdl(): no file dialog" );

    if ( ERRCODE_NONE == pFileDlg->GetError() )
    {
        String aPath = pFileDlg->GetPath();
        INetURLObject aObj( aPath );

        // we want to keep the original extension when exporting, the file open dialog
        // always sets the extension to *.vor
        if ( pFileDlg->GetDialogType() ==
                com::sun::star::ui::dialogs::TemplateDescription::FILESAVE_SIMPLE )
        {
            if ( aObj.hasExtension() )
                aObj.removeExtension();

            aObj.setExtension( m_sExtension4Save );
            aPath = aObj.GetMainURL( INetURLObject::DECODE_TO_IURI );
        }

        aObj.removeSegment();
        aLastDir = aObj.GetMainURL( INetURLObject::DECODE_TO_IURI );

        if ( aPath.Len() && !aMgr.CopyTo( m_nRegion, m_nIndex, aPath ) )
        {
            String aText( SfxResId( STR_ERROR_COPY_TEMPLATE ) );
            aText.SearchAndReplaceAscii( "$1", aPath );
            ErrorBox( pDialog, WB_OK, aText ).Execute();
        }
    }

    return 0L;
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxOrganizeDlg_Impl, ExportHdl, sfx2::FileDialogHelper *, EMPTYARG )
{
    DBG_ASSERT( pFileDlg, "SfxOrganizeDlg_Impl::ImportHdl(): no file dialog" );
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aPaths;

    if ( ERRCODE_NONE == pFileDlg->GetError() )
    {
        aPaths = pFileDlg->GetMPath();
        sal_Int32 lastCount = aPaths.getLength() - 1;
        INetURLObject aObj( aPaths.getArray()[ lastCount ] );

        aObj.removeSegment();
        aLastDir = aObj.GetMainURL( INetURLObject::DECODE_TO_IURI );
    }

    sal_Int32 nCount = aPaths.getLength();
    if ( 1 == nCount )
    {
       String aPath = String( aPaths.getArray()[0] );
       if ( aPath.Len() && !aMgr.CopyFrom( pFocusBox, m_nRegion, m_nIndex, aPath ) )
       {
           String aText( SfxResId( STR_ERROR_COPY_TEMPLATE ) );
           aText.SearchAndReplaceAscii( "$1", aPath );
           ErrorBox( pDialog, WB_OK, aText ).Execute();
       }
    }
    else if ( nCount > 1 )
    {
        INetURLObject aPathObj( aPaths[0] );
        aPathObj.setFinalSlash();
        for ( sal_uInt16 i = 1; i < nCount; ++i )
        {
            if ( 1 == i )
                aPathObj.Append( aPaths[i] );
            else
                aPathObj.setName( aPaths[i] );
            String aPath = aPathObj.GetMainURL( INetURLObject::NO_DECODE );
            if ( aPath.Len() && !aMgr.CopyFrom( pFocusBox, m_nRegion, m_nIndex, aPath ) )
            {
                String aText( SfxResId( STR_ERROR_COPY_TEMPLATE ) );
                aText.SearchAndReplaceAscii( "$1", aPath );
                ErrorBox( pDialog, WB_OK, aText ).Execute();
            }
        }
    }

    return 0L;
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxOrganizeDlg_Impl, AddFilesHdl, sfx2::FileDialogHelper *, EMPTYARG )
{
    if ( ERRCODE_NONE == pFileDlg->GetError() )
    {
        String aPath = pFileDlg->GetPath();
        aMgr.InsertFile( pFocusBox, aPath );
        INetURLObject aObj( aPath );
        aObj.removeSegment();
        aObj.setFinalSlash();
        aLastDir = aObj.GetMainURL( INetURLObject::DECODE_TO_IURI );
    }

    return 0L;
}

//-------------------------------------------------------------------------

short SfxTemplateOrganizeDlg::Execute()

/*  [Beschreibung]

    "Uberladene Execute- Methode; speichert gfs. "Anderungen an den
    Dokumentvorlagen
    (SV-Methode)

*/

{
    const short nRet = ModalDialog::Execute();
    if(RET_CANCEL != nRet)
    {
        pImp->aMgr.SaveAll(this);
        SfxTemplateDialog* pTemplDlg = SFX_APP()->GetTemplateDialog();
        if(pTemplDlg)
            pTemplDlg->Update();
    }
    return nRet;
}


//-------------------------------------------------------------------------

SfxTemplateOrganizeDlg::SfxTemplateOrganizeDlg(Window * pParent,
                                                SfxDocumentTemplates *pTempl)
:   ModalDialog( pParent, SfxResId(DLG_ORGANIZE)),
    pImp( new SfxOrganizeDlg_Impl(this, pTempl) )

/*  [Beschreibung]

    Konstruktor

*/
{
    FreeResource();
}

//-------------------------------------------------------------------------

SfxTemplateOrganizeDlg::~SfxTemplateOrganizeDlg()
{
    GetpApp()->RemoveAccel(&pImp->aEditAcc);
    delete pImp->pPrt;
    delete pImp;
}

