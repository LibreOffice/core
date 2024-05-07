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

#include "listselectiondlg.hxx"

#include "formstrings.hxx"
#include <comphelper/sequence.hxx>
#include <utility>
#include <comphelper/diagnose_ex.hxx>

namespace pcr
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;

    ListSelectionDialog::ListSelectionDialog(weld::Window* pParent, const Reference< XPropertySet >& _rxListBox,
            OUString _sPropertyName, const OUString& _rPropertyUIName)
        : GenericDialogController(pParent, u"modules/spropctrlr/ui/listselectdialog.ui"_ustr, u"ListSelectDialog"_ustr)
        , m_xListBox     ( _rxListBox     )
        , m_sPropertyName(std::move( _sPropertyName ))
        , m_xFrame(m_xBuilder->weld_frame(u"frame"_ustr))
        , m_xEntries(m_xBuilder->weld_tree_view(u"treeview"_ustr))
    {
        OSL_PRECOND( m_xListBox.is(), "ListSelectionDialog::ListSelectionDialog: invalid list box!" );

        m_xEntries->set_size_request(m_xEntries->get_approximate_digit_width() * 40, m_xEntries->get_height_rows(9));

        m_xDialog->set_title(_rPropertyUIName);
        m_xFrame->set_label(_rPropertyUIName);

        initialize( );
    }

    ListSelectionDialog::~ListSelectionDialog()
    {
    }

    short ListSelectionDialog::run()
    {
        short nResult = GenericDialogController::run();

        if ( RET_OK == nResult )
            commitSelection();

        return nResult;
    }


    void ListSelectionDialog::initialize( )
    {
        if ( !m_xListBox.is() )
            return;

        try
        {
            // initialize the multi-selection flag
            bool bMultiSelection = false;
            OSL_VERIFY( m_xListBox->getPropertyValue( PROPERTY_MULTISELECTION ) >>= bMultiSelection );
            m_xEntries->set_selection_mode(bMultiSelection ? SelectionMode::Single : SelectionMode::Multiple);

            // fill the list box with all entries
            Sequence< OUString > aListEntries;
            OSL_VERIFY( m_xListBox->getPropertyValue( PROPERTY_STRINGITEMLIST ) >>= aListEntries );
            fillEntryList( aListEntries );

            // select entries according to the property
            Sequence< sal_Int16 > aSelection;
            OSL_VERIFY( m_xListBox->getPropertyValue( m_sPropertyName ) >>= aSelection );
            selectEntries( aSelection );
        }
        catch( const Exception& )
        {
            TOOLS_WARN_EXCEPTION( "extensions.propctrlr", "ListSelectionDialog::initialize" );
        }
    }

    void ListSelectionDialog::commitSelection()
    {
        if ( !m_xListBox.is() )
            return;

        std::vector< sal_Int16 > aSelection;
        collectSelection( aSelection );

        try
        {
            m_xListBox->setPropertyValue( m_sPropertyName, Any( comphelper::containerToSequence(aSelection) ) );
        }
        catch( const Exception& )
        {
            TOOLS_WARN_EXCEPTION( "extensions.propctrlr", "ListSelectionDialog::commitSelection" );
        }
    }

    void ListSelectionDialog::fillEntryList( const Sequence< OUString >& _rListEntries )
    {
        m_xEntries->freeze();
        m_xEntries->clear();
        for (auto const & entry : _rListEntries)
            m_xEntries->append_text(entry);
        m_xEntries->thaw();
    }

    void ListSelectionDialog::collectSelection( std::vector< sal_Int16 >& /* [out] */ _rSelection )
    {
        auto aSelection = m_xEntries->get_selected_rows();
        _rSelection.resize(aSelection.size());
        for (auto row : aSelection)
            _rSelection.push_back(row);
    }

    void ListSelectionDialog::selectEntries( const Sequence< sal_Int16 >& /* [in ] */ _rSelection )
    {
        m_xEntries->unselect_all();
        for (auto const & selection : _rSelection)
            m_xEntries->select(selection);
    }


}   // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
