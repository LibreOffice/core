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



#ifndef EXTENSIONS_ABP_TYPESELECTIONPAGE_HXX
#define EXTENSIONS_ABP_TYPESELECTIONPAGE_HXX

#include "abspage.hxx"
#include "addresssettings.hxx"
#include <vcl/edit.hxx>

//.........................................................................
namespace abp
{
//.........................................................................

    //=====================================================================
    //= TypeSelectionPage
    //=====================================================================
    class TypeSelectionPage : public AddressBookSourcePage
    {
    protected:
        FixedText       m_aHint;
        FixedLine       m_aTypeSep;
        RadioButton     m_aEvolution;
        RadioButton     m_aEvolutionGroupwise;
        RadioButton     m_aEvolutionLdap;
        RadioButton     m_aMORK;
        RadioButton     m_aThunderbird;
        RadioButton     m_aKab;
        RadioButton     m_aMacab;
        RadioButton     m_aLDAP;
        RadioButton     m_aOutlook;
        RadioButton     m_aOE;
        RadioButton     m_aOther;

        struct ButtonItem {
            RadioButton *m_pItem;
            AddressSourceType m_eType;
            bool         m_bVisible;

            ButtonItem( RadioButton *pItem,
                        AddressSourceType eType,
                        bool         bVisible ) :
                    m_pItem( pItem ),
                    m_eType( eType ),
                    m_bVisible( bVisible )
            {}
        };

        ::std::vector< ButtonItem > m_aAllTypes;

    public:
        TypeSelectionPage( OAddessBookSourcePilot* _pParent );
        ~TypeSelectionPage();

        // retrieves the currently selected type
        AddressSourceType   getSelectedType() const;

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
        DECL_LINK( OnTypeSelected, void* );

        void                selectType( AddressSourceType _eType );
    };

//.........................................................................
}   // namespace abp
//.........................................................................

#endif // EXTENSIONS_ABP_TYPESELECTIONPAGE_HXX

