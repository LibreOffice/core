/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"

#include <stdio.h>

#include <svtools/prnsetup.hxx>
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
#include <sfx2/fcontnr.hxx>
#include <svtools/localresaccess.hxx>
#include <svtools/addresstemplate.hxx>
#include <comphelper/processfactory.hxx>

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
    const Image &rOFolder, const Image &rCFolder, const Image &rODoc, const Image &rCDoc )
{
    aOpenedFolderBmp = rOFolder;
    aClosedFolderBmp = rCFolder;
    aOpenedDocBmp = rODoc;
    aClosedDocBmp = rCDoc;

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
    SvTreeListBox*                 pSourceView;
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

    std::vector<rtl::OUString> GetAllFactoryURLs_Impl() const;
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

    DECL_LINK(ImportHdl, void *);
    DECL_LINK(ExportHdl, void *);
    DECL_LINK(AddFilesHdl, void *);

    sal_Bool        DontDelete_Impl( SvLBoxEntry* pEntry );

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
            // * use a new SfxDocumentTemplates instance for the update
            //   (knowing that they all share the same implementation class)
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
        String aProgURL = SvtPathOptions().SubstituteVariable( rtl::OUString("$(PROGURL)") );
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

    aLeftLb.SetBitmaps( aOpenedFolderBmp, aClosedFolderBmp, aOpenedDocBmp, aClosedDocBmp );
    aRightLb.SetBitmaps( aOpenedFolderBmp, aClosedFolderBmp, aOpenedDocBmp, aClosedDocBmp );
}

//=========================================================================

sal_Bool QueryDelete_Impl(Window *pParent,      // Parent to QueryBox
                             sal_uInt16 nId,            // Resource Id
                             const String &rTemplateName)   // Name of template to be deleted
/*  [Description]

    Delete Query

*/
{
    SfxResId aResId( nId );
    String aEntryText( aResId.toString() );
    aEntryText.SearchAndReplaceAscii( "$1", rTemplateName );
    QueryBox aBox( pParent, WB_YES_NO | WB_DEF_NO, aEntryText );
    return RET_NO != aBox.Execute();
}

//-------------------------------------------------------------------------

void ErrorDelete_Impl(Window *pParent, const String &rName, sal_Bool bFolder = sal_False )

/*  [Description]

    User information that the template rName could not be deleted.
*/
{
    if ( bFolder )
    {
        rtl::OUString aText(SfxResId(STR_ERROR_DELETE_TEMPLATE_DIR).toString());
        ErrorBox( pParent, WB_OK, aText ).Execute();
    }
    else
    {
        String aText( SfxResId(STR_ERROR_DELETE_TEMPLATE).toString() );
        aText.SearchAndReplaceAscii( "$1", rName );
        ErrorBox( pParent, WB_OK, aText ).Execute();
    }
}


//=========================================================================

/*  [Description]

    Implementation class, the benchmark for sal_uInt16-Array
*/

struct ImpPath_Impl
{
    std::vector<sal_uInt16> aUS;
    sal_uInt16      nRef;

    ImpPath_Impl();
    ImpPath_Impl( const ImpPath_Impl& rCopy );
};

//-------------------------------------------------------------------------

ImpPath_Impl::ImpPath_Impl() : nRef(1)
{
}

//-------------------------------------------------------------------------

ImpPath_Impl::ImpPath_Impl( const ImpPath_Impl& rCopy ) :

    aUS(rCopy.aUS), nRef( 1 )

{
}

//==========================================================================

/*  [Description]

    Implementation class, presentation of a position in the Outline-
    Listbox ass sal_uInt16-Array, this describes the position of each as
    Postions relative to the parent entry
*/
class Path
{
    ImpPath_Impl *pData;
public:
    Path(SvTreeListBox *pBox, SvLBoxEntry *pEntry);
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
    sal_uInt16 Count() const { return pData->aUS.size(); }
    sal_uInt16 operator[]( sal_uInt16 i ) const
    {
        return i < Count()? pData->aUS[i]: INDEX_IGNORE;
    }
};

