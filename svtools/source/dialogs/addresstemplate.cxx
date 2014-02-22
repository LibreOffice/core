/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <stdio.h>
#include <svtools/addresstemplate.hxx>
#include "addresstemplate.hrc"
#include <svtools/svtools.hrc>
#include <svtools/helpid.hrc>
#include <svtools/svtresid.hxx>
#include <tools/debug.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/interaction.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <vcl/stdtext.hxx>
#include <vcl/waitobj.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/settings.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <com/sun/star/ui/dialogs/AddressBookSourcePilot.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <svtools/localresaccess.hxx>
#include "svl/filenotation.hxx"
#include <tools/urlobj.hxx>
#include <algorithm>
#include <map>


namespace svt
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::ui::dialogs;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::task;
    using namespace ::comphelper;
    using namespace ::utl;

    typedef std::vector<OUString> StringArray;
    typedef std::set<OUString> StringBag;
    typedef std::map<OUString, OUString> MapString2String;

    namespace
    {
        OUString lcl_getSelectedDataSource( const ComboBox& _dataSourceCombo )
        {
            OUString selectedDataSource = _dataSourceCombo.GetText();
            if ( _dataSourceCombo.GetEntryPos( selectedDataSource ) == LISTBOX_ENTRY_NOTFOUND )
            {
                
                OFileNotation aFileNotation( selectedDataSource, OFileNotation::N_SYSTEM );
                selectedDataSource = aFileNotation.get( OFileNotation::N_URL );
            }
            return selectedDataSource;
        }
    }

    
    
    
    class IAssigmentData
    {
    public:
        virtual ~IAssigmentData();

        
        virtual OUString getDatasourceName() const = 0;

        
        virtual OUString getCommand() const = 0;

        /** the command type to use for the address book
            @return
                a <type scope="com.sun.star.sdb">CommandType</type> value
        */
        virtual sal_Int32       getCommandType() const = 0;

        
        virtual sal_Bool        hasFieldAssignment(const OUString& _rLogicalName) = 0;
        
        virtual OUString getFieldAssignment(const OUString& _rLogicalName) = 0;

        
        virtual void            setFieldAssignment(const OUString& _rLogicalName, const OUString& _rAssignment) = 0;
        
        virtual void            clearFieldAssignment(const OUString& _rLogicalName) = 0;

        virtual void    setDatasourceName(const OUString& _rName) = 0;
        virtual void    setCommand(const OUString& _rCommand) = 0;
    };

    
    IAssigmentData::~IAssigmentData()
    {
    }

    
    
    
    class AssigmentTransientData : public IAssigmentData
    {
    protected:
        Reference< XDataSource >    m_xDataSource;
        OUString             m_sDSName;
        OUString             m_sTableName;
        MapString2String            m_aAliases;

public:
        AssigmentTransientData(
            const Reference< XDataSource >& _rxDataSource,
            const OUString& _rDataSourceName,
            const OUString& _rTableName,
            const Sequence< AliasProgrammaticPair >& _rFields
        );

        
        virtual OUString getDatasourceName() const;
        virtual OUString getCommand() const;
        virtual sal_Int32       getCommandType() const;

        virtual sal_Bool        hasFieldAssignment(const OUString& _rLogicalName);
        virtual OUString getFieldAssignment(const OUString& _rLogicalName);
        virtual void            setFieldAssignment(const OUString& _rLogicalName, const OUString& _rAssignment);
        virtual void            clearFieldAssignment(const OUString& _rLogicalName);

        virtual void    setDatasourceName(const OUString& _rName);
        virtual void    setCommand(const OUString& _rCommand);
    };

    
    AssigmentTransientData::AssigmentTransientData( const Reference< XDataSource >& _rxDataSource,
            const OUString& _rDataSourceName, const OUString& _rTableName,
            const Sequence< AliasProgrammaticPair >& _rFields )
        :m_xDataSource( _rxDataSource )
        ,m_sDSName( _rDataSourceName )
        ,m_sTableName( _rTableName )
    {
        
        
        StringBag aKnownNames;

        OUString sLogicalFieldNames(SVT_RESSTR(STR_LOGICAL_FIELD_NAMES));
        sal_Int32 nIndex = 0;
        do
        {
            OUString aToken = sLogicalFieldNames.getToken(0, ';', nIndex);
            aKnownNames.insert(aToken);
        }
        while ( nIndex >= 0);

        
        const AliasProgrammaticPair* pFields = _rFields.getConstArray();
        const AliasProgrammaticPair* pFieldsEnd = pFields + _rFields.getLength();
        for (;pFields != pFieldsEnd; ++pFields)
        {
            StringBag::const_iterator aKnownPos = aKnownNames.find( pFields->ProgrammaticName );
            if ( aKnownNames.end() != aKnownPos )
            {
                m_aAliases[ pFields->ProgrammaticName ] = pFields->Alias;
            }
            else
            {
                OSL_FAIL(   (   OString("AssigmentTransientData::AssigmentTransientData: unknown programmatic name (")
                                +=  OString(pFields->ProgrammaticName.getStr(), pFields->ProgrammaticName.getLength(), RTL_TEXTENCODING_ASCII_US)
                                +=  OString(")!")
                                ).getStr()
                            );
            }
        }
    }

    
    OUString AssigmentTransientData::getDatasourceName() const
    {
        return m_sDSName;
    }

    
    OUString AssigmentTransientData::getCommand() const
    {
        return m_sTableName;
    }

    
    sal_Int32 AssigmentTransientData::getCommandType() const
    {
        return CommandType::TABLE;
    }

    
    sal_Bool AssigmentTransientData::hasFieldAssignment(const OUString& _rLogicalName)
    {
        MapString2String::const_iterator aPos = m_aAliases.find( _rLogicalName );
        return  ( m_aAliases.end() != aPos )
            &&  ( !aPos->second.isEmpty() );
    }

    
    OUString AssigmentTransientData::getFieldAssignment(const OUString& _rLogicalName)
    {
        OUString sReturn;
        MapString2String::const_iterator aPos = m_aAliases.find( _rLogicalName );
        if ( m_aAliases.end() != aPos )
            sReturn = aPos->second;

        return sReturn;
    }

    
    void AssigmentTransientData::setFieldAssignment(const OUString& _rLogicalName, const OUString& _rAssignment)
    {
        m_aAliases[ _rLogicalName ] = _rAssignment;
    }

    
    void AssigmentTransientData::clearFieldAssignment(const OUString& _rLogicalName)
    {
        MapString2String::iterator aPos = m_aAliases.find( _rLogicalName );
        if ( m_aAliases.end() != aPos )
            m_aAliases.erase( aPos );
    }

    
    void AssigmentTransientData::setDatasourceName(const OUString&)
    {
        OSL_FAIL( "AssigmentTransientData::setDatasourceName: cannot be implemented for transient data!" );
    }

    
    void AssigmentTransientData::setCommand(const OUString&)
    {
        OSL_FAIL( "AssigmentTransientData::setCommand: cannot be implemented for transient data!" );
    }

    
    
    
    class AssignmentPersistentData
            :public ::utl::ConfigItem
            ,public IAssigmentData
    {
    protected:
        StringBag       m_aStoredFields;

    protected:
        ::com::sun::star::uno::Any
                        getProperty(const OUString& _rLocalName) const;
        ::com::sun::star::uno::Any
                        getProperty(const sal_Char* _pLocalName) const;

        OUString getStringProperty(const sal_Char* _pLocalName) const;
        sal_Int32       getInt32Property(const sal_Char* _pLocalName) const;

        OUString getStringProperty(const OUString& _rLocalName) const;

        void            setStringProperty(const sal_Char* _pLocalName, const OUString& _rValue);

    public:
        AssignmentPersistentData();
        ~AssignmentPersistentData();

        
        virtual OUString getDatasourceName() const;
        virtual OUString getCommand() const;
        virtual sal_Int32       getCommandType() const;

        virtual sal_Bool        hasFieldAssignment(const OUString& _rLogicalName);
        virtual OUString getFieldAssignment(const OUString& _rLogicalName);
        virtual void            setFieldAssignment(const OUString& _rLogicalName, const OUString& _rAssignment);
        virtual void            clearFieldAssignment(const OUString& _rLogicalName);

        virtual void    setDatasourceName(const OUString& _rName);
        virtual void    setCommand(const OUString& _rCommand);

        virtual void    Notify( const com::sun::star::uno::Sequence<OUString>& aPropertyNames);
        virtual void    Commit();
    };


