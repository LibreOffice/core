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
#ifndef INCLUDED_SFX2_DINFDLG_HXX
#define INCLUDED_SFX2_DINFDLG_HXX

#include <sal/config.h>
#include <sfx2/dllapi.h>

#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Duration.hpp>
#include <com/sun/star/document/CmisProperty.hpp>

#include <svl/stritem.hxx>
#include <svl/zforlist.hxx>

#include <vcl/headbar.hxx>
#include <svtools/svmedit.hxx>

#include <unotools/syslocale.hxx>

#include <vcl/edit.hxx>
#include <vcl/field.hxx>
#include <vcl/idle.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/fixed.hxx>
#include <vcl/ctrl.hxx>

#include <sfx2/tabdlg.hxx>

#include <boost/optional/optional.hpp>
#include <memory>

namespace com { namespace sun { namespace star {
    namespace document {
        class XDocumentProperties;
    }
} } }

struct CustomProperty;

// class SfxDocumentInfoItem ---------------------------------------------

class SFX2_DLLPUBLIC SfxDocumentInfoItem : public SfxStringItem
{
private:
    sal_Int32                    m_AutoloadDelay;
    OUString                     m_AutoloadURL;
    bool                         m_isAutoloadEnabled;
    OUString                     m_DefaultTarget;
    OUString const               m_TemplateName;
    OUString                     m_Author;
    css::util::DateTime          m_CreationDate;
    OUString                     m_ModifiedBy;
    css::util::DateTime          m_ModificationDate;
    OUString                     m_PrintedBy;
    css::util::DateTime          m_PrintDate;
    sal_Int16                    m_EditingCycles;
    sal_Int32                    m_EditingDuration;
    OUString                     m_Description;
    OUString                     m_Keywords;
    OUString                     m_Subject;
    OUString                     m_Title;
    bool                         m_bHasTemplate;
    bool                         m_bDeleteUserData;
    bool                         m_bUseUserData;
    bool                         m_bUseThumbnailSave;
    std::vector< std::unique_ptr<CustomProperty> >    m_aCustomProperties;
    css::uno::Sequence< css::document::CmisProperty > m_aCmisProperties;

public:
    static SfxPoolItem* CreateDefault();
    SfxDocumentInfoItem();
    SfxDocumentInfoItem( const OUString &rFileName,
        const css::uno::Reference< css::document::XDocumentProperties> & i_xDocProps,
        const css::uno::Sequence< css::document::CmisProperty> & i_cmisProps,
        bool bUseUserData, bool bUseThumbnailSave );
    SfxDocumentInfoItem( const SfxDocumentInfoItem& );
    virtual ~SfxDocumentInfoItem() override;

    /// update i_xDocProps with the data in this object
    void UpdateDocumentInfo(
        const css::uno::Reference< css::document::XDocumentProperties> & i_xDocProps,
        bool i_bDoNotUpdateUserDefined = false)
        const;
    bool        isCmisDocument() const { return m_aCmisProperties.getLength() > 0;}

    bool        isAutoloadEnabled() const { return m_isAutoloadEnabled; }
    void        setAutoloadEnabled(bool i_val) { m_isAutoloadEnabled = i_val; }
    sal_Int32   getAutoloadDelay() const { return m_AutoloadDelay; }
    void        setAutoloadDelay(sal_Int32 i_val) { m_AutoloadDelay = i_val; }
    const OUString& getAutoloadURL() const { return m_AutoloadURL; }
    void        setAutoloadURL(const OUString& i_val) { m_AutoloadURL = i_val; }
    const OUString& getDefaultTarget() const { return m_DefaultTarget; }
    void        setDefaultTarget(const OUString& i_val) { m_DefaultTarget = i_val; }
    const OUString& getTemplateName() const { return m_TemplateName; }
    const OUString& getAuthor() const { return m_Author; }
    void        setAuthor(const OUString& i_val) { m_Author = i_val; }

    const css::util::DateTime&
                getCreationDate() const { return m_CreationDate; }
    const OUString& getModifiedBy() const { return m_ModifiedBy; }
    void        setModifiedBy(const OUString& i_val) { m_ModifiedBy = i_val; }

