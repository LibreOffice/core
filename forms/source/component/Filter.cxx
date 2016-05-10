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

#include "Filter.hxx"
#include "FormComponent.hxx"
#include "frm_resource.hrc"
#include "frm_resource.hxx"
#include "property.hrc"
#include "property.hxx"
#include "services.hxx"

#include <com/sun/star/awt/VclWindowPeerAttribute.hpp>
#include <com/sun/star/awt/XCheckBox.hpp>
#include <com/sun/star/awt/XComboBox.hpp>
#include <com/sun/star/awt/XListBox.hpp>
#include <com/sun/star/awt/XRadioButton.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/sdb/BooleanComparisonMode.hpp>
#include <com/sun/star/sdb/ErrorMessageDialog.hpp>
#include <com/sun/star/sdb/XColumn.hpp>
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/util/NumberFormatter.hpp>
#include <com/sun/star/awt/XItemList.hpp>

#include <comphelper/numbers.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/property.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <connectivity/dbconversion.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/formattedcolumnvalue.hxx>
#include <connectivity/predicateinput.hxx>
#include <rtl/ustrbuf.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/localedatawrapper.hxx>
#include <vcl/stdtext.hxx>
#include <vcl/svapp.hxx>
#include <tools/wintypes.hxx>


namespace frm
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::ui::dialogs;

    using namespace ::connectivity;

    OFilterControl::OFilterControl( const Reference< XComponentContext >& _rxORB )
        :UnoControl()
        ,m_aTextListeners( *this )
        ,m_xContext( _rxORB )
#if HAVE_FEATURE_DBCONNECTIVITY
        ,m_aParser( _rxORB )
