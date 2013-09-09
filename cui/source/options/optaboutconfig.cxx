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

#define SHORT_LEN_LIMIT     7
#define LONG_LEN_LIMIT      11
#define HYPER_LEN_LIMIT     20

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
    {}
};

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeCuiCustomMultilineEdit(Window *pParent, VclBuilder::stringmap &)
{
    return new CuiCustomMultilineEdit(pParent, WB_LEFT|WB_VCENTER|WB_BORDER|WB_3DLOOK);
}


void CuiCustomMultilineEdit::KeyInput( const KeyEvent& rKeyEvent )
{
    bool bValid;
    bool bNonSpace = rKeyEvent.GetKeyCode().GetCode() != KEY_SPACE;
    if( bNumericOnly && bNonSpace )
    {
        const KeyCode& rKeyCode = rKeyEvent.GetKeyCode();
        sal_uInt16 nGroup = rKeyCode.GetGroup();
        sal_uInt16 nKey = rKeyCode.GetCode();
        bValid = ( KEYGROUP_NUM == nGroup || KEYGROUP_CURSOR == nGroup ||
                 ( KEYGROUP_MISC == nGroup &&
                 ( nKey == KEY_SUBTRACT || nKey == KEY_COMMA || nKey == KEY_POINT
                   || nKey < KEY_ADD || nKey > KEY_EQUAL ) ) );
        if ( !bValid && ( rKeyCode.IsMod1() && (
             KEY_A == nKey || KEY_C == nKey || KEY_V == nKey || KEY_X == nKey || KEY_Z == nKey ) ) )
            bValid = sal_True;
    }
    else
        bValid = sal_True;
    if( bValid )
        Edit::KeyInput( rKeyEvent );
}

void CuiCustomMultilineEdit::setBehaviour( bool bNumeric, int nLimit )
{
    bNumericOnly = bNumeric;
    SetMaxTextLen(nLimit);
}

CuiAboutConfigTabPage::CuiAboutConfigTabPage( Window* pParent, const SfxItemSet& rItemSet ) :
    SfxTabPage( pParent, "AboutConfig", "cui/ui/aboutconfigdialog.ui", rItemSet),
    m_pPrefCtrl( get<SvSimpleTableContainer>("preferences") ),
    m_pDefaultBtn( get<PushButton>("default") ),
    m_pEditBtn( get<PushButton>("edit") ),
    m_vectorOfModified(),
    m_pPrefBox( new OptHeaderTabListBox( *m_pPrefCtrl,
                                         WB_SCROLL | WB_HSCROLL | WB_VSCROLL ) )
{
    Size aControlSize(200,200);
    m_pPrefCtrl->set_width_request(aControlSize.Width());
    m_pPrefCtrl->set_height_request(aControlSize.Height());

    m_pEditBtn->SetClickHdl( LINK( this, CuiAboutConfigTabPage, StandardHdl_Impl ) );

    HeaderBar &rBar = m_pPrefBox->GetTheHeaderBar();
    rBar.InsertItem( ITEMID_PREFNAME, get<FixedText>("preference")->GetText(), 0, HIB_LEFT | HIB_VCENTER );
    rBar.InsertItem( ITEMID_PROPERTY, get<FixedText>("property")->GetText(), 0,  HIB_LEFT | HIB_VCENTER );
    rBar.InsertItem( ITEMID_TYPE, get<FixedText>("type")->GetText(), 0,  HIB_LEFT | HIB_VCENTER );
    rBar.InsertItem( ITEMID_VALUE, get<FixedText>("value")->GetText(), 0,  HIB_LEFT | HIB_VCENTER );

    long aTabs[] = {4,120,50,50,50};//TODO: Not works correctly hardcoded for now.

    aTabs[2] += aTabs[1] + rBar.GetTextWidth(rBar.GetItemText(1));
    aTabs[3] += aTabs[2] + 160; //rBar.GetTextWidth(rBar.GetItemText(2));
    aTabs[4] += aTabs[3] + 40; //rBar.GetTextWidth(rBar.GetItemText(3));

    m_pPrefBox->SetTabs(aTabs, MAP_PIXEL);
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

    m_pPrefBox->Insert( pEntry );
}