    const css::util::DateTime&
                getModificationDate() const { return m_ModificationDate; }
    const OUString& getPrintedBy() const { return m_PrintedBy; }
    void        setPrintedBy(const OUString& i_val) { m_PrintedBy = i_val; }
    const css::util::DateTime&
                getPrintDate() const { return m_PrintDate; }
    sal_Int16   getEditingCycles() const { return m_EditingCycles; }
    void        setEditingCycles(sal_Int16 i_val) { m_EditingCycles = i_val; }
    sal_Int32   getEditingDuration() const { return m_EditingDuration; }
    void        setEditingDuration(sal_Int32 i_val) { m_EditingDuration = i_val; }
    const OUString& getDescription() const { return m_Description; }
    void        setDescription(const OUString& i_val) { m_Description = i_val; }
    const OUString& getKeywords() const { return m_Keywords; }
    void        setKeywords(const OUString& i_val) { m_Keywords = i_val; }
    const OUString& getSubject() const { return m_Subject; }
    void        setSubject(const OUString& i_val) { m_Subject = i_val; }
    const OUString& getTitle() const { return m_Title; }
    void        setTitle(const OUString& i_val) { m_Title = i_val; }

    /// reset user-specific data (author, modified-by, ...)
    void        resetUserData(const OUString & i_rAuthor);

    void        SetTemplate( bool b ) { m_bHasTemplate = b; }
    bool        HasTemplate() const { return m_bHasTemplate; }
    void        SetDeleteUserData( bool bSet );
    void        SetUseUserData( bool bSet );
    void        SetUseThumbnailSave( bool bSet );
    bool        IsDeleteUserData() const { return m_bDeleteUserData;}
    bool        IsUseUserData() const { return m_bUseUserData;}
    bool        IsUseThumbnailSave() const { return m_bUseThumbnailSave;}


    std::vector< std::unique_ptr<CustomProperty> > GetCustomProperties() const;
    void        ClearCustomProperties();
    void        AddCustomProperty(  const OUString& sName,
                                    const css::uno::Any& rValue );

    const css::uno::Sequence< css::document::CmisProperty >&
                        GetCmisProperties() const { return m_aCmisProperties;}

    void        SetCmisProperties(const css::uno::Sequence< css::document::CmisProperty >& cmisProps );
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = nullptr ) const override;
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;
};

// class SfxDocumentPage -------------------------------------------------

class SfxDocumentPage : public SfxTabPage
{
private:
    OUString                    m_aUnknownSize;
    OUString                    m_aMultiSignedStr;

    bool                        bEnableUseUserData  : 1,
                                bHandleDelete       : 1;

    std::unique_ptr<weld::Image> m_xBmp;
    std::unique_ptr<weld::Label> m_xNameED;
    std::unique_ptr<weld::Button> m_xChangePassBtn;

    std::unique_ptr<weld::Label> m_xShowTypeFT;
    std::unique_ptr<weld::Label> m_xFileValEd;
    std::unique_ptr<weld::Label> m_xShowSizeFT;

    std::unique_ptr<weld::Label> m_xCreateValFt;
    std::unique_ptr<weld::Label> m_xChangeValFt;
    std::unique_ptr<weld::Label> m_xSignedValFt;
    std::unique_ptr<weld::Button> m_xSignatureBtn;
    std::unique_ptr<weld::Label> m_xPrintValFt;
    std::unique_ptr<weld::Label> m_xTimeLogValFt;
    std::unique_ptr<weld::Label> m_xDocNoValFt;

    std::unique_ptr<weld::CheckButton> m_xUseUserDataCB;
    std::unique_ptr<weld::Button> m_xDeleteBtn;
    std::unique_ptr<weld::CheckButton> m_xUseThumbnailSaveCB;

    std::unique_ptr<weld::Label> m_xTemplFt;
    std::unique_ptr<weld::Label> m_xTemplValFt;

    DECL_LINK(DeleteHdl, weld::Button&, void);
    DECL_LINK(SignatureHdl, weld::Button&, void);
    DECL_LINK(ChangePassHdl, weld::Button&, void);
    void                ImplUpdateSignatures();
    void                ImplCheckPasswordState();

protected:
    virtual ~SfxDocumentPage() override;

