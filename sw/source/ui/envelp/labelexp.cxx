/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/AutoTextContainer.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <comphelper/processfactory.hxx>
#include <svtools/treelistentry.hxx>
#include <swtypes.hxx>
#include <labfmt.hxx>
#include <unotools.hxx>
#include <unoatxt.hxx>
#include <unomid.h>
#include <unoprnms.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::uno;
using namespace ::comphelper;

void SwVisitingCardPage::InitFrameControl()
{
    Link<SwOneExampleFrame&,void> aLink(LINK(this, SwVisitingCardPage, FrameControlInitializedHdl));
    m_pExampleWIN->Show();
    pExampleFrame = new SwOneExampleFrame( *m_pExampleWIN,
                                    EX_SHOW_BUSINESS_CARDS, &aLink );

    Reference< XComponentContext > xContext = comphelper::getProcessComponentContext();

    //now the AutoText ListBoxes have to be filled

    m_xAutoText = text::AutoTextContainer::create( xContext );

    uno::Sequence<OUString> aNames = m_xAutoText->getElementNames();
    const OUString* pGroups = aNames.getConstArray();

    for(sal_Int32 i = 0; i < aNames.getLength(); ++i)
    {
        uno::Any aGroup = m_xAutoText->getByName(pGroups[i]);
        uno::Reference< text::XAutoTextGroup >  xGroup;
        aGroup >>= xGroup;
        uno::Reference< container::XIndexAccess >  xIdxAcc(xGroup, uno::UNO_QUERY);
        try
        {
            if(!xIdxAcc.is() || xIdxAcc->getCount())
            {
                uno::Reference< beans::XPropertySet >  xPrSet(xGroup, uno::UNO_QUERY);
                uno::Any aTitle = xPrSet->getPropertyValue( UNO_NAME_TITLE );
                OUString uTitle;
                aTitle >>= uTitle;
                const sal_Int32 nEntry = m_pAutoTextGroupLB->InsertEntry(uTitle);
                m_pAutoTextGroupLB->SetEntryData(nEntry, new OUString(pGroups[i]));
            }
        }
        catch (const Exception&)
        {
        }
    }
    if(m_pAutoTextGroupLB->GetEntryCount())
    {
        if(LISTBOX_ENTRY_NOTFOUND == m_pAutoTextGroupLB->GetSelectEntryPos())
            m_pAutoTextGroupLB->SelectEntryPos(0);
        const OUString *pCurGroupName(
            static_cast<const OUString*>(m_pAutoTextGroupLB->GetSelectEntryData()));
        if(m_xAutoText->hasByName(*pCurGroupName))
        {
            uno::Any aGroup = m_xAutoText->getByName(*pCurGroupName);
            try
            {
                uno::Reference< text::XAutoTextGroup >  xGroup;
                aGroup >>= xGroup;
                uno::Sequence< OUString > aBlockNames = xGroup->getElementNames();
                uno::Sequence< OUString > aTitles = xGroup->getTitles() ;

                SetUserData( aBlockNames.getLength(), aTitles.getConstArray(),
                            aBlockNames.getConstArray() );
            }
            catch (const uno::RuntimeException&)
            {
                // we'll be her if path settings were wrong
            }
        }
    }
}

IMPL_LINK_NOARG_TYPED(SwVisitingCardPage, FrameControlInitializedHdl, SwOneExampleFrame&, void)
{
    SvTreeListEntry* pSel = m_pAutoTextLB->FirstSelected();
    OUString sEntry;
    if( pSel )
        sEntry = *static_cast<OUString*>(pSel->GetUserData());
    uno::Reference< text::XTextCursor > & xCrsr = pExampleFrame->GetTextCursor();
    OUString uEntry(sEntry);

    if(LISTBOX_ENTRY_NOTFOUND != m_pAutoTextGroupLB->GetSelectEntryPos())
    {
        const OUString *pGroup( static_cast<const OUString*>(m_pAutoTextGroupLB->GetSelectEntryData()) );
        uno::Any aGroup = m_xAutoText->getByName(*pGroup);
        uno::Reference< text::XAutoTextGroup >  xGroup;
        aGroup >>= xGroup;

        if( !sEntry.isEmpty() && xGroup->hasByName( uEntry ) )
        {
            uno::Any aEntry(xGroup->getByName(uEntry));
            uno::Reference< text::XAutoTextEntry >  xEntry;
            aEntry >>= xEntry;
            if(xEntry.is())
            {
                uno::Reference< text::XTextRange >  xRange(xCrsr, uno::UNO_QUERY);
                xEntry->applyTo(xRange);
            }
            UpdateFields();
        }
    }
}

