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


#ifndef RPTUI_PAGENUMBER_HXX
#define RPTUI_PAGENUMBER_HXX

#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#include <vcl/lstbox.hxx>
#include <vcl/field.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <com/sun/star/report/XReportDefinition.hpp>


namespace rptui
{
class OReportController;
/*************************************************************************
|*
|* Groups and Sorting dialog
|*
\************************************************************************/
class OPageNumberDialog :   public ModalDialog
{
    FixedLine                               m_aFormat;
    RadioButton                             m_aPageN;
    RadioButton                             m_aPageNofM;

    FixedLine                               m_aPosition;
    RadioButton                             m_aTopPage;
    RadioButton                             m_aBottomPage;
    FixedLine                               m_aMisc;
    FixedText                               m_aAlignment;
    ListBox                                 m_aAlignmentLst;

    CheckBox                                m_aShowNumberOnFirstPage;
    FixedLine                               m_aFl1;
    OKButton                                m_aPB_OK;
    CancelButton                            m_aPB_CANCEL;
    HelpButton                              m_aPB_Help;


    ::rptui::OReportController*             m_pController;
    ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition>
                                            m_xHoldAlive;

    OPageNumberDialog(const OPageNumberDialog&);
    void operator =(const OPageNumberDialog&);
public:
    OPageNumberDialog( Window* pParent
                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition>& _xHoldAlive
                        ,::rptui::OReportController* _pController);
    virtual ~OPageNumberDialog();
    virtual short   Execute();
};
// =============================================================================
} // namespace rptui
// =============================================================================
#endif // RPTUI_PAGENUMBER_HXX