//-------------------------------------------------------------------------

Path::Path(SvTreeListBox *pBox, SvLBoxEntry *pEntry) :
    pData(new ImpPath_Impl)
{
    DBG_ASSERT(pEntry != 0, "EntryPtr ist NULL");
    if(!pEntry)
        return;
    SvLBoxEntry *pParent = pBox->GetParent(pEntry);
    do {
        pData->aUS.insert(pData->aUS.begin(),
                (sal_uInt16)pBox->GetModel()->GetRelPos(pEntry));
        if(0 == pParent)
            break;
        pEntry = pParent;
        pParent = pBox->GetParent(pEntry);
    } while(1);
}

//-------------------------------------------------------------------------

SvLBoxEntry *GetIndices_Impl(SvTreeListBox *pBox,
                               SvLBoxEntry *pEntry,
                               sal_uInt16 &rRegion,
                               sal_uInt16 &rOffset)
/*  [Description]

    Region and position within a range for a template is determined.

    [Parameter]

    SvTreeListBox *pBox            Listbox where the event occurred
    SvLBoxEntry *pEntry     Entry whose position is to be determined
    sal_uInt16 &rRegion         the region within the region of the
                            document template (Out-Parameter)
    sal_uInt16 &rOffset         the position within the region of the
                            document template (Out-Parameter)


    [Cross-references]

    <class Path>    (in certain circumstances this function can also be
                     dispensed in favor of the Path)
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
        aPath[nLevel+1], aPath[nLevel+2]);
    return SvTreeListBox::Select(pEntry,bSelect);
}

//-------------------------------------------------------------------------

sal_Bool SfxOrganizeListBox_Impl::MoveOrCopyTemplates(SvTreeListBox *pSourceBox,
                                            SvLBoxEntry *pSource,
                                            SvLBoxEntry* pTarget,
                                            SvLBoxEntry *&pNewParent,
                                            sal_uIntPtr &rIdx,
                                            sal_Bool bCopy)
/*  [Description]

    Move or copy a document templates

    [Parameter]

    SvTreeListBox *pSourceBox          Source Listbox, at which the event occurred
    SvLBoxEntry* pTarget        Target entry, to where it will be moved
    SvLBoxEntry *pSource        Source entry, to be copied / moved
    SvLBoxEntry *&pNewParent    the parent of the target position generated
                                at entry (out parameter)
    sal_uIntPtr &rIdx                 Index of the target entry
    sal_Bool bCopy                  Flag for Copy / Move


    [Return value]              sal_Bool: Success or failure

    [Cross-references]

    <SfxOrganizeListBox_Impl::MoveOrCopyContents(SvTreeListBox *pSourceBox,
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
                    SvTreeListBox::NotifyCopying(
                        pTarget, pSource, pNewParent, rIdx);
            }
        }
        else if ( bCopy )
        {
            // the template organizer always tries copy after the move, so no error is required for move case
            String aText( SfxResId( bCopy ? STR_ERROR_COPY_TEMPLATE : STR_ERROR_MOVE_TEMPLATE ).toString() );
            aText.SearchAndReplaceAscii( "$1",
                                         ( (SvTreeListBox *)pSourceBox )->GetEntryText( pSource ) );
            ErrorBox( this, WB_OK, aText ).Execute();
        }
    }
    return bOk;
}

//-------------------------------------------------------------------------

sal_Bool SfxOrganizeListBox_Impl::MoveOrCopyContents(SvTreeListBox *pSourceBox,
                                            SvLBoxEntry *pSource,
                                            SvLBoxEntry* pTarget,
                                            SvLBoxEntry *&pNewParent,
                                            sal_uIntPtr &rIdx,
                                            sal_Bool bCopy)
/*  [Description]

    Move or copy document contents

    [Parameter]

    SvTreeListBox *pSourceBox          Source Listbox, at which the event occurred
    SvLBoxEntry* pTarget        Target entry, to where it will be moved
    SvLBoxEntry *pSource        Source entry, to be copied / moved
    SvLBoxEntry *&pNewParent    the parent of the target position generated
                                at entry (out parameter)
    sal_uIntPtr &rIdx                 Index of the target entry
    sal_Bool bCopy                  Flag for Copy / Move


    [Return value]              sal_Bool: Success or failure

    [Cross-references]

    <SfxOrganizeListBox_Impl::MoveOrCopyTemplates(SvTreeListBox *pSourceBox,
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
        // Evaluate Position correction
        // a = Document content
        // b = Position Sub-content 1
        // c = Position Sub-content 2
        // Delete duplicate entries
        if(bOk)
        {
            SvLBoxEntry *pParentIter = pTarget;
            // Up to the document level as
            // the general reference point
            while(GetModel()->GetDepth(pParentIter) != nTLevel)
                pParentIter = GetParent(pParentIter);
            if(pParentIter->HasChildrenOnDemand() &&
                !GetModel()->HasChildren(pParentIter))
                RequestingChildren(pParentIter);
            SvLBoxEntry *pChildIter = 0;

            sal_uInt16 i = 0;
            while(i < 2 && p[i+1] != INDEX_IGNORE)
            {
                pChildIter = FirstChild(pParentIter);
                // To the index of the current level
                for(sal_uInt16 j = 0; j < p[i]; ++j)
                    pChildIter = NextSibling(pChildIter);
                // If possible, fill in Items onDemand
                ++i;
                if(i < 2 && p[i+1] != INDEX_IGNORE &&
                   pChildIter->HasChildrenOnDemand() &&
                   !GetModel()->HasChildren(pChildIter))
                    RequestingChildren(pChildIter);
                pParentIter = pChildIter;
            }
            rIdx = p[i];
            pNewParent = pParentIter;
            if(!IsExpanded(pNewParent) &&
               pNewParent->HasChildrenOnDemand() &&
               !GetModel()->HasChildren(pNewParent))
            {
                bOk = sal_False;
                if(!bCopy)
                    pSourceBox->GetModel()->Remove(pSource);
            }
            // Remove deleted items
            // (can be done by overwriting)
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
                //pool styles that are moved produce
                //an rIdx == INDEX_IGNORE
                //the method has to return true to keep the box content consistent
                bRemovedFromSource = aSourceDoc->Remove(aSource[nSLevel+1],
                                     aSource[nSLevel+2],
                                     aSource[nSLevel+3]);
            }
        }
    }
    return (((rIdx != INDEX_IGNORE)|| bRemovedFromSource) && bOk )
        ? bKeepExpansion? (sal_Bool)2: sal_True: sal_False;
}

//-------------------------------------------------------------------------

sal_Bool SfxOrganizeListBox_Impl::NotifyMoving(SvLBoxEntry *pTarget,
                                        SvLBoxEntry* pSource,
                                        SvLBoxEntry *&pNewParent,
                                        sal_uIntPtr &rIdx)

/*  [Description]

    Notification that an item will be moved.
    (SV-Handler)

    [Parameter]

    SvLBoxEntry* pTarget        Target entry, to where it will be moved
    SvLBoxEntry *pSource        Source entry, to be moved
    SvLBoxEntry *&pNewParent    the parent of the target position generated
                                at entry (out parameter)
    sal_uIntPtr &rIdx                 Index of the target entry


    [Return value]              sal_Bool: Sucess or failure

    [Cross-references]

    <SfxOrganizeListBox_Impl::MoveOrCopyTemplates(SvTreeListBox *pSourceBox,
                                            SvLBoxEntry *pSource,
                                            SvLBoxEntry* pTarget,
                                            SvLBoxEntry *&pNewParent,
                                            sal_uIntPtr &rIdx,
                                            sal_Bool bCopy)>
    <SfxOrganizeListBox_Impl::MoveOrCopyContents(SvTreeListBox *pSourceBox,
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
    SvTreeListBox* pSourceBox = GetSourceView();
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
/*  [Description]

    Notification that an item will be moved.
    (SV-Handler)

    [Parameter]

    SvLBoxEntry* pTarget        Target entry, to where it will be copied
    SvLBoxEntry *pSource        Source entry, to be copied
    SvLBoxEntry *&pNewParent    the parent of the target position generated
                                at entry (out parameter)
    sal_uIntPtr &rIdx                 Index of the target entry

    [Return value]              sal_Bool: Sucess or failure

    [Cross-references]

    <SfxOrganizeListBox_Impl::MoveOrCopyTemplates(SvTreeListBox *pSourceBox,
                                            SvLBoxEntry *pSource,
                                            SvLBoxEntry* pTarget,
                                            SvLBoxEntry *&pNewParent,
                                            sal_uIntPtr &rIdx,
                                            sal_Bool bCopy)>
    <SfxOrganizeListBox_Impl::MoveOrCopyContents(SvTreeListBox *pSourceBox,
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
    SvTreeListBox* pSourceBox = GetSourceView();
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

/*  [Description]

    Check whether an item can be edited
    (SV-Handler)

    [Cross-references]
    <SfxOrganizeListBox_Impl::EditedEntry(SvLBoxEntry* pEntry, const rtl::OUString& rText)>
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

sal_Bool SfxOrganizeListBox_Impl::EditedEntry(SvLBoxEntry* pEntry, const rtl::OUString& rText)

/*  [Description]

    The name of an entry has been processed, if the name you entered is a
    valid name (length> 0) then the model is updated.
    (SV-Handler)

    [Return value]

    sal_Bool            sal_True:  The name in the display should be changed
                        sal_False: The name should not be changed

    [Cross-references]
    <SfxOrganizeListBox_Impl::EditingEntry(SvLBoxEntry* pEntry, const String& rText)>
*/

