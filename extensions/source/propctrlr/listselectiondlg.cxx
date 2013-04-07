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
#include "listselectiondlg.hrc"

#include "modulepcr.hxx"
#include "formresid.hrc"
#include "formstrings.hxx"
#include <vcl/msgbox.hxx>

//........................................................................
namespace pcr
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;

    //====================================================================
    //= ListSelectionDialog
    //====================================================================
    //--------------------------------------------------------------------
    ListSelectionDialog::ListSelectionDialog( Window* _pParent, const Reference< XPropertySet >& _rxListBox,
            const OUString& _rPropertyName, const String& _rPropertyUIName )
        :ModalDialog( _pParent, PcrRes( RID_DLG_SELECTION ) )
        ,m_aLabel   ( this, PcrRes( FT_ENTRIES ) )
        ,m_aEntries ( this, PcrRes( LB_ENTRIES ) )
        ,m_aOK      ( this, PcrRes( PB_OK      ) )
        ,m_aCancel  ( this, PcrRes( PB_CANCEL  ) )
        ,m_aHelp    ( this, PcrRes( PB_HELP    ) )
        ,m_xListBox     ( _rxListBox     )
        ,m_sPropertyName( _rPropertyName )
    {
        FreeResource();

        OSL_PRECOND( m_xListBox.is(), "ListSelectionDialog::ListSelectionDialog: invalid list box!" );

        SetText( _rPropertyUIName );
        m_aLabel.SetText( _rPropertyUIName );

        initialize( );
    }

    //------------------------------------------------------------------------
    short ListSelectionDialog::Execute()
    {
        short nResult = ModalDialog::Execute();

        if ( RET_OK == nResult )
            commitSelection();

        return nResult;
    }

    //--------------------------------------------------------------------
    void ListSelectionDialog::initialize( )
    {
        if ( !m_xListBox.is() )
            return;

        m_aEntries.SetStyle( GetStyle() | WB_SIMPLEMODE );

        try
        {
            // initialize the multi-selection flag
            sal_Bool bMultiSelection = sal_False;
            OSL_VERIFY( m_xListBox->getPropertyValue( PROPERTY_MULTISELECTION ) >>= bMultiSelection );
            m_aEntries.EnableMultiSelection( bMultiSelection );

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

    //--------------------------------------------------------------------
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

    //--------------------------------------------------------------------
    void ListSelectionDialog::fillEntryList( const Sequence< OUString >& _rListEntries )
    {
        m_aEntries.Clear();
        const OUString* _pListEntries = _rListEntries.getConstArray();
        const OUString* _pListEntriesEnd = _rListEntries.getConstArray() + _rListEntries.getLength();
        for ( ; _pListEntries < _pListEntriesEnd; ++_pListEntries )
            m_aEntries.InsertEntry( *_pListEntries );
    }

    //--------------------------------------------------------------------
    void ListSelectionDialog::collectSelection( Sequence< sal_Int16 >& /* [out] */ _rSelection )
    {
        sal_uInt16 nSelectedCount = m_aEntries.GetSelectEntryCount( );
        _rSelection.realloc( nSelectedCount );
        sal_Int16* pSelection = _rSelection.getArray();
        for ( sal_uInt16 selected = 0; selected < nSelectedCount; ++selected, ++pSelection )
            *pSelection = static_cast< sal_Int16 >( m_aEntries.GetSelectEntryPos( selected ) );
    }

    //--------------------------------------------------------------------
    void ListSelectionDialog::selectEntries( const Sequence< sal_Int16 >& /* [in ] */ _rSelection )
    {
        m_aEntries.SetNoSelection();
        const sal_Int16* pSelection = _rSelection.getConstArray();
        const sal_Int16* pSelectionEnd = _rSelection.getConstArray() + _rSelection.getLength();
        for ( ; pSelection != pSelectionEnd; ++pSelection )
            m_aEntries.SelectEntryPos( *pSelection );
    }

//........................................................................
}   // namespace pcr
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
