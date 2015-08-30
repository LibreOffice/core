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

#include "modulepcr.hxx"
#include "formresid.hrc"
#include "formstrings.hxx"
#include <vcl/msgbox.hxx>

namespace pcr
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;

    ListSelectionDialog::ListSelectionDialog(vcl::Window* _pParent, const Reference< XPropertySet >& _rxListBox,
            const OUString& _rPropertyName, const OUString& _rPropertyUIName)
        : ModalDialog( _pParent, "ListSelectDialog", "modules/spropctrlr/ui/listselectdialog.ui" )
        ,m_xListBox     ( _rxListBox     )
        ,m_sPropertyName( _rPropertyName )
    {
        OSL_PRECOND( m_xListBox.is(), "ListSelectionDialog::ListSelectionDialog: invalid list box!" );

        get(m_pEntries, "treeview");
        Size aSize(LogicToPixel(Size(85, 97), MAP_APPFONT));
        m_pEntries->set_width_request(aSize.Width());
        m_pEntries->set_height_request(aSize.Height());

        SetText(_rPropertyUIName);
        get<VclFrame>("frame")->set_label(_rPropertyUIName);

        initialize( );
    }

    ListSelectionDialog::~ListSelectionDialog()
    {
        disposeOnce();
    }

    void ListSelectionDialog::dispose()
    {
        m_pEntries.clear();
        ModalDialog::dispose();
    }

    short ListSelectionDialog::Execute()
    {
        short nResult = ModalDialog::Execute();

        if ( RET_OK == nResult )
            commitSelection();

        return nResult;
    }


    void ListSelectionDialog::initialize( )
    {
        if ( !m_xListBox.is() )
            return;

        m_pEntries->SetStyle( GetStyle() | WB_SIMPLEMODE );

        try
        {
            // initialize the multi-selection flag
            bool bMultiSelection = false;
            OSL_VERIFY( m_xListBox->getPropertyValue( PROPERTY_MULTISELECTION ) >>= bMultiSelection );
            m_pEntries->EnableMultiSelection( bMultiSelection );

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
            OSL_FAIL( "ListSelectionDialog::initialize: caught an exception!" );
        }
    }


    void ListSelectionDialog::commitSelection()
    {
        if ( !m_xListBox.is() )
            return;

        Sequence< sal_Int16 > aSelection;
        collectSelection( aSelection );

        try
        {
            m_xListBox->setPropertyValue( m_sPropertyName, makeAny( aSelection ) );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "ListSelectionDialog::commitSelection: caught an exception!" );
        }
    }


    void ListSelectionDialog::fillEntryList( const Sequence< OUString >& _rListEntries )
    {
        m_pEntries->Clear();
        const OUString* _pListEntries = _rListEntries.getConstArray();
        const OUString* _pListEntriesEnd = _rListEntries.getConstArray() + _rListEntries.getLength();
        for ( ; _pListEntries < _pListEntriesEnd; ++_pListEntries )
            m_pEntries->InsertEntry( *_pListEntries );
    }


    void ListSelectionDialog::collectSelection( Sequence< sal_Int16 >& /* [out] */ _rSelection )
    {
        const sal_Int32 nSelectedCount = m_pEntries->GetSelectEntryCount( );
        _rSelection.realloc( nSelectedCount );
        sal_Int16* pSelection = _rSelection.getArray();
        for ( sal_Int32 selected = 0; selected < nSelectedCount; ++selected, ++pSelection )
            *pSelection = static_cast< sal_Int16 >( m_pEntries->GetSelectEntryPos( selected ) );
    }


    void ListSelectionDialog::selectEntries( const Sequence< sal_Int16 >& /* [in ] */ _rSelection )
    {
        m_pEntries->SetNoSelection();
        const sal_Int16* pSelection = _rSelection.getConstArray();
        const sal_Int16* pSelectionEnd = _rSelection.getConstArray() + _rSelection.getLength();
        for ( ; pSelection != pSelectionEnd; ++pSelection )
            m_pEntries->SelectEntryPos( *pSelection );
    }


}   // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
