/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include "Filter.hxx"

#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/awt/XCheckBox.hpp>
#include <com/sun/star/awt/XRadioButton.hpp>
#include <com/sun/star/awt/XListBox.hpp>
#include <com/sun/star/awt/XComboBox.hpp>
#include <com/sun/star/awt/VclWindowPeerAttribute.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sdb/XColumn.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

#include <connectivity/dbtools.hxx>
#include <connectivity/dbconversion.hxx>
#include <comphelper/property.hxx>
#include <comphelper/numbers.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/wintypes.hxx>
#include <vcl/svapp.hxx>
#include <vcl/stdtext.hxx>
#include <rtl/ustrbuf.hxx>

#include "property.hrc"
#include "frm_resource.hxx"
#include "frm_resource.hrc"
#include "formsmodule.hxx"
namespace binfilter {

//--------------------------------------------------------------------------
extern "C" void SAL_CALL createRegistryInfo_OFilterControl()
{
    static ::binfilter::frm::OMultiInstanceAutoRegistration< ::binfilter::frm::OFilterControl > aAutoRegistration;//STRIP008 	static ::frm::OMultiInstanceAutoRegistration< ::frm::OFilterControl > aAutoRegistration;
}

//.........................................................................
namespace frm
{
//.........................................................................

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

    //=====================================================================
    // OFilterControl
    //=====================================================================
    //---------------------------------------------------------------------
    OFilterControl::OFilterControl( const Reference< XMultiServiceFactory >& _rxORB )
        :m_aTextListeners( *this )
        ,m_bFilterList( sal_False )
        ,m_bMultiLine( sal_False )
        ,m_nControlClass( FormComponentType::TEXTFIELD )
        ,m_bFilterListFilled( sal_False )
        ,m_xORB( _rxORB )
        ,m_aParser( _rxORB )
    {
    }

