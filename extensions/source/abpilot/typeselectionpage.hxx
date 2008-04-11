/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: typeselectionpage.hxx,v $
 * $Revision: 1.12 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
        RadioButton     m_aMORK;
        RadioButton     m_aThunderbird;
        RadioButton     m_aEvolutionGroupwise;
        RadioButton     m_aEvolutionLdap;
        RadioButton     m_aEvolution;
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

        // retrieves the currently selected type
        AddressSourceType   getSelectedType() const;

    protected:
        // OWizardPage overridables
        virtual void        initializePage();
        virtual sal_Bool    commitPage( CommitPageReason _eReason );

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

