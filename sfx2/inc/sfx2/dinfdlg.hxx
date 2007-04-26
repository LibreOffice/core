/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dinfdlg.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-26 10:33:25 $
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
#ifndef _SFX_DINFDLG_HXX
#define _SFX_DINFDLG_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#ifndef _SV_EDIT_HXX //autogen wg. Edit
#include <vcl/edit.hxx>
#endif
#ifndef _SV_FIELD_HXX //autogen wg. NumericField
#include <vcl/field.hxx>
#endif
#ifndef _STDCTRL_HXX //autogen wg. FixedText, FixedInfo
#include <svtools/stdctrl.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen wg. SfxStringItem
#include <svtools/stritem.hxx>
#endif
#ifndef _SVEDIT_HXX //autogen wg. MultiLineEdit
#include <svtools/svmedit.hxx>
#endif

#include <sfx2/tabdlg.hxx>
#include <sfx2/docinf.hxx>

// class SfxDocumentInfoItem ---------------------------------------------

class SFX2_DLLPUBLIC SfxDocumentInfoItem : public SfxStringItem
{
private:
    SfxDocumentInfo         aDocInfo;
    FASTBOOL                bHasTemplate;
    FASTBOOL                bOwnFormat;

public:
    TYPEINFO();
    SfxDocumentInfoItem();
    SfxDocumentInfoItem( const String &rFileName, const SfxDocumentInfo &, BOOL );
    SfxDocumentInfoItem( const String &rFileName, const SfxDocumentInfo & );
    SfxDocumentInfoItem( const SfxDocumentInfoItem& );
    virtual ~SfxDocumentInfoItem();

    BOOL                    IsOwnFormat() const;

    void                    SetTemplate( BOOL b ) { bHasTemplate = b; }
    FASTBOOL                HasTemplate() const { return bHasTemplate; }
    void                    SetDeleteUserData( BOOL bSet );
    BOOL                    IsDeleteUserData() const;

    SfxDocumentInfo&        operator()() { return aDocInfo; }
    const SfxDocumentInfo&  operator()() const { return aDocInfo; }
    SfxDocumentInfo&        GetDocInfo() { return aDocInfo; }
    const SfxDocumentInfo&  GetDocInfo() const { return aDocInfo; }

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = NULL ) const;
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );
};

// class SfxDocumentPage -------------------------------------------------

class SfxDocumentPage : public SfxTabPage
{
private:
    FixedImage                  aBmp1;
    Edit                        aNameED;

    FixedLine                   aLine1FL;
    FixedText                   aTypeFT;
    svt::SelectableFixedText    aShowTypeFT;
    CheckBox                    aReadOnlyCB;
    FixedText                   aFileFt;
    svt::SelectableFixedText    aFileValFt;
    FixedText                   aSizeFT;
    svt::SelectableFixedText    aShowSizeFT;

    FixedLine                   aLine2FL;
    FixedText                   aCreateFt;
    svt::SelectableFixedText    aCreateValFt;
    FixedText                   aChangeFt;
    svt::SelectableFixedText    aChangeValFt;
    FixedText                   aSignedFt;
    svt::SelectableFixedText    aSignedValFt;
    PushButton                  aSignatureBtn;
    FixedText                   aPrintFt;
    svt::SelectableFixedText    aPrintValFt;
    FixedText                   aTimeLogFt;
    svt::SelectableFixedText    aTimeLogValFt;
    FixedText                   aDocNoFt;
    svt::SelectableFixedText    aDocNoValFt;
    CheckBox                    aUseUserDataCB;
    PushButton                  aDeleteBtn;

    FixedLine                   aLine3FL;
    FixedText                   aTemplFt;
    svt::SelectableFixedText    aTemplValFt;

    String                      aUnknownSize;
    String                      aMultiSignedStr;

    BOOL                        bEnableUseUserData  : 1,
                                bHandleDelete       : 1;

//#if 0 // _SOLAR__PRIVATE
    DECL_LINK(          DeleteHdl, PushButton * );
    DECL_LINK(          SignatureHdl, PushButton * );
    void                ImplUpdateSignatures();
//#endif

protected:
    SfxDocumentPage( Window* pParent, const SfxItemSet& );

    virtual BOOL        FillItemSet( SfxItemSet& );
    virtual void        Reset( const SfxItemSet& );

public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& );

    void                EnableUseUserData();
};