    //---------------------------------------------------------------------
    sal_Bool OFilterControl::ensureInitialized( )
    {
        if ( !m_xField.is() )
        {
            OSL_ENSURE( sal_False, "OFilterControl::ensureInitialized: improperly initialized: no field!" );
            return sal_False;
        }

        if ( !m_xMetaData.is() )
        {
            OSL_ENSURE( sal_False, "OFilterControl::ensureInitialized: improperly initialized: no database meta data!" );
            return sal_False;
        }

        if ( !m_xFormatter.is() )
        {
            // we can create one from the connection, if it's an SDB connection
            Reference< XConnection > xConn( m_xMetaData->getConnection() );
            Reference< XNumberFormatsSupplier > xFormatSupplier = ::dbtools::getNumberFormats( xConn, sal_True, m_xORB );

            if ( xFormatSupplier.is() )
            {
                m_xFormatter = m_xFormatter.query(
                    m_xORB->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.NumberFormatter" ) ) ) );
                if ( m_xFormatter.is() )
                    m_xFormatter->attachNumberFormatsSupplier( xFormatSupplier );
            }
        }
        if ( !m_xFormatter.is() )
        {
            OSL_ENSURE( sal_False, "OFilterControl::ensureInitialized: no number formatter!" );
            // no fallback anymore
            return sal_False;
        }

        return sal_True;
    }

    //---------------------------------------------------------------------
    Any	SAL_CALL OFilterControl::queryAggregation( const Type & rType ) throw(RuntimeException)
    {
        Any aRet = UnoControl::queryAggregation( rType);
        if(!aRet.hasValue())
            aRet = OFilterControl_BASE::queryInterface(rType);

        return aRet;
    }

    //---------------------------------------------------------------------
    ::rtl::OUString OFilterControl::GetComponentServiceName()
    {
        ::rtl::OUString aServiceName;
        switch (m_nControlClass)
        {
            case FormComponentType::RADIOBUTTON:
                aServiceName = ::rtl::OUString::createFromAscii("radiobutton");
                break;
            case FormComponentType::CHECKBOX:
                aServiceName = ::rtl::OUString::createFromAscii("checkbox");
                break;
            case FormComponentType::COMBOBOX:
                aServiceName = ::rtl::OUString::createFromAscii("combobox");
                break;
            case FormComponentType::LISTBOX:
                aServiceName = ::rtl::OUString::createFromAscii("listbox");
                break;
            default:
                if (m_bMultiLine)
                    aServiceName = ::rtl::OUString::createFromAscii("MultiLineEdit");
                else
                    aServiceName = ::rtl::OUString::createFromAscii("Edit");
        }
        return aServiceName;
    }

    // XComponent
    //---------------------------------------------------------------------
    void OFilterControl::dispose() throw( RuntimeException  )
    {
        EventObject aEvt(*this);
        m_aTextListeners.disposeAndClear( aEvt );
        UnoControl::dispose();
    }

    //---------------------------------------------------------------------
    void OFilterControl::createPeer( const Reference< XToolkit > & rxToolkit, const Reference< XWindowPeer >  & rParentPeer ) throw(RuntimeException)
    {
        UnoControl::createPeer( rxToolkit, rParentPeer );

        Reference< XVclWindowPeer >  xVclWindow( getPeer(), UNO_QUERY );
        Any aValue;
        if (xVclWindow.is())
        {
            switch (m_nControlClass)
            {
                case FormComponentType::CHECKBOX:
                {
                    // checkboxes always have a tristate-mode
                    sal_Bool bB(sal_True);
                    aValue.setValue(&bB,::getBooleanCppuType());
                    xVclWindow->setProperty(PROPERTY_TRISTATE, aValue);

                    aValue <<= (sal_Int32)STATE_DONTKNOW;
                    xVclWindow->setProperty(PROPERTY_STATE, aValue);

                    Reference< XCheckBox >  xBox( getPeer(), UNO_QUERY );
                    xBox->addItemListener(this);

                }	break;
                case FormComponentType::RADIOBUTTON:
                {
                    aValue <<= (sal_Int32)STATE_NOCHECK;
                    xVclWindow->setProperty(PROPERTY_STATE, aValue);

                    Reference< XRadioButton >  xRadio( getPeer(), UNO_QUERY );
                    xRadio->addItemListener(this);
                }	break;
                case FormComponentType::LISTBOX:
                {
                    Reference< XListBox >  xListBox( getPeer(), UNO_QUERY );
                    xListBox->addItemListener(this);
                }
                case FormComponentType::COMBOBOX: // no break;
                {
                    sal_Bool bB(sal_True);
                    aValue.setValue(&bB,::getBooleanCppuType());
                    xVclWindow->setProperty(PROPERTY_AUTOCOMPLETE, aValue);
                }
                default:	// no break;
                {
                    Reference< XWindow >  xWindow( getPeer(), UNO_QUERY );
                    xWindow->addFocusListener(this);

                    Reference< XTextComponent >  xText( getPeer(), UNO_QUERY );
                    if (xText.is())
                        xText->setMaxTextLen(0);
                }	break;
            }
        }

        // filter controls are _never_ readonly
        // #107013# - 2002-02-03 - fs@openoffice.org
        Reference< XPropertySet > xModel( getModel(), UNO_QUERY );
        OSL_ENSURE( xModel.is(), "OFilterControl::createPeer: no model!" );
        Reference< XPropertySetInfo > xModelPSI;
        if ( xModel.is() )
            xModelPSI = xModel->getPropertySetInfo();
        if ( xModelPSI.is() && xModelPSI->hasPropertyByName( PROPERTY_READONLY ) )
            xVclWindow->setProperty( PROPERTY_READONLY, makeAny( sal_False ) );

        if (m_bFilterList)
            m_bFilterListFilled = sal_False;
    }

    //---------------------------------------------------------------------
    void OFilterControl::PrepareWindowDescriptor( WindowDescriptor& rDescr )
    {
        if (m_bFilterList)
            rDescr.WindowAttributes |= VclWindowPeerAttribute::DROPDOWN;
    }

    //---------------------------------------------------------------------
    void OFilterControl::ImplSetPeerProperty( const ::rtl::OUString& rPropName, const Any& rVal )
    {
        // these properties are ignored
        if (rPropName == PROPERTY_TEXT ||
            rPropName == PROPERTY_STATE)
            return;

        UnoControl::ImplSetPeerProperty( rPropName, rVal );
    }

    // XEventListener
    //---------------------------------------------------------------------
    void SAL_CALL OFilterControl::disposing(const EventObject& Source) throw( RuntimeException )
    {
        UnoControl::disposing(Source);
    }

    // XItemListener
    //---------------------------------------------------------------------
    void SAL_CALL OFilterControl::itemStateChanged( const ItemEvent& rEvent ) throw(RuntimeException)
    {
        ::rtl::OUString aText;
        switch (m_nControlClass)
        {
            case FormComponentType::CHECKBOX:
            {
                switch (rEvent.Selected)
                {
                    case STATE_CHECK:
                        aText = ::rtl::OUString::createFromAscii("1");
                        break;
                    case STATE_NOCHECK:
                        aText = ::rtl::OUString::createFromAscii("0");
                        break;
                }
            }	break;
            case FormComponentType::LISTBOX:
            {
                Sequence< ::rtl::OUString> aValueSelection;
                Reference< XPropertySet > aPropertyPointer(getModel(), UNO_QUERY);
                aPropertyPointer->getPropertyValue(PROPERTY_VALUE_SEQ) >>= aValueSelection;
                if (rEvent.Selected <= aValueSelection.getLength())
                    aText = aValueSelection.getConstArray()[rEvent.Selected];
            } break;
            case FormComponentType::RADIOBUTTON:
            {
                if (rEvent.Selected == STATE_CHECK)
                    aText = ::comphelper::getString(Reference< XPropertySet > (getModel(), UNO_QUERY)->getPropertyValue(PROPERTY_REFVALUE));
            } break;
        }

        if (m_aText.compareTo(aText))
        {
            m_aText = aText;
            TextEvent aEvt;
            aEvt.Source = *this;
            ::cppu::OInterfaceIteratorHelper aIt( m_aTextListeners );
            while( aIt.hasMoreElements() )
                ((XTextListener *)aIt.next())->textChanged( aEvt );
        }
    }

    //---------------------------------------------------------------------
    void OFilterControl::implInitFilterList()
    {
        if ( !ensureInitialized( ) )
            // already asserted in ensureInitialized
            return;

        // declare here for later disposal
        Reference< XResultSet > xListCursor;
        Reference< XStatement > xStatement;

        try
        {
            m_bFilterListFilled = sal_True;

            Reference< XPropertySet >  xSet(getModel(), UNO_QUERY);
            if (xSet.is() && m_xField.is())
            {
                ::rtl::OUString sName;
                m_xField->getPropertyValue(PROPERTY_NAME) >>= sName;

                // here we need a table to which the field belongs to
                Reference< XChild > xModelAsChild( xSet, UNO_QUERY );
                Reference< XRowSet > xForm( xModelAsChild->getParent(), UNO_QUERY );
                Reference< XPropertySet > xFormAsSet( xForm, UNO_QUERY );

                // Connection holen
                Reference< XConnection > xConnection;
                if ( xForm.is() )
                    xConnection = ::dbtools::getConnection( xForm );
                Reference< XSQLQueryComposerFactory >  xFactory( xConnection, UNO_QUERY );
                OSL_ENSURE( xFactory.is() && xFormAsSet.is(), "OFilterControl::implInitFilterList: invalid form or invalid connection!" );
                if ( !xFactory.is() || !xFormAsSet.is() )
                    return;

                // create a query composer
                Reference< XSQLQueryComposer > xComposer = xFactory->createQueryComposer();
                OSL_ENSURE( xComposer.is() , "OFilterControl::implInitFilterList: invalid query composer!" );
                if ( !xComposer.is() )
                    return;

                // set the statement on the composer, ...
                ::rtl::OUString	sStatement;
                xFormAsSet->getPropertyValue( PROPERTY_ACTIVECOMMAND ) >>= sStatement;
                 xComposer->setQuery( sStatement );

                // ... and ask it for the involved tables and queries
                Reference< XTablesSupplier > xSuppTables( xComposer, UNO_QUERY );
                Reference< XColumnsSupplier > xSuppColumns( xComposer, UNO_QUERY );

                Reference< XNameAccess > xFieldNames;
                if ( xSuppColumns.is() ) xFieldNames = xSuppColumns->getColumns();
                Reference< XNameAccess > xTablesNames;
                if ( xSuppTables.is() ) xTablesNames = xSuppTables->getTables();

                if ( !xFieldNames.is() || !xTablesNames.is() )
                {
                    OSL_ENSURE( sal_False, "OFilterControl::implInitFilterList: invalid query composer (no fields or no tables supplied)!" );
                    return;
                }

                // search the field
                Reference< XPropertySet >  xComposerFieldAsSet;
                if ( xFieldNames->hasByName( sName ) )
                    xFieldNames->getByName( sName ) >>= xComposerFieldAsSet;
                
                if	(	xComposerFieldAsSet.is()
                    &&	::comphelper::hasProperty( PROPERTY_TABLENAME, xComposerFieldAsSet )
                    &&	::comphelper::hasProperty( PROPERTY_REALNAME, xComposerFieldAsSet )
                    )
                {
                    ::rtl::OUString sFieldName, sTableName;
                    xComposerFieldAsSet->getPropertyValue(PROPERTY_REALNAME) >>= sFieldName;
                    xComposerFieldAsSet->getPropertyValue(PROPERTY_TABLENAME) >>= sTableName;

                    // no possibility to create a select statement
                    // looking for the complete table name
                    if (!xTablesNames->hasByName(sTableName))
                        return;

                    // this is the tablename
                    Reference< XNamed > xName;
                    xTablesNames->getByName(sTableName) >>= xName;
                    OSL_ENSURE(xName.is(),"No XName interface!");
                    sTableName = xName->getName();

                    // ein Statement aufbauen und abschicken als query
                    // Access to the connection

                    Reference< XColumn >  xDataField;

                    Reference< XDatabaseMetaData >  xMeta = xConnection->getMetaData();
                    ::rtl::OUString aQuote = xMeta->getIdentifierQuoteString();
                    ::rtl::OUStringBuffer aStatement;
                    aStatement.appendAscii( "SELECT DISTINCT" );
                    aStatement.append( ::dbtools::quoteName( aQuote, sName ) );

                    if ( sFieldName.getLength() && ( sName != sFieldName ) )
                    {
                        aStatement.appendAscii(" AS ");
                        aStatement.append( ::dbtools::quoteName(aQuote, sFieldName) );
                    }

                    aStatement.appendAscii( " FROM " );
                    aStatement.append( ::dbtools::quoteTableName( xMeta, sTableName, ::dbtools::eInDataManipulation ) );
                    ::rtl::OUString sStatement( aStatement.makeStringAndClear( ) );

                    xStatement = xConnection->createStatement();
                    xListCursor = xStatement->executeQuery( sStatement );

                    Reference< XColumnsSupplier >  xSupplyCols(xListCursor, UNO_QUERY);
                    Reference< XIndexAccess >  xFields;
                    if (xSupplyCols.is())
                        xFields = Reference< XIndexAccess > (xSupplyCols->getColumns(), UNO_QUERY);
                    if (xFields.is())
                        xFields->getByIndex(0) >>= xDataField;
                    if (!xDataField.is())
                        return;


                    sal_Int16 i = 0;
                    ::std::vector< ::rtl::OUString>	aStringList;
                    aStringList.reserve(16);
                    ::rtl::OUString	aStr;

                    ::com::sun::star::util::Date aNullDate( ::dbtools::DBTypeConversion::getStandardDate() );
                    sal_Int32 nFormatKey = 0;
                    try
                    {
                        m_xFormatter->getNumberFormatsSupplier()->getNumberFormatSettings()->getPropertyValue(::rtl::OUString::createFromAscii("NullDate"))
                            >>= aNullDate;
                        nFormatKey = ::comphelper::getINT32(m_xField->getPropertyValue(PROPERTY_FORMATKEY));
                    }
                    catch(const Exception&)
                    {
                    }


                    sal_Int16 nKeyType = ::comphelper::getNumberFormatType(m_xFormatter->getNumberFormatsSupplier()->getNumberFormats(), nFormatKey);
                    while (!xListCursor->isAfterLast() && i++ < SHRT_MAX) // max anzahl eintraege
                    {
                        aStr = ::dbtools::DBTypeConversion::getValue(xDataField, m_xFormatter, aNullDate, nFormatKey, nKeyType);

                        aStringList.push_back(aStr);
                        xListCursor->next();
                    }

                    Sequence< ::rtl::OUString> aStringSeq(aStringList.size());
                    ::rtl::OUString* pustrStrings = aStringSeq.getArray();
                    for (i = 0; i < (sal_Int16)aStringList.size(); ++i)
                        pustrStrings[i] = aStringList[i];

                    Reference< XComboBox >  xComboBox( getPeer(), UNO_QUERY);
                    if ( xComboBox.is() )
                    {
                        xComboBox->addItems(aStringSeq, 0);
                        // set the drop down line count
                        sal_Int16 nLineCount = ::std::min( (sal_Int16)10, (sal_Int16)aStringSeq.getLength() );
                        xComboBox->setDropDownLineCount( nLineCount );
                    }
                }
            }
        }
        catch( const Exception& e )
        {
            e; // make compiler happy
            OSL_ENSURE( sal_False, "OFilterControl::implInitFilterList: caught an exception!" );
        }

        ::comphelper::disposeComponent( xListCursor );
        ::comphelper::disposeComponent( xStatement );
    }

    // XFocusListener
    //---------------------------------------------------------------------
    void SAL_CALL OFilterControl::focusGained(const FocusEvent& e)  throw( RuntimeException  )
    {
        // should we fill the combobox?
        if (m_bFilterList && !m_bFilterListFilled)
            implInitFilterList();
    }

    //---------------------------------------------------------------------
    sal_Bool SAL_CALL OFilterControl::commit() throw(RuntimeException)
    {
            OSL_ENSURE( false, "OFilterControl::commit: dead code!" );
            return sal_False;
    }

    // XTextComponent
    //---------------------------------------------------------------------
    void SAL_CALL OFilterControl::addTextListener(const Reference< XTextListener > & l) throw(RuntimeException)
    {
        m_aTextListeners.addInterface( l );
    }

    //---------------------------------------------------------------------
    void SAL_CALL OFilterControl::removeTextListener(const Reference< XTextListener > & l) throw(RuntimeException)
    {
        m_aTextListeners.removeInterface( l );
    }

    //---------------------------------------------------------------------
    void SAL_CALL OFilterControl::setText( const ::rtl::OUString& aText ) throw(RuntimeException)
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
                    if (aText.equals(::rtl::OUString::createFromAscii("1")))
                        aValue <<= (sal_Int32)STATE_CHECK;
                    else if (aText.compareTo(::rtl::OUString::createFromAscii("0")) == 0)
                        aValue <<= (sal_Int32)STATE_NOCHECK;
                    else
                        aValue <<= (sal_Int32)STATE_DONTKNOW;

                    m_aText = aText;
                    xVclWindow->setProperty(PROPERTY_STATE, aValue);
                }
            }	break;
            case FormComponentType::RADIOBUTTON:
            {
                Reference< XVclWindowPeer >  xVclWindow( getPeer(), UNO_QUERY );
                if (xVclWindow.is())
                {
                    ::rtl::OUString aRefText = ::comphelper::getString(com::sun::star::uno::Reference< XPropertySet > (getModel(), UNO_QUERY)->getPropertyValue(PROPERTY_REFVALUE));
                    Any aValue;
                    if (aText == aRefText)
                        aValue <<= (sal_Int32)STATE_CHECK;
                    else
                        aValue <<= (sal_Int32)STATE_NOCHECK;
                    m_aText = aText;
                    xVclWindow->setProperty(PROPERTY_STATE, aValue);
                }
            }	break;
            case FormComponentType::LISTBOX:
            {
                Reference< XListBox >  xListBox( getPeer(), UNO_QUERY );
                if (xListBox.is())
                {
                    m_aText = aText;
                    xListBox->selectItem(m_aText, sal_True);
                }
            } break;
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