{
    DBG_ASSERT(pEntry, "No Entry selected");
    delete pDlg->pSuspend;
    pDlg->pSuspend = NULL;
    SvLBoxEntry* pParent = GetParent(pEntry);
    if( rText.isEmpty() )
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
    SvTreeListBox *pSource = GetSourceView();
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

/*  [Description]

    Determines on which level there are documents (in the template view or
    the document view)

    [Return value]

    sal_uInt16             Document level

*/

{
    return eViewType == VIEW_FILES? 0: 1;
}

//-------------------------------------------------------------------------

SfxObjectShellRef SfxOrganizeListBox_Impl::GetObjectShell(const Path &rPath)

/*  [Description]

    Access to the ObjectShell to which the current entry is connected.

    [Parameter]

    const Path &rPath       Description of the current entry

    [Return value]

    SfxObjectShellRef       Reference to the ObjectShell

    [Cross-references]

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

void SfxOrganizeListBox_Impl::RequestingChildren( SvLBoxEntry* pEntry )

/*  [Description]

    Sent to the children of an entry that is going to be inserted.
    (SV-Handler)

    [Parameter]

    SvLBoxEntry* pEntry     the entry whose children is requested
*/

{
    if ( !GetModel()->HasChildren( pEntry ) )
    {
        WaitObject aWaitCursor( this );

        // Choose the correct mask color dependent from eColorMode. This
        // must be adopted if we change the mask color for normal images, too!
        Color aMaskColor( COL_LIGHTMAGENTA );

        // Here are all the initial inserted
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
                const sal_uInt16 nCount = aRef->GetContentCount(aPath[nDocLevel+1]);
                String aText;
                Bitmap aClosedBmp, aOpenedBmp;
                const bool bCanHaveChildren =
                    aRef->CanHaveChildren(aPath[nDocLevel+1],
                                        aPath[nDocLevel+2]);
                for(sal_uInt16 i = 0; i < nCount; ++i)
                {
                    sal_Bool bDeletable;
                    aRef->GetContent(
                        aText, aClosedBmp, aOpenedBmp, bDeletable,
                        i, aPath[nDocLevel+1]);

                    // Create image with the correct mask color
                    Image aClosedImage( aClosedBmp, aMaskColor );
                    Image aOpenedImage( aOpenedBmp, aMaskColor );

                    SvLBoxEntry *pNew = SvTreeListBox::InsertEntry(
                        aText, aOpenedImage, aClosedImage,
                        pEntry, bCanHaveChildren);
                    pNew->SetUserData(bDeletable ? &bDeletable : 0);
                }
            }
        }
    }
}