void CuiAboutConfigTabPage::Reset( const SfxItemSet& )
{
    OUString sRootNodePath = "";
    m_pPrefBox->Clear();

    m_vectorOfModified.clear();
    m_pPrefBox->GetModel()->SetSortMode( SortNone );

    m_pDefaultBtn->Enable(sal_False);

    m_pPrefBox->SetUpdateMode(sal_False);
    Reference< XNameAccess > xConfigAccess = getConfigAccess( sRootNodePath, sal_False );
    FillItems( xConfigAccess, sRootNodePath );
    m_pPrefBox->SetUpdateMode(sal_True);
}

sal_Bool CuiAboutConfigTabPage::FillItemSet( SfxItemSet& )
{
    sal_Bool bModified = sal_False;
    Reference< XNameAccess > xUpdateAccess = getConfigAccess( "/", sal_True );

    for( size_t nInd = 0; nInd < m_vectorOfModified.size(); ++nInd )
    {
        Prop_Impl* aNamedValue = m_vectorOfModified[ nInd ];

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
            Any aProp = xHierarchicalNameAccess->getByHierarchicalName(seqItems[i]);

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
                                for(int nInd=0;  nInd < seqLong.getLength(); ++nInd)
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

                        if( OUString("[]hyper") == aProp.getValueTypeName() )
                        {
                            uno::Sequence< sal_Int64 > seqHyp;
                            if( aProp >>= seqHyp )
                            {
                                for(int nInd = 0; nInd < seqHyp.getLength(); ++nInd)
                                {
                                    OUString sHyper( OUString::number( seqHyp[nInd] ) );
                                    sValue += sHyper;
                                    sValue += OUString(",");
                                }
                            }
                        }
                    }
                    break;

                    default:
                    {
                        if( OUString("hyper") == aProp.getValueTypeName() )
                        {
                            sal_Int64 nHyp = 0;
                            if(aProp >>= nHyp)
                            {
                                OUString aHyp( OUString::number( nHyp ) );
                                sValue = aHyp;
                            }
                        }else
                            sValue = OUString("");
                    }
                }
            }

            OUString sType = aProp.getValueTypeName();
            InsertEntry( sPath, seqItems [ i ], sType, sValue);
        }
    }
}

Reference< XNameAccess > CuiAboutConfigTabPage::getConfigAccess( OUString sNodePath, sal_Bool bUpdate )
{
    uno::Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );

    uno::Reference< lang::XMultiServiceFactory > xConfigProvider(
                com::sun::star::configuration::theDefaultProvider::get( xContext  ) );

    if( sNodePath == OUString("") )
        sNodePath = OUString("/");
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

void CuiAboutConfigTabPage::AddToModifiedVector( Prop_Impl* rProp )
{
    bool isModifiedBefore = false;
    //Check if value modified before
    for( size_t nInd = 0; nInd < m_vectorOfModified.size() ; ++nInd )
    {
        if( rProp->Name == m_vectorOfModified[nInd]->Name && rProp->Value == m_vectorOfModified[nInd]->Value )
        {
            //property modified before. assing reference to the modified value
            //do your changes on this object. They will be saved later.
            m_vectorOfModified[nInd] = rProp;
            isModifiedBefore = true;
            break;
        }
    }

    if( !isModifiedBefore )
        m_vectorOfModified.push_back( rProp );
    //property is not modified be
}

CuiAboutConfigValueDialog::CuiAboutConfigValueDialog( Window* pWindow,
                                                      const OUString& rValue,
                                                      int limit ) :
    ModalDialog( pWindow, "AboutConfigValueDialog", "cui/ui/aboutconfigvalue   dialog.ui" ),
    m_pEDValue( get<CuiCustomMultilineEdit>("valuebox") )
{
    if( limit != 0)
    {
        //numericonly, limit
        m_pEDValue->setBehaviour( true, limit );
    }
    else if ( limit == 0 )
        m_pEDValue->setBehaviour( false, EDIT_NOLIMIT );
    m_pEDValue->SetText( rValue );

}

IMPL_LINK( CuiAboutConfigTabPage, HeaderSelect_Impl, HeaderBar*, /*pBar*/ )
{
    return 0;
}

    //if ( pBar && pBar->GetCurItemId() != ITEMID_TYPE )
        //return 0;

    //HeaderBarItemBits nBits = pBar->GetItemBits(ITEMID_TYPE);
    //sal_Bool bUp = ( ( nBits & HIB_UPARROW ) == HIB_UPARROW );
    //SvSortMode eMode = SortAscending;

    //if ( bUp )
    //{
        //nBits &= ~HIB_UPARROW;
        //nBits |= HIB_DOWNARROW;
        //eMode = SortDescending;
    //}
    //else
    //{
        //nBits &= ~HIB_DOWNARROW;
        //nBits |= HIB_UPARROW;
    //}
    //pBar->SetItemBits( ITEMID_TYPE, nBits );
    //SvTreeList* pModel = m_pPrefBox->GetModel();
    //pModel->SetSortMode( eMode );
    //pModel->Resort();
    //return 1;
    //}

