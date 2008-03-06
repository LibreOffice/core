/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: typeselectionpage.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:39:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef EXTENSIONS_ABP_TYPESELECTIONPAGE_HXX
#define EXTENSIONS_ABP_TYPESELECTIONPAGE_HXX

#ifndef EXTENSIONS_ABP_ABSPAGE_HXX
#include "abspage.hxx"
#endif
#ifndef EXTENSIONS_ABP_ADDRESSSETTINGS_HXX
#include "addresssettings.hxx"
#endif
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif

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