    //---------------------------------------------------------------------
    void SAL_CALL OFilterControl::insertText( const ::com::sun::star::awt::Selection& rSel, const ::rtl::OUString& aText ) throw(::com::sun::star::uno::RuntimeException)
    {
        Reference< XTextComponent >  xText( getPeer(), UNO_QUERY );
        if (xText.is())
        {
            xText->insertText(rSel, aText);
            m_aText = xText->getText();
        }
    }

    //---------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OFilterControl::getText() throw(RuntimeException)
    {
        return m_aText;
    }

    //---------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OFilterControl::getSelectedText( void ) throw(RuntimeException)
    {
        ::rtl::OUString aSelected;
        Reference< XTextComponent >  xText( getPeer(), UNO_QUERY );
        if (xText.is())
            aSelected = xText->getSelectedText();

        return aSelected;
    }

    //---------------------------------------------------------------------
    void SAL_CALL OFilterControl::setSelection( const ::com::sun::star::awt::Selection& aSelection ) throw(::com::sun::star::uno::RuntimeException)
    {
        Reference< XTextComponent >  xText( getPeer(), UNO_QUERY );
        if (xText.is())
            xText->setSelection( aSelection );
    }

    //---------------------------------------------------------------------
    ::com::sun::star::awt::Selection SAL_CALL OFilterControl::getSelection( void ) throw(::com::sun::star::uno::RuntimeException)
    {
        ::com::sun::star::awt::Selection aSel;
        Reference< XTextComponent >  xText( getPeer(), UNO_QUERY );
        if (xText.is())
            aSel = xText->getSelection();
        return aSel;
    }

