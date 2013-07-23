/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "optaboutconfig.hxx"
#include "optHeaderTabListbox.hxx"
#include <svtools/svlbitm.hxx>
#include <svtools/treelistentry.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>


using namespace svx;
using namespace ::com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::container;

#define ITEMID_PREF     1
#define ITEMID_TYPE     2
#define ITEMID_STATUS   3
#define ITEMID_VALUE    4

CuiAboutConfigTabPage::CuiAboutConfigTabPage( Window* pParent, const SfxItemSet& rItemSet )
    :SfxTabPage( pParent, "AboutConfig", "cui/ui/aboutconfigdialog.ui", rItemSet)
{
    get(m_pDefaultBtn,"default");
    get(m_pEditBtn, "edit");

    m_pPrefCtrl = get<SvxSimpleTableContainer>("preferences");

    Size aControlSize(200,200);
    m_pPrefCtrl->set_width_request(aControlSize.Width());
    m_pPrefCtrl->set_height_request(aControlSize.Height());

    WinBits nBits = WB_SCROLL | WB_SORT | WB_HSCROLL | WB_VSCROLL;
    pPrefBox = new svx::OptHeaderTabListBox( *m_pPrefCtrl, nBits );

    HeaderBar &rBar = pPrefBox->GetTheHeaderBar();
    rBar.InsertItem( ITEMID_PREF, get<FixedText>("preference")->GetText(), 0, HIB_LEFT | HIB_VCENTER );
    rBar.InsertItem( ITEMID_TYPE, get<FixedText>("status")->GetText(), 0,  HIB_LEFT | HIB_VCENTER );
    rBar.InsertItem( ITEMID_STATUS, get<FixedText>("type")->GetText(), 0,  HIB_LEFT | HIB_VCENTER );
    rBar.InsertItem( ITEMID_VALUE, get<FixedText>("value")->GetText(), 0,  HIB_LEFT | HIB_VCENTER );

    long aTabs[] = {4,0,12,12,12};

    aTabs[2] += aTabs[1] + rBar.GetTextWidth(rBar.GetItemText(1));
    aTabs[3] += aTabs[2] + rBar.GetTextWidth(rBar.GetItemText(2));
    aTabs[4] += aTabs[3] + rBar.GetTextWidth(rBar.GetItemText(3));

    pPrefBox->SetTabs(aTabs, MAP_PIXEL);

}

CuiAboutConfigTabPage::~CuiAboutConfigTabPage()
{
    delete pPrefBox;
}

SfxTabPage* CuiAboutConfigTabPage::Create( Window* pParent, const SfxItemSet& rItemSet )
{
    return ( new CuiAboutConfigTabPage( pParent, rItemSet) );
}

void CuiAboutConfigTabPage::InsertEntry( OUString& rProp, OUString&  rStatus, OUString& rType, OUString& rValue)
{
    SvTreeListEntry* pEntry = new SvTreeListEntry;

    pEntry->AddItem( new SvLBoxString( pEntry, 0, rProp));
    pEntry->AddItem( new SvLBoxString( pEntry, 0, rStatus));
    pEntry->AddItem( new SvLBoxString( pEntry, 0, rType));
    pEntry->AddItem( new SvLBoxString( pEntry, 0, rValue));

    pPrefBox->Insert( pEntry );
}

void CuiAboutConfigTabPage::Reset( const SfxItemSet& )
{
   pPrefBox->Clear();

   Reference< XNameAccess > xConfigAccess = getConfigAccess();

   FillItems( xConfigAccess, OUString("org.openoffice") );
}

void CuiAboutConfigTabPage::FillItems( Reference< XNameAccess >xNameAccess, OUString sPath)
{
    sal_Bool bIsLeafNode;

    Reference< XHierarchicalNameAccess > xHierarchicalNameAccess( xNameAccess, uno::UNO_QUERY_THROW );

    uno::Sequence< OUString > seqItems = xNameAccess->getElementNames();
    for( sal_Int16 i = 0; i < seqItems.getLength(); ++i )
    {
        Any aNode = xHierarchicalNameAccess->getByHierarchicalName( seqItems[i] );
        Reference< XHierarchicalNameAccess >xNextHierarchicalNameAccess( aNode, uno::UNO_QUERY_THROW );
        Reference< XNameAccess > xNextNameAccess( xNextHierarchicalNameAccess, uno::UNO_QUERY_THROW );

        bIsLeafNode = sal_True;

        try
        {
            uno::Sequence < OUString  > seqNext = xNextNameAccess->getElementNames();
            FillItems( xNextNameAccess, sPath + OUString("/") + seqItems[i] );
            bIsLeafNode = sal_False;

        }
        catch( uno::Exception& )
        {
        }

        if( bIsLeafNode )
        {
            //InsertEntry( sPath, "", "", "");
            //Reference< beans::Property > aProperty = xHierarchicalNameAccess->getAsProperty();//getPropertyValue( seqItems[ i ] );
            //InsertEntry( sPath + OUString("/") + seqItems[ i ], OUString(""), OUString(""), xNameAccess->getPropertyValue( seqItems[ i ] ) );
        }
    }
}

Reference< XNameAccess > CuiAboutConfigTabPage::getConfigAccess()
{
    uno::Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );

    uno::Reference< lang::XMultiServiceFactory > xConfigProvider(
                com::sun::star::configuration::theDefaultProvider::get( xContext  ) );

    beans::NamedValue aProperty;
    aProperty.Name = "nodepath";
    aProperty.Value = uno::makeAny( OUString("org.openoffice") );

    uno::Sequence< uno::Any > aArgumentList( 1 );
    aArgumentList[0] = uno::makeAny( aProperty );

    uno::Reference< container::XNameAccess > xNameAccess(
                xConfigProvider->createInstanceWithArguments(
                    "com.sun.star.configuration.ConfigurationAccess", aArgumentList ),
                uno::UNO_QUERY_THROW );

    return xNameAccess;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