//-------------------------------------------------------------------------

long SfxOrganizeListBox_Impl::ExpandingHdl()

/*  [Description]

    SV-handler, before and after the unfolding of an entry is called.
    Used if possibly to close the ObjectShell again, the entries with the
    contents of this shell are also removed.
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
            // Release ObjectShell when closing the files
            if(eViewType == VIEW_FILES && nLevel == 0)
                pMgr->DeleteObjectShell(aPath[0]);
            else
                pMgr->DeleteObjectShell(aPath[0], aPath[1]);
            // Delete all SubEntries
            SvLBoxEntry *pToDel = SvTreeListBox::GetEntry(pEntry, 0);
            while(pToDel)
            {
                GetModel()->Remove(pToDel);
                pToDel = SvTreeListBox::GetEntry(pEntry, 0);
            }
        }
    }
    return sal_True;
}

//-------------------------------------------------------------------------

sal_Bool SfxOrganizeListBox_Impl::IsUniqName_Impl(const String &rText,
                                         SvLBoxEntry* pParent, SvLBoxEntry *pEntry) const

/*  [Description]

    Checks whether a name is unique at its level.

    [Parameter]

    const String &         Name of the search entry
    SvLBoxEntry* pSibling  Siblings (referred to the level)

    [Return value]

    sal_Bool                     sal_True, if the name is unique, otherwise sal_False
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

SvLBoxEntry* SfxOrganizeListBox_Impl::InsertEntryByBmpType(
    const XubString& rText,
    BMPTYPE eBmpType,
    SvLBoxEntry* pParent,
    sal_Bool bChildrenOnDemand,
    sal_uIntPtr nPos,
    void* pUserData
)
{
    SvLBoxEntry*    pEntry = NULL;
    const Image*    pExp = NULL;
    const Image*    pCol = NULL;

    switch( eBmpType )
    {
        case BMPTYPE_FOLDER:
            pExp = &aOpenedFolderBmp;
            pCol = &aClosedFolderBmp;
            break;

        case BMPTYPE_DOC:
            pExp = &aOpenedDocBmp;
            pCol = &aClosedDocBmp;
            break;

        default:
            OSL_FAIL( "SfxOrganizeListBox_Impl::InsertEntryByBmpType(): something forgotten?!" );
    }

    pEntry = SvTreeListBox::InsertEntry( rText, *pExp, *pCol, pParent, bChildrenOnDemand, nPos, pUserData );

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

/*  [Description]

    Constructor SfxOrganizeListBox

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
        SvTreeListBox* pSourceView = GetSourceView();
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

/*  [Description]

    Paste the items in the ListBox
*/

