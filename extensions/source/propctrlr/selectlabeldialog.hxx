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



#ifndef _EXTENSIONS_PROPCTRLR_SELECTLABELDIALOG_HXX_
#define _EXTENSIONS_PROPCTRLR_SELECTLABELDIALOG_HXX_

#include <vcl/fixed.hxx>
#include <svtools/svtreebx.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/image.hxx>
#include <vcl/dialog.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#ifndef _EXTENSIONS_PROPCTRLR_MODULEPRC_HXX_
#include "modulepcr.hxx"
#endif

//............................................................................
namespace pcr
{
//............................................................................

    //========================================================================
    // OSelectLabelDialog
    //========================================================================
    class OSelectLabelDialog
            :public ModalDialog
            ,public PcrClient
    {
        FixedText       m_aMainDesc;
        SvTreeListBox   m_aControlTree;
        CheckBox        m_aNoAssignment;
        FixedLine       m_aSeparator;
        OKButton        m_aOk;
        CancelButton    m_aCancel;

        ImageList       m_aModelImages;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   m_xControlModel;
        ::rtl::OUString m_sRequiredService;
        Image           m_aRequiredControlImage;
        SvLBoxEntry*    m_pInitialSelection;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   m_xInitialLabelControl;

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   m_xSelectedControl;
        SvLBoxEntry*    m_pLastSelected;
        sal_Bool        m_bHaveAssignableControl;

    public:
        OSelectLabelDialog(Window* pParent, ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  _xControlModel);
        ~OSelectLabelDialog();

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  GetSelected() const { return m_aNoAssignment.IsChecked() ? ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > () : m_xSelectedControl; }

    protected:
        sal_Int32 InsertEntries(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xContainer, SvLBoxEntry* pContainerEntry);

        DECL_LINK(OnEntrySelected, SvTreeListBox*);
        DECL_LINK(OnNoAssignmentClicked, Button*);
    };

//............................................................................
}   // namespace pcr
//............................................................................

#endif // _EXTENSIONS_PROPCTRLR_SELECTLABELDIALOG_HXX_