IMPL_LINK_NOARG( CuiAboutConfigTabPage, StandardHdl_Impl )
{
    SvTreeListEntry* pEntry = m_pPrefBox->FirstSelected();

    OUString sPropertyPath = m_pPrefBox->GetEntryText( pEntry, 0 );
    OUString sPropertyName = m_pPrefBox->GetEntryText( pEntry, 1 );
    OUString sPropertyType = m_pPrefBox->GetEntryText( pEntry, 2 );
    OUString sPropertyValue = m_pPrefBox->GetEntryText( pEntry, 3 );

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
    }
    else// if ( sPropertyType == OUString( "string" ) )
    {
        //TODO: handle void etc.
        sDialogValue = sPropertyValue;
        bOpenDialog = true;
    }

    try
    {
        if( bOpenDialog )
        {
            //Cosmetic length limit for integer values.
            int limit=0;
            if( sPropertyType == OUString("short") )
                limit = SHORT_LEN_LIMIT;
            else if( sPropertyType == OUString("long") )
                limit = LONG_LEN_LIMIT;
            else if( sPropertyType == OUString("hyper") )
                limit = HYPER_LEN_LIMIT;

            CuiAboutConfigValueDialog* pValueDialog = new CuiAboutConfigValueDialog(0, sDialogValue, limit);

            bool ret = pValueDialog->Execute();
            if( ret == RET_OK )
            {
                sNewValue = pValueDialog->getValue();
                if ( sPropertyType == OUString("short"))
                {
                    sal_Int16 nShort;
                    sal_Int32 nNumb = sNewValue.toInt32();

                    //if the value is 0 and length is not 1, there is something wrong
                    if( !( nNumb==0 && sNewValue.getLength()!=1 ) && nNumb < SAL_MAX_INT16 && nNumb > SAL_MIN_INT16)
                        nShort = (sal_Int16) nNumb;
                    else
                        throw uno::Exception();
                    pProperty->Value = uno::makeAny( nShort );
                }
                else
                    if( sPropertyType == OUString("long"))
                    {
                        sal_Int32 nLong = sNewValue.toInt32();
                        if( !( nLong==0 && sNewValue.getLength()!=1 ) && nLong < SAL_MAX_INT32 && nLong > SAL_MIN_INT32)
                            pProperty->Value = uno::makeAny( nLong );
                        else
                            throw uno::Exception();
                    }
                    else if( sPropertyType == OUString("hyper"))
                    {
                        sal_Int64 nHyper = sNewValue.toInt64();
                        if( !( nHyper==0 && sNewValue.getLength()!=1 ) && nHyper < SAL_MAX_INT32 && nHyper > SAL_MIN_INT32)
                            pProperty->Value = uno::makeAny( nHyper );
                        else
                            throw uno::Exception();
                    }
                    else if( sPropertyType == OUString("double"))
                    {
                        double nDoub = sNewValue.toDouble();
                        if( !( nDoub ==0 && sNewValue.getLength()!=1 ) && nDoub < SAL_MAX_INT32 && nDoub > SAL_MIN_INT32)
                            pProperty->Value = uno::makeAny( nDoub );
                        else
                            throw uno::Exception();
                    }
                    else if( sPropertyType == OUString("float"))
                    {
                        float nFloat = sNewValue.toFloat();
                        if( !( nFloat ==0 && sNewValue.getLength()!=1 ) && nFloat < SAL_MAX_INT32 && nFloat > SAL_MIN_INT32)
                            pProperty->Value = uno::makeAny( nFloat );
                        else
                            throw uno::Exception();
                    }
                    else
                        pProperty->Value = uno::makeAny( sNewValue );

                AddToModifiedVector( pProperty );
                sDialogValue = sNewValue;
            }
            else
                delete pProperty;
        }
        else
            delete pProperty;

        //update listbox value.
        m_pPrefBox->SetEntryText( sDialogValue,  pEntry, 3 );
    }
    catch( uno::Exception& )
    {
        delete pProperty;
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
