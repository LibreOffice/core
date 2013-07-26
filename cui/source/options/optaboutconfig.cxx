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
#include <com/sun/star/beans/XProperty.hpp>
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
    rBar.InsertItem( ITEMID_PREF, get<FixedText>("preference")->GetText(), 0, HIB_LEFT | HIB_VCENTER | HIB_CLICKABLE | HIB_UPARROW);
    rBar.InsertItem( ITEMID_TYPE, get<FixedText>("status")->GetText(), 0,  HIB_LEFT | HIB_VCENTER | HIB_CLICKABLE | HIB_UPARROW );
    rBar.InsertItem( ITEMID_STATUS, get<FixedText>("type")->GetText(), 0,  HIB_LEFT | HIB_VCENTER | HIB_CLICKABLE | HIB_UPARROW );
    rBar.InsertItem( ITEMID_VALUE, get<FixedText>("value")->GetText(), 0,  HIB_LEFT | HIB_VCENTER | HIB_CLICKABLE | HIB_UPARROW );

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

    pEntry->AddItem( new SvLBoxContextBmp( pEntry, 0, Image(), Image(), 0)); //It is needed, otherwise causes crash
    pEntry->AddItem( new SvLBoxString( pEntry, 0, rProp));
    pEntry->AddItem( new SvLBoxString( pEntry, 0, rStatus));
    pEntry->AddItem( new SvLBoxString( pEntry, 0, rType));
    pEntry->AddItem( new SvLBoxString( pEntry, 0, rValue));

    pPrefBox->Insert( pEntry );
}

void CuiAboutConfigTabPage::Reset( const SfxItemSet& )
{
   OUString sRootNodePath = "/";
   pPrefBox->Clear();

   Reference< XNameAccess > xConfigAccess = getConfigAccess( sRootNodePath );

   FillItems( xConfigAccess, sRootNodePath );
}

void CuiAboutConfigTabPage::FillItems( Reference< XNameAccess >xNameAccess, OUString sPath)
{
    sal_Bool bIsLeafNode;

    Reference< XHierarchicalNameAccess > xHierarchicalNameAccess( xNameAccess, uno::UNO_QUERY_THROW );

    uno::Sequence< OUString > seqItems = xNameAccess->getElementNames();
    for( sal_Int16 i = 0; i < seqItems.getLength(); ++i )
    {
        Any aNode = xHierarchicalNameAccess->getByHierarchicalName( seqItems[i] );

        bIsLeafNode = sal_True;

        try
        {
            Reference< XHierarchicalNameAccess >xNextHierarchicalNameAccess( aNode, uno::UNO_QUERY_THROW );
            Reference< XNameAccess > xNextNameAccess( xNextHierarchicalNameAccess, uno::UNO_QUERY_THROW );
            uno::Sequence < OUString  > seqNext = xNextNameAccess->getElementNames();
            FillItems( xNextNameAccess, sPath + OUString("/") + seqItems[i] );
            bIsLeafNode = sal_False;

        }
        catch( uno::Exception& )
        {
        }

        if( bIsLeafNode )
        {
            Any aProp = xHierarchicalNameAccess->getByHierarchicalName(seqItems[i]); //xProperty->getAsProperty();

            OUString test;
            if( aProp.hasValue() )
            {
                switch( aProp.getValueType().getTypeClass() )
                {
                    case ::com::sun::star::uno::TypeClass_UNSIGNED_SHORT :
                    case ::com::sun::star::uno::TypeClass_SHORT :
                    case ::com::sun::star::uno::TypeClass_UNSIGNED_LONG :
                    case ::com::sun::star::uno::TypeClass_LONG :
                    //case ::com::sun::star::uno::TypeClass_INT :
                    {
                        sal_Int32 nVal = 0;
                        if(aProp >>= nVal)
                        {
                            OUString aNumber( OUString::valueOf( nVal ) );
                            test = aNumber;
                        }
                    }
                    break;

                    case ::com::sun::star::uno::TypeClass_BOOLEAN :
                    {
                        sal_Bool bVal = sal_False;
                        if(aProp >>= bVal  )
                        {
                            OUString sBoolean( OUString::valueOf( bVal ) );
                            test = sBoolean;
                        }
                    }
                    break;

                    case ::com::sun::star::uno::TypeClass_STRING :
                    {
                        OUString sString;
                        if(aProp >>= sString)
                        {
                            test = sString;
                        }

                    }
                    break;

                    case ::com::sun::star::uno::TypeClass_SEQUENCE :
                    //case ::com::sun::star::uno::TypeClass_ARRAY :
                    {
                        test = OUString("");
                        if( OUString("[]long").equals(aProp.getValueTypeName()) )
                        {
                            uno::Sequence<sal_Int32> seqLong;
                            if( aProp >>= seqLong )
                            {
                                //test = OUString("Congrats bro!");
                                for(sal_Int16 nInd=0;  nInd < seqLong.getLength(); ++nInd)
                                {
                                    OUString sNumber( OUString::valueOf(seqLong[nInd]) );
                                    test += sNumber;
                                    test += OUString(",");
                                }
                            }
                        }

                        if( OUString("[]string") == aProp.getValueTypeName() )
                        {
                            uno::Sequence< OUString > seqOUString;
                            if( aProp >>= seqOUString )
                            {
                                for( sal_Int16 nInd=0; nInd < seqOUString.getLength(); ++nInd )
                                {
                                    test += seqOUString[nInd] + OUString(",");
                                }
                            }
                        }
                    }
                    break;

                    default:
                    {
                        test = OUString("test");
                    }
                }
            }

            OUString sType = aProp.getValueTypeName();//.getTypeName() ;//Type.getTypeName();
            OUString sPrefName = sPath + OUString("-") + seqItems[i] ;
            InsertEntry( sPrefName, test, sType , sPath );//for testing only will change
        }
    }
}

Reference< XNameAccess > CuiAboutConfigTabPage::getConfigAccess( OUString sNodePath )
{
    uno::Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );

    uno::Reference< lang::XMultiServiceFactory > xConfigProvider(
                com::sun::star::configuration::theDefaultProvider::get( xContext  ) );

    beans::NamedValue aProperty;
    aProperty.Name = "nodepath";
    aProperty.Value = uno::makeAny( sNodePath );

    uno::Sequence< uno::Any > aArgumentList( 1 );
    aArgumentList[0] = uno::makeAny( aProperty );

    uno::Reference< container::XNameAccess > xNameAccess(
                xConfigProvider->createInstanceWithArguments(
                    "com.sun.star.configuration.ConfigurationAccess", aArgumentList ),
                uno::UNO_QUERY_THROW );

    return xNameAccess;
}

IMPL_LINK( CuiAboutConfigTabPage, HeaderSelect_Impl, HeaderBar*, pBar )
{
    if ( pBar && pBar->GetCurItemId() != ITEMID_TYPE )
        return 0;

    HeaderBarItemBits nBits = pBar->GetItemBits(ITEMID_TYPE);
    sal_Bool bUp = ( ( nBits & HIB_UPARROW ) == HIB_UPARROW );
    SvSortMode eMode = SortAscending;

    if ( bUp )
    {
        nBits &= ~HIB_UPARROW;
        nBits |= HIB_DOWNARROW;
        eMode = SortDescending;
    }
    else
    {
        nBits &= ~HIB_DOWNARROW;
        nBits |= HIB_UPARROW;
    }
    pBar->SetItemBits( ITEMID_TYPE, nBits );
    SvTreeList* pModel = pPrefBox->GetModel();
    pModel->SetSortMode( eMode );
    pModel->Resort();
    return 1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
