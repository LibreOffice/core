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


#include <memory>
#include <svtools/addresstemplate.hxx>
#include <svtools/strings.hrc>
#include <svtools/svtresid.hxx>
#include <tools/debug.hxx>
#include <comphelper/interaction.hxx>
#include <comphelper/string.hxx>
#include <unotools/configitem.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/stdtext.hxx>
#include <vcl/waitobj.hxx>
#include <vcl/settings.hxx>
#include <vcl/weld.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <toolkit/helper/vclunohelper.hxx>
#include <com/sun/star/util/AliasProgrammaticPair.hpp>
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
#include <svl/filenotation.hxx>
#include <tools/urlobj.hxx>
#include <algorithm>
#include <map>
#include <set>
#include <array>
#include <strings.hxx>


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

    typedef std::set<OUString> StringBag;
    typedef std::map<OUString, OUString> MapString2String;

    namespace
    {
        OUString lcl_getSelectedDataSource( const weld::ComboBox& dataSourceCombo )
        {
            OUString selectedDataSource = dataSourceCombo.get_active_text();
            if (dataSourceCombo.find_text(selectedDataSource) == -1)
            {
                // none of the pre-selected entries -> assume a path to a database document
                OFileNotation aFileNotation( selectedDataSource, OFileNotation::N_SYSTEM );
                selectedDataSource = aFileNotation.get( OFileNotation::N_URL );
            }
            return selectedDataSource;
        }
    }


    // = IAssigmentData

    class IAssigmentData
    {
    public:
        virtual ~IAssigmentData();

        /// the data source to use for the address book
        virtual OUString getDatasourceName() const = 0;

        /// the command to use for the address book
        virtual OUString getCommand() const = 0;

        /// checks whether or not there is an assignment for a given logical field
        virtual bool        hasFieldAssignment(const OUString& _rLogicalName) = 0;
        /// retrieves the assignment for a given logical field
        virtual OUString getFieldAssignment(const OUString& _rLogicalName) = 0;

        /// set the assignment for a given logical field
        virtual void            setFieldAssignment(const OUString& _rLogicalName, const OUString& _rAssignment) = 0;

        virtual void    setDatasourceName(const OUString& _rName) = 0;
        virtual void    setCommand(const OUString& _rCommand) = 0;
    };


    IAssigmentData::~IAssigmentData()
    {
    }


    // = AssigmentTransientData

    class AssigmentTransientData : public IAssigmentData
    {
    protected:
        OUString const       m_sDSName;
        OUString const       m_sTableName;
        MapString2String     m_aAliases;

    public:
        AssigmentTransientData(
            const OUString& _rDataSourceName,
            const OUString& _rTableName,
            const Sequence< AliasProgrammaticPair >& _rFields
        );

        // IAssigmentData overridables
        virtual OUString getDatasourceName() const override;
        virtual OUString getCommand() const override;

        virtual bool     hasFieldAssignment(const OUString& _rLogicalName) override;
        virtual OUString getFieldAssignment(const OUString& _rLogicalName) override;
        virtual void     setFieldAssignment(const OUString& _rLogicalName, const OUString& _rAssignment) override;

        virtual void    setDatasourceName(const OUString& _rName) override;
        virtual void    setCommand(const OUString& _rCommand) override;
    };


    AssigmentTransientData::AssigmentTransientData(
            const OUString& _rDataSourceName, const OUString& _rTableName,
            const Sequence< AliasProgrammaticPair >& _rFields )
        :m_sDSName( _rDataSourceName )
        ,m_sTableName( _rTableName )
    {
        // fill our aliases structure
        // first collect all known programmatic names
        StringBag aKnownNames;

        OUString const sLogicalFieldNames(STR_LOGICAL_FIELD_NAMES);
        sal_Int32 nIndex = 0;
        do
        {
            OUString aToken = sLogicalFieldNames.getToken(0, ';', nIndex);
            aKnownNames.insert(aToken);
        }
        while ( nIndex >= 0);

        // loop through the given names
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
                SAL_WARN( "svtools", "AssigmentTransientData::AssigmentTransientData: unknown programmatic name: "
                          << pFields->ProgrammaticName );
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


    bool AssigmentTransientData::hasFieldAssignment(const OUString& _rLogicalName)
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


    void AssigmentTransientData::setDatasourceName(const OUString&)
    {
        OSL_FAIL( "AssigmentTransientData::setDatasourceName: cannot be implemented for transient data!" );
    }


    void AssigmentTransientData::setCommand(const OUString&)
    {
        OSL_FAIL( "AssigmentTransientData::setCommand: cannot be implemented for transient data!" );
    }


    // = AssignmentPersistentData

    class AssignmentPersistentData
            :public ::utl::ConfigItem
            ,public IAssigmentData
    {
    protected:
        StringBag       m_aStoredFields;

    protected:
        css::uno::Any   getProperty(const OUString& _rLocalName) const;

        OUString        getStringProperty(const sal_Char* _pLocalName) const;
        OUString        getStringProperty(const OUString& _rLocalName) const;

        void            setStringProperty(const sal_Char* _pLocalName, const OUString& _rValue);

    public:
        AssignmentPersistentData();

        // IAssigmentData overridables
        virtual OUString getDatasourceName() const override;
        virtual OUString getCommand() const override;

        virtual bool     hasFieldAssignment(const OUString& _rLogicalName) override;
        virtual OUString getFieldAssignment(const OUString& _rLogicalName) override;
        virtual void     setFieldAssignment(const OUString& _rLogicalName, const OUString& _rAssignment) override;

        virtual void    setDatasourceName(const OUString& _rName) override;
        virtual void    setCommand(const OUString& _rCommand) override;

        virtual void    Notify( const css::uno::Sequence<OUString>& aPropertyNames) override;

    private:
        virtual void    ImplCommit() override;
        void            clearFieldAssignment(const OUString& _rLogicalName);
    };


void AssignmentPersistentData::Notify( const css::uno::Sequence<OUString>& )
{
}

void AssignmentPersistentData::ImplCommit()
{
}


    AssignmentPersistentData::AssignmentPersistentData()
        :ConfigItem("Office.DataAccess/AddressBook")
    {
        Sequence< OUString > aStoredNames = GetNodeNames("Fields");
        const OUString* pStoredNames = aStoredNames.getConstArray();
        for (sal_Int32 i=0; i<aStoredNames.getLength(); ++i, ++pStoredNames)
            m_aStoredFields.insert(*pStoredNames);
    }

    bool AssignmentPersistentData::hasFieldAssignment(const OUString& _rLogicalName)
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
        getProperty(OUString::createFromAscii(_pLocalName)) >>= sReturn;
        return sReturn;
    }


    void AssignmentPersistentData::setStringProperty(const sal_Char* _pLocalName, const OUString& _rValue)
    {
        Sequence< OUString > aNames { OUString::createFromAscii(_pLocalName) };
        Sequence< Any > aValues(1);
        aValues[0] <<= _rValue;
        PutProperties(aNames, aValues);
    }


    void AssignmentPersistentData::setFieldAssignment(const OUString& _rLogicalName, const OUString& _rAssignment)
    {
        if (_rAssignment.isEmpty())
        {
            if (hasFieldAssignment(_rLogicalName))
            {
                // the assignment exists but it should be reset
                clearFieldAssignment(_rLogicalName);
            }
            return;
        }

        // Fields
        OUString sDescriptionNodePath("Fields");

        // Fields/<field>
        OUString sFieldElementNodePath(sDescriptionNodePath);
        sFieldElementNodePath += "/";
        sFieldElementNodePath += _rLogicalName;

        Sequence< PropertyValue > aNewFieldDescription(2);
        // Fields/<field>/ProgrammaticFieldName
        aNewFieldDescription[0].Name = sFieldElementNodePath + "/ProgrammaticFieldName";
        aNewFieldDescription[0].Value <<= _rLogicalName;
        // Fields/<field>/AssignedFieldName
        aNewFieldDescription[1].Name = sFieldElementNodePath + "/AssignedFieldName";
        aNewFieldDescription[1].Value <<= _rAssignment;

        // just set the new value
        bool bSuccess =
            SetSetProperties(sDescriptionNodePath, aNewFieldDescription);
        DBG_ASSERT(bSuccess, "AssignmentPersistentData::setFieldAssignment: could not commit the changes a field!");
    }


    void AssignmentPersistentData::clearFieldAssignment(const OUString& _rLogicalName)
    {
        if (!hasFieldAssignment(_rLogicalName))
            // nothing to do
            return;

        Sequence< OUString > aNames(&_rLogicalName, 1);
        ClearNodeElements("Fields", aNames);
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


    // = AddressBookSourceDialogData

    struct AddressBookSourceDialogData
    {
        std::array<std::unique_ptr<weld::Label>, FIELD_PAIRS_VISIBLE*2> pFieldLabels;
        std::array<std::unique_ptr<weld::ComboBox>, FIELD_PAIRS_VISIBLE*2> pFields;

        /// when working transient, we need the data source
        Reference< XDataSource >
                        m_xTransientDataSource;
        /// current scroll pos in the field list
        sal_Int32       nFieldScrollPos;
        /// indicates that we've an odd field number. This member is for efficiency only, it's redundant.
        bool        bOddFieldNumber : 1;
        /// indicates that we're working with the real persistent configuration
        bool const      bWorkingPersistent : 1;

        /// the strings to use as labels for the field selection listboxes
        std::vector<OUString>     aFieldLabels;
        // the current field assignment
        std::vector<OUString>     aFieldAssignments;
        /// the logical field names
        std::vector<OUString>     aLogicalFieldNames;

        std::unique_ptr<IAssigmentData> pConfigData;


        AddressBookSourceDialogData( )
            :pFieldLabels{{nullptr}}
            ,pFields{{nullptr}}
            ,nFieldScrollPos(0)
            ,bOddFieldNumber(false)
            ,bWorkingPersistent( true )
            ,pConfigData( new AssignmentPersistentData )
        {
        }

        AddressBookSourceDialogData( const Reference< XDataSource >& _rxTransientDS, const OUString& _rDataSourceName,
            const OUString& _rTableName, const Sequence< AliasProgrammaticPair >& _rFields )
            :pFieldLabels{{nullptr}}
            ,pFields{{nullptr}}
            ,m_xTransientDataSource( _rxTransientDS )
            ,nFieldScrollPos(0)
            ,bOddFieldNumber(false)
            ,bWorkingPersistent( false )
            ,pConfigData( new AssigmentTransientData( _rDataSourceName, _rTableName, _rFields ) )
        {
        }

        // Copy assignment is forbidden and not implemented.
        AddressBookSourceDialogData (const AddressBookSourceDialogData &) = delete;
        AddressBookSourceDialogData & operator= (const AddressBookSourceDialogData &) = delete;
    };

    // = AddressBookSourceDialog
    AddressBookSourceDialog::AddressBookSourceDialog(weld::Window* pParent,
            const Reference< XComponentContext >& _rxORB )
        : GenericDialogController(pParent, "svt/ui/addresstemplatedialog.ui", "AddressTemplateDialog")
        , m_sNoFieldSelection(SvtResId(STR_NO_FIELD_SELECTION))
        , m_xORB(_rxORB)
        , m_pImpl( new AddressBookSourceDialogData )
    {
        implConstruct();
    }

    AddressBookSourceDialog::AddressBookSourceDialog(weld::Window* pParent, const Reference< XComponentContext >& _rxORB,
        const Reference< XDataSource >& _rxTransientDS, const OUString& _rDataSourceName,
        const OUString& _rTable, const Sequence< AliasProgrammaticPair >& _rMapping )
        : GenericDialogController(pParent, "svt/ui/addresstemplatedialog.ui", "AddressTemplateDialog")
        , m_sNoFieldSelection(SvtResId(STR_NO_FIELD_SELECTION))
        , m_xORB(_rxORB)
        , m_pImpl( new AddressBookSourceDialogData( _rxTransientDS, _rDataSourceName, _rTable, _rMapping ) )
    {
        implConstruct();
    }

    void AddressBookSourceDialog::implConstruct()
    {
        m_xOKButton = m_xBuilder->weld_button("ok");
        m_xDatasource = m_xBuilder->weld_combo_box("datasource");
        m_xAdministrateDatasources = m_xBuilder->weld_button("admin");
        m_xTable = m_xBuilder->weld_combo_box("datatable");
        m_xFieldScroller = m_xBuilder->weld_scrolled_window("scrollwindow");
        m_xFieldScroller->set_user_managed_scrolling();
        m_xGrid = m_xBuilder->weld_widget("grid");
        m_xFieldScroller->set_size_request(-1, m_xGrid->get_preferred_size().Height());

        for (sal_Int32 row=0; row<FIELD_PAIRS_VISIBLE; ++row)
        {
            for (sal_Int32 column=0; column<2; ++column)
            {
                // the label
                m_pImpl->pFieldLabels[row * 2 + column] = m_xBuilder->weld_label(OString("label") + OString::number(row * 2 + column));
                // the listbox
                m_pImpl->pFields[row * 2 + column] = m_xBuilder->weld_combo_box(OString("box") + OString::number(row * 2 + column));
                m_pImpl->pFields[row * 2 + column]->connect_changed(LINK(this, AddressBookSourceDialog, OnFieldSelect));
            }
        }

        initializeDatasources();

        // for the moment, we have a hard coded list of all known fields.
        // A better solution would be to store all known field translations in the configuration, which could be
        // extensible by the user in an arbitrary way.
        // But for the moment we need a quick solution ...
        // (the main thing would be to store the translations to use here in the user interface, besides that, the code
        // should be adjustable with a rather small effort.)

        // initialize the strings for the field labels
        m_pImpl->aFieldLabels.push_back( SvtResId( STR_FIELD_FIRSTNAME ));
        m_pImpl->aFieldLabels.push_back( SvtResId( STR_FIELD_LASTNAME ));
        m_pImpl->aFieldLabels.push_back( SvtResId( STR_FIELD_COMPANY));
        m_pImpl->aFieldLabels.push_back( SvtResId( STR_FIELD_DEPARTMENT ));
        m_pImpl->aFieldLabels.push_back( SvtResId( STR_FIELD_STREET ));
        m_pImpl->aFieldLabels.push_back( SvtResId( STR_FIELD_ZIPCODE ));
        m_pImpl->aFieldLabels.push_back( SvtResId( STR_FIELD_CITY ));
        m_pImpl->aFieldLabels.push_back( SvtResId( STR_FIELD_STATE));
        m_pImpl->aFieldLabels.push_back( SvtResId( STR_FIELD_COUNTRY ));
        m_pImpl->aFieldLabels.push_back( SvtResId( STR_FIELD_HOMETEL ));
        m_pImpl->aFieldLabels.push_back( SvtResId( STR_FIELD_WORKTEL ));
        m_pImpl->aFieldLabels.push_back( SvtResId( STR_FIELD_OFFICETEL));
        m_pImpl->aFieldLabels.push_back( SvtResId( STR_FIELD_MOBILE));
        m_pImpl->aFieldLabels.push_back( SvtResId( STR_FIELD_TELOTHER));
        m_pImpl->aFieldLabels.push_back( SvtResId( STR_FIELD_PAGER));
        m_pImpl->aFieldLabels.push_back( SvtResId( STR_FIELD_FAX ));
        m_pImpl->aFieldLabels.push_back( SvtResId( STR_FIELD_EMAIL ));
        m_pImpl->aFieldLabels.push_back( SvtResId( STR_FIELD_URL ));
        m_pImpl->aFieldLabels.push_back( SvtResId( STR_FIELD_TITLE ));
        m_pImpl->aFieldLabels.push_back( SvtResId( STR_FIELD_POSITION ));
        m_pImpl->aFieldLabels.push_back( SvtResId( STR_FIELD_INITIALS ));
        m_pImpl->aFieldLabels.push_back( SvtResId( STR_FIELD_ADDRFORM ));
        m_pImpl->aFieldLabels.push_back( SvtResId( STR_FIELD_SALUTATION ));
        m_pImpl->aFieldLabels.push_back( SvtResId( STR_FIELD_ID));
        m_pImpl->aFieldLabels.push_back( SvtResId( STR_FIELD_CALENDAR));
        m_pImpl->aFieldLabels.push_back( SvtResId( STR_FIELD_INVITE));
        m_pImpl->aFieldLabels.push_back( SvtResId( STR_FIELD_NOTE));
        m_pImpl->aFieldLabels.push_back( SvtResId( STR_FIELD_USER1));
        m_pImpl->aFieldLabels.push_back( SvtResId( STR_FIELD_USER2));
        m_pImpl->aFieldLabels.push_back( SvtResId( STR_FIELD_USER3));
        m_pImpl->aFieldLabels.push_back( SvtResId( STR_FIELD_USER4));

        long nLabelWidth = 0;
        long nListBoxWidth = m_pImpl->pFields[0]->get_approximate_digit_width() * 18;
        for (auto const& fieldLabel : m_pImpl->aFieldLabels)
        {
            m_pImpl->pFieldLabels[0]->set_label(fieldLabel);
            nLabelWidth = std::max(nLabelWidth, m_pImpl->pFieldLabels[0]->get_preferred_size().Width());
        }
        for (sal_Int32 row=0; row<FIELD_PAIRS_VISIBLE; ++row)
        {
            for (sal_Int32 column=0; column<2; ++column)
            {
                m_pImpl->pFieldLabels[row * 2 + column]->set_size_request(nLabelWidth, -1);
                m_pImpl->pFields[row * 2 + column]->set_size_request(nListBoxWidth, -1);
            }
        }

        // force a even number of known fields
        m_pImpl->bOddFieldNumber = (m_pImpl->aFieldLabels.size() % 2) != 0;
        if (m_pImpl->bOddFieldNumber)
            m_pImpl->aFieldLabels.emplace_back();

        // limit the scrollbar range accordingly
        sal_Int32 nOverallFieldPairs = m_pImpl->aFieldLabels.size() / 2;
        m_xFieldScroller->vadjustment_configure(0, 0, nOverallFieldPairs,
                                                1, FIELD_PAIRS_VISIBLE - 1, FIELD_PAIRS_VISIBLE);

        // reset the current field assignments
        m_pImpl->aFieldAssignments.resize(m_pImpl->aFieldLabels.size());
        // (empty strings mean "no assignment")

        // some knittings
        m_xFieldScroller->connect_vadjustment_changed(LINK(this, AddressBookSourceDialog, OnFieldScroll));
        m_xAdministrateDatasources->connect_clicked(LINK(this, AddressBookSourceDialog, OnAdministrateDatasources));
        m_xDatasource->set_entry_completion(true);
        m_xTable->set_entry_completion(true);
        m_xTable->connect_focus_in(LINK(this, AddressBookSourceDialog, OnComboGetFocus));
        m_xDatasource->connect_focus_in(LINK(this, AddressBookSourceDialog, OnComboGetFocus));
        m_xTable->connect_focus_out(LINK(this, AddressBookSourceDialog, OnComboLoseFocus));
        m_xDatasource->connect_focus_out(LINK(this, AddressBookSourceDialog, OnComboLoseFocus));
        m_xTable->connect_changed(LINK(this, AddressBookSourceDialog, OnComboSelect));
        m_xDatasource->connect_changed(LINK(this, AddressBookSourceDialog, OnComboSelect));
        m_xOKButton->connect_clicked(LINK(this, AddressBookSourceDialog, OnOkClicked));

        // initialize the field controls
        resetFields();
        m_xFieldScroller->vadjustment_set_value(0);
        m_pImpl->nFieldScrollPos = -1;
        implScrollFields(0, false, false);

        // the logical names
        OUString sLogicalFieldNames(STR_LOGICAL_FIELD_NAMES);
        sal_Int32 nAdjustedTokenCount = comphelper::string::getTokenCount(sLogicalFieldNames, ';') + (m_pImpl->bOddFieldNumber ? 1 : 0);
        DBG_ASSERT(nAdjustedTokenCount == static_cast<sal_Int32>(m_pImpl->aFieldLabels.size()),
            "AddressBookSourceDialog::AddressBookSourceDialog: inconsistence between logical and UI field names!");
        m_pImpl->aLogicalFieldNames.reserve(nAdjustedTokenCount);
        sal_Int32 nIdx{ 0 };
        for (sal_Int32 i = 0; i<nAdjustedTokenCount; ++i)
            m_pImpl->aLogicalFieldNames.push_back(sLogicalFieldNames.getToken(0, ';', nIdx));

        Application::PostUserEvent(LINK(this, AddressBookSourceDialog, OnDelayedInitialize), nullptr, false);

        // so the dialog will at least show up before we do the loading of the
        // configuration data and the (maybe time consuming) analysis of the data source/table to select

        if (m_pImpl->bWorkingPersistent)
            return;

        m_xAdministrateDatasources->hide();
    }

    void AddressBookSourceDialog::getFieldMapping(Sequence< AliasProgrammaticPair >& _rMapping) const
    {
        _rMapping.realloc( m_pImpl->aLogicalFieldNames.size() );
        AliasProgrammaticPair* pPair = _rMapping.getArray();

        for (auto const& logicalFieldName : m_pImpl->aLogicalFieldNames)
        {
            if ( m_pImpl->pConfigData->hasFieldAssignment(logicalFieldName) )
            {
                // the user gave us an assignment for this field
                pPair->ProgrammaticName = logicalFieldName;
                pPair->Alias = m_pImpl->pConfigData->getFieldAssignment(logicalFieldName);
                ++pPair;
            }
        }

        _rMapping.realloc( pPair - _rMapping.getArray() );
    }

    void AddressBookSourceDialog::loadConfiguration()
    {
        OUString sName = m_pImpl->pConfigData->getDatasourceName();
        INetURLObject aURL( sName );
        if( aURL.GetProtocol() != INetProtocol::NotValid )
        {
            OFileNotation aFileNotation( aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
            sName = aFileNotation.get(OFileNotation::N_SYSTEM);
        }

        m_xDatasource->set_entry_text(sName);
        m_xTable->set_entry_text(m_pImpl->pConfigData->getCommand());
        // we ignore the CommandType: only tables are supported

        // the logical names for the fields
        // AddressBookSourceDialog::loadConfiguration: inconsistence between field names and field assignments!
        assert(m_pImpl->aLogicalFieldNames.size() == m_pImpl->aFieldAssignments.size());

        auto aAssignment = m_pImpl->aFieldAssignments.begin();
        for (auto const& logicalFieldName : m_pImpl->aLogicalFieldNames)
        {
            *aAssignment = m_pImpl->pConfigData->getFieldAssignment(logicalFieldName);
            ++aAssignment;
        }
    }

    AddressBookSourceDialog::~AddressBookSourceDialog()
    {
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
                ShowServiceNotAvailableError(m_xDialog.get(), sContextServiceName, false);
                return;
            }
        }
        m_xDatasource->clear();

        // fill the datasources listbox
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
            m_xDatasource->append_text(*pDatasourceNames);
    }

    IMPL_LINK(AddressBookSourceDialog, OnFieldScroll, weld::ScrolledWindow&, rScrollBar, void)
    {
        implScrollFields(rScrollBar.vadjustment_get_value(), true, true);
    }

    void AddressBookSourceDialog::resetTables()
    {
        if (!m_xDatabaseContext.is())
            return;

        weld::WaitObject aWaitCursor(m_xDialog.get());

        // no matter what we do here, we handled the currently selected data source (no matter if successful or not)
        m_xDatasource->save_value();

        // create an interaction handler (may be needed for connecting)
        Reference< XInteractionHandler > xHandler;
        try
        {
            xHandler.set(
                InteractionHandler::createWithParent(m_xORB, m_xDialog->GetXWindow()),
                UNO_QUERY_THROW );
        }
        catch(const Exception&) { }
        if (!xHandler.is())
        {
            const OUString sInteractionHandlerServiceName("com.sun.star.task.InteractionHandler");
            ShowServiceNotAvailableError(m_xDialog.get(), sInteractionHandlerServiceName, true);
            return;
        }

        // the currently selected table
        OUString sOldTable = m_xTable->get_active_text();

        m_xTable->clear();

        m_xCurrentDatasourceTables= nullptr;

        // get the tables of the connection
        Sequence< OUString > aTableNames;
        Any aException;
        try
        {
            Reference< XCompletedConnection > xDS;
            if ( m_pImpl->bWorkingPersistent )
            {
                OUString sSelectedDS = lcl_getSelectedDataSource(*m_xDatasource);

                // get the data source the user has chosen and let it build a connection
                INetURLObject aURL( sSelectedDS );
                if ( aURL.GetProtocol() != INetProtocol::NotValid || m_xDatabaseContext->hasByName(sSelectedDS) )
                    m_xDatabaseContext->getByName( sSelectedDS ) >>= xDS;
            }
            else
            {
                xDS.set(m_pImpl->m_xTransientDataSource, css::uno::UNO_QUERY);
            }

            // build the connection
            Reference< XConnection > xConn;
            if (xDS.is())
                xConn = xDS->connectWithCompletion(xHandler);

            // get the table names
            Reference< XTablesSupplier > xSupplTables(xConn, UNO_QUERY);
            if (xSupplTables.is())
            {
                m_xCurrentDatasourceTables.set(xSupplTables->getTables(), UNO_QUERY);
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

        bool bKnowOldTable = false;
        // fill the table list
        const OUString* pTableNames = aTableNames.getConstArray();
        const OUString* pEnd = pTableNames + aTableNames.getLength();
        for (;pTableNames != pEnd; ++pTableNames)
        {
            m_xTable->append_text(*pTableNames);
            if (*pTableNames == sOldTable)
                bKnowOldTable = true;
        }

        // set the old table, if the new data source knows a table with this name, too. Else reset the tables edit field.
        if (!bKnowOldTable)
            sOldTable.clear();
        m_xTable->set_entry_text(sOldTable);

        resetFields();
    }

    void AddressBookSourceDialog::resetFields()
    {
        weld::WaitObject aWaitCursor(m_xDialog.get());

        // no matter what we do here, we handled the currently selected table (no matter if successful or not)
        m_xDatasource->save_value();

        OUString sSelectedTable = m_xTable->get_active_text();
        Sequence< OUString > aColumnNames;
        try
        {
            if (m_xCurrentDatasourceTables.is())
            {
                // get the table and the columns
                Reference< XColumnsSupplier > xSuppTableCols;
                if (m_xCurrentDatasourceTables->hasByName(sSelectedTable))
                    xSuppTableCols.set(
                        m_xCurrentDatasourceTables->getByName(sSelectedTable),
                        css::uno::UNO_QUERY);
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

        // for quicker access
        ::std::set< OUString > aColumnNameSet;
        for (pColumnNames = aColumnNames.getConstArray(); pColumnNames != pEnd; ++pColumnNames)
            aColumnNameSet.insert(*pColumnNames);

        std::vector<OUString>::iterator aInitialSelection = m_pImpl->aFieldAssignments.begin() + m_pImpl->nFieldScrollPos;

        OUString sSaveSelection;
        for (sal_Int32 i=0; i<FIELD_CONTROLS_VISIBLE; ++i, ++aInitialSelection)
        {
            weld::ComboBox* pListbox = m_pImpl->pFields[i].get();
            sSaveSelection = pListbox->get_active_text();

            pListbox->clear();

            // the one entry for "no selection"
            pListbox->append_text(m_sNoFieldSelection);
            // as it's entry data, set the index of the list box in our array
            pListbox->set_id(0, OUString::number(i));

            // the field names
            for (pColumnNames = aColumnNames.getConstArray(); pColumnNames != pEnd; ++pColumnNames)
                pListbox->append_text(*pColumnNames);

            if (!aInitialSelection->isEmpty() && (aColumnNameSet.end() != aColumnNameSet.find(*aInitialSelection)))
                // we can select the entry as specified in our field assignment array
                pListbox->set_active_text(*aInitialSelection);
            else
                // try to restore the selection
                if (aColumnNameSet.end() != aColumnNameSet.find(sSaveSelection))
                    // the old selection is a valid column name
                    pListbox->set_active_text(sSaveSelection);
                else
                    // select the <none> entry
                    pListbox->set_active(0);
        }

        // adjust m_pImpl->aFieldAssignments
        for (auto & fieldAssignment : m_pImpl->aFieldAssignments)
            if (!fieldAssignment.isEmpty())
                if (aColumnNameSet.end() == aColumnNameSet.find(fieldAssignment))
                    fieldAssignment.clear();
    }

    IMPL_LINK(AddressBookSourceDialog, OnFieldSelect, weld::ComboBox&, rListbox, void)
    {
        // the index of the affected list box in our array
        sal_Int32 nListBoxIndex = rListbox.get_id(0).toInt32();
        DBG_ASSERT(nListBoxIndex >= 0 && nListBoxIndex < FIELD_CONTROLS_VISIBLE,
            "AddressBookSourceDialog::OnFieldScroll: invalid list box entry!");

        // update the array where we remember the field selections
        if (0 == rListbox.get_active())
            // it's the "no field selection" entry
            m_pImpl->aFieldAssignments[m_pImpl->nFieldScrollPos * 2 + nListBoxIndex].clear();
        else
            // it's a regular field entry
            m_pImpl->aFieldAssignments[m_pImpl->nFieldScrollPos * 2 + nListBoxIndex] = rListbox.get_active_text();
    }


    void AddressBookSourceDialog::implScrollFields(sal_Int32 _nPos, bool _bAdjustFocus, bool _bAdjustScrollbar)
    {
        if (_nPos == m_pImpl->nFieldScrollPos)
            // nothing to do
            return;

        // loop through our field control rows and do some adjustments
        // for the new texts
        auto pLeftLabelControl = m_pImpl->pFieldLabels.begin();
        auto pRightLabelControl = pLeftLabelControl+1;
        auto pLeftColumnLabel = m_pImpl->aFieldLabels.cbegin() + 2 * _nPos;
        auto pRightColumnLabel = pLeftColumnLabel + 1;

        // for the focus movement and the selection scroll
        auto pLeftListControl = m_pImpl->pFields.begin();
        auto pRightListControl = pLeftListControl + 1;

        // for the focus movement
        sal_Int32 nOldFocusRow = -1;
        sal_Int32 nOldFocusColumn = 0;

        // for the selection scroll
        auto pLeftAssignment = m_pImpl->aFieldAssignments.cbegin() + 2 * _nPos;
        auto pRightAssignment = pLeftAssignment + 1;

        // loop
        for (sal_Int32 i=0; i<FIELD_PAIRS_VISIBLE; ++i)
        {
            if ((*pLeftListControl)->has_focus())
            {
                nOldFocusRow = i;
                nOldFocusColumn = 0;
            }
            else if ((*pRightListControl)->has_focus())
            {
                nOldFocusRow = i;
                nOldFocusColumn = 1;
            }

            // the new texts of the label controls
            (*pLeftLabelControl)->set_label(*pLeftColumnLabel);
            (*pRightLabelControl)->set_label(*pRightColumnLabel);

            // we may have to hide the controls in the right column, if we have no label text for it
            // (which means we have an odd number of fields, though we forced our internal arrays to
            // be even-sized for easier handling)
            // (If sometimes we support an arbitrary number of field assignments, we would have to care for
            // an invisible left hand side column, too. But right now, the left hand side controls are always
            // visible)
            bool bHideRightColumn = pRightColumnLabel->isEmpty();
            (*pRightLabelControl)->set_visible(!bHideRightColumn);
            (*pRightListControl)->set_visible(!bHideRightColumn);
            // the new selections of the listboxes
            implSelectField(pLeftListControl->get(), *pLeftAssignment);
            implSelectField(pRightListControl->get(), *pRightAssignment);

            // increment ...
            if ( i < FIELD_PAIRS_VISIBLE - 1 )
            {   // (not in the very last round, here the +=2 could result in an invalid
                // iterator position, which causes an abort in a non-product version
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
        {   // we have to adjust the focus and one of the list boxes has the focus
            sal_Int32 nDelta = m_pImpl->nFieldScrollPos - _nPos;
            // the new row for the focus
            sal_Int32 nNewFocusRow = nOldFocusRow + nDelta;
            // normalize
            nNewFocusRow = std::min(nNewFocusRow, sal_Int32(FIELD_PAIRS_VISIBLE - 1), ::std::less< sal_Int32 >());
            nNewFocusRow = std::max(nNewFocusRow, sal_Int32(0), ::std::less< sal_Int32 >());
            // set the new focus (in the same column)
            m_pImpl->pFields[nNewFocusRow * 2 + nOldFocusColumn]->grab_focus();
        }

        m_pImpl->nFieldScrollPos = _nPos;

        if (_bAdjustScrollbar)
            m_xFieldScroller->vadjustment_set_value(m_pImpl->nFieldScrollPos);
    }

    void AddressBookSourceDialog::implSelectField(weld::ComboBox* pBox, const OUString& rText)
    {
        if (!rText.isEmpty())
            // a valid field name
            pBox->set_active_text(rText);
        else
            // no selection for this item
            pBox->set_active(0);
    }

    IMPL_LINK_NOARG(AddressBookSourceDialog, OnDelayedInitialize, void*, void)
    {
        // load the initial data from the configuration
        loadConfiguration();
        resetTables();
            // will reset the tables/fields implicitly

        if ( !m_pImpl->bWorkingPersistent )
            if ( m_pImpl->pFields[0] )
                m_pImpl->pFields[0]->grab_focus();
    }

    IMPL_LINK(AddressBookSourceDialog, OnComboSelect, weld::ComboBox&, rBox, void)
    {
        if (&rBox == m_xDatasource.get())
            resetTables();
        else
            resetFields();
    }

    IMPL_STATIC_LINK(AddressBookSourceDialog, OnComboGetFocus, weld::Widget&, rBox, void)
    {
        dynamic_cast<weld::ComboBox&>(rBox).save_value();
    }

    IMPL_LINK(AddressBookSourceDialog, OnComboLoseFocus, weld::Widget&, rControl, void)
    {
        weld::ComboBox& rBox = dynamic_cast<weld::ComboBox&>(rControl);
        if (rBox.get_value_changed_from_saved())
        {
            if (&rBox == m_xDatasource.get())
                resetTables();
            else
                resetFields();
        }
    }

    IMPL_LINK_NOARG(AddressBookSourceDialog, OnOkClicked, weld::Button&, void)
    {
        OUString sSelectedDS = lcl_getSelectedDataSource(*m_xDatasource);
        if ( m_pImpl->bWorkingPersistent )
        {
            m_pImpl->pConfigData->setDatasourceName(sSelectedDS);
            m_pImpl->pConfigData->setCommand(m_xTable->get_active_text());
        }

        // AddressBookSourceDialog::loadConfiguration: inconsistence between field names and field assignments!
        assert(m_pImpl->aLogicalFieldNames.size() == m_pImpl->aFieldAssignments.size());

        // set the field assignments
        auto aAssignment = m_pImpl->aFieldAssignments.cbegin();
        for (auto const& logicalFieldName : m_pImpl->aLogicalFieldNames)
        {
            m_pImpl->pConfigData->setFieldAssignment(logicalFieldName, *aAssignment);
            ++aAssignment;
        }

        m_xDialog->response(RET_OK);
    }

    IMPL_LINK_NOARG(AddressBookSourceDialog, OnAdministrateDatasources, weld::Button&, void)
    {
        // create the dialog object
        Reference< XExecutableDialog > xAdminDialog;
        try
        {
            xAdminDialog = AddressBookSourcePilot::createWithParent(m_xORB, m_xDialog->GetXWindow());
        }
        catch(const Exception&) { }
        if (!xAdminDialog.is())
        {
            ShowServiceNotAvailableError(m_xDialog.get(), "com.sun.star.ui.dialogs.AddressBookSourcePilot", true);
            return;
        }

        // execute the dialog
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
                    if( aURL.GetProtocol() != INetProtocol::NotValid )
                    {
                        OFileNotation aFileNotation( aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
                        sName = aFileNotation.get(OFileNotation::N_SYSTEM);
                    }
                    m_xDatasource->append_text(sName);
                    m_pImpl->pConfigData.reset( new AssignmentPersistentData );
                    loadConfiguration();
                    resetTables();
                    // will reset the fields implicitly
                }
            }
        }
        catch(const Exception&)
        {
            OSL_FAIL("AddressBookSourceDialog::OnAdministrateDatasources: an error occurred while executing the administration dialog!");
        }

        // re-fill the data source list
        // try to preserve the current selection

//      initializeDatasources();
    }

}   // namespace svt


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
