/*************************************************************************
 *
 *  $RCSfile: labelexp.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: os $ $Date: 2002-01-02 14:12:41 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _COM_SUN_STAR_TEXT_XTEXTFIELDSSUPPLIER_HPP_
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XREFRESHABLE_HPP_
#include <com/sun/star/util/XRefreshable.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _LABFMT_HXX
#include <labfmt.hxx>
#endif
#ifndef _UNOTOOLS_HXX
#include <unotools.hxx>
#endif
#ifndef _UNOATXT_HXX //autogen wg. SwXAutoTextEntry
#include <unoatxt.hxx>
#endif
#ifndef _UNOOBJ_HXX //
#include <unoobj.hxx>
#endif
#ifndef _UNOPRNMS_HXX //
#include <unoprnms.hxx>
#endif


using namespace ::com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::uno;
using namespace ::comphelper;
using namespace ::rtl;

#define C2U(char) rtl::OUString::createFromAscii(char)

/* -----------------08.07.99 15:15-------------------

 --------------------------------------------------*/
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
    OUString uTitleName( C2U(SW_PROP_NAME_STR(UNO_NAME_TITLE)) );

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
        catch(Exception&)
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
            catch( uno::RuntimeException& )
            {
                // we'll be her if path settings were wrong
            }
        }
    }
}
/* -----------------01.10.99 13:19-------------------

 --------------------------------------------------*/
IMPL_LINK( SwVisitingCardPage, FrameControlInitializedHdl, void*, EMPTYARG )
{
    SvLBoxEntry* pSel = aAutoTextLB.FirstSelected();
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
/* -----------------22.07.99 11:06-------------------

 --------------------------------------------------*/
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
            pExampleFrame->ClearDocument( TRUE );
    }
    return 0;
}

/* -----------------01.10.99 11:59-------------------

 --------------------------------------------------*/
void SwVisitingCardPage::UpdateFields()
{
    uno::Reference< frame::XModel >  xModel;
    if( pExampleFrame && (xModel = pExampleFrame->GetModel()).is())
    {
        SwLabDlg::UpdateFieldInformation(xModel, aLabItem);
    }
}
/* -----------------01.10.99 15:16-------------------

 --------------------------------------------------*/
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
        String sFldName( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM(
                            "com.sun.star.text.FieldMaster.User." )));
        OUString uCntName( C2U( SW_PROP_NAME_STR(UNO_NAME_CONTENT )));
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
    catch( uno::RuntimeException&)
    {
        //
    }

    uno::Reference< container::XEnumerationAccess >  xFldAcc = xFlds->getTextFields();
    uno::Reference< util::XRefreshable >  xRefresh(xFldAcc, uno::UNO_QUERY);
    xRefresh->refresh();
}

