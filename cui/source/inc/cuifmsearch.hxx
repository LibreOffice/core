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
#include <rtl/ustring.hxx>

namespace svxform {
    class FmSearchConfigItem;
}

struct FmSearchProgress;

class FmSearchEngine;

/// Dialog for searching in Forms/Tables
class FmSearchDialog final : public weld::GenericDialogController
{
    friend class FmSearchEngine;

    OUString        m_sSearch;
    OUString        m_sCancel;

    Link<FmFoundRecordInformation&,void>  m_lnkFoundHandler;          ///< Handler for "found"
    Link<FmFoundRecordInformation&,void>  m_lnkCanceledNotFoundHdl;   ///< Handler for Positioning the Cursors

    Link<FmSearchContext&,sal_uInt32>  m_lnkContextSupplier;       ///< for search in contexts

    /// memorize the currently selected field for every context
    std::vector<OUString> m_arrContextFields;

    std::unique_ptr<FmSearchEngine> m_pSearchEngine;

    // see EnableSearchUI
    std::unique_ptr<::svxform::FmSearchConfigItem>      m_pConfig;

    // my all Controls
    std::unique_ptr<weld::RadioButton> m_prbSearchForText;
    std::unique_ptr<weld::RadioButton> m_prbSearchForNull;
    std::unique_ptr<weld::RadioButton> m_prbSearchForNotNull;
    std::unique_ptr<weld::ComboBox> m_pcmbSearchText;
    std::unique_ptr<weld::Label> m_pftForm;
    std::unique_ptr<weld::ComboBox> m_plbForm;
    std::unique_ptr<weld::RadioButton> m_prbAllFields;
    std::unique_ptr<weld::RadioButton> m_prbSingleField;
    std::unique_ptr<weld::ComboBox> m_plbField;
    std::unique_ptr<weld::Label> m_pftPosition;
    std::unique_ptr<weld::ComboBox> m_plbPosition;
    std::unique_ptr<weld::CheckButton> m_pcbUseFormat;
    std::unique_ptr<weld::CheckButton> m_pcbCase;
    std::unique_ptr<weld::CheckButton> m_pcbBackwards;
    std::unique_ptr<weld::CheckButton> m_pcbStartOver;
    std::unique_ptr<weld::CheckButton> m_pcbWildCard;
    std::unique_ptr<weld::CheckButton> m_pcbRegular;
    std::unique_ptr<weld::CheckButton> m_pcbApprox;
    std::unique_ptr<weld::Button> m_ppbApproxSettings;
    std::unique_ptr<weld::CheckButton> m_pHalfFullFormsCJK;
    std::unique_ptr<weld::CheckButton> m_pSoundsLikeCJK;
    std::unique_ptr<weld::Button> m_pSoundsLikeCJKSettings;
    std::unique_ptr<weld::Label> m_pftRecord;
    std::unique_ptr<weld::Label> m_pftHint;
    std::unique_ptr<weld::Button> m_pbSearchAgain;
    std::unique_ptr<weld::Button> m_pbClose;

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
    FmSearchDialog(weld::Window* pParent, const OUString& strInitialText, const std::vector< OUString >& _rContexts, sal_Int16 nInitialContext,
        const Link<FmSearchContext&,sal_uInt32>& lnkContextSupplier);

    virtual short run() override;

    virtual ~FmSearchDialog() override;

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

private:
    void Init(const OUString& strVisibleFields, const OUString& strInitialText);
    // only to be used out of the constructors

    void OnFound(const css::uno::Any& aCursorPos, sal_Int16 nFieldPos);

    void EnableSearchUI(bool bEnable);

    void EnableSearchForDependees(bool bEnable);

    void InitContext(sal_Int16 nContext);

    void LoadParams();
    void SaveParams() const;

    // Handler for the Controls
    DECL_LINK( OnClickedFieldRadios, weld::Button&, void );
    DECL_LINK( OnClickedSearchAgain, weld::Button&, void );
    DECL_LINK( OnClickedSpecialSettings, weld::Button&, void );

    DECL_LINK( OnSearchTextModified, weld::ComboBox&, void );

    DECL_LINK( OnPositionSelected, weld::ComboBox&, void );
    DECL_LINK( OnFieldSelected, weld::ComboBox&, void );

    DECL_LINK( OnFocusGrabbed, weld::Widget&, void );
    DECL_LINK( OnCheckBoxToggled, weld::ToggleButton&, void );

    DECL_LINK( OnContextSelection, weld::ComboBox&, void );

    DECL_LINK( OnSearchProgress, const FmSearchProgress*, void );

    void initCommon( const css::uno::Reference< css::sdbc::XResultSet >& _rxCursor );
};

inline void FmSearchDialog::SetActiveField(const OUString& strField)
{
    int nInitialField = m_plbField->find_text(strField);
    if (nInitialField == -1)
        nInitialField = 0;
    m_plbField->set_active(nInitialField);
    OnFieldSelected(*m_plbField);
}

#endif // INCLUDED_CUI_SOURCE_INC_CUIFMSEARCH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
