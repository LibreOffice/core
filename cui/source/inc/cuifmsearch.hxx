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

#ifndef INCLUDED_CUI_SOURCE_INC_CUIFMSEARCH_HXX
#define INCLUDED_CUI_SOURCE_INC_CUIFMSEARCH_HXX

#include <com/sun/star/sdbc/XResultSet.hpp>

#include <svx/fmsearch.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/combobox.hxx>
#include <vcl/lstbox.hxx>
#include <tools/link.hxx>
#include <comphelper/uno3.hxx>
#include <rtl/ustring.hxx>

namespace svxform {
    class FmSearchConfigItem;
}

struct FmSearchProgress;

class FmSearchEngine;

/// Dialog for searching in Forms/Tables
class FmSearchDialog : public ModalDialog
{
    friend class FmSearchEngine;

    // my all Controls
    VclPtr<RadioButton>     m_prbSearchForText;
    VclPtr<RadioButton>     m_prbSearchForNull;
    VclPtr<RadioButton>     m_prbSearchForNotNull;
    VclPtr<ComboBox>        m_pcmbSearchText;
    VclPtr<FixedText>       m_pftForm;
    VclPtr<ListBox>         m_plbForm;
    VclPtr<RadioButton>     m_prbAllFields;
    VclPtr<RadioButton>     m_prbSingleField;
    VclPtr<ListBox>         m_plbField;
    VclPtr<FixedText>       m_pftPosition;
    VclPtr<ListBox>         m_plbPosition;
    VclPtr<CheckBox>        m_pcbUseFormat;
    VclPtr<CheckBox>        m_pcbCase;
    VclPtr<CheckBox>        m_pcbBackwards;
    VclPtr<CheckBox>        m_pcbStartOver;
    VclPtr<CheckBox>        m_pcbWildCard;
    VclPtr<CheckBox>        m_pcbRegular;
    VclPtr<CheckBox>        m_pcbApprox;
    VclPtr<PushButton>      m_ppbApproxSettings;
    VclPtr<CheckBox>        m_pHalfFullFormsCJK;
    VclPtr<CheckBox>        m_pSoundsLikeCJK;
    VclPtr<PushButton>      m_pSoundsLikeCJKSettings;
    VclPtr<FixedText>       m_pftRecord;
    VclPtr<FixedText>       m_pftHint;
    VclPtr<PushButton>      m_pbSearchAgain;
    VclPtr<CancelButton>    m_pbClose;
    OUString        m_sSearch;
    OUString        m_sCancel;

    VclPtr<vcl::Window>         m_pPreSearchFocus;

    Link<FmFoundRecordInformation&,void>  m_lnkFoundHandler;          ///< Handler for "found"
    Link<FmFoundRecordInformation&,void>  m_lnkCanceledNotFoundHdl;   ///< Handler for Positioning the Cursors

    Link<FmSearchContext&,sal_uInt32>  m_lnkContextSupplier;       ///< for search in contexts

    /// memorize the currently selected field for every context
    ::std::vector<OUString> m_arrContextFields;

    FmSearchEngine* m_pSearchEngine;

    Timer           m_aDelayedPaint;
    // see EnableSearchUI

    ::svxform::FmSearchConfigItem*      m_pConfig;
public:
    /** This can search in different sets of fields. There is a number of contexts; their names are in strContexts (separated
        by ';'), the user can choose one of them.
        When the user chooses a context, lnkContextSupplier is called, it gets a pointer on an FmSearchContext-structure,
        that has to be filled.
        The following counts for the search :
        a) in case of formatted search the iterator itself is used (like in the first constructor)
        b) in case of formatted search NOT the FormatKey at the fields of the iterator is used, but the respective TextComponent
            is asked (that's why the original iterator is used; by its move the controls behind the TextComponent-interface are
            updated hopefully)
        c) in case of not-formatted search a clone of the iterator is used (because the TextComponent-interfaces don't need to
            be asked)
        (of course needed : the string number i in strUsedFields of a context must correspond with the interface number i in the
        arrFields of the context)
    */
    FmSearchDialog(vcl::Window* pParent, const OUString& strInitialText, const ::std::vector< OUString >& _rContexts, sal_Int16 nInitialContext,
        const Link<FmSearchContext&,sal_uInt32>& lnkContextSupplier);