    //---------------------------------------------------------------------
    sal_Bool SAL_CALL OFilterControl::isEditable( void ) throw(RuntimeException)
    {
        Reference< XTextComponent >  xText( getPeer(), UNO_QUERY );
        return xText.is() && xText->isEditable();
    }

    //---------------------------------------------------------------------
    void SAL_CALL OFilterControl::setEditable( sal_Bool bEditable ) throw(RuntimeException)
    {
        Reference< XTextComponent >  xText( getPeer(), UNO_QUERY );
        if (xText.is())
            xText->setEditable(bEditable);
    }

    //---------------------------------------------------------------------
    sal_Int16 SAL_CALL OFilterControl::getMaxTextLen() throw(RuntimeException)
    {
        Reference< XTextComponent >  xText( getPeer(), UNO_QUERY );
        return xText.is() ? xText->getMaxTextLen() : 0;
    }

    //---------------------------------------------------------------------
    void SAL_CALL OFilterControl::setMaxTextLen( sal_Int16 nLength ) throw(RuntimeException)
    {
        Reference< XTextComponent >  xText( getPeer(), UNO_QUERY );
        if (xText.is())
            xText->setMaxTextLen(nLength);
    }

    //---------------------------------------------------------------------
    void SAL_CALL OFilterControl::initialize( const Sequence< Any >& aArguments ) throw (Exception, RuntimeException)
    {
        const Any* pArguments = aArguments.getConstArray();
        const Any* pArgumentsEnd = pArguments + aArguments.getLength();

        PropertyValue aProp;
        NamedValue aValue;
        const ::rtl::OUString* pName = NULL;
        const Any* pValue = NULL;

        for ( ; pArguments != pArgumentsEnd; ++pArguments )
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
                DBG_ERROR( "OFilterControl::initialize: unrecognized argument!" );
                continue;
            }