    virtual bool        FillItemSet( SfxItemSet* ) override;
    virtual void        Reset( const SfxItemSet* ) override;

public:
    SfxDocumentPage(TabPageParent pParent, const SfxItemSet&);
    static VclPtr<SfxTabPage> Create( TabPageParent pParent, const SfxItemSet* );

    void                EnableUseUserData();
};

// class SfxDocumentDescPage ---------------------------------------------

class SfxDocumentDescPage : public SfxTabPage
{
private:
    SfxDocumentInfoItem* m_pInfoItem;
    std::unique_ptr<weld::Entry> m_xTitleEd;
    std::unique_ptr<weld::Entry> m_xThemaEd;
    std::unique_ptr<weld::Entry> m_xKeywordsEd;
    std::unique_ptr<weld::TextView> m_xCommentEd;

protected:
    virtual ~SfxDocumentDescPage() override;

    virtual bool            FillItemSet( SfxItemSet* ) override;
    virtual void            Reset( const SfxItemSet* ) override;

public:
    SfxDocumentDescPage(TabPageParent pParent, const SfxItemSet&);
    static VclPtr<SfxTabPage> Create( TabPageParent pParent, const SfxItemSet* );
};

// class SfxDocumentInfoDialog -------------------------------------------

class SFX2_DLLPUBLIC SfxDocumentInfoDialog : public SfxTabDialog
{
private:
    sal_uInt16 m_nDocInfoId;
protected:
    virtual void    PageCreated( sal_uInt16 nId, SfxTabPage& rPage ) override;

public:
    SfxDocumentInfoDialog( vcl::Window* pParent, const SfxItemSet& );
    void AddFontTabPage();
};

// class CustomPropertiesRemoveButton ------------------------------------
struct CustomPropertyLine;

class CustomPropertiesEdit : public Edit
{
private:
    CustomPropertyLine* const       m_pLine;

public:
    CustomPropertiesEdit(vcl::Window* pParent, WinBits nStyle, CustomPropertyLine* pLine)
        : Edit(pParent, nStyle)
        , m_pLine(pLine)
    {
    }

    CustomPropertyLine*      GetLine() const { return m_pLine; }
};

class CustomPropertiesTypeBox : public ListBox
{
private:
    CustomPropertyLine* const       m_pLine;

public:
    CustomPropertiesTypeBox(vcl::Window* pParent, CustomPropertyLine* pLine);
    CustomPropertyLine*      GetLine() const { return m_pLine; }
};

class CustomPropertiesDateField : public DateField
{
public:
    ::boost::optional<sal_Int16> m_TZ;

    CustomPropertiesDateField(vcl::Window* pParent, WinBits nStyle)
        : DateField(pParent, nStyle)
    {
    }
};

class CustomPropertiesTimeField : public TimeField
{
public:
    bool m_isUTC;

    CustomPropertiesTimeField(vcl::Window* pParent, WinBits nStyle)
        : TimeField(pParent, nStyle)
        , m_isUTC(false)
    {
    }
};

class CustomPropertiesDurationField : public Edit
{
    CustomPropertyLine* const       m_pLine;
    css::util::Duration             m_aDuration;
protected:
    virtual void    RequestHelp(const HelpEvent& rEvt) override;
public:
    CustomPropertiesDurationField(vcl::Window* pParent, WinBits nStyle, CustomPropertyLine* pLine);

    void SetDuration( const css::util::Duration& rDuration );
    const css::util::Duration& GetDuration() const { return m_aDuration; }
};

class CustomPropertiesEditButton : public PushButton
{
    CustomPropertyLine*             m_pLine;

public:
    CustomPropertiesEditButton(vcl::Window* pParent, WinBits nStyle, CustomPropertyLine* pLine);

    DECL_LINK(ClickHdl, Button*, void);
};

