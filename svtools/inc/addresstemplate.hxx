/*************************************************************************
 *
 *  $RCSfile: addresstemplate.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: fs $ $Date: 2001-07-30 16:40:13 $
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
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_ALIASPROGRAMMATICPAIR_HPP_
#include <com/sun/star/util/AliasProgrammaticPair.hpp>
#endif
#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

// .......................................................................
namespace svt
{
// .......................................................................

    // ===================================================================
    // = AddressBookSourceDialog
    // ===================================================================
    struct AddressBookSourceDialogData;
    class AddressBookSourceDialog : public ModalDialog
    {
    protected:
        // Controls
        FixedLine       m_aDatasourceFrame;
        FixedText       m_aDatasourceLabel;
        ComboBox        m_aDatasource;
        PushButton      m_aAdministrateDatasources;
        FixedText       m_aTableLabel;
        ComboBox        m_aTable;

        FixedText       m_aFieldsTitle;
        Window          m_aFieldsFrame;

        ScrollBar       m_aFieldScroller;
        OKButton        m_aOK;
        CancelButton    m_aCancel;
        HelpButton      m_aHelp;

        // string to display for "no selection"
        const String    m_sNoFieldSelection;

        AddressBookSourceDialogData*
                        m_pImpl;

        /// the DatabaseContext for selecting data sources
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                        m_xDatabaseContext;
        // the ORB for creating objects
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                        m_xORB;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                        m_xCurrentDatasourceTables;

    public:
        AddressBookSourceDialog( Window* _pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB );

        // if you use this ctor, the dialog
        // * will not store it's data in the configuration (nor initially retrieve it from there)
        // * will not allow to change the data source name
        // * will not allow to change the table name
        // * will not allow to call the data source administration dialog
        AddressBookSourceDialog( Window* _pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            const ::rtl::OUString& _rDS,
            const ::rtl::OUString& _rTable,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::util::AliasProgrammaticPair >& _rMapping
        );

        ~AddressBookSourceDialog();

        // to be used if the object was constructed for editing a field mapping only
        void        getFieldMapping(
            ::com::sun::star::uno::Sequence< ::com::sun::star::util::AliasProgrammaticPair >& _rMapping) const;

    protected:
        void    implConstruct();

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