    virtual ~FmSearchDialog();
    virtual void dispose() SAL_OVERRIDE;

    /** The found-handler gets in the 'found'-case a pointer on a FmFoundRecordInformation-structure
        (which is only valid in the handler; so if one needs to memorize the data, don't copy the pointer but
        the structure).
        This handler MUST be set.
        Furthermore, it should be considered, that during the handler the search-dialog is still modal.
    */
    void SetFoundHandler(const Link<FmFoundRecordInformation&,void>& lnk) { m_lnkFoundHandler = lnk; }
    /**
        If the search has been cancelled or has been finished without success, the current data set is always displayed in the
        search dialog. This handler exists to make this synchronous with the possible display of the caller (it does not
        necessarily need to be set).
        The pointer that is passed to the handler points to a FmFoundRecordInformation-structure, for which aPosition and
        possibly (in a search with contexts) nContext are valid.
    */
    void SetCanceledNotFoundHdl(const Link<FmFoundRecordInformation&,void>& lnk) { m_lnkCanceledNotFoundHdl = lnk; }

    inline void SetActiveField(const OUString& strField);

protected:
    virtual bool Close() SAL_OVERRIDE;

    void Init(const OUString& strVisibleFields, const OUString& strInitialText);
    // only to be used out of the constructors

    void OnFound(const css::uno::Any& aCursorPos, sal_Int16 nFieldPos);
    /** When searching in an own thread I naturally want to disable the UI for starting the search and for setting search
        parameters. If bEnable == sal_False, for all affected controls painting is turned off and shortly after turned on
        again using m_aDelayedPaint. If there is a demand with bEnable == sal_True inbetween, the timer is stopped and
        painting is turned on immediately. As a consequence for this intricateness there is no flickering when turning
        off and on quickly.
    */
    void EnableSearchUI(bool bEnable);

    void EnableSearchForDependees(bool bEnable);

    void EnableControlPaint(bool bEnable);

    void InitContext(sal_Int16 nContext);

    void LoadParams();
    void SaveParams() const;

private:
    // Handler for the Controls
    DECL_LINK_TYPED( OnClickedFieldRadios, Button*, void );
    DECL_LINK_TYPED(OnClickedSearchAgain, Button *, void);
    DECL_LINK_TYPED( OnClickedSpecialSettings, Button*, void );

    DECL_LINK(OnSearchTextModified, void *);

    DECL_LINK( OnPositionSelected, ListBox* );
    DECL_LINK( OnFieldSelected, ListBox* );

    DECL_LINK_TYPED( OnFocusGrabbed, Control&, void );
    DECL_LINK_TYPED( OnCheckBoxToggled, CheckBox&, void );

    DECL_LINK( OnContextSelection, ListBox* );

    DECL_LINK_TYPED( OnSearchProgress, const FmSearchProgress*, void );

    DECL_LINK_TYPED( OnDelayedPaint, Timer*, void ); ///< see EnableSearchUI

    void initCommon( const css::uno::Reference< css::sdbc::XResultSet >& _rxCursor );
};

inline void FmSearchDialog::SetActiveField(const OUString& strField)
{
    sal_Int32 nInitialField = m_plbField->GetEntryPos(strField);
    if (nInitialField == LISTBOX_ENTRY_NOTFOUND)
        nInitialField = 0;
    m_plbField->SelectEntryPos(nInitialField);
    LINK(this, FmSearchDialog, OnFieldSelected).Call(m_plbField);
}

#endif // INCLUDED_CUI_SOURCE_INC_CUIFMSEARCH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