class CustomPropertiesRemoveButton : public ImageButton
{
private:
    CustomPropertyLine* const       m_pLine;

public:
    CustomPropertiesRemoveButton(vcl::Window* pParent, WinBits nStyle, CustomPropertyLine* pLine)
        : ImageButton(pParent, nStyle)
        , m_pLine(pLine)
    {
    }

    CustomPropertyLine*      GetLine() const { return m_pLine; }
};

class CustomPropertiesYesNoButton : public Control
{
private:
    VclPtr<RadioButton>             m_aYesButton;
    VclPtr<RadioButton>             m_aNoButton;

public:
    CustomPropertiesYesNoButton(vcl::Window* pParent);
    virtual ~CustomPropertiesYesNoButton() override;
    virtual void dispose() override;

    virtual void    Resize() override;

    void     CheckYes() { m_aYesButton->Check(); }
    void     CheckNo() { m_aNoButton->Check(); }
    bool     IsYesChecked() const { return m_aYesButton->IsChecked(); }
};

// struct CustomPropertyLine ---------------------------------------------

struct CustomPropertyLine
{
    ScopedVclPtr<VclGrid>                       m_aLine;
    ScopedVclPtr<ComboBox>                      m_aNameBox;
    ScopedVclPtr<CustomPropertiesTypeBox>       m_aTypeBox;
    ScopedVclPtr<CustomPropertiesEdit>          m_aValueEdit;
    ScopedVclPtr<CustomPropertiesDateField>     m_aDateField;
    ScopedVclPtr<CustomPropertiesTimeField>     m_aTimeField;
    const OUString                              m_sDurationFormat;
    ScopedVclPtr<CustomPropertiesDurationField> m_aDurationField;
    ScopedVclPtr<CustomPropertiesEditButton>    m_aEditButton;
    ScopedVclPtr<CustomPropertiesYesNoButton>   m_aYesNoButton;
    ScopedVclPtr<CustomPropertiesRemoveButton>  m_aRemoveButton;

    bool                                        m_bTypeLostFocus;

    CustomPropertyLine( vcl::Window* pParent );
    void Clear();
    void Hide();
};

// class CustomPropertiesWindow ------------------------------------------

class CustomPropertiesWindow : public vcl::Window
{
private:
    VclPtr<HeaderBar>                   m_pHeaderBar;
    VclPtr<ScrollBar>                   m_pScrollBar;
    VclPtr<FixedText>                   m_pHeaderAccName;
    VclPtr<FixedText>                   m_pHeaderAccType;
    VclPtr<FixedText>                   m_pHeaderAccValue;

    sal_Int32                           m_nWidgetHeight;
    sal_Int32                           m_nRemoveButtonWidth;
    sal_Int32                           m_nTypeBoxWidth;
    sal_Int32                           m_nLineHeight;
    sal_Int32                           m_nScrollPos;
    std::vector<std::unique_ptr<CustomProperty>> m_aCustomProperties;
    std::vector<std::unique_ptr<CustomPropertyLine>> m_aCustomPropertiesLines;
    CustomPropertyLine*                 m_pCurrentLine;
    SvNumberFormatter                   m_aNumberFormatter;
    Idle                                m_aEditLoseFocusIdle;
    Idle                                m_aBoxLoseFocusIdle;
    Link<void*,void>                    m_aRemovedHdl;

    DECL_LINK(TypeHdl, ListBox&, void);
    DECL_LINK(RemoveHdl, Button*, void);
    DECL_LINK(EditLoseFocusHdl, Control&, void);
    DECL_LINK(BoxLoseFocusHdl, Control&, void);
    //add lose focus handlers of Date/TimeField?

    DECL_LINK(EditTimeoutHdl, Timer *, void);
    DECL_LINK(BoxTimeoutHdl, Timer *, void);

    bool        IsLineValid( CustomPropertyLine* pLine ) const;
    void        ValidateLine( CustomPropertyLine* pLine, bool bIsFromTypeBox );
    void        CreateNewLine();
    void        ReloadLinesContent();
    void        StoreCustomProperties();
    sal_uInt32  GetCurrentDataModelPosition() const { return -1 * m_nScrollPos / m_nLineHeight; }

public:
    CustomPropertiesWindow(vcl::Window* pParent,
        FixedText *pHeaderAccName,
        FixedText *pHeaderAccType,
        FixedText *pHeaderAccValue);
    void Init(HeaderBar* pHeaderBar, ScrollBar* pScrollBar);
    virtual ~CustomPropertiesWindow() override;
    virtual void dispose() override;

