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



#ifndef EXTENSIONS_ABP_TABLESELECTIONPAGE_HXX
#define EXTENSIONS_ABP_TABLESELECTIONPAGE_HXX

#include "abspage.hxx"
#include <vcl/lstbox.hxx>

//.........................................................................
namespace abp
{
//.........................................................................

    //=====================================================================
    //= TableSelectionPage
    //=====================================================================
    class TableSelectionPage : public AddressBookSourcePage
    {
    protected:
        FixedText       m_aLabel;
        ListBox         m_aTableList;

    public:
        TableSelectionPage( OAddessBookSourcePilot* _pParent );

    protected:
        // OWizardPage overridables
        virtual void        initializePage();
        virtual sal_Bool    commitPage( ::svt::WizardTypes::CommitPageReason _eReason );

        // TabDialog overridables
        virtual void        ActivatePage();
        virtual void        DeactivatePage();

        // OImportPage overridables
        virtual bool        canAdvance() const;

    private:
        DECL_LINK( OnTableSelected, void* );
        DECL_LINK( OnTableDoubleClicked, void* );
    };

//.........................................................................
}   // namespace abp
//.........................................................................

#endif // EXTENSIONS_ABP_TABLESELECTIONPAGE_HXX

