/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dinfdlg.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 14:57:08 $
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

#include <com/sun/star/util/DateTime.hpp>

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

#include "tabdlg.hxx"


namespace com { namespace sun { namespace star {
    namespace document {
        class XDocumentInfo;
        class XDocumentProperties;
    }
} } }


// class SfxDocumentInfoItem ---------------------------------------------

class SFX2_DLLPUBLIC SfxDocumentInfoItem : public SfxStringItem
{
private:
    sal_Int32                           m_AutoloadDelay;
    ::rtl::OUString                     m_AutoloadURL;
    sal_Bool                            m_isAutoloadEnabled;
    ::rtl::OUString                     m_DefaultTarget;
    ::rtl::OUString                     m_TemplateName;
    ::rtl::OUString                     m_Author;
    ::com::sun::star::util::DateTime    m_CreationDate;
    ::rtl::OUString                     m_ModifiedBy;
    ::com::sun::star::util::DateTime    m_ModificationDate;
    String                              m_PrintedBy;
    ::com::sun::star::util::DateTime    m_PrintDate;
    sal_Int16                           m_EditingCycles;
    sal_Int32                           m_EditingDuration;
    ::rtl::OUString                     m_Description;
    ::rtl::OUString                     m_Keywords;
    ::rtl::OUString                     m_Subject;
    ::rtl::OUString                     m_Title;
    ::rtl::OUString                     m_UserDefinedFieldTitles[4];
    ::rtl::OUString                     m_UserDefinedFieldValues[4];
    sal_Bool                            bHasTemplate;
    sal_Bool                            bDeleteUserData;
    sal_Bool                            bIsUseUserData;

public:
    TYPEINFO();
    SfxDocumentInfoItem();
//FIXME: remove XDocumentInfo when implementing "Custom" tab
    SfxDocumentInfoItem( const String &rFileName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::document::XDocumentProperties> & i_xDocProps,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::document::XDocumentInfo> & i_xDocInfo,
        sal_Bool bUseUserData );
    SfxDocumentInfoItem( const SfxDocumentInfoItem& );
    virtual ~SfxDocumentInfoItem();

//FIXME: remove XDocumentInfo when implementing "Custom" tab
    /// update i_xDocProps with the data in this object
    void updateDocumentInfo(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::document::XDocumentProperties> & i_xDocProps,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::document::XDocumentInfo> & i_xDocInfo) const;

    sal_Bool    isAutoloadEnabled() const { return m_isAutoloadEnabled; }
    void        setAutoloadEnabled(sal_Bool i_val) { m_isAutoloadEnabled = i_val; }
    sal_Int32   getAutoloadDelay() const { return m_AutoloadDelay; }
    void        setAutoloadDelay(sal_Int32 i_val) { m_AutoloadDelay = i_val; }
    ::rtl::OUString getAutoloadURL() const { return m_AutoloadURL; }
    void        setAutoloadURL(::rtl::OUString i_val) { m_AutoloadURL = i_val; }
    ::rtl::OUString getDefaultTarget() const { return m_DefaultTarget; }
    void        setDefaultTarget(::rtl::OUString i_val) { m_DefaultTarget = i_val; }
    ::rtl::OUString getTemplateName() const { return m_TemplateName; }
    void        setTemplateName(::rtl::OUString i_val) { m_TemplateName = i_val; }
    ::rtl::OUString getAuthor() const { return m_Author; }
    void        setAuthor(::rtl::OUString i_val) { m_Author = i_val; }

    ::com::sun::star::util::DateTime
                getCreationDate() const { return m_CreationDate; }
    void        setCreationDate(::com::sun::star::util::DateTime i_val) {
                    m_CreationDate = i_val;
                }
    ::rtl::OUString getModifiedBy() const { return m_ModifiedBy; }
    void        setModifiedBy(::rtl::OUString i_val) { m_ModifiedBy = i_val; }

    ::com::sun::star::util::DateTime
                getModificationDate() const { return m_ModificationDate; }
    void        setModificationDate(::com::sun::star::util::DateTime i_val) {
                    m_ModificationDate = i_val;
                }
    ::rtl::OUString getPrintedBy() const { return m_PrintedBy; }
    void        setPrintedBy(::rtl::OUString i_val) { m_PrintedBy = i_val; }
    ::com::sun::star::util::DateTime
                getPrintDate() const { return m_PrintDate; }
    void        setPrintDate(::com::sun::star::util::DateTime i_val) {
                    m_PrintDate = i_val;
                }
    sal_Int16   getEditingCycles() const { return m_EditingCycles; }
    void        setEditingCycles(sal_Int16 i_val) { m_EditingCycles = i_val; }
    sal_Int32   getEditingDuration() const { return m_EditingDuration; }
    void        setEditingDuration(sal_Int32 i_val) { m_EditingDuration = i_val; }
    ::rtl::OUString getDescription() const { return m_Description; }
    void        setDescription(::rtl::OUString i_val) { m_Description = i_val; }
    ::rtl::OUString getKeywords() const { return m_Keywords; }
    void        setKeywords(::rtl::OUString i_val) { m_Keywords = i_val; }
    ::rtl::OUString getSubject() const { return m_Subject; }
    void        setSubject(::rtl::OUString i_val) { m_Subject = i_val; }
    ::rtl::OUString getTitle() const { return m_Title; }
    void        setTitle(::rtl::OUString i_val) { m_Title = i_val; }
    ::rtl::OUString getUserDefinedFieldTitle(size_t i_ix) const;
    void        setUserDefinedFieldTitle(size_t i_ix, ::rtl::OUString i_val);
    ::rtl::OUString getUserDefinedFieldValue(size_t i_ix) const;
    void        setUserDefinedFieldValue(size_t i_ix, ::rtl::OUString i_val);

    /// reset user-specific data (author, modified-by, ...)
    void        resetUserData(const ::rtl::OUString & i_rAuthor);

    void                    SetTemplate( BOOL b ) { bHasTemplate = b; }
    FASTBOOL                HasTemplate() const { return bHasTemplate; }
    void                    SetDeleteUserData( BOOL bSet );
    void                    SetUseUserData( BOOL bSet );
    BOOL                    IsDeleteUserData() const;
    BOOL                    IsUseUserData() const;

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = NULL ) const;
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );
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

    DECL_LINK(          DeleteHdl, PushButton * );
    DECL_LINK(          SignatureHdl, PushButton * );
    void                ImplUpdateSignatures();

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

#if _SOLAR__PRIVATE
    DECL_LINK( EditLabelHdl, PushButton * );

    String              GetLabelText_Impl( FixedText* pLabel );
    void                SetLabelText_Impl( FixedText* pLabel, const String& rNewLabel );
#endif

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