void AssignmentPersistentData::Notify( const com::sun::star::uno::Sequence<OUString>& )
{
}

void AssignmentPersistentData::Commit()
{
}

    
    AssignmentPersistentData::AssignmentPersistentData()
        :ConfigItem( OUString( "Office.DataAccess/AddressBook" ))
    {
        Sequence< OUString > aStoredNames = GetNodeNames(OUString("Fields"));
        const OUString* pStoredNames = aStoredNames.getConstArray();
        for (sal_Int32 i=0; i<aStoredNames.getLength(); ++i, ++pStoredNames)
            m_aStoredFields.insert(*pStoredNames);
    }

    
    AssignmentPersistentData::~AssignmentPersistentData()
    {
    }

    
    sal_Bool AssignmentPersistentData::hasFieldAssignment(const OUString& _rLogicalName)
    {
        return (m_aStoredFields.end() != m_aStoredFields.find(_rLogicalName));
    }

    
    OUString AssignmentPersistentData::getFieldAssignment(const OUString& _rLogicalName)
    {
        OUString sAssignment;
        if (hasFieldAssignment(_rLogicalName))
        {
            OUString sFieldPath("Fields/");
            sFieldPath += _rLogicalName;
            sFieldPath += "/AssignedFieldName";
            sAssignment = getStringProperty(sFieldPath);
        }
        return sAssignment;
    }

    
    Any AssignmentPersistentData::getProperty(const sal_Char* _pLocalName) const
    {
        return getProperty(OUString::createFromAscii(_pLocalName));
    }

    
    Any AssignmentPersistentData::getProperty(const OUString& _rLocalName) const
    {
        Sequence< OUString > aProperties(&_rLocalName, 1);
        Sequence< Any > aValues = const_cast<AssignmentPersistentData*>(this)->GetProperties(aProperties);
        DBG_ASSERT(aValues.getLength() == 1, "AssignmentPersistentData::getProperty: invalid sequence length!");
        return aValues[0];
    }

    
    OUString AssignmentPersistentData::getStringProperty(const OUString& _rLocalName) const
    {
        OUString sReturn;
        getProperty( _rLocalName ) >>= sReturn;
        return sReturn;
    }

    
    OUString AssignmentPersistentData::getStringProperty(const sal_Char* _pLocalName) const
    {
        OUString sReturn;
        getProperty( _pLocalName ) >>= sReturn;
        return sReturn;
    }

    
    sal_Int32 AssignmentPersistentData::getInt32Property(const sal_Char* _pLocalName) const
    {
        sal_Int32 nReturn = 0;
        getProperty( _pLocalName ) >>= nReturn;
        return nReturn;
    }

    
    void AssignmentPersistentData::setStringProperty(const sal_Char* _pLocalName, const OUString& _rValue)
    {
        Sequence< OUString > aNames(1);
        Sequence< Any > aValues(1);
        aNames[0] = OUString::createFromAscii(_pLocalName);
        aValues[0] <<= _rValue;
        PutProperties(aNames, aValues);
    }

    
    void AssignmentPersistentData::setFieldAssignment(const OUString& _rLogicalName, const OUString& _rAssignment)
    {
        if (_rAssignment.isEmpty())
        {
            if (hasFieldAssignment(_rLogicalName))
            {
                
                clearFieldAssignment(_rLogicalName);
            }
            return;
        }

        
        OUString sDescriptionNodePath("Fields");

        
        OUString sFieldElementNodePath(sDescriptionNodePath);
        sFieldElementNodePath += "/";
        sFieldElementNodePath += _rLogicalName;

        Sequence< PropertyValue > aNewFieldDescription(2);
        
        aNewFieldDescription[0].Name = sFieldElementNodePath + "/ProgrammaticFieldName";
        aNewFieldDescription[0].Value <<= _rLogicalName;
        
        aNewFieldDescription[1].Name = sFieldElementNodePath + "/AssignedFieldName";
        aNewFieldDescription[1].Value <<= _rAssignment;

        
#ifdef DBG_UTIL
        sal_Bool bSuccess =
#endif
        SetSetProperties(sDescriptionNodePath, aNewFieldDescription);
        DBG_ASSERT(bSuccess, "AssignmentPersistentData::setFieldAssignment: could not commit the changes a field!");
    }

    
    void AssignmentPersistentData::clearFieldAssignment(const OUString& _rLogicalName)
    {
        if (!hasFieldAssignment(_rLogicalName))
            
            return;

        OUString sDescriptionNodePath("Fields");
        Sequence< OUString > aNames(&_rLogicalName, 1);
        ClearNodeElements(sDescriptionNodePath, aNames);
    }

    
    OUString AssignmentPersistentData::getDatasourceName() const
    {
        return getStringProperty( "DataSourceName" );
    }

    
    OUString AssignmentPersistentData::getCommand() const
    {
        return getStringProperty( "Command" );
    }

    
    void AssignmentPersistentData::setDatasourceName(const OUString& _rName)
    {
        setStringProperty( "DataSourceName", _rName );
    }

    
    void AssignmentPersistentData::setCommand(const OUString& _rCommand)
    {
        setStringProperty( "Command", _rCommand );
    }

    
    sal_Int32 AssignmentPersistentData::getCommandType() const
    {
        return getInt32Property( "CommandType" );
    }

    
    
    
    struct AddressBookSourceDialogData
    {
        FixedText*      pFieldLabels[FIELD_PAIRS_VISIBLE * 2];
        ListBox*        pFields[FIELD_PAIRS_VISIBLE * 2];

        
        Reference< XDataSource >
                        m_xTransientDataSource;
        
        sal_Int32       nFieldScrollPos;
        
        sal_Int32       nLastVisibleListIndex;
        
        sal_Bool        bOddFieldNumber : 1;
        
        sal_Bool        bWorkingPersistent : 1;

        
        StringArray     aFieldLabels;
        
        StringArray     aFieldAssignments;
        
        StringArray     aLogicalFieldNames;

        IAssigmentData* pConfigData;

        
        AddressBookSourceDialogData( )
            :nFieldScrollPos(0)
            ,nLastVisibleListIndex(0)
            ,bOddFieldNumber(sal_False)
            ,bWorkingPersistent( sal_True )
            ,pConfigData( new AssignmentPersistentData )
        {
        }

        
        AddressBookSourceDialogData( const Reference< XDataSource >& _rxTransientDS, const OUString& _rDataSourceName,
            const OUString& _rTableName, const Sequence< AliasProgrammaticPair >& _rFields )
            :m_xTransientDataSource( _rxTransientDS )
            ,nFieldScrollPos(0)
            ,nLastVisibleListIndex(0)
            ,bOddFieldNumber(sal_False)
            ,bWorkingPersistent( sal_False )
            ,pConfigData( new AssigmentTransientData( m_xTransientDataSource, _rDataSourceName, _rTableName, _rFields ) )
        {
        }

        ~AddressBookSourceDialogData()
        {
            delete pConfigData;
        }

    };

    
    
    

    
    AddressBookSourceDialog::AddressBookSourceDialog(Window* _pParent,
            const Reference< XComponentContext >& _rxORB )
        : ModalDialog(_pParent, "AddressTemplateDialog", "svt/ui/addresstemplatedialog.ui")
        , m_sNoFieldSelection(SVT_RESSTR(STR_NO_FIELD_SELECTION))
        , m_xORB(_rxORB)
        , m_pImpl( new AddressBookSourceDialogData )
    {
        implConstruct();
    }

    
    AddressBookSourceDialog::AddressBookSourceDialog( Window* _pParent, const Reference< XComponentContext >& _rxORB,
        const Reference< XDataSource >& _rxTransientDS, const OUString& _rDataSourceName,
        const OUString& _rTable, const Sequence< AliasProgrammaticPair >& _rMapping )
        : ModalDialog(_pParent, "AddressTemplateDialog", "svt/ui/addresstemplatedialog.ui")
        , m_sNoFieldSelection(SVT_RESSTR(STR_NO_FIELD_SELECTION))
        , m_xORB(_rxORB)
        , m_pImpl( new AddressBookSourceDialogData( _rxTransientDS, _rDataSourceName, _rTable, _rMapping ) )
    {
        implConstruct();
    }

    
    void AddressBookSourceDialog::implConstruct()
    {
        get(m_pDatasource, "datasource");
        get(m_pAdministrateDatasources, "admin");
        get(m_pTable, "datatable");
        VclScrolledWindow *pScrollWindow = get<VclScrolledWindow>("scrollwindow");
        pScrollWindow->setUserManagedScrolling(true);
        m_pFieldScroller = &pScrollWindow->getVertScrollBar();

        for (sal_Int32 row=0; row<FIELD_PAIRS_VISIBLE; ++row)
        {
            for (sal_Int32 column=0; column<2; ++column)
            {
                
                m_pImpl->pFieldLabels[row * 2 + column] = get<FixedText>(OString("label") + OString::number(row * 2 + column));
                
                m_pImpl->pFields[row * 2 + column] = get<ListBox>(OString("box") + OString::number(row * 2 + column));
                m_pImpl->pFields[row * 2 + column]->SetSelectHdl(LINK(this, AddressBookSourceDialog, OnFieldSelect));

                m_pImpl->pFields[row * 2 + column]->SetHelpId("svt/ui/addresstemplatedialog/assign");
            }
        }

        initializeDatasources();

        
        
        
        
        
        

        
        m_pImpl->aFieldLabels.push_back( SVT_RESSTR( STR_FIELD_FIRSTNAME ));
        m_pImpl->aFieldLabels.push_back( SVT_RESSTR( STR_FIELD_LASTNAME ));
        m_pImpl->aFieldLabels.push_back( SVT_RESSTR( STR_FIELD_COMPANY));
        m_pImpl->aFieldLabels.push_back( SVT_RESSTR( STR_FIELD_DEPARTMENT ));
        m_pImpl->aFieldLabels.push_back( SVT_RESSTR( STR_FIELD_STREET ));
        m_pImpl->aFieldLabels.push_back( SVT_RESSTR( STR_FIELD_ZIPCODE ));
        m_pImpl->aFieldLabels.push_back( SVT_RESSTR( STR_FIELD_CITY ));
        m_pImpl->aFieldLabels.push_back( SVT_RESSTR( STR_FIELD_STATE));
        m_pImpl->aFieldLabels.push_back( SVT_RESSTR( STR_FIELD_COUNTRY ));
        m_pImpl->aFieldLabels.push_back( SVT_RESSTR( STR_FIELD_HOMETEL ));
        m_pImpl->aFieldLabels.push_back( SVT_RESSTR( STR_FIELD_WORKTEL ));
        m_pImpl->aFieldLabels.push_back( SVT_RESSTR( STR_FIELD_OFFICETEL));
        m_pImpl->aFieldLabels.push_back( SVT_RESSTR( STR_FIELD_MOBILE));
        m_pImpl->aFieldLabels.push_back( SVT_RESSTR( STR_FIELD_TELOTHER));
        m_pImpl->aFieldLabels.push_back( SVT_RESSTR( STR_FIELD_PAGER));
        m_pImpl->aFieldLabels.push_back( SVT_RESSTR( STR_FIELD_FAX ));
        m_pImpl->aFieldLabels.push_back( SVT_RESSTR( STR_FIELD_EMAIL ));
        m_pImpl->aFieldLabels.push_back( SVT_RESSTR( STR_FIELD_URL ));
        m_pImpl->aFieldLabels.push_back( SVT_RESSTR( STR_FIELD_TITLE ));
        m_pImpl->aFieldLabels.push_back( SVT_RESSTR( STR_FIELD_POSITION ));
        m_pImpl->aFieldLabels.push_back( SVT_RESSTR( STR_FIELD_INITIALS ));
        m_pImpl->aFieldLabels.push_back( SVT_RESSTR( STR_FIELD_ADDRFORM ));
        m_pImpl->aFieldLabels.push_back( SVT_RESSTR( STR_FIELD_SALUTATION ));
        m_pImpl->aFieldLabels.push_back( SVT_RESSTR( STR_FIELD_ID));
        m_pImpl->aFieldLabels.push_back( SVT_RESSTR( STR_FIELD_CALENDAR));
        m_pImpl->aFieldLabels.push_back( SVT_RESSTR( STR_FIELD_INVITE));
        m_pImpl->aFieldLabels.push_back( SVT_RESSTR( STR_FIELD_NOTE));
        m_pImpl->aFieldLabels.push_back( SVT_RESSTR( STR_FIELD_USER1));
        m_pImpl->aFieldLabels.push_back( SVT_RESSTR( STR_FIELD_USER2));
        m_pImpl->aFieldLabels.push_back( SVT_RESSTR( STR_FIELD_USER3));
        m_pImpl->aFieldLabels.push_back( SVT_RESSTR( STR_FIELD_USER4));

        long nLabelWidth = 0;
        long nListBoxWidth = m_pImpl->pFields[0]->approximate_char_width() * 20;
        for (StringArray::const_iterator aI = m_pImpl->aFieldLabels.begin(), aEnd = m_pImpl->aFieldLabels.end(); aI != aEnd; ++aI)
        {
            nLabelWidth = std::max(nLabelWidth, FixedText::getTextDimensions(m_pImpl->pFieldLabels[0], *aI, 0x7FFFFFFF).Width());
        }
        for (sal_Int32 row=0; row<FIELD_PAIRS_VISIBLE; ++row)
        {
            for (sal_Int32 column=0; column<2; ++column)
            {
                m_pImpl->pFieldLabels[row * 2 + column]->set_width_request(nLabelWidth);
                m_pImpl->pFields[row * 2 + column]->set_width_request(nListBoxWidth);
            }
        }


        
        m_pImpl->bOddFieldNumber = (m_pImpl->aFieldLabels.size() % 2) != 0;
        if (m_pImpl->bOddFieldNumber)
            m_pImpl->aFieldLabels.push_back( OUString() );

        
        sal_Int32 nOverallFieldPairs = m_pImpl->aFieldLabels.size() / 2;
        m_pFieldScroller->SetRange( Range(0, nOverallFieldPairs - FIELD_PAIRS_VISIBLE) );
        m_pFieldScroller->SetLineSize(1);
        m_pFieldScroller->SetPageSize(FIELD_PAIRS_VISIBLE);

        
        m_pImpl->aFieldAssignments.resize(m_pImpl->aFieldLabels.size());
            

        
        m_pFieldScroller->SetScrollHdl(LINK(this, AddressBookSourceDialog, OnFieldScroll));
        m_pAdministrateDatasources->SetClickHdl(LINK(this, AddressBookSourceDialog, OnAdministrateDatasources));
        m_pDatasource->EnableAutocomplete(true);
        m_pTable->EnableAutocomplete(true);
        m_pTable->SetGetFocusHdl(LINK(this, AddressBookSourceDialog, OnComboGetFocus));
        m_pDatasource->SetGetFocusHdl(LINK(this, AddressBookSourceDialog, OnComboGetFocus));
        m_pTable->SetLoseFocusHdl(LINK(this, AddressBookSourceDialog, OnComboLoseFocus));
        m_pDatasource->SetLoseFocusHdl(LINK(this, AddressBookSourceDialog, OnComboLoseFocus));
        m_pTable->SetSelectHdl(LINK(this, AddressBookSourceDialog, OnComboSelect));
        m_pDatasource->SetSelectHdl(LINK(this, AddressBookSourceDialog, OnComboSelect));
        get<OKButton>("ok")->SetClickHdl(LINK(this, AddressBookSourceDialog, OnOkClicked));

        
        resetFields();
        m_pFieldScroller->SetThumbPos(0);
        m_pImpl->nFieldScrollPos = -1;
        implScrollFields(0, sal_False, sal_False);

        
        OUString sLogicalFieldNames(SVT_RESSTR(STR_LOGICAL_FIELD_NAMES));
        sal_Int32 nAdjustedTokenCount = comphelper::string::getTokenCount(sLogicalFieldNames, ';') + (m_pImpl->bOddFieldNumber ? 1 : 0);
        DBG_ASSERT(nAdjustedTokenCount == (sal_Int32)m_pImpl->aFieldLabels.size(),
            "AddressBookSourceDialog::AddressBookSourceDialog: inconsistence between logical and UI field names!");
        m_pImpl->aLogicalFieldNames.reserve(nAdjustedTokenCount);
        for (sal_Int32 i = 0; i<nAdjustedTokenCount; ++i)
            m_pImpl->aLogicalFieldNames.push_back(comphelper::string::getToken(sLogicalFieldNames, i, ';'));

        PostUserEvent(LINK(this, AddressBookSourceDialog, OnDelayedInitialize));
            
            

        if ( !m_pImpl->bWorkingPersistent )
        {
            StyleSettings aSystemStyle = GetSettings().GetStyleSettings();
            const ::Color& rNewColor = aSystemStyle.GetDialogColor();

            m_pDatasource->SetReadOnly( sal_True );
            m_pDatasource->SetBackground( Wallpaper( rNewColor ) );
            m_pDatasource->SetControlBackground( rNewColor );

            m_pTable->SetReadOnly( sal_True );
            m_pTable->SetBackground( Wallpaper( rNewColor ) );
            m_pTable->SetControlBackground( rNewColor );

            m_pAdministrateDatasources->Hide( );
        }
    }

    
    void AddressBookSourceDialog::getFieldMapping(Sequence< AliasProgrammaticPair >& _rMapping) const
    {
        _rMapping.realloc( m_pImpl->aLogicalFieldNames.size() );
        AliasProgrammaticPair* pPair = _rMapping.getArray();

        OUString sCurrent;
        for (   StringArray::const_iterator aProgrammatic = m_pImpl->aLogicalFieldNames.begin();
                aProgrammatic != m_pImpl->aLogicalFieldNames.end();
                ++aProgrammatic
            )
        {
            sCurrent = *aProgrammatic;
            if ( m_pImpl->pConfigData->hasFieldAssignment( sCurrent ) )
            {
                
                pPair->ProgrammaticName = *aProgrammatic;
                pPair->Alias = m_pImpl->pConfigData->getFieldAssignment( *aProgrammatic );
                ++pPair;
            }
        }

        _rMapping.realloc( pPair - _rMapping.getArray() );
    }

    
    void AddressBookSourceDialog::loadConfiguration()
    {
        OUString sName = m_pImpl->pConfigData->getDatasourceName();
        INetURLObject aURL( sName );
        if( aURL.GetProtocol() != INET_PROT_NOT_VALID )
        {
            OFileNotation aFileNotation( aURL.GetMainURL( INetURLObject::NO_DECODE ) );
            sName = aFileNotation.get(OFileNotation::N_SYSTEM);
        }

        m_pDatasource->SetText(sName);
        m_pTable->SetText(m_pImpl->pConfigData->getCommand());
        

        
        
        assert(m_pImpl->aLogicalFieldNames.size() == m_pImpl->aFieldAssignments.size());

        StringArray::const_iterator aLogical = m_pImpl->aLogicalFieldNames.begin();
        StringArray::iterator aAssignment = m_pImpl->aFieldAssignments.begin();
        for (   ;
                aLogical != m_pImpl->aLogicalFieldNames.end();
                ++aLogical, ++aAssignment
            )
            *aAssignment = m_pImpl->pConfigData->getFieldAssignment(*aLogical);
    }

    
    AddressBookSourceDialog::~AddressBookSourceDialog()
    {
        delete m_pImpl;
    }

    
    void AddressBookSourceDialog::initializeDatasources()
    {
        if (!m_xDatabaseContext.is())
        {
            DBG_ASSERT(m_xORB.is(), "AddressBookSourceDialog::initializeDatasources: no service factory!");
            if (!m_xORB.is())
                return;

            try
            {
                m_xDatabaseContext = DatabaseContext::create(m_xORB);
            }
            catch(const Exception&) { }
            if (!m_xDatabaseContext.is())
            {
                const OUString sContextServiceName("com.sun.star.sdb.DatabaseContext");
                ShowServiceNotAvailableError( this, sContextServiceName, false);
                return;
            }
        }
        m_pDatasource->Clear();

        
        Sequence< OUString > aDatasourceNames;
        try
        {
            aDatasourceNames = m_xDatabaseContext->getElementNames();
        }
        catch(Exception&)
        {
            OSL_FAIL("AddressBookSourceDialog::initializeDatasources: caught an exception while asking for the data source names!");
        }
        const OUString* pDatasourceNames = aDatasourceNames.getConstArray();
        const OUString* pEnd = pDatasourceNames + aDatasourceNames.getLength();
        for (; pDatasourceNames < pEnd; ++pDatasourceNames)
            m_pDatasource->InsertEntry(*pDatasourceNames);
    }

    
    IMPL_LINK(AddressBookSourceDialog, OnFieldScroll, ScrollBar*, _pScrollBar)
    {
        implScrollFields( _pScrollBar->GetThumbPos(), sal_True, sal_True );
        return 0L;
    }

    
    void AddressBookSourceDialog::resetTables()
    {
        if (!m_xDatabaseContext.is())
            return;

        WaitObject aWaitCursor(this);

        
        m_pDatasource->SaveValue();

        
        Reference< XInteractionHandler > xHandler;
        try
        {
            xHandler.set(
                InteractionHandler::createWithParent(m_xORB, 0),
                UNO_QUERY_THROW );
        }
        catch(const Exception&) { }
        if (!xHandler.is())
        {
            const OUString sInteractionHandlerServiceName("com.sun.star.task.InteractionHandler");
            ShowServiceNotAvailableError(this, sInteractionHandlerServiceName, true);
            return;
        }

        
        OUString sOldTable = m_pTable->GetText();

        m_pTable->Clear();

        m_xCurrentDatasourceTables= NULL;

        
        Sequence< OUString > aTableNames;
        Any aException;
        try
        {
            Reference< XCompletedConnection > xDS;
            if ( m_pImpl->bWorkingPersistent )
            {
                OUString sSelectedDS = lcl_getSelectedDataSource(*m_pDatasource);

                
                INetURLObject aURL( sSelectedDS );
                if ( aURL.GetProtocol() != INET_PROT_NOT_VALID || m_xDatabaseContext->hasByName(sSelectedDS) )
                    m_xDatabaseContext->getByName( sSelectedDS ) >>= xDS;
            }
            else
            {
                xDS = xDS.query( m_pImpl->m_xTransientDataSource );
            }

            
            Reference< XConnection > xConn;
            if (xDS.is())
                xConn = xDS->connectWithCompletion(xHandler);

            
            Reference< XTablesSupplier > xSupplTables(xConn, UNO_QUERY);
            if (xSupplTables.is())
            {
                m_xCurrentDatasourceTables = Reference< XNameAccess >(xSupplTables->getTables(), UNO_QUERY);
                if (m_xCurrentDatasourceTables.is())
                    aTableNames = m_xCurrentDatasourceTables->getElementNames();
            }
        }
        catch(const SQLContext& e) { aException <<= e; }
        catch(const SQLWarning& e) { aException <<= e; }
        catch(const SQLException& e) { aException <<= e; }
        catch(Exception&)
        {
            OSL_FAIL("AddressBookSourceDialog::resetTables: could not retrieve the table!");
        }

        if (aException.hasValue())
        {
            Reference< XInteractionRequest > xRequest = new OInteractionRequest(aException);
            try
            {
                xHandler->handle(xRequest);
            }
            catch(Exception&) { }
            return;
        }

        sal_Bool bKnowOldTable = sal_False;
        
        const OUString* pTableNames = aTableNames.getConstArray();
        const OUString* pEnd = pTableNames + aTableNames.getLength();
        for (;pTableNames != pEnd; ++pTableNames)
        {
            m_pTable->InsertEntry(*pTableNames);
            if (0 == pTableNames->compareTo(sOldTable))
                bKnowOldTable = sal_True;
        }

        
        if (!bKnowOldTable)
            sOldTable = OUString();
        m_pTable->SetText(sOldTable);

        resetFields();
    }

    
    void AddressBookSourceDialog::resetFields()
    {
        WaitObject aWaitCursor(this);

        
        m_pDatasource->SaveValue();

        OUString sSelectedTable = m_pTable->GetText();
        Sequence< OUString > aColumnNames;
        try
        {
            if (m_xCurrentDatasourceTables.is())
            {
                
                Reference< XColumnsSupplier > xSuppTableCols;
                if (m_xCurrentDatasourceTables->hasByName(sSelectedTable))
                    ::cppu::extractInterface(xSuppTableCols, m_xCurrentDatasourceTables->getByName(sSelectedTable));
                Reference< XNameAccess > xColumns;
                if (xSuppTableCols.is())
                    xColumns = xSuppTableCols->getColumns();
                if (xColumns.is())
                    aColumnNames = xColumns->getElementNames();
            }
        }
        catch (const Exception&)
        {
            OSL_FAIL("AddressBookSourceDialog::resetFields: could not retrieve the table columns!");
        }


        const OUString* pColumnNames = aColumnNames.getConstArray();
        const OUString* pEnd = pColumnNames + aColumnNames.getLength();

        
        ::std::set< OUString > aColumnNameSet;
        for (pColumnNames = aColumnNames.getConstArray(); pColumnNames != pEnd; ++pColumnNames)
            aColumnNameSet.insert(*pColumnNames);

        std::vector<OUString>::iterator aInitialSelection = m_pImpl->aFieldAssignments.begin() + m_pImpl->nFieldScrollPos;

        ListBox** pListbox = m_pImpl->pFields;
        OUString sSaveSelection;
        for (sal_Int32 i=0; i<FIELD_CONTROLS_VISIBLE; ++i, ++pListbox, ++aInitialSelection)
        {
            sSaveSelection = (*pListbox)->GetSelectEntry();

            (*pListbox)->Clear();

            
            (*pListbox)->InsertEntry(m_sNoFieldSelection, 0);
            
            (*pListbox)->SetEntryData(0, reinterpret_cast<void*>(i));

            
            for (pColumnNames = aColumnNames.getConstArray(); pColumnNames != pEnd; ++pColumnNames)
                (*pListbox)->InsertEntry(*pColumnNames);

            if (!aInitialSelection->isEmpty() && (aColumnNameSet.end() != aColumnNameSet.find(*aInitialSelection)))
                
                (*pListbox)->SelectEntry(*aInitialSelection);
            else
                
                if (aColumnNameSet.end() != aColumnNameSet.find(sSaveSelection))
                    
                    (*pListbox)->SelectEntry(sSaveSelection);
                else
                    
                    (*pListbox)->SelectEntryPos(0);
        }

        
        for (   StringArray::iterator aAdjust = m_pImpl->aFieldAssignments.begin();
                aAdjust != m_pImpl->aFieldAssignments.end();
                ++aAdjust
            )
            if (!aAdjust->isEmpty())
                if (aColumnNameSet.end() == aColumnNameSet.find(*aAdjust))
                    (*aAdjust) = "";
    }

    
    IMPL_LINK(AddressBookSourceDialog, OnFieldSelect, ListBox*, _pListbox)
    {
        
        sal_IntPtr nListBoxIndex = reinterpret_cast<sal_IntPtr>(_pListbox->GetEntryData(0));
        DBG_ASSERT(nListBoxIndex >= 0 && nListBoxIndex < FIELD_CONTROLS_VISIBLE,
            "AddressBookSourceDialog::OnFieldScroll: invalid list box entry!");

        
        if (0 == _pListbox->GetSelectEntryPos())
            
            m_pImpl->aFieldAssignments[m_pImpl->nFieldScrollPos * 2 + nListBoxIndex] = "";
        else
            
            m_pImpl->aFieldAssignments[m_pImpl->nFieldScrollPos * 2 + nListBoxIndex] = _pListbox->GetSelectEntry();

        return 0L;
    }

    
    void AddressBookSourceDialog::implScrollFields(sal_Int32 _nPos, sal_Bool _bAdjustFocus, sal_Bool _bAdjustScrollbar)
    {
        if (_nPos == m_pImpl->nFieldScrollPos)
            
            return;

        
        
        FixedText** pLeftLabelControl = m_pImpl->pFieldLabels;
        FixedText** pRightLabelControl = pLeftLabelControl + 1;
        StringArray::const_iterator pLeftColumnLabel = m_pImpl->aFieldLabels.begin() + 2 * _nPos;
        StringArray::const_iterator pRightColumnLabel = pLeftColumnLabel + 1;

        
        ListBox** pLeftListControl = m_pImpl->pFields;
        ListBox** pRightListControl = pLeftListControl + 1;

        
        sal_Int32 nOldFocusRow = -1;
        sal_Int32 nOldFocusColumn = 0;

        
        StringArray::const_iterator pLeftAssignment = m_pImpl->aFieldAssignments.begin() + 2 * _nPos;
        StringArray::const_iterator pRightAssignment = pLeftAssignment + 1;

        m_pImpl->nLastVisibleListIndex = -1;
        
        for (sal_Int32 i=0; i<FIELD_PAIRS_VISIBLE; ++i)
        {
            if ((*pLeftListControl)->HasChildPathFocus())
            {
                nOldFocusRow = i;
                nOldFocusColumn = 0;
            }
            else if ((*pRightListControl)->HasChildPathFocus())
            {
                nOldFocusRow = i;
                nOldFocusColumn = 1;
            }

            
            (*pLeftLabelControl)->SetText(*pLeftColumnLabel);
            (*pRightLabelControl)->SetText(*pRightColumnLabel);

            
            
            
            
            
            
            sal_Bool bHideRightColumn = pRightColumnLabel->isEmpty();
            (*pRightLabelControl)->Show(!bHideRightColumn);
            (*pRightListControl)->Show(!bHideRightColumn);
            
            implSelectField(*pLeftListControl, *pLeftAssignment);
            implSelectField(*pRightListControl, *pRightAssignment);

            
            ++m_pImpl->nLastVisibleListIndex;   
            if (!bHideRightColumn)
                ++m_pImpl->nLastVisibleListIndex;

            
            if ( i < FIELD_PAIRS_VISIBLE - 1 )
            {   
                
                pLeftLabelControl += 2;
                pRightLabelControl += 2;
                pLeftColumnLabel += 2;
                pRightColumnLabel += 2;

                pLeftListControl += 2;
                pRightListControl += 2;
                pLeftAssignment += 2;
                pRightAssignment += 2;
            }
        }

        if (_bAdjustFocus && (nOldFocusRow >= 0))
        {   
            sal_Int32 nDelta = m_pImpl->nFieldScrollPos - _nPos;
            
            sal_Int32 nNewFocusRow = nOldFocusRow + nDelta;
            
            nNewFocusRow = std::min(nNewFocusRow, (sal_Int32)(FIELD_PAIRS_VISIBLE - 1), ::std::less< sal_Int32 >());
            nNewFocusRow = std::max(nNewFocusRow, (sal_Int32)0, ::std::less< sal_Int32 >());
            
            m_pImpl->pFields[nNewFocusRow * 2 + nOldFocusColumn]->GrabFocus();
        }

        m_pImpl->nFieldScrollPos = _nPos;

        if (_bAdjustScrollbar)
            m_pFieldScroller->SetThumbPos(m_pImpl->nFieldScrollPos);
    }

    
    void AddressBookSourceDialog::implSelectField(ListBox* _pBox, const OUString& _rText)
    {
        if (!_rText.isEmpty())
            
            _pBox->SelectEntry(_rText);
        else
            
            _pBox->SelectEntryPos(0);
    }

    
    IMPL_LINK_NOARG(AddressBookSourceDialog, OnDelayedInitialize)
    {
        
        loadConfiguration();
        resetTables();
            

        if ( !m_pImpl->bWorkingPersistent )
            if ( m_pImpl->pFields[0] )
                m_pImpl->pFields[0]->GrabFocus();

        return 0L;
    }

    
    IMPL_LINK(AddressBookSourceDialog, OnComboSelect, ComboBox*, _pBox)
    {
        if (_pBox == m_pDatasource)
            resetTables();
        else
            resetFields();
        return 0;
    }

    
    IMPL_LINK(AddressBookSourceDialog, OnComboGetFocus, ComboBox*, _pBox)
    {
        _pBox->SaveValue();
        return 0L;
    }

    
    IMPL_LINK(AddressBookSourceDialog, OnComboLoseFocus, ComboBox*, _pBox)
    {
        if (_pBox->GetSavedValue() != _pBox->GetText())
        {
            if (_pBox == m_pDatasource)
                resetTables();
            else
                resetFields();
        }
        return 0L;
    }

    
    IMPL_LINK_NOARG(AddressBookSourceDialog, OnOkClicked)
    {
        OUString sSelectedDS = lcl_getSelectedDataSource(*m_pDatasource);
        if ( m_pImpl->bWorkingPersistent )
        {
            m_pImpl->pConfigData->setDatasourceName(sSelectedDS);
            m_pImpl->pConfigData->setCommand(m_pTable->GetText());
        }

        
        assert(m_pImpl->aLogicalFieldNames.size() == m_pImpl->aFieldAssignments.size());

        
        StringArray::const_iterator aLogical = m_pImpl->aLogicalFieldNames.begin();
        StringArray::const_iterator aAssignment = m_pImpl->aFieldAssignments.begin();
        for (   ;
                aLogical != m_pImpl->aLogicalFieldNames.end();
                ++aLogical, ++aAssignment
            )
            m_pImpl->pConfigData->setFieldAssignment(*aLogical, *aAssignment);


        EndDialog(RET_OK);
        return 0L;
    }

    
    IMPL_LINK_NOARG(AddressBookSourceDialog, OnAdministrateDatasources)
    {
        
        Reference< XExecutableDialog > xAdminDialog;
        try
        {
            xAdminDialog = AddressBookSourcePilot::createWithParent( m_xORB, VCLUnoHelper::GetInterface(this) );
        }
        catch(const Exception&) { }
        if (!xAdminDialog.is())
        {
            ShowServiceNotAvailableError(this, OUString("com.sun.star.ui.dialogs.AddressBookSourcePilot"), true);
            return 1L;
        }

        
        try
        {
            if ( xAdminDialog->execute() == RET_OK )
            {
                Reference<XPropertySet> xProp(xAdminDialog,UNO_QUERY);
                if ( xProp.is() )
                {
                    OUString sName;
                    xProp->getPropertyValue("DataSourceName") >>= sName;

                    INetURLObject aURL( sName );
                    if( aURL.GetProtocol() != INET_PROT_NOT_VALID )
                    {
                        OFileNotation aFileNotation( aURL.GetMainURL( INetURLObject::NO_DECODE ) );
                        sName = aFileNotation.get(OFileNotation::N_SYSTEM);
                    }
                    m_pDatasource->InsertEntry(sName);
                    delete m_pImpl->pConfigData;
                    m_pImpl->pConfigData = new AssignmentPersistentData();
                    loadConfiguration();
                    resetTables();
                    
                }
            }
        }
        catch(const Exception&)
        {
            OSL_FAIL("AddressBookSourceDialog::OnAdministrateDatasources: an error occurred while executing the administration dialog!");
        }

        
        



        return 0L;
    }

    
    bool AddressBookSourceDialog::PreNotify( NotifyEvent& _rNEvt )
    {
        switch (_rNEvt.GetType())
        {
            case EVENT_KEYINPUT:
            {
                const KeyEvent* pKeyEvent = _rNEvt.GetKeyEvent();
                sal_uInt16 nCode  = pKeyEvent->GetKeyCode().GetCode();
                sal_Bool   bShift = pKeyEvent->GetKeyCode().IsShift();
                sal_Bool   bCtrl  = pKeyEvent->GetKeyCode().IsMod1();
                sal_Bool   bAlt =   pKeyEvent->GetKeyCode().IsMod2();

                if (KEY_TAB == nCode)
                {   
                    if (!bAlt && !bCtrl && !bShift)
                    {   
                        if (m_pImpl->pFields[m_pImpl->nLastVisibleListIndex]->HasChildPathFocus())
                            
                            if (m_pImpl->nFieldScrollPos < m_pFieldScroller->GetRangeMax())
                            {   
                                sal_Int32 nNextFocusList = m_pImpl->nLastVisibleListIndex + 1 - 2;
                                
                                implScrollFields(m_pImpl->nFieldScrollPos + 1, sal_False, sal_True);
                                
                                m_pImpl->pFields[nNextFocusList]->GrabFocus();
                                
                                return true;
                            }
                    }
                    else if (!bAlt && !bCtrl && bShift)
                    {   
                        if (m_pImpl->pFields[0]->HasChildPathFocus())
                            
                            if (m_pImpl->nFieldScrollPos > 0)
                            {   
                                
                                implScrollFields(m_pImpl->nFieldScrollPos - 1, sal_False, sal_True);
                                
                                m_pImpl->pFields[0 - 1 + 2]->GrabFocus();
                                
                                return true;
                            }
                    }
                }
            }
            break;
        }
        return ModalDialog::PreNotify(_rNEvt);
    }


}   


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