    virtual void Resize() override;
    //these consts are unhelpful, this changes the state of the widgets
    //that belong to CustomPropertyLine, but they are held by VclPtr
    //and operator-> on a VclPtr is a const method that returns the
    //non-const contents of the VclPtr, but loplugin:constparams
    //correctly sees that it could all be set to const, so we end
    //up with this unhappy situation
    void                SetWidgetWidths(const CustomPropertyLine* pLine) const;
    sal_uInt16          GetExistingLineCount() const { return m_aCustomPropertiesLines.size(); }
    sal_uInt16          GetTotalLineCount() const { return m_aCustomProperties.size(); }
    sal_uInt16          GetVisibleLineCount() const;
    void                SetVisibleLineCount(sal_uInt32 nCount);
    sal_Int32           GetLineHeight() const { return m_nLineHeight; }
    void                AddLine( const OUString& sName, css::uno::Any const & rAny );
    bool                AreAllLinesValid() const;
    void                ClearAllLines();
    void                DoScroll( sal_Int32 nNewPos );

    css::uno::Sequence< css::beans::PropertyValue >
                        GetCustomProperties();
    void                SetCustomProperties(std::vector< std::unique_ptr<CustomProperty> >&& rProperties);
    void                SetRemovedHdl( const Link<void*,void>& rLink ) { m_aRemovedHdl = rLink; }
};

// class CustomPropertiesControl -----------------------------------------

class CustomPropertiesControl : public vcl::Window
{
private:
    VclPtr<VclVBox>                m_pVBox;
    VclPtr<HeaderBar>              m_pHeaderBar;
    VclPtr<VclHBox>                m_pBody;
    VclPtr<CustomPropertiesWindow> m_pPropertiesWin;
    VclPtr<ScrollBar>              m_pVertScroll;

    sal_Int32               m_nThumbPos;

    DECL_LINK( ScrollHdl, ScrollBar*, void );
    DECL_LINK( RemovedHdl, void*, void );

public:
    CustomPropertiesControl(vcl::Window* pParent);
    virtual ~CustomPropertiesControl() override;
    virtual void dispose() override;

    void         AddLine(css::uno::Any const & rAny);

    bool         AreAllLinesValid() const { return m_pPropertiesWin->AreAllLinesValid(); }
    void         ClearAllLines() { m_pPropertiesWin->ClearAllLines(); }

    css::uno::Sequence<css::beans::PropertyValue>
                 GetCustomProperties() const
                        { return m_pPropertiesWin->GetCustomProperties(); }
    void         SetCustomProperties(std::vector< std::unique_ptr<CustomProperty> >&& rProperties);

    void         Init(VclBuilderContainer& rParent);
    virtual void Resize() override;
};

// class SfxCustomPropertiesPage -----------------------------------------

class SfxCustomPropertiesPage : public SfxTabPage
{
private:
    VclPtr<CustomPropertiesControl> m_pPropertiesCtrl;

    DECL_LINK(AddHdl, Button*, void);

    using TabPage::DeactivatePage;

protected:
    virtual ~SfxCustomPropertiesPage() override;
    virtual void dispose() override;

    virtual bool        FillItemSet( SfxItemSet* ) override;
    virtual void        Reset( const SfxItemSet* ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

public:
    SfxCustomPropertiesPage( vcl::Window* pParent, const SfxItemSet& );
    static VclPtr<SfxTabPage> Create( TabPageParent pParent, const SfxItemSet* );
};

struct CmisValue : public VclBuilderContainer
{
    VclPtr<Edit>   m_aValueEdit;

    CmisValue( vcl::Window* pParent, const OUString& aStr );
};

struct CmisDateTime : public VclBuilderContainer
{
    VclPtr<DateField>  m_aDateField;
    VclPtr<TimeField>  m_aTimeField;