// class SfxDocumentDescPage ---------------------------------------------

class SfxDocumentDescPage : public SfxTabPage
{
private:
    FixedText               aTitleFt;
    Edit                    aTitleEd;
    FixedText               aThemaFt;
    Edit                    aThemaEd;
    FixedText               aKeywordsFt;
    Edit                    aKeywordsEd;
    FixedText               aCommentFt;
    MultiLineEdit           aCommentEd;
    SfxDocumentInfoItem*    pInfoItem;

protected:
    SfxDocumentDescPage( Window* pParent, const SfxItemSet& );

    virtual BOOL            FillItemSet( SfxItemSet& );
    virtual void            Reset( const SfxItemSet& );

public:
    static SfxTabPage*      Create( Window* pParent, const SfxItemSet& );
};

// class SfxDocumentUserPage ---------------------------------------------

class SfxDocumentUserPage : public SfxTabPage
{
private:
    BOOL                    bLabelModified;

    FixedText               aInfo1Ft;
    Edit                    aInfo1Ed;
    FixedText               aInfo2Ft;
    Edit                    aInfo2Ed;
    FixedText               aInfo3Ft;
    Edit                    aInfo3Ed;
    FixedText               aInfo4Ft;
    Edit                    aInfo4Ed;
    PushButton              aEditLabelBtn;
    SfxDocumentInfoItem*    pInfoItem;

//#if 0 // _SOLAR__PRIVATE
    DECL_LINK( EditLabelHdl, PushButton * );

    String              GetLabelText_Impl( FixedText* pLabel );
    void                SetLabelText_Impl( FixedText* pLabel, const String& rNewLabel );
//#endif

protected:
    SfxDocumentUserPage( Window* pParent, const SfxItemSet& );

    virtual BOOL        FillItemSet( SfxItemSet& );
    virtual void        Reset( const SfxItemSet& );

public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& );
};

// class SfxInternetPage -------------------------------------------------

class TargetList;
namespace sfx2
{
    class FileDialogHelper;
}

class SfxInternetPage : public SfxTabPage
{
private:
    RadioButton             aRBNoAutoUpdate;
    RadioButton             aRBReloadUpdate;
    RadioButton             aRBForwardUpdate;

    FixedText               aFTEvery;
    NumericField            aNFReload;
    FixedText               aFTReloadSeconds;

    FixedText               aFTAfter;
    NumericField            aNFAfter;
    FixedText               aFTAfterSeconds;
    FixedText               aFTURL;
    Edit                    aEDForwardURL;
    PushButton              aPBBrowseURL;
    FixedText               aFTFrame;
    ComboBox                aCBFrame;

    String                  aForwardErrorMessg;
    String                  aBaseURL;
    SfxDocumentInfoItem*    pInfoItem;
    sfx2::FileDialogHelper* pFileDlg;

    enum STATE              { S_Init, S_NoUpdate, S_Reload, S_Forward };
                            // S_Init is only valid as initial value
    STATE                   eState;

    void                    ChangeState( STATE eNewState );     // S_Init is not a valid value here
                                                                // also checks corresponding radiobutton
    void                    EnableNoUpdate( BOOL bEnable );
    void                    EnableReload( BOOL bEnable );
    void                    EnableForward( BOOL bEnable );

    DECL_LINK( ClickHdlNoUpdate, Control* );
    DECL_LINK( ClickHdlReload, Control* );
    DECL_LINK( ClickHdlForward, Control* );
    DECL_LINK( ClickHdlBrowseURL, PushButton* );
    DECL_LINK( DialogClosedHdl, sfx2::FileDialogHelper* );

        using TabPage::DeactivatePage;

protected:
    SfxInternetPage( Window* pParent, const SfxItemSet& );
    ~SfxInternetPage();

    virtual BOOL            FillItemSet( SfxItemSet& );
    virtual void            Reset( const SfxItemSet& );
    virtual int                     DeactivatePage( SfxItemSet* pSet = 0 );

public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& );
};

// class SfxDocumentInfoDialog -------------------------------------------

class SFX2_DLLPUBLIC SfxDocumentInfoDialog : public SfxTabDialog
{
protected:
    virtual void    PageCreated( USHORT nId, SfxTabPage& rPage );

public:
    SfxDocumentInfoDialog(  Window* pParent, const SfxItemSet& );
};

#endif

