/*************************************************************************
 *
 *  $RCSfile: addresstemplate.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-11-24 12:29:05 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SVT_ADDRESSTEMPLATE_HXX_
#define _SVT_ADDRESSTEMPLATE_HXX_

#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_GROUP_HXX
#include <vcl/group.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_COMBOBOX_HXX
#include <vcl/combobox.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/imagebtn.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_SCRBAR_HXX
#include <vcl/scrbar.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

// .......................................................................
namespace svt
{
// .......................................................................

    // ===================================================================
    // = AddressBookAssignment
    // ===================================================================
    class AddressBookAssignment : public ::utl::ConfigItem
    {
    protected:
        DECLARE_STL_STDKEY_SET( ::rtl::OUString, UStringBag );
        UStringBag      m_aStoredFields;

    protected:
        ::com::sun::star::uno::Any
                getProperty(const ::rtl::OUString& _rLocalName);
        ::com::sun::star::uno::Any
                        getProperty(const sal_Char* _pLocalName);

        ::rtl::OUString getStringProperty(const sal_Char* _pLocalName);
        sal_Int32       getInt32Property(const sal_Char* _pLocalName);

        ::rtl::OUString getStringProperty(const ::rtl::OUString& _rLocalName);

        void            setStringProperty(const sal_Char* _pLocalName, const ::rtl::OUString& _rValue);

    public:
        AddressBookAssignment();
        ~AddressBookAssignment();

        /// the data source to use for the address book
        ::rtl::OUString getDatasourceName();
        /// the command to use for the address book
        ::rtl::OUString getCommand();
        /** the command type to use for the address book
            @return
                a <type scope="com.sun.star.sdb">CommandType</type> value
        */
        sal_Int32       getCommandType();

        /// checks whether or not there is an assignment for a given logical field
        sal_Bool        hasFieldAssignment(const ::rtl::OUString& _rLogicalName);
        /// retrieves the assignment for a given logical field
        ::rtl::OUString getFieldAssignment(const ::rtl::OUString& _rLogicalName);

        /// set the assignment for a given logical field
        void            setFieldAssignment(const ::rtl::OUString& _rLogicalName, const ::rtl::OUString& _rAssignment);
        /// clear the assignment for a given logical field
        void            clearFieldAssignment(const ::rtl::OUString& _rLogicalName);

        void    setDatasourceName(const ::rtl::OUString& _rName);
        void    setCommand(const ::rtl::OUString& _rCommand);
    };

    // ===================================================================
    // = AddressBookSourceDialog
    // ===================================================================
    class AddressBookSourceDialog : public ModalDialog
    {
    protected:
        // Controls
        GroupBox        m_aDatasourceFrame;
        FixedText       m_aDatasourceLabel;
        ComboBox        m_aDatasource;
        PushButton      m_aAdministrateDatasources;
        FixedText       m_aTableLabel;
        ComboBox        m_aTable;

        GroupBox        m_aFieldsFrame;
        FixedText*      m_pFieldLabels[5 * 2];
        ListBox*        m_pFields[5 * 2];

        ScrollBar       m_aFieldScroller;
        OKButton        m_aOK;
        CancelButton    m_aCancel;

        // string to display for "no selection"
        const String    m_sNoFieldSelection;

        /// current scroll pos in the field list
        sal_Int32   m_nFieldScrollPos;
        /// the index within m_pFields of the last visible list box. This is redundant, it could be extracted from other members
        sal_Int32   m_nLastVisibleListIndex;
        /// indicates that we've an odd field number. This member is for efficiency only, it's redundant.
        sal_Bool    m_bOddFieldNumber : 1;

        DECLARE_STL_VECTOR( String, StringArray );
        /// the strings to use as labels for the field selection listboxes
        StringArray     m_aFieldLabels;
        // the current field assignment
        StringArray     m_aFieldAssignments;
        /// the logical field names
        StringArray     m_aLogicalFieldNames;

        /// the DatabaseContext for selecting data sources
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                        m_xDatabaseContext;
        // the ORB for creating objects
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                        m_xORB;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                        m_xCurrentDatasourceTables;

        AddressBookAssignment
                        m_aConfigData;

    public:
        AddressBookSourceDialog( Window* _pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB );

        ~AddressBookSourceDialog();

    protected:
        // Window overridables
        virtual long        PreNotify( NotifyEvent& _rNEvt );

        // implementations
        void    implScrollFields(sal_Int32 _nPos, sal_Bool _bAdjustFocus, sal_Bool _bAdjustScrollbar);
        void    implSelectField(ListBox* _pBox, const String& _rText);

        void    initalizeListBox(ListBox* _pList);
        void    resetTables();
        void    resetFields();

        // fill in the data sources listbox
        void    initializeDatasources();

        // initialize the dialog from the configuration data
        void    loadConfiguration();

        DECL_LINK(OnFieldScroll, ScrollBar*);
        DECL_LINK(OnFieldSelect, ListBox*);
        DECL_LINK(OnAdministrateDatasources, void*);
        DECL_LINK(OnComboGetFocus, ComboBox*);
        DECL_LINK(OnComboLoseFocus, ComboBox*);
        DECL_LINK(OnComboSelect, ComboBox*);
        DECL_LINK(OnOkClicked, Button*);
        DECL_LINK(OnDelayedInitialize, void*);
    };


// .......................................................................
}   // namespace svt
// .......................................................................

#endif // _SVT_ADDRESSTEMPLATE_HXX_

