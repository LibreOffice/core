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



#ifndef _EXTENSIONS_DBP_COMMONPAGESDBP_HXX_
#define _EXTENSIONS_DBP_COMMONPAGESDBP_HXX_

#include "controlwizard.hxx"
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <com/sun/star/container/XNameAccess.hpp>

//.........................................................................
namespace dbp
{
//.........................................................................

    //=====================================================================
    //= OTableSelectionPage
    //=====================================================================
    class OTableSelectionPage : public OControlWizardPage
    {
    protected:
        FixedLine       m_aData;
        FixedText       m_aExplanation;
        FixedText       m_aDatasourceLabel;
        ListBox         m_aDatasource;
        PushButton      m_aSearchDatabase;
        FixedText       m_aTableLabel;
        ListBox         m_aTable;

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                        m_xDSContext;

    public:
        OTableSelectionPage(OControlWizard* _pParent);

    protected:
        // TabPage overridables
        void ActivatePage();

        // OWizardPage overridables
        virtual void        initializePage();
        virtual sal_Bool    commitPage( ::svt::WizardTypes::CommitPageReason _eReason );

    protected:
        DECL_LINK( OnListboxSelection, ListBox* );
        DECL_LINK( OnListboxDoubleClicked, ListBox* );
        DECL_LINK( OnSearchClicked, PushButton* );

        void implCollectDatasource();
        void implFillTables(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >&
                        _rxConn = ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >());

        // OControlWizardPage overridables
        virtual bool    canAdvance() const;
    };

    //=====================================================================
    //= OMaybeListSelectionPage
    //=====================================================================
    class OMaybeListSelectionPage : public OControlWizardPage
    {
    protected:
        RadioButton*    m_pYes;
        RadioButton*    m_pNo;
        ListBox*        m_pList;

    public:
        OMaybeListSelectionPage( OControlWizard* _pParent, const ResId& _rId );

    protected:
        DECL_LINK( OnRadioSelected, RadioButton* );

        // TabPage overridables
        void ActivatePage();

        // own helper
        void    announceControls(
            RadioButton& _rYesButton,
            RadioButton& _rNoButton,
            ListBox& _rSelection);

        void implEnableWindows();

        void implInitialize(const String& _rSelection);
        void implCommit(String& _rSelection);
    };

    //=====================================================================
    //= ODBFieldPage
    //=====================================================================
    class ODBFieldPage : public OMaybeListSelectionPage
    {
    protected:
        FixedLine       m_aFrame;
        FixedText       m_aDescription;
        FixedText       m_aQuestion;
        RadioButton     m_aStoreYes;
        RadioButton     m_aStoreNo;
        ListBox         m_aStoreWhere;

    public:
        ODBFieldPage( OControlWizard* _pParent );

    protected:
        void setDescriptionText(const String& _rDesc) { m_aDescription.SetText(_rDesc); }

        // OWizardPage overridables
        virtual void initializePage();
        virtual sal_Bool commitPage( ::svt::WizardTypes::CommitPageReason _eReason );

        // own overridables
        virtual String& getDBFieldSetting() = 0;
    };

//.........................................................................
}   // namespace dbp
//.........................................................................


#endif // _EXTENSIONS_DBP_COMMONPAGESDBP_HXX_