{
    DBG_ASSERT( pMgr != 0, "No Manager" );
    // Delete contents
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
        for ( SfxObjectList::const_iterator it = rList.begin(); it != rList.end(); ++it )
            InsertEntryByBmpType( rList.GetBaseName(*it), BMPTYPE_DOC, 0, sal_True );

    }
    SetUpdateMode(sal_True);
    Invalidate();
    Update();
}

//-------------------------------------------------------------------------

const Image &SfxOrganizeListBox_Impl::GetClosedBmp(sal_uInt16 nLevel) const

/*  [Description]

    Access to the bitmap for a closed entry at each level

    [Parameter]

    sal_uInt16 nLevel       Indicator of the level, 2 levels are allowed

    [Return value]

    const Image &       The Image on level nLevel
*/

{
    const Image*    pRet = NULL;

    switch( nLevel )
    {
        default:    OSL_FAIL( "Bitmap index overflow" );

        case 0:     pRet = &aClosedFolderBmp;
            break;
        case 1:     pRet = &aClosedDocBmp;
            break;
    }

    return *pRet;
}

//-------------------------------------------------------------------------

const Image &SfxOrganizeListBox_Impl::GetOpenedBmp(sal_uInt16 nLevel) const

/*  [Description]

    Access to the bitmap for an open entry at each level

    [Parameter]

    sal_uInt16 nLevel       Indicator of the level, 2 levels are allowed

    [Return value]

    const Image &       the image on the level nLevel

*/