IMPL_LINK_NOARG_TYPED( SwVisitingCardPage, AutoTextSelectTreeListBoxHdl, SvTreeListBox*, void )
{
    if(m_xAutoText.is() && pExampleFrame->IsInitialized())
        pExampleFrame->ClearDocument( true );
}
IMPL_LINK_TYPED( SwVisitingCardPage, AutoTextSelectHdl, ListBox&, rBox, void )
{
    if(m_xAutoText.is())
    {
        if (m_pAutoTextGroupLB == &rBox)
        {
            const OUString *pGroup( static_cast<const OUString*>(m_pAutoTextGroupLB->GetSelectEntryData()));
            uno::Any aGroup = m_xAutoText->getByName(*pGroup);
            uno::Reference< text::XAutoTextGroup >  xGroup;
            aGroup >>= xGroup;

            ClearUserData();
            m_pAutoTextLB->Clear();

            uno::Sequence<OUString> aBlockNames = xGroup->getElementNames();
            uno::Sequence< OUString > aTitles = xGroup->getTitles() ;
            SetUserData( aBlockNames.getLength(), aTitles.getConstArray(),
                        aBlockNames.getConstArray() );
        }
        if(pExampleFrame->IsInitialized())
            pExampleFrame->ClearDocument( true );
    }
}

void SwVisitingCardPage::UpdateFields()
{
    uno::Reference< frame::XModel >  xModel;
    if( pExampleFrame && (xModel = pExampleFrame->GetModel()).is())
    {
        SwLabDlg::UpdateFieldInformation(xModel, aLabItem);
    }
}

void SwLabDlg::UpdateFieldInformation(uno::Reference< frame::XModel > & xModel, const SwLabItem& rItem)
{
    uno::Reference< text::XTextFieldsSupplier >  xFields(xModel, uno::UNO_QUERY);
    uno::Reference< container::XNameAccess >  xFieldMasters = xFields->getTextFieldMasters();

    static const struct _SwLabItemMap {
        const char* pName;
        OUString SwLabItem:: *pValue;
    }  aArr[] = {
        { "BC_PRIV_FIRSTNAME"  , &SwLabItem::aPrivFirstName },
        { "BC_PRIV_NAME"       , &SwLabItem::aPrivName },
        { "BC_PRIV_INITIALS"   , &SwLabItem::aPrivShortCut },
        { "BC_PRIV_FIRSTNAME_2", &SwLabItem::aPrivFirstName2 },
        { "BC_PRIV_NAME_2"     , &SwLabItem::aPrivName2 },
        { "BC_PRIV_INITIALS_2" , &SwLabItem::aPrivShortCut2 },
        { "BC_PRIV_STREET"     , &SwLabItem::aPrivStreet },
        { "BC_PRIV_ZIP"        , &SwLabItem::aPrivZip },
        { "BC_PRIV_CITY"       , &SwLabItem::aPrivCity },
        { "BC_PRIV_COUNTRY"    , &SwLabItem::aPrivCountry },
        { "BC_PRIV_STATE"      , &SwLabItem::aPrivState },
        { "BC_PRIV_TITLE"      , &SwLabItem::aPrivTitle },
        { "BC_PRIV_PROFESSION" , &SwLabItem::aPrivProfession },
        { "BC_PRIV_PHONE"      , &SwLabItem::aPrivPhone },
        { "BC_PRIV_MOBILE"     , &SwLabItem::aPrivMobile },
        { "BC_PRIV_FAX"        , &SwLabItem::aPrivFax },
        { "BC_PRIV_WWW"        , &SwLabItem::aPrivWWW },
        { "BC_PRIV_MAIL"       , &SwLabItem::aPrivMail },
        { "BC_COMP_COMPANY"    , &SwLabItem::aCompCompany },
        { "BC_COMP_COMPANYEXT" , &SwLabItem::aCompCompanyExt },
        { "BC_COMP_SLOGAN"     , &SwLabItem::aCompSlogan },
        { "BC_COMP_STREET"     , &SwLabItem::aCompStreet },
        { "BC_COMP_ZIP"        , &SwLabItem::aCompZip },
        { "BC_COMP_CITY"       , &SwLabItem::aCompCity },
        { "BC_COMP_COUNTRY"    , &SwLabItem::aCompCountry },
        { "BC_COMP_STATE"      , &SwLabItem::aCompState },
        { "BC_COMP_POSITION"   , &SwLabItem::aCompPosition },
        { "BC_COMP_PHONE"      , &SwLabItem::aCompPhone },
        { "BC_COMP_MOBILE"     , &SwLabItem::aCompMobile },
        { "BC_COMP_FAX"        , &SwLabItem::aCompFax },
        { "BC_COMP_WWW"        , &SwLabItem::aCompWWW },
        { "BC_COMP_MAIL"       , &SwLabItem::aCompMail },
        { 0, 0 }
    };

    try
    {
        for( const _SwLabItemMap* p = aArr; p->pName; ++p )
        {
            OUString uFieldName(
                "com.sun.star.text.FieldMaster.User."
                + OUString::createFromAscii(p->pName));
            if( xFieldMasters->hasByName( uFieldName ))
            {
                uno::Any aFirstName = xFieldMasters->getByName( uFieldName );
                uno::Reference< beans::XPropertySet >  xField;
                aFirstName >>= xField;
                uno::Any aContent;
                aContent <<= rItem.*p->pValue;
                xField->setPropertyValue( UNO_NAME_CONTENT, aContent );
            }
        }
    }
    catch (const uno::RuntimeException&)
    {

    }

    uno::Reference< container::XEnumerationAccess >  xFieldAcc = xFields->getTextFields();
    uno::Reference< util::XRefreshable >  xRefresh(xFieldAcc, uno::UNO_QUERY);
    xRefresh->refresh();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
