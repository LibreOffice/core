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
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>

#include <vector>

using namespace svx;
using namespace ::com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::container;

#define ITEMID_PREFNAME     1
#define ITEMID_PROPERTY     2
#define ITEMID_TYPE         3
#define ITEMID_VALUE        4

struct Prop_Impl
{
    OUString    Name;
    OUString    Property;
    Any         Value;

    Prop_Impl( const OUString& sName, const OUString& sProperty, const Any& aValue )
        : Name( sName )
        , Property( sProperty )
        , Value( aValue )
    {
    }
};

CuiAboutConfigTabPage::CuiAboutConfigTabPage( Window* pParent, const SfxItemSet& rItemSet )
    :SfxTabPage( pParent, "AboutConfig", "cui/ui/aboutconfigdialog.ui", rItemSet)
{
    get(m_pDefaultBtn,"default");
    get(m_pEditBtn, "edit");

    m_pPrefCtrl = get<SvSimpleTableContainer>("preferences");

    Size aControlSize(200,200);
    m_pPrefCtrl->set_width_request(aControlSize.Width());
    m_pPrefCtrl->set_height_request(aControlSize.Height());

    WinBits nBits = WB_SCROLL | WB_SORT | WB_HSCROLL | WB_VSCROLL;
    pPrefBox = new svx::OptHeaderTabListBox( *m_pPrefCtrl, nBits );

    m_pEditBtn->SetClickHdl( LINK( this, CuiAboutConfigTabPage, StandardHdl_Impl ) );

    HeaderBar &rBar = pPrefBox->GetTheHeaderBar();
    rBar.InsertItem( ITEMID_PREFNAME, get<FixedText>("preference")->GetText(), 0, HIB_LEFT | HIB_VCENTER | HIB_CLICKABLE | HIB_UPARROW);
    rBar.InsertItem( ITEMID_PROPERTY, get<FixedText>("property")->GetText(), 0,  HIB_LEFT | HIB_VCENTER | HIB_CLICKABLE | HIB_UPARROW );
    rBar.InsertItem( ITEMID_TYPE, get<FixedText>("type")->GetText(), 0,  HIB_LEFT | HIB_VCENTER | HIB_CLICKABLE | HIB_UPARROW );
    rBar.InsertItem( ITEMID_VALUE, get<FixedText>("value")->GetText(), 0,  HIB_LEFT | HIB_VCENTER | HIB_CLICKABLE | HIB_UPARROW );

    long aTabs[] = {4,120,50,50,50};//TODO: Not works correctly hardcoded for now.

    aTabs[2] += aTabs[1] + rBar.GetTextWidth(rBar.GetItemText(1));
    aTabs[3] += aTabs[2] + 160; //rBar.GetTextWidth(rBar.GetItemText(2));
    aTabs[4] += aTabs[3] + 40; //rBar.GetTextWidth(rBar.GetItemText(3));

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

    VectorOfModified.clear();

    m_pDefaultBtn->Enable(sal_False);
    //m_pEditBtn->Enable(sal_False);

    const char* entries[] = {
           "/org.openoffice.Office.Common",
           "/org.openoffice.Office.Math",
           "/org.openoffice.Office.Addons" };

    for (size_t nInd = 0; nInd < SAL_N_ELEMENTS(entries); ++nInd )
    {
        sRootNodePath = OUString::createFromAscii( entries[nInd] );
        Reference< XNameAccess > xConfigAccess = getConfigAccess( sRootNodePath, sal_False );
        FillItems( xConfigAccess, sRootNodePath );
    }
}

sal_Bool CuiAboutConfigTabPage::FillItemSet( SfxItemSet& )
{
    sal_Bool bModified = sal_False;
    Reference< XNameAccess > xUpdateAccess = getConfigAccess( "/", sal_True );
    //Reference< XNameReplace > xNameReplace( xUpdateAccess, UNO_QUERY_THROW );

    //if( !xNameReplace.is() )
        //return bModified;

    for( size_t nInd = 0; nInd < VectorOfModified.size(); ++nInd )
    {
        //beans::NamedValue aNamedValue = VectorOfModified[ nInd ];
        Prop_Impl* aNamedValue = VectorOfModified[ nInd ];

        xUpdateAccess = getConfigAccess( aNamedValue->Name , sal_True );
        Reference< XNameReplace > xNameReplace( xUpdateAccess, UNO_QUERY_THROW );

        xNameReplace->replaceByName( aNamedValue->Property, aNamedValue->Value );
        bModified = sal_True;
    }

    Reference< util::XChangesBatch > xChangesBatch( xUpdateAccess, UNO_QUERY_THROW );

    xChangesBatch->commitChanges();

    return bModified;

}

