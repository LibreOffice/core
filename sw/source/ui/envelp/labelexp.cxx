/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
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
using ::rtl::OUString;

#ifdef SW_PROP_NAME_STR
#undef SW_PROP_NAME_STR
#endif
#define SW_PROP_NAME_STR(nId) SwGetPropName((nId)).pName

void SwVisitingCardPage::InitFrameControl()
{
    Link aLink(LINK(this, SwVisitingCardPage, FrameControlInitializedHdl));
    pExampleFrame = new SwOneExampleFrame( aExampleWIN,
                                    EX_SHOW_BUSINESS_CARDS, &aLink );

    uno::Reference< lang::XMultiServiceFactory >  xMgr =
                                            getProcessServiceFactory();
    //now the AutoText ListBoxes have to be filled

    uno::Reference< uno::XInterface >  xAText =
        xMgr->createInstance( C2U("com.sun.star.text.AutoTextContainer") );
    _xAutoText = uno::Reference< container::XNameAccess >(xAText, uno::UNO_QUERY);

    uno::Sequence<OUString> aNames = _xAutoText->getElementNames();
    const OUString* pGroups = aNames.getConstArray();
    OUString uTitleName( rtl::OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_TITLE)) );

    for(sal_uInt16 i = 0; i < aNames.getLength(); i++)
    {
        uno::Any aGroup = _xAutoText->getByName(pGroups[i]);
        uno::Reference< text::XAutoTextGroup >  xGroup;
        aGroup >>= xGroup;
        uno::Reference< container::XIndexAccess >  xIdxAcc(xGroup, uno::UNO_QUERY);
        try
        {
            if(!xIdxAcc.is() || xIdxAcc->getCount())
            {
                uno::Reference< beans::XPropertySet >  xPrSet(xGroup, uno::UNO_QUERY);
                uno::Any aTitle = xPrSet->getPropertyValue( uTitleName );
                OUString uTitle;
                aTitle >>= uTitle;
                String sGroup(pGroups[i]);
                sal_uInt16 nEntry = aAutoTextGroupLB.InsertEntry(uTitle);
                aAutoTextGroupLB.SetEntryData(nEntry, new String(sGroup));
            }
        }
        catch (const Exception&)
        {
        }
    }
    if(aAutoTextGroupLB.GetEntryCount())
    {
        if(LISTBOX_ENTRY_NOTFOUND == aAutoTextGroupLB.GetSelectEntryPos())
            aAutoTextGroupLB.SelectEntryPos(0);
        String sCurGroupName(
            *(String*)aAutoTextGroupLB.GetEntryData(aAutoTextGroupLB.GetSelectEntryPos()));
        if(_xAutoText->hasByName(sCurGroupName))
        {
            uno::Any aGroup = _xAutoText->getByName(sCurGroupName);
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

IMPL_LINK_NOARG(SwVisitingCardPage, FrameControlInitializedHdl)
{
    SvTreeListEntry* pSel = aAutoTextLB.FirstSelected();
    String sEntry;
    if( pSel )
        sEntry = *(String*)pSel->GetUserData();
    uno::Reference< text::XTextCursor > & xCrsr = pExampleFrame->GetTextCursor();
    OUString uEntry(sEntry);

    if(LISTBOX_ENTRY_NOTFOUND != aAutoTextGroupLB.GetSelectEntryPos())
    {
        String sGroup( *(String*)aAutoTextGroupLB.GetEntryData(
                                    aAutoTextGroupLB.GetSelectEntryPos() ) );
        uno::Any aGroup = _xAutoText->getByName(sGroup);
        uno::Reference< text::XAutoTextGroup >  xGroup;
        aGroup >>= xGroup;

        if( sEntry.Len() && xGroup->hasByName( uEntry ) )
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
    return 0;
}

IMPL_LINK( SwVisitingCardPage, AutoTextSelectHdl, void*, pBox )
{
    if(_xAutoText.is())
    {
        if( &aAutoTextGroupLB == pBox )
        {
            String sGroup( *(String*)aAutoTextGroupLB.GetEntryData(
                                    aAutoTextGroupLB.GetSelectEntryPos()));
            uno::Any aGroup = _xAutoText->getByName(sGroup);
            uno::Reference< text::XAutoTextGroup >  xGroup;
            aGroup >>= xGroup;

            ClearUserData();
            aAutoTextLB.Clear();

            uno::Sequence<OUString> aBlockNames = xGroup->getElementNames();
            uno::Sequence< OUString > aTitles = xGroup->getTitles() ;
            SetUserData( aBlockNames.getLength(), aTitles.getConstArray(),
                        aBlockNames.getConstArray() );
        }
        if(pExampleFrame->IsInitialized())
            pExampleFrame->ClearDocument( sal_True );
    }
    return 0;
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
    uno::Reference< text::XTextFieldsSupplier >  xFlds(xModel, uno::UNO_QUERY);
    uno::Reference< container::XNameAccess >  xFldMasters = xFlds->getTextFieldMasters();

    static const struct _SwLabItemMap {
        const char* pName;
        rtl::OUString SwLabItem:: *pValue;
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
        rtl::OUString sFldName("com.sun.star.text.FieldMaster.User.");
        OUString uCntName( rtl::OUString::createFromAscii( SW_PROP_NAME_STR(UNO_NAME_CONTENT )));
        for( const _SwLabItemMap* p = aArr; p->pName; ++p )
        {
            String sCurFldName( sFldName );
            sCurFldName.AppendAscii( p->pName );
            OUString uFldName( sCurFldName );
            if( xFldMasters->hasByName( uFldName ))
            {
                uno::Any aFirstName = xFldMasters->getByName( uFldName );
                uno::Reference< beans::XPropertySet >  xFld;
                aFirstName >>= xFld;
                uno::Any aContent;
                aContent <<= rItem.*p->pValue;
                xFld->setPropertyValue( uCntName, aContent );
            }
        }
    }
    catch (const uno::RuntimeException&)
    {
        //
    }

    uno::Reference< container::XEnumerationAccess >  xFldAcc = xFlds->getTextFields();
    uno::Reference< util::XRefreshable >  xRefresh(xFldAcc, uno::UNO_QUERY);
    xRefresh->refresh();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
