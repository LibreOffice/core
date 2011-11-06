/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef EXTENSIONS_SOURCE_PROPCTRLR_LISTSELECTIONDLG_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_LISTSELECTIONDLG_HXX

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/lstbox.hxx>

/** === begin UNO includes === **/
#include <com/sun/star/beans/XPropertySet.hpp>
/** === end UNO includes === **/

//........................................................................
namespace pcr
{
//........................................................................

    //====================================================================
    //= ListSelectionDialog
    //====================================================================
    class ListSelectionDialog : public ModalDialog
    {
    private:
        FixedText       m_aLabel;
        ListBox         m_aEntries;
        OKButton        m_aOK;
        CancelButton    m_aCancel;
        HelpButton      m_aHelp;

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                        m_xListBox;
        ::rtl::OUString m_sPropertyName;

    public:
        ListSelectionDialog(
            Window* _pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxListBox,
            const ::rtl::OUString& _rPropertyName,
            const String& _rPropertyUIName
        );

        // Dialog overridables
        virtual short   Execute();

    private:
        void    initialize( );
        void    commitSelection();

        void    fillEntryList   ( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rListEntries );

        void    selectEntries   ( const ::com::sun::star::uno::Sequence< sal_Int16 >& /* [in ] */ _rSelection );
        void    collectSelection(       ::com::sun::star::uno::Sequence< sal_Int16 >& /* [out] */ _rSelection );
    };

//........................................................................
} // namespacepcr
//........................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_LISTSELECTIONDLG_HXX