void CuiAboutConfigTabPage::FillItems( Reference< XNameAccess >xNameAccess, OUString sPath)
{
    sal_Bool bIsLeafNode;

    Reference< XHierarchicalNameAccess > xHierarchicalNameAccess( xNameAccess, uno::UNO_QUERY_THROW );

    pPrefBox->SetUpdateMode(sal_False);

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

            OUString sValue;
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
                            OUString aNumber( OUString::number( nVal ) );
                            sValue = aNumber;
                        }
                    }
                    break;

                    case ::com::sun::star::uno::TypeClass_BOOLEAN :
                    {
                        sal_Bool bVal = sal_False;
                        if(aProp >>= bVal  )
                        {
                            OUString sBoolean( OUString::boolean( bVal ) );
                            sValue = sBoolean;
                        }
                    }
                    break;

                    case ::com::sun::star::uno::TypeClass_STRING :
                    {
                        OUString sString;
                        if(aProp >>= sString)
                        {
                            sValue = sString;
                        }

                    }
                    break;

                    case ::com::sun::star::uno::TypeClass_SEQUENCE :
                    //case ::com::sun::star::uno::TypeClass_ARRAY :
                    {
                        sValue = OUString("");
                        if( OUString("[]long") ==aProp.getValueTypeName() ||
                                OUString("[]short")==aProp.getValueTypeName() )
                        {
                            uno::Sequence<sal_Int32> seqLong;
                            if( aProp >>= seqLong )
                            {
                                for(sal_Int16 nInd=0;  nInd < seqLong.getLength(); ++nInd)
                                {
                                    OUString sNumber( OUString::number(seqLong[nInd]) );
                                    sValue += sNumber;
                                    sValue += OUString(",");
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
                                    sValue += seqOUString[nInd] + OUString(",");
                                }
                            }
                        }
                    }
                    break;

                    default:
                    {
                        sValue = OUString("");
                    }
                }
            }

            OUString sType = aProp.getValueTypeName();
            //OUString sPrefName = sPath + OUString("-") + seqItems[i] ;
            InsertEntry( sPath, seqItems [ i ], sType, sValue);
        }
    }

    pPrefBox->SetUpdateMode(sal_True);
}

Reference< XNameAccess > CuiAboutConfigTabPage::getConfigAccess( OUString sNodePath, sal_Bool bUpdate )
{
    uno::Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );

    uno::Reference< lang::XMultiServiceFactory > xConfigProvider(
                com::sun::star::configuration::theDefaultProvider::get( xContext  ) );

    beans::NamedValue aProperty;
    aProperty.Name = "nodepath";
    aProperty.Value = uno::makeAny( sNodePath );

    uno::Sequence< uno::Any > aArgumentList( 1 );
    aArgumentList[0] = uno::makeAny( aProperty );

    OUString sAccessString;

    if( bUpdate )
        sAccessString = "com.sun.star.configuration.ConfigurationUpdateAccess";
    else
        sAccessString = "com.sun.star.configuration.ConfigurationAccess";

    uno::Reference< container::XNameAccess > xNameAccess(
                xConfigProvider->createInstanceWithArguments(
                    sAccessString, aArgumentList ),
                uno::UNO_QUERY_THROW );

    return xNameAccess;
}

//void CuiAboutConfigTabPage::AddToModifiedVector( beans::NamedValue& rProp )
void CuiAboutConfigTabPage::AddToModifiedVector( Prop_Impl* rProp )
{
    bool isModifiedBefore = false;
    //Check if value modified before
    for( size_t nInd = 0; nInd < VectorOfModified.size() ; ++nInd )
    {
        if( rProp->Name == VectorOfModified[nInd]->Name && rProp->Value == VectorOfModified[nInd]->Value )
        {
            //property modified before. assing reference to the modified value
            //do your changes on this object. They will be saved later.
            VectorOfModified[nInd] = rProp;
            isModifiedBefore = true;
            break;
        }
    }

    if( !isModifiedBefore )
    {
        VectorOfModified.push_back( rProp );
    }
    //property is not modified be
}

CuiAboutConfigValueDialog::CuiAboutConfigValueDialog( Window* pWindow, const OUString& rValue )
    :ModalDialog( pWindow, "AboutConfigValueDialog", "cui/ui/aboutconfigvaluedialog.ui" )
{
    get(m_pEDValue, "valuebox");

    m_pEDValue->SetText( rValue );

}

CuiAboutConfigValueDialog::~CuiAboutConfigValueDialog()
{
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

IMPL_LINK_NOARG( CuiAboutConfigTabPage, StandardHdl_Impl )
{
    SvTreeListEntry* pEntry = pPrefBox->FirstSelected();

    OUString sPropertyPath = pPrefBox->GetEntryText( pEntry, 0 );
    OUString sPropertyName = pPrefBox->GetEntryText( pEntry, 1 );
    OUString sPropertyType = pPrefBox->GetEntryText( pEntry, 2 );
    OUString sPropertyValue = pPrefBox->GetEntryText( pEntry, 3 );


    //beans::NamedValue aProperty;

    //aProperty.Name = sPropertyPath + OUString("/") + sPropertyName;

    Prop_Impl* pProperty = new Prop_Impl( sPropertyPath, sPropertyName, makeAny( sPropertyValue ) );

    bool bOpenDialog;
    OUString sDialogValue;
    OUString sNewValue;

    if( sPropertyType == OUString( "boolean" ) )
    {
        bool bValue;
        if( sPropertyValue == OUString("true") )
        {
            sDialogValue = OUString("false");
            bValue = false;
        }
        else
        {
            sDialogValue = OUString("true");
            bValue = true;
        }

        pProperty->Value = uno::makeAny( bValue );
        bOpenDialog = false;
    }else// if ( sPropertyType == OUString( "string" ) )
    {
        //TODO: handle void etc.
        sDialogValue = sPropertyValue;
        bOpenDialog = true;
    }


    if( bOpenDialog )
    {
        CuiAboutConfigValueDialog* pValueDialog = new CuiAboutConfigValueDialog(0, sDialogValue);

        bool ret = pValueDialog->Execute();
        if( ret == RET_OK )
        {
            sNewValue = pValueDialog->getValue();
            //TODO: parse the value according to the type?
            pProperty->Value = uno::makeAny( sNewValue );
            AddToModifiedVector( pProperty );

            sDialogValue = sNewValue;
        }
        else
            delete pProperty;
    }
    else
        delete pProperty;

    pPrefBox->SetEntryText( sDialogValue,  pEntry, 3 );
    //TODO:update listbox value.

    return 0;

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