    CmisDateTime( vcl::Window* pParent, const css::util::DateTime& aDateTime );
};

struct CmisYesNo : public VclBuilderContainer
{
    VclPtr<RadioButton> m_aYesButton;
    VclPtr<RadioButton> m_aNoButton;

    CmisYesNo( vcl::Window* pParent, bool bValue);
};

// struct CmisPropertyLine ---------------------------------------------

struct CmisPropertyLine : public VclBuilderContainer
{
    VclPtr<VclFrame>              m_pFrame;
    OUString                      m_sId;
    OUString                      m_sType;
    bool                          m_bUpdatable;
    bool                          m_bRequired;
    bool                          m_bMultiValued;
    bool                          m_bOpenChoice;
    VclPtr<FixedText>             m_aName;
    VclPtr<FixedText>             m_aType;
    std::vector< std::unique_ptr<CmisValue> >     m_aValues;
    std::vector< std::unique_ptr<CmisDateTime> >  m_aDateTimes;
    std::vector< std::unique_ptr<CmisYesNo> >     m_aYesNos;
    long getItemHeight() const;
    CmisPropertyLine( vcl::Window* pParent );
    virtual ~CmisPropertyLine() override;
};

// class CmisPropertiesWindow ------------------------------------------

class CmisPropertiesWindow
{
private:
    VclPtr<VclBox>                      m_pBox;
    sal_Int32                           m_nItemHeight;
    SvNumberFormatter                   m_aNumberFormatter;
    std::vector< std::unique_ptr<CmisPropertyLine> > m_aCmisPropertiesLines;
public:
    CmisPropertiesWindow(SfxTabPage* pParent);
    ~CmisPropertiesWindow();

    sal_Int32 GetItemHeight() const { return m_nItemHeight; }
    long getBoxHeight() const { return VclContainer::getLayoutRequisition(*m_pBox).Height(); };
    void AddLine( const OUString& sId, const OUString& sName,
                  const OUString& sType, const bool bUpdatable,
                  const bool bRequired, const bool bMultiValued,
                  const bool bOpenChoice,
                  css::uno::Any& aChoices,
                  css::uno::Any const & rAny );
    void ClearAllLines();
    void DoScroll( sal_Int32 nNewPos );

    css::uno::Sequence< css::document::CmisProperty >
                        GetCmisProperties() const;
};

// class CmisPropertiesControl -----------------------------------------

class CmisPropertiesControl
{
private:
    CmisPropertiesWindow    m_pPropertiesWin;
    VclScrolledWindow&      m_rScrolledWindow;
    ScrollBar&              m_rVertScroll;
    DECL_LINK( ScrollHdl, ScrollBar*, void );

    void checkAutoVScroll();

public:
    CmisPropertiesControl(SfxTabPage* pParent);
    void setScrollRange();

    void AddLine( const OUString& sId, const OUString& sName,
                  const OUString& sType, const bool bUpdatable,
                  const bool bRequired, const bool bMultiValude,
                  const bool bOpenChoice,
                  css::uno::Any& aChoices,
                  css::uno::Any const & rAny );

    void ClearAllLines();
    css::uno::Sequence< css::document::CmisProperty >
                    GetCmisProperties() const
                        { return m_pPropertiesWin.GetCmisProperties(); }
};

// class SfxCmisPropertiesPage -------------------------------------------------

class SfxCmisPropertiesPage : public SfxTabPage
{
private:
    CmisPropertiesControl m_pPropertiesCtrl;
    using TabPage::DeactivatePage;

protected:
    virtual bool        FillItemSet( SfxItemSet* ) override;
    virtual void        Reset( const SfxItemSet* ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

public:
    SfxCmisPropertiesPage( vcl::Window* pParent, const SfxItemSet& );
    virtual ~SfxCmisPropertiesPage() override;
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create( TabPageParent pParent, const SfxItemSet* );
    virtual void SetPosSizePixel(const Point& rAllocPos, const Size& rAllocation) override;
    virtual void SetSizePixel(const Size& rAllocation) override;
    virtual void SetPosPixel(const Point& rAllocPos) override;
};

#endif // #ifndef _ INCLUDED_SFX2_DINFDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