{
    const Image* pRet = NULL;

    switch( nLevel )
    {
        case 0:
            pRet = &aOpenedFolderBmp; break;
        case 1:
            pRet = &aOpenedDocBmp; break;
        default:
            pRet = &aClosedFolderBmp; break;
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

/*  [Description]

    Get path from FileDialog, for Import / Export of Templates

    [Parameter]

    sal_Bool bOpen                     Flag: Open / Save
    const String& rFileName        Current file name as default

    [Return value]                 File name with path or empty string if
                                   users has pressed 'Cancel'
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
    pFileDlg->AddFilter( SfxResId(STR_SFX_FILTERNAME_ALL).toString(),
                         DEFINE_CONST_UNICODE( FILEDIALOG_FILTER_ALL ) );
    // add template filter
    String sFilterName( SfxResId(STR_TEMPLATE_FILTER).toString() );
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
    pFileDlg->AddFilter( SfxResId(STR_SFX_FILTERNAME_ALL).toString(),
                         DEFINE_CONST_UNICODE( FILEDIALOG_FILTER_ALL ) );

    // add template filter
    String sFilterName( SfxResId(STR_TEMPLATE_FILTER).toString() );
    String sFilterExt;
    // add filters of modules which are installed
    SvtModuleOptions aModuleOpt;
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SWRITER ) )
        sFilterExt += DEFINE_CONST_UNICODE( "*.ott;*.stw;*.oth;*.dot;*.dotm;*.dotx" );
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SCALC ) )
    {
        if ( sFilterExt.Len() > 0 )
            sFilterExt += ';';
        sFilterExt += DEFINE_CONST_UNICODE( "*.ots;*.stci;*.xlt;*.xltm;*.xltm" );
    }
    if ( aModuleOpt.IsModuleInstalled( SvtModuleOptions::E_SIMPRESS ) )
    {
        if ( sFilterExt.Len() > 0 )
            sFilterExt += ';';
        sFilterExt += DEFINE_CONST_UNICODE( "*.otp;*.sti;*.pot;*.potm;*.potx" );
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
       //Delete using GetContent forbidden
       nDepth==2 || // Template / Not deleting config header
       (nDepth==1 && SfxOrganizeListBox_Impl::VIEW_FILES ==
        pFocusBox->GetViewType()) || //Not deleting Files
       (0 == nDepth && pFocusBox->GetLevelCount_Impl(0) < 2))
        //At least keep one template
    {
        return sal_True;
    }

    //If delete is pressed on e.g. a style entry go up the chain to find the
    //owning template
    while (nDepth > 1)
    {
        pEntry = pFocusBox->GetParent(pEntry);
        --nDepth;
    }
    SvLBoxEntry *pTemplateEntry = pEntry;

    sal_uInt16 nRegion = 0, nIndex = 0;
    GetIndices_Impl( pFocusBox, pTemplateEntry, nRegion, nIndex );
    const SfxDocumentTemplates* pTemplates = aMgr.GetTemplates();
    if ( !pTemplates || !pTemplates->HasUserContents( nRegion, nIndex ) )
        return sal_True;

    return sal_False;
}

std::vector<rtl::OUString> SfxOrganizeDlg_Impl::GetAllFactoryURLs_Impl( ) const
{
    SvtModuleOptions aModOpt;
    const ::com::sun::star::uno::Sequence < ::rtl::OUString >& aServiceNames = aModOpt.GetAllServiceNames() ;
    std::vector<rtl::OUString> aList;
    sal_Int32 nCount = aServiceNames.getLength();
    for( sal_Int32 i=0; i<nCount; ++i )
    {
        if ( SfxObjectFactory::GetStandardTemplate( aServiceNames[i] ).Len() > 0 )
        {
            SvtModuleOptions::EFactory eFac = SvtModuleOptions::E_WRITER;
            SvtModuleOptions::ClassifyFactoryByName( aServiceNames[i], eFac );
            aList.push_back(aModOpt.GetFactoryEmptyDocumentURL(eFac));
        }
    }

    return aList;
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

/*  [Description]

    Processing the events from MenuButton or Accelerator

    [Parameter]

    sal_uInt16 nId                      Event ID

    [Return value]                  1: Event has been processed,
                                    0: Event has not been processed (SV-Menu)
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
                    const rtl::OUString aNoName(SFX2_RESSTR(STR_NONAME));
                    SvLBoxEntry* pParent = pFocusBox->GetParent(pEntry);
                    rtl::OUString aName(aNoName);
                    sal_Int32 n = 1;
                    while(!pFocusBox->IsUniqName_Impl(aName, pParent))
                    {
                        aName = aNoName + rtl::OUString::valueOf(n++);
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
            //Content Format
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

    if ( !bHandled && ( nId > ID_RESET_DEFAULT_TEMPLATE && nId <= ID_RESET_DEFAULT_TEMPLATE_END ) )
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

/*  [Description]

    SelectHandler of Menu and Menubuttons (SV)

    [Parameter]

    MenuButton *pBtn                Button triggering the event

    [Return value]                  1: Event has been processed,
                                    0: Event has not been processed (SV-Menu)
*/
{
    return Dispatch_Impl( pMenu->GetCurItemId(), pMenu );
}
IMPL_LINK_INLINE_END( SfxOrganizeDlg_Impl, MenuSelect_Impl, Menu *, pMenu )

//-------------------------------------------------------------------------

IMPL_LINK( SfxOrganizeDlg_Impl, AccelSelect_Impl, Accelerator *, pAccel )

/*  [Description]

    SelectHandler of Accelerators (SV)

    [Parameter]

    Accelerator *pAccel            Accelerator triggering the event

    [Return value]                  1: Event has been processed,
                                    0: Event has not been processed (SV)
*/

{
    SvLBoxEntry* pEntry = pFocusBox && pFocusBox->GetSelectionCount() ?
        pFocusBox->FirstSelected() : NULL ;
    return pEntry && ( pAccel->GetCurItemId() == ID_NEW  || !DontDelete_Impl( pEntry ) ) ?
        Dispatch_Impl( pAccel->GetCurItemId(), NULL ) : 0;
}

IMPL_LINK( SfxOrganizeDlg_Impl, MenuActivate_Impl, Menu *, pMenu )

/*  [Description]

    ActivateHandler of Menu and Menubuttons (SV)

    [Parameter]

    Menu *pMenu                     Event triggering the Menu

    [Return value]                  1: Event has been processed,
                                    0: Event has not been processed (SV-Menu)
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
        // Create only Template
    pMenu->EnableItem( ID_NEW, bEnable && 0 == nDepth && SfxOrganizeListBox_Impl::VIEW_TEMPLATES == eVT );
    // Template: Delete level 0,1,3ff
    //           At least one region must be retained
    // Files:    Delete level> 2

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
    std::vector<rtl::OUString> aList(GetAllFactoryURLs_Impl());
    if (!aList.empty())
    {
        PopupMenu* pSubMenu = new PopupMenu;
        sal_uInt16 nItemId = ID_RESET_DEFAULT_TEMPLATE + 1;
        for(std::vector<rtl::OUString>::const_iterator i = aList.begin(); i != aList.end(); ++i)
        {
            INetURLObject aObj(*i);
            String aTitle = SvFileInformationManager::GetDescription(aObj);
            pSubMenu->InsertItem(nItemId, aTitle, SvFileInformationManager::GetImage(aObj, false));
            pSubMenu->SetItemCommand(nItemId++, *i);
            DBG_ASSERT( nItemId <= ID_RESET_DEFAULT_TEMPLATE_END, "menu item id overflow" );
        }
        pMenu->SetPopupMenu( ID_RESET_DEFAULT_TEMPLATE, pSubMenu );
    }
    else
        bEnable = sal_False;

    pMenu->EnableItem( ID_RESET_DEFAULT_TEMPLATE, bEnable );

    return 1;
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxOrganizeDlg_Impl, GetFocus_Impl, SfxOrganizeListBox_Impl *, pBox )

/*  [Description]

    GetFocus-Handler, is called from the Select-Handler of ListBox
    Used in the listBox that has the focus and to determine its condition.

    [Parameter]

    SfxOrganizeListBox *pBox        The calling Box
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

/*  [Description]

    Select-Handler, is called from the Select-Handler of ListBox.
    If the mode of the Boxes are different (Document view or Template view)
    then the models are separated, otherwise joined together.

    [Parameter]

    ListBox *pBox               The calling Box
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

/*  [Description]

    Select-Handler, will be called by Select-Handler of the ListBox.
    If the mode of the Boxes are different (Document view or Template view)
    then the models are separated, otherwise joined together.

    [Parameter]

    ListBox *pBox               The calling Box
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
            // Separate models
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

/*  [Description]

    Handler of the button for adding files through Dialog.

    [Parameter]

    Button *                Button, triggering this Event

*/
{
    (void)pButton; //unused
    if ( pFileDlg )
        delete pFileDlg;
    pFileDlg = new sfx2::FileDialogHelper(
            ui::dialogs::TemplateDescription::FILEOPEN_READONLY_VERSION,
            0, String() );

    // add config and basic filter
    static String sOpenBracket( DEFINE_CONST_UNICODE( " (" ) );
    static String sCloseBracket( DEFINE_CONST_UNICODE( ")" ) );
    static String sConfigExt( DEFINE_CONST_UNICODE( "*.cfg" ) );
    static String sBasicExt( DEFINE_CONST_UNICODE( "*.sbl" ) );

    String sFilterName( SfxResId(RID_STR_FILTCONFIG).toString() );
    sFilterName += sOpenBracket;
    sFilterName += sConfigExt;
    sFilterName += sCloseBracket;
    pFileDlg->AddFilter( sFilterName, sConfigExt );

    sFilterName = SfxResId(RID_STR_FILTBASIC).toString();
    sFilterName += sOpenBracket;
    sFilterName += sBasicExt;
    sFilterName += sCloseBracket;
    pFileDlg->AddFilter( sFilterName, sBasicExt );

    // set "All" filter as current
    pFileDlg->SetCurrentFilter( SfxResId(STR_SFX_FILTERNAME_ALL).toString() );

    if ( aLastDir.Len() )
        pFileDlg->SetDisplayDirectory( aLastDir );

    pFileDlg->StartExecuteModal( LINK( this, SfxOrganizeDlg_Impl, AddFilesHdl ) );

    return 0;
}

//-------------------------------------------------------------------------

IMPL_LINK_NOARG(SfxOrganizeDlg_Impl, ExportHdl)
{
    DBG_ASSERT( pFileDlg, "SfxOrganizeDlg_Impl::ExportHdl(): no file dialog" );

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
            String aText( SfxResId(STR_ERROR_COPY_TEMPLATE).toString() );
            aText.SearchAndReplaceAscii( "$1", aPath );
            ErrorBox( pDialog, WB_OK, aText ).Execute();
        }
    }

    return 0L;
}

//-------------------------------------------------------------------------

IMPL_LINK_NOARG(SfxOrganizeDlg_Impl, ImportHdl)
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
           String aText( SfxResId(STR_ERROR_COPY_TEMPLATE).toString() );
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
                String aText( SfxResId(STR_ERROR_COPY_TEMPLATE).toString() );
                aText.SearchAndReplaceAscii( "$1", aPath );
                ErrorBox( pDialog, WB_OK, aText ).Execute();
            }
        }
    }

    return 0L;
}

//-------------------------------------------------------------------------

IMPL_LINK_NOARG(SfxOrganizeDlg_Impl, AddFilesHdl)
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

/*  [Description]

    Overloaded Execute method; stores changes to the document templates
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

/*  [Description]

    Constructor
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