            if ( 0 == pName->compareToAscii( "MessageParent" ) )
            {
                // the message parent
                *pValue >>= m_xMessageParent;
                OSL_ENSURE( m_xMessageParent.is(), "OFilterControl::initialize: invalid MessageParent!" );
            }
            else if ( 0 == pName->compareToAscii( "NumberFormatter" ) )
            {
                // the number format. This argument is optional.
                *pValue >>= m_xFormatter;
                OSL_ENSURE( m_xFormatter.is(), "OFilterControl::initialize: invalid NumberFormatter!" );
            }
            else if ( 0 == pName->compareToAscii( "ControlModel" ) )
            {
                // the control model for which we act as filter control
                Reference< XPropertySet > xControlModel;
                if ( !(*pValue >>= xControlModel ) || !xControlModel.is() )
                {
                    OSL_ENSURE( sal_False, "OFilterControl::initialize: invalid control model argument!" );
                    continue;
                }

                // some properties which are "derived" from the control model we're working for
                // ...................................................
                // the field
                m_xField.clear();
                OSL_ENSURE( ::comphelper::hasProperty( PROPERTY_BOUNDFIELD, xControlModel ), "OFilterControl::initialize: control model needs a bound field property!" );
                xControlModel->getPropertyValue( PROPERTY_BOUNDFIELD ) >>= m_xField;

                // ...................................................
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
                            break;
                        default:
                            m_bMultiLine = ::comphelper::hasProperty( PROPERTY_MULTILINE, xControlModel ) && ::comphelper::getBOOL( xControlModel->getPropertyValue( PROPERTY_MULTILINE ) );
                            m_nControlClass = FormComponentType::TEXTFIELD;
                            break;
                    }
                }

                // ...................................................
                // the connection meta data for the form which we're working for
                Reference< XChild > xModel( xControlModel, UNO_QUERY );
                Reference< XRowSet > xForm;
                if ( xModel.is() )
                    xForm = xForm.query( xModel->getParent() );
                Reference< XConnection > xConn = ::dbtools::getConnection( xForm );

                m_xMetaData.clear();
                if ( xConn.is() )
                    m_xMetaData = xConn->getMetaData();
                OSL_ENSURE( m_xMetaData.is(), "OFilterControl::initialize: unable to determine the database meta data for the form's connection!" );
            }
        }
    }

    //---------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OFilterControl::getImplementationName(  ) throw (RuntimeException)
    {
        return getImplementationName_Static();
    }

    //---------------------------------------------------------------------
    sal_Bool SAL_CALL OFilterControl::supportsService( const ::rtl::OUString& ServiceName ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aSupported( getSupportedServiceNames() );
        const ::rtl::OUString* pArray = aSupported.getConstArray();
        for( sal_Int32 i = 0; i < aSupported.getLength(); ++i, ++pArray )
            if( pArray->equals( ServiceName ) )
                return sal_True;
        return sal_False;
    }

    //---------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL OFilterControl::getSupportedServiceNames(  ) throw (RuntimeException)
    {
        return getSupportedServiceNames_Static();
    }

    //---------------------------------------------------------------------
    ::rtl::OUString	SAL_CALL OFilterControl::getImplementationName_Static()
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.forms.OFilterControl" ) );
    }

    //---------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL OFilterControl::getSupportedServiceNames_Static()
    {
        Sequence< ::rtl::OUString > aNames( 2 );
        aNames[ 0 ] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.control.FilterControl" ) );
        aNames[ 1 ] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControl" ) );
        return aNames;
    }

    //---------------------------------------------------------------------
    Reference< XInterface > SAL_CALL OFilterControl::Create( const Reference< XMultiServiceFactory >& _rxFactory )
    {
        return static_cast< XServiceInfo* >( new OFilterControl( _rxFactory ) );
    }

//.........................................................................
}	// namespace frm
//.........................................................................
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