#endif
        ,m_nControlClass( FormComponentType::TEXTFIELD )
        ,m_bFilterList( false )
        ,m_bMultiLine( false )
        ,m_bFilterListFilled( false )
    {
    }


    bool OFilterControl::ensureInitialized( )
    {
#if HAVE_FEATURE_DBCONNECTIVITY
        if ( !m_xField.is() )
        {
            OSL_FAIL( "OFilterControl::ensureInitialized: improperly initialized: no field!" );
            return false;
        }

        if ( !m_xConnection.is() )
        {
            OSL_FAIL( "OFilterControl::ensureInitialized: improperly initialized: no connection!" );
            return false;
        }

        if ( !m_xFormatter.is() )
        {
            // we can create one from the connection, if it's an SDB connection

            Reference< XNumberFormatsSupplier > xFormatSupplier = ::dbtools::getNumberFormats( m_xConnection, true, m_xContext );

            if ( xFormatSupplier.is() )
            {
                m_xFormatter.set(NumberFormatter::create(m_xContext), UNO_QUERY_THROW );
                m_xFormatter->attachNumberFormatsSupplier( xFormatSupplier );
            }
        }
        if ( !m_xFormatter.is() )
        {
            OSL_FAIL( "OFilterControl::ensureInitialized: no number formatter!" );
            // no fallback anymore
            return false;
        }
#endif
        return true;
    }


    Any SAL_CALL OFilterControl::queryAggregation( const Type & rType ) throw(RuntimeException, std::exception)
    {
        Any aRet = UnoControl::queryAggregation( rType);
        if(!aRet.hasValue())
            aRet = OFilterControl_BASE::queryInterface(rType);

        return aRet;
    }


    OUString OFilterControl::GetComponentServiceName()
    {
        OUString aServiceName;
        switch (m_nControlClass)
        {
            case FormComponentType::RADIOBUTTON:
                aServiceName = "radiobutton";
                break;
            case FormComponentType::CHECKBOX:
                aServiceName = "checkbox";
                break;
            case FormComponentType::COMBOBOX:
                aServiceName = "combobox";
                break;
            case FormComponentType::LISTBOX:
                aServiceName = "listbox";
                break;
            default:
                if (m_bMultiLine)
                    aServiceName = "MultiLineEdit";
                else
                    aServiceName = "Edit";
        }
        return aServiceName;
    }

    // XComponent

    void OFilterControl::dispose() throw( RuntimeException, std::exception  )
    {
        EventObject aEvt(*this);
        m_aTextListeners.disposeAndClear( aEvt );
        UnoControl::dispose();
    }


    void OFilterControl::createPeer( const Reference< XToolkit > & rxToolkit, const Reference< XWindowPeer >  & rParentPeer ) throw(RuntimeException, std::exception)
    {
        UnoControl::createPeer( rxToolkit, rParentPeer );

        try
        {
            Reference< XVclWindowPeer >  xVclWindow( getPeer(), UNO_QUERY_THROW );
            switch ( m_nControlClass )
            {
                case FormComponentType::CHECKBOX:
                {
                    // checkboxes always have a tristate-mode
                    xVclWindow->setProperty( PROPERTY_TRISTATE, makeAny( true ) );
                    xVclWindow->setProperty( PROPERTY_STATE, makeAny( sal_Int32( TRISTATE_INDET ) ) );

                    Reference< XCheckBox >  xBox( getPeer(), UNO_QUERY_THROW );
                    xBox->addItemListener( this );

                }
                break;

                case FormComponentType::RADIOBUTTON:
                {
                    xVclWindow->setProperty( PROPERTY_STATE, makeAny( sal_Int32( TRISTATE_FALSE ) ) );

                    Reference< XRadioButton >  xRadio( getPeer(), UNO_QUERY_THROW );
                    xRadio->addItemListener( this );
                }
                break;

                case FormComponentType::LISTBOX:
                {
                    Reference< XListBox >  xListBox( getPeer(), UNO_QUERY_THROW );
                    xListBox->addItemListener( this );
                    SAL_FALLTHROUGH;
                }

                case FormComponentType::COMBOBOX:
                {
                    xVclWindow->setProperty(PROPERTY_AUTOCOMPLETE, makeAny( true ) );
                    SAL_FALLTHROUGH;
                }

                default:
                {
                    Reference< XWindow >  xWindow( getPeer(), UNO_QUERY );
                    xWindow->addFocusListener( this );

                    Reference< XTextComponent >  xText( getPeer(), UNO_QUERY );
                    if (xText.is())
                        xText->setMaxTextLen(0);
                }
                break;
            }

            // filter controls are _never_ readonly
            Reference< XPropertySet > xModel( getModel(), UNO_QUERY_THROW );
            Reference< XPropertySetInfo > xModelPSI( xModel->getPropertySetInfo(), UNO_SET_THROW );
            if ( xModelPSI->hasPropertyByName( PROPERTY_READONLY ) )
                xVclWindow->setProperty( PROPERTY_READONLY, makeAny( false ) );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        if (m_bFilterList)
            m_bFilterListFilled = false;
    }


    void OFilterControl::PrepareWindowDescriptor( WindowDescriptor& rDescr )
    {
        if (m_bFilterList)
            rDescr.WindowAttributes |= VclWindowPeerAttribute::DROPDOWN;
    }


    void OFilterControl::ImplSetPeerProperty( const OUString& rPropName, const Any& rVal )
    {
        // these properties are ignored
        if (rPropName == PROPERTY_TEXT ||
            rPropName == PROPERTY_STATE)
            return;

        UnoControl::ImplSetPeerProperty( rPropName, rVal );
    }

    // XEventListener

    void SAL_CALL OFilterControl::disposing(const EventObject& Source) throw( RuntimeException, std::exception )
    {
        UnoControl::disposing(Source);
    }

    // XItemListener

    void SAL_CALL OFilterControl::itemStateChanged( const ItemEvent& rEvent ) throw(RuntimeException, std::exception)
    {
#if !HAVE_FEATURE_DBCONNECTIVITY
        (void) rEvent;
#else
        OUStringBuffer aText;
        switch (m_nControlClass)
        {
            case FormComponentType::CHECKBOX:
            {
                if ( ( rEvent.Selected == TRISTATE_TRUE ) || ( rEvent.Selected == TRISTATE_FALSE ) )
                {
                    sal_Int32 nBooleanComparisonMode = ::dbtools::DatabaseMetaData( m_xConnection ).getBooleanComparisonMode();

                    bool bSelected = ( rEvent.Selected == TRISTATE_TRUE );

                    OUString sExpressionMarker( "$expression$" );
                    ::dbtools::getBooleanComparisonPredicate(
                        sExpressionMarker,
                        bSelected,
                        nBooleanComparisonMode,
                        aText
                    );

                    OUString sText( aText.makeStringAndClear() );
                    sal_Int32 nMarkerPos( sText.indexOf( sExpressionMarker ) );
                    OSL_ENSURE( nMarkerPos == 0, "OFilterControl::itemStateChanged: unsupported boolean comparison mode!" );
                    // If this assertion fails, then getBoleanComparisonPredicate created a predicate which
                    // does not start with the expression we gave it. The only known case is when
                    // the comparison mode is ACCESS_COMPAT, and the value is TRUE. In this case,
                    // the expression is rather complex.
                    // Well, so this is a known issue - the filter controls (and thus the form based filter)
                    // do not work with boolean MS Access fields.
                    // To fix this, we would probably have to revert here to always return "1" or "0" as normalized
                    // filter, and change our client code to properly translate this (which could be some effort).
                    if ( nMarkerPos == 0 )
                        aText.append( sText.copy( sExpressionMarker.getLength() ) );
                    else
                    {
                        // fallback
                        aText.appendAscii( bSelected ? "1" : "0" );
                    }
                }
            }
            break;

            case FormComponentType::LISTBOX:
            {
                try
                {
                    const Reference< XItemList > xItemList( getModel(), UNO_QUERY_THROW );
                    OUString sItemText( xItemList->getItemText( rEvent.Selected ) );

                    const MapString2String::const_iterator itemPos = m_aDisplayItemToValueItem.find( sItemText );
                    if ( itemPos != m_aDisplayItemToValueItem.end() )
                    {
                        sItemText = itemPos->second;
                        if ( !sItemText.isEmpty() )
                        {
                            ::dbtools::OPredicateInputController aPredicateInput( m_xContext, m_xConnection, getParseContext() );
                            OUString sErrorMessage;
                            OSL_VERIFY( aPredicateInput.normalizePredicateString( sItemText, m_xField, &sErrorMessage ) );
                        }
                    }
                    aText.append( sItemText );
                }
                catch( const Exception& )
                {
                    DBG_UNHANDLED_EXCEPTION();
                }
            }
            break;

            case FormComponentType::RADIOBUTTON:
            {
                if ( rEvent.Selected == TRISTATE_TRUE )
                    aText.append( ::comphelper::getString( Reference< XPropertySet >( getModel(), UNO_QUERY )->getPropertyValue( PROPERTY_REFVALUE ) ) );
            }
            break;
        }

        OUString sText( aText.makeStringAndClear() );
        if ( m_aText.compareTo( sText ) )
        {
            m_aText = sText;
            TextEvent aEvt;
            aEvt.Source = *this;
            ::cppu::OInterfaceIteratorHelper aIt( m_aTextListeners );
            while( aIt.hasMoreElements() )
                static_cast<XTextListener *>(aIt.next())->textChanged( aEvt );
        }
#endif
    }


    void OFilterControl::implInitFilterList()
    {
#if HAVE_FEATURE_DBCONNECTIVITY
        if ( !ensureInitialized( ) )
            // already asserted in ensureInitialized
            return;

        // ensure the cursor and the statement are disposed as soon as we leave
        ::utl::SharedUNOComponent< XResultSet > xListCursor;
        ::utl::SharedUNOComponent< XStatement > xStatement;

        try
        {
            m_bFilterListFilled = true;

            if ( !m_xField.is() )
                return;

            OUString sFieldName;
            m_xField->getPropertyValue( PROPERTY_NAME ) >>= sFieldName;

            // here we need a table to which the field belongs to
            const Reference< XChild > xModelAsChild( getModel(), UNO_QUERY_THROW );
            const Reference< XRowSet > xForm( xModelAsChild->getParent(), UNO_QUERY_THROW );
            const Reference< XPropertySet > xFormProps( xForm, UNO_QUERY_THROW );

            // create a query composer
            Reference< XColumnsSupplier > xSuppColumns;
            xFormProps->getPropertyValue("SingleSelectQueryComposer") >>= xSuppColumns;

            const Reference< XConnection > xConnection( ::dbtools::getConnection( xForm ), UNO_SET_THROW );
            const Reference< XNameAccess > xFieldNames( xSuppColumns->getColumns(), UNO_SET_THROW );
            if ( !xFieldNames->hasByName( sFieldName ) )
                return;
            OUString sRealFieldName, sTableName;
            const Reference< XPropertySet > xComposerFieldProps( xFieldNames->getByName( sFieldName ), UNO_QUERY_THROW );
            xComposerFieldProps->getPropertyValue( PROPERTY_REALNAME ) >>= sRealFieldName;
            xComposerFieldProps->getPropertyValue( PROPERTY_TABLENAME ) >>= sTableName;

            // obtain the table of the field
            const Reference< XTablesSupplier > xSuppTables( xSuppColumns, UNO_QUERY_THROW );
            const Reference< XNameAccess > xTablesNames( xSuppTables->getTables(), UNO_SET_THROW );
            const Reference< XNamed > xNamedTable( xTablesNames->getByName( sTableName ), UNO_QUERY_THROW );
            sTableName = xNamedTable->getName();

            // create a statement selecting all values for the given field
            OUStringBuffer aStatement;

            const Reference< XDatabaseMetaData >  xMeta( xConnection->getMetaData(), UNO_SET_THROW );
            const OUString sQuoteChar = xMeta->getIdentifierQuoteString();

            aStatement.append( "SELECT DISTINCT " );
            aStatement.append( sQuoteChar );
            aStatement.append( sRealFieldName );
            aStatement.append( sQuoteChar );

            // if the field had an alias in our form's statement, give it this alias in the new statement, too
            if ( !sFieldName.isEmpty() && ( sFieldName != sRealFieldName ) )
            {
                aStatement.append(" AS ");
                aStatement.append( sQuoteChar );
                aStatement.append( sFieldName );
                aStatement.append( sQuoteChar );
            }

            aStatement.append( " FROM " );

            OUString sCatalog, sSchema, sTable;
            ::dbtools::qualifiedNameComponents( xMeta, sTableName, sCatalog, sSchema, sTable, ::dbtools::eInDataManipulation );
            aStatement.append( ::dbtools::composeTableNameForSelect( xConnection, sCatalog, sSchema, sTable ) );

            // execute the statement
            xStatement.reset( xConnection->createStatement() );
            const OUString sSelectStatement( aStatement.makeStringAndClear( ) );
            xListCursor.reset( xStatement->executeQuery( sSelectStatement ) );

            // retrieve the one column which we take the values from
            const Reference< XColumnsSupplier > xSupplyCols( xListCursor, UNO_QUERY_THROW );
            const Reference< XIndexAccess > xFields( xSupplyCols->getColumns(), UNO_QUERY_THROW );
            const Reference< XPropertySet > xDataField( xFields->getByIndex(0), UNO_QUERY_THROW );

            // ensure the values will be  formatted according to the field format
            const ::dbtools::FormattedColumnValue aFormatter( m_xFormatter, xDataField );

            ::std::vector< OUString > aProposals;
            aProposals.reserve(16);

            while ( xListCursor->next() && ( aProposals.size() < size_t( SHRT_MAX ) ) )
            {
                const OUString sCurrentValue = aFormatter.getFormattedValue();
                aProposals.push_back( sCurrentValue );
            }

            // fill the list items into our peer
            Sequence< OUString> aStringSeq( comphelper::containerToSequence(aProposals) );

            const Reference< XComboBox >  xComboBox( getPeer(), UNO_QUERY_THROW );
            xComboBox->addItems( aStringSeq, 0 );

            // set the drop down line count to something reasonable
            const sal_Int16 nLineCount = ::std::min( sal_Int16( 16 ), sal_Int16( aStringSeq.getLength() ) );
            xComboBox->setDropDownLineCount( nLineCount );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
#endif
    }

    // XFocusListener

    void SAL_CALL OFilterControl::focusGained(const FocusEvent& /*e*/)  throw( RuntimeException, std::exception  )
    {
        // should we fill the combobox?
        if (m_bFilterList && !m_bFilterListFilled)
            implInitFilterList();
    }


    void SAL_CALL OFilterControl::focusLost(const FocusEvent& /*e*/) throw( RuntimeException, std::exception )
    {
    }


    sal_Bool SAL_CALL OFilterControl::commit() throw(RuntimeException, std::exception)
    {
#if HAVE_FEATURE_DBCONNECTIVITY
        if ( !ensureInitialized( ) )
            // already asserted in ensureInitialized
            return sal_True;

        OUString aText;
        switch (m_nControlClass)
        {
            case FormComponentType::TEXTFIELD:
            case FormComponentType::COMBOBOX:
            {
                Reference< XTextComponent >  xText( getPeer(), UNO_QUERY );
                if (xText.is())
                    aText = xText->getText();
            }   break;
            default:
                return sal_True;
        }
        if (m_aText.compareTo(aText))
        {
            // check the text with the SQL-Parser
            OUString aNewText(aText);
            aNewText = aNewText.trim();
            if ( !aNewText.isEmpty() )
            {
                ::dbtools::OPredicateInputController aPredicateInput( m_xContext, m_xConnection, getParseContext() );
                OUString sErrorMessage;
                if ( !aPredicateInput.normalizePredicateString( aNewText, m_xField, &sErrorMessage ) )
                {
                    // display the error and outta here
                    SQLContext aError;
                    aError.Message = FRM_RES_STRING( RID_STR_SYNTAXERROR );
                    aError.Details = sErrorMessage;
                    displayException( aError );
                    return sal_False;
                }
            }

            setText(aNewText);
            TextEvent aEvt;
            aEvt.Source = *this;
            ::cppu::OInterfaceIteratorHelper aIt( m_aTextListeners );
            while( aIt.hasMoreElements() )
                static_cast< XTextListener* >( aIt.next() )->textChanged( aEvt );
        }
#endif
        return sal_True;
    }

    // XTextComponent

    void SAL_CALL OFilterControl::addTextListener(const Reference< XTextListener > & l) throw(RuntimeException, std::exception)
    {
        m_aTextListeners.addInterface( l );
    }


    void SAL_CALL OFilterControl::removeTextListener(const Reference< XTextListener > & l) throw(RuntimeException, std::exception)
    {
        m_aTextListeners.removeInterface( l );
    }


    void SAL_CALL OFilterControl::setText( const OUString& aText ) throw(RuntimeException, std::exception)
    {
        if ( !ensureInitialized( ) )
            // already asserted in ensureInitialized
            return;

        switch (m_nControlClass)
        {
            case FormComponentType::CHECKBOX:
            {
                Reference< XVclWindowPeer >  xVclWindow( getPeer(), UNO_QUERY );
                if (xVclWindow.is())
                {
                    Any aValue;
                    if  (   aText == "1"
                        ||  aText.equalsIgnoreAsciiCase("TRUE")
                        ||  aText.equalsIgnoreAsciiCase("IS TRUE")
                        )
                    {
                        aValue <<= (sal_Int32)TRISTATE_TRUE;
                    }
                    else if ( aText == "0" || aText.equalsIgnoreAsciiCase("FALSE") )
                    {
                        aValue <<= (sal_Int32)TRISTATE_FALSE;
                    }
                    else
                        aValue <<= (sal_Int32)TRISTATE_INDET;

                    m_aText = aText;
                    xVclWindow->setProperty( PROPERTY_STATE, aValue );
                }
            }   break;
            case FormComponentType::RADIOBUTTON:
            {
                Reference< XVclWindowPeer >  xVclWindow( getPeer(), UNO_QUERY );
                if (xVclWindow.is())
                {
                    OUString aRefText = ::comphelper::getString(css::uno::Reference< XPropertySet > (getModel(), UNO_QUERY)->getPropertyValue(PROPERTY_REFVALUE));
                    Any aValue;
                    if (aText == aRefText)
                        aValue <<= (sal_Int32)TRISTATE_TRUE;
                    else
                        aValue <<= (sal_Int32)TRISTATE_FALSE;
                    m_aText = aText;
                    xVclWindow->setProperty(PROPERTY_STATE, aValue);
                }
            }   break;
            case FormComponentType::LISTBOX:
            {
                Reference< XListBox >  xListBox( getPeer(), UNO_QUERY );
                if (xListBox.is())
                {
                    m_aText = aText;
                    MapString2String::const_iterator itemPos = m_aDisplayItemToValueItem.find( m_aText );
                    if ( itemPos == m_aDisplayItemToValueItem.end() )
                    {
                        const bool isQuoted =   ( m_aText.getLength() > 1 )
                                            &&  ( m_aText[0] == '\'' )
                                            &&  ( m_aText[ m_aText.getLength() - 1 ] == '\'' );
                        if ( isQuoted )
                        {
                            m_aText = m_aText.copy( 1, m_aText.getLength() - 2 );
                            itemPos = m_aDisplayItemToValueItem.find( m_aText );
                        }
                    }

                    OSL_ENSURE( ( itemPos != m_aDisplayItemToValueItem.end() ) || m_aText.isEmpty(),
                        "OFilterControl::setText: this text is not in my display list!" );
                    if ( itemPos == m_aDisplayItemToValueItem.end() )
                        m_aText.clear();

                    if ( m_aText.isEmpty() )
                    {
                        while ( xListBox->getSelectedItemPos() >= 0 )
                        {
                            xListBox->selectItemPos( xListBox->getSelectedItemPos(), sal_False );
                        }
                    }
                    else
                    {
                        xListBox->selectItem( m_aText, sal_True );
                    }
                }
            }
            break;

            default:
            {
                Reference< XTextComponent >  xText( getPeer(), UNO_QUERY );
                if (xText.is())
                {
                    m_aText = aText;
                    xText->setText(aText);
                }
            }
        }
    }


    void SAL_CALL OFilterControl::insertText( const css::awt::Selection& rSel, const OUString& aText ) throw(css::uno::RuntimeException, std::exception)
    {
        Reference< XTextComponent >  xText( getPeer(), UNO_QUERY );
        if (xText.is())
        {
            xText->insertText(rSel, aText);
            m_aText = xText->getText();
        }
    }


    OUString SAL_CALL OFilterControl::getText() throw(RuntimeException, std::exception)
    {
        return m_aText;
    }


    OUString SAL_CALL OFilterControl::getSelectedText() throw(RuntimeException, std::exception)
    {
        OUString aSelected;
        Reference< XTextComponent >  xText( getPeer(), UNO_QUERY );
        if (xText.is())
            aSelected = xText->getSelectedText();

        return aSelected;
    }


    void SAL_CALL OFilterControl::setSelection( const css::awt::Selection& aSelection ) throw(css::uno::RuntimeException, std::exception)
    {
        Reference< XTextComponent >  xText( getPeer(), UNO_QUERY );
        if (xText.is())
            xText->setSelection( aSelection );
    }


    css::awt::Selection SAL_CALL OFilterControl::getSelection() throw(css::uno::RuntimeException, std::exception)
    {
        css::awt::Selection aSel;
        Reference< XTextComponent >  xText( getPeer(), UNO_QUERY );
        if (xText.is())
            aSel = xText->getSelection();
        return aSel;
    }


    sal_Bool SAL_CALL OFilterControl::isEditable() throw(RuntimeException, std::exception)
    {
        Reference< XTextComponent >  xText( getPeer(), UNO_QUERY );
        return xText.is() && xText->isEditable();
    }


    void SAL_CALL OFilterControl::setEditable( sal_Bool bEditable ) throw(RuntimeException, std::exception)
    {
        Reference< XTextComponent >  xText( getPeer(), UNO_QUERY );
        if (xText.is())
            xText->setEditable(bEditable);
    }


    sal_Int16 SAL_CALL OFilterControl::getMaxTextLen() throw(RuntimeException, std::exception)
    {
        Reference< XTextComponent >  xText( getPeer(), UNO_QUERY );
        return xText.is() ? xText->getMaxTextLen() : 0;
    }


    void SAL_CALL OFilterControl::setMaxTextLen( sal_Int16 nLength ) throw(RuntimeException, std::exception)
    {
        Reference< XTextComponent >  xText( getPeer(), UNO_QUERY );
        if (xText.is())
            xText->setMaxTextLen(nLength);
    }


    void OFilterControl::displayException( const css::sdb::SQLContext& _rExcept )
    {
        try
        {
            Reference< XExecutableDialog > xErrorDialog = ErrorMessageDialog::create( m_xContext, "",  m_xMessageParent, makeAny(_rExcept));
            xErrorDialog->execute();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }


    void SAL_CALL OFilterControl::initialize( const Sequence< Any >& aArguments ) throw (Exception, RuntimeException, std::exception)
    {
        const Any* pArguments = aArguments.getConstArray();
        const Any* pArgumentsEnd = pArguments + aArguments.getLength();

        PropertyValue aProp;
        NamedValue aValue;
        const OUString* pName = nullptr;
        const Any* pValue = nullptr;
        Reference< XPropertySet > xControlModel;

        if (aArguments.getLength() == 3
            && (aArguments[0] >>= m_xMessageParent)
            && (aArguments[1] >>= m_xFormatter)
            && (aArguments[2] >>= xControlModel))
        {
            initControlModel(xControlModel);
        }
        else for ( ; pArguments != pArgumentsEnd; ++pArguments )
        {
            // we recognize PropertyValues and NamedValues
            if ( *pArguments >>= aProp )
            {
                pName = &aProp.Name;
                pValue = &aProp.Value;
            }
            else if ( *pArguments >>= aValue )
            {
                pName = &aValue.Name;
                pValue = &aValue.Value;
            }
            else
            {
                OSL_FAIL( "OFilterControl::initialize: unrecognized argument!" );
                continue;
            }

            if ( *pName == "MessageParent" )
            {
                // the message parent
                *pValue >>= m_xMessageParent;
                OSL_ENSURE( m_xMessageParent.is(), "OFilterControl::initialize: invalid MessageParent!" );
            }
            else if ( *pName == "NumberFormatter" )
            {
                // the number format. This argument is optional.
                *pValue >>= m_xFormatter;
                OSL_ENSURE( m_xFormatter.is(), "OFilterControl::initialize: invalid NumberFormatter!" );
            }
            else if ( *pName == "ControlModel" )
            {
                // the control model for which we act as filter control
                if ( !(*pValue >>= xControlModel ) )
                {
                    OSL_FAIL( "OFilterControl::initialize: invalid control model argument!" );
                    continue;
                }
                initControlModel(xControlModel);
            }
        }
    }

    void OFilterControl::initControlModel(Reference< XPropertySet >& xControlModel)
    {
#if !HAVE_FEATURE_DBCONNECTIVITY
        (void) xControlModel;
#else
        if ( !xControlModel.is() )
        {
            OSL_FAIL( "OFilterControl::initialize: invalid control model argument!" );
            return;
        }
        // some properties which are "derived" from the control model we're working for

        // the field
        m_xField.clear();
        OSL_ENSURE( ::comphelper::hasProperty( PROPERTY_BOUNDFIELD, xControlModel ), "OFilterControl::initialize: control model needs a bound field property!" );
        xControlModel->getPropertyValue( PROPERTY_BOUNDFIELD ) >>= m_xField;


        // filter list and control class
        m_bFilterList = ::comphelper::hasProperty( PROPERTY_FILTERPROPOSAL, xControlModel ) && ::comphelper::getBOOL( xControlModel->getPropertyValue( PROPERTY_FILTERPROPOSAL ) );
        if ( m_bFilterList )
            m_nControlClass = FormComponentType::COMBOBOX;
        else
        {
            sal_Int16 nClassId = ::comphelper::getINT16( xControlModel->getPropertyValue( PROPERTY_CLASSID ) );
            switch (nClassId)
            {
                case FormComponentType::CHECKBOX:
                case FormComponentType::RADIOBUTTON:
                case FormComponentType::LISTBOX:
                case FormComponentType::COMBOBOX:
                    m_nControlClass = nClassId;
                    if ( FormComponentType::LISTBOX == nClassId )
                    {
                        Sequence< OUString > aDisplayItems;
                        OSL_VERIFY( xControlModel->getPropertyValue( PROPERTY_STRINGITEMLIST ) >>= aDisplayItems );
                        Sequence< OUString > aValueItems;
                        OSL_VERIFY( xControlModel->getPropertyValue( PROPERTY_VALUE_SEQ ) >>= aValueItems );
                        OSL_ENSURE( aDisplayItems.getLength() == aValueItems.getLength(), "OFilterControl::initialize: inconsistent item lists!" );
                        for ( sal_Int32 i=0; i < ::std::min( aDisplayItems.getLength(), aValueItems.getLength() ); ++i )
                            m_aDisplayItemToValueItem[ aDisplayItems[i] ] = aValueItems[i];
                    }
                    break;
                default:
                    m_bMultiLine = ::comphelper::hasProperty( PROPERTY_MULTILINE, xControlModel ) && ::comphelper::getBOOL( xControlModel->getPropertyValue( PROPERTY_MULTILINE ) );
                    m_nControlClass = FormComponentType::TEXTFIELD;
                    break;
            }
        }


        // the connection meta data for the form which we're working for
        Reference< XChild > xModel( xControlModel, UNO_QUERY );
        Reference< XRowSet > xForm;
        if ( xModel.is() )
            xForm.set(xModel->getParent(), css::uno::UNO_QUERY);
        m_xConnection = ::dbtools::getConnection( xForm );
        OSL_ENSURE( m_xConnection.is(), "OFilterControl::initialize: unable to determine the form's connection!" );
#endif
    }

    OUString SAL_CALL OFilterControl::getImplementationName(  ) throw (RuntimeException, std::exception)
    {
        return getImplementationName_Static();
    }

    sal_Bool SAL_CALL OFilterControl::supportsService( const OUString& ServiceName ) throw (RuntimeException, std::exception)
    {
        return cppu::supportsService(this, ServiceName);
    }

    Sequence< OUString > SAL_CALL OFilterControl::getSupportedServiceNames(  ) throw (RuntimeException, std::exception)
    {
        return getSupportedServiceNames_Static();
    }

    OUString SAL_CALL OFilterControl::getImplementationName_Static()
    {
        return OUString( "com.sun.star.comp.forms.OFilterControl" );
    }

    Sequence< OUString > SAL_CALL OFilterControl::getSupportedServiceNames_Static()
    {
        Sequence< OUString > aNames( 2 );
        aNames[ 0 ] = "com.sun.star.form.control.FilterControl";
        aNames[ 1 ] = "com.sun.star.awt.UnoControl";
        return aNames;
    }
}   // namespace frm

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_comp_forms_OFilterControl_get_implementation(css::uno::XComponentContext* context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::OFilterControl(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
