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
#include <comphelper/sequence.hxx>
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
#include <boost/shared_ptr.hpp>

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
    bool bValid = false;
    bool bNonSpace = rKeyEvent.GetKeyCode().GetCode() != KEY_SPACE;
    if( bNumericOnly && bNonSpace )
    {
        const KeyCode& rKeyCode = rKeyEvent.GetKeyCode();
        sal_uInt16 nGroup = rKeyCode.GetGroup();
        sal_uInt16 nKey = rKeyCode.GetCode();

        switch ( nGroup ) {
            case KEYGROUP_NUM :
            case KEYGROUP_CURSOR :
            {
                bValid = true;
                break;
            }

            case KEYGROUP_MISC :
            {
                switch ( nKey ) {
                    case KEY_SUBTRACT :
                    case KEY_COMMA :
                    case KEY_POINT :
                    {
                        bValid = true;
                        break;
                    }

                    default :
                    {
                        if( nKey < KEY_ADD || nKey > KEY_EQUAL )
                            bValid = true;
                        break;
                    }
                }
                break;
            }

            default :
            {
                bValid = false;
                break;
            }
        }

        //Select all, Copy, Paste, Cut, Undo Keys
        if ( !bValid && ( rKeyCode.IsMod1() && (
             KEY_A == nKey || KEY_C == nKey || KEY_V == nKey || KEY_X == nKey || KEY_Z == nKey ) ) )
            bValid = sal_True;
    }
    else
        bValid = sal_True;
    if( bValid )
        Edit::KeyInput( rKeyEvent );
}

Size CuiCustomMultilineEdit::GetOptimalSize() const
{
    return LogicToPixel(Size(150,30),MAP_APPFONT);
}

Size CuiAboutConfigTabPage::GetOptimalSize() const
{
    return LogicToPixel(Size(1024,800),MAP_APPFONT);
}

CuiAboutConfigTabPage::CuiAboutConfigTabPage( Window* pParent/*, const SfxItemSet& rItemSet*/ ) :
    ModalDialog( pParent, "AboutConfig", "cui/ui/aboutconfigdialog.ui"),
    m_pPrefCtrl( get<SvSimpleTableContainer>("preferences") ),
    m_pResetBtn( get<PushButton>("reset") ),
    m_pEditBtn( get<PushButton>("edit") ),
    m_vectorOfModified(),
    m_pPrefBox( new OptHeaderTabListBox( *m_pPrefCtrl,
                                         WB_SCROLL | WB_HSCROLL | WB_VSCROLL ) )
{
    Size aControlSize(200,200);
    m_pPrefCtrl->set_width_request(aControlSize.Width());
    m_pPrefCtrl->set_height_request(aControlSize.Height());

    m_pEditBtn->SetClickHdl( LINK( this, CuiAboutConfigTabPage, StandardHdl_Impl ) );
    m_pResetBtn->SetClickHdl( LINK( this, CuiAboutConfigTabPage, ResetBtnHdl_Impl ) );
    m_pPrefBox->SetDoubleClickHdl( LINK(this, CuiAboutConfigTabPage, StandardHdl_Impl) );

    HeaderBar &rBar = m_pPrefBox->GetTheHeaderBar();
    rBar.InsertItem( ITEMID_PREFNAME, get<FixedText>("preference")->GetText(), 0, HIB_LEFT | HIB_VCENTER );
    rBar.InsertItem( ITEMID_PROPERTY, get<FixedText>("property")->GetText(), 0,  HIB_LEFT | HIB_VCENTER );
    rBar.InsertItem( ITEMID_TYPE, get<FixedText>("type")->GetText(), 0,  HIB_LEFT | HIB_VCENTER );
    rBar.InsertItem( ITEMID_VALUE, get<FixedText>("value")->GetText(), 0,  HIB_LEFT | HIB_VCENTER );

    long aTabs[] = {4,900,50,50,50};//TODO: Not works correctly hardcoded for now.

    aTabs[2] += aTabs[1] + rBar.GetTextWidth(rBar.GetItemText(1));
    aTabs[3] += aTabs[2] + 160; //rBar.GetTextWidth(rBar.GetItemText(2));
    aTabs[4] += aTabs[3] + 40; //rBar.GetTextWidth(rBar.GetItemText(3));

    m_pPrefBox->SetTabs(aTabs, MAP_PIXEL);
}

void CuiAboutConfigTabPage::InsertEntry(const OUString& rProp, const OUString& rStatus, const OUString& rType, const OUString& rValue)
{
    SvTreeListEntry* pEntry = new SvTreeListEntry;

    pEntry->AddItem( new SvLBoxContextBmp( pEntry, 0, Image(), Image(), 0)); //It is needed, otherwise causes crash
    pEntry->AddItem( new SvLBoxString( pEntry, 0, rProp));
    pEntry->AddItem( new SvLBoxString( pEntry, 0, rStatus));
    pEntry->AddItem( new SvLBoxString( pEntry, 0, rType));
    pEntry->AddItem( new SvLBoxString( pEntry, 0, rValue));

    m_pPrefBox->Insert( pEntry );
}

void CuiAboutConfigTabPage::Reset(/* const SfxItemSet&*/ )
{
    OUString sRootNodePath = "";
    m_pPrefBox->Clear();

    m_vectorOfModified.clear();
    m_pPrefBox->GetModel()->SetSortMode( SortNone );

    m_pPrefBox->SetUpdateMode(sal_False);
    Reference< XNameAccess > xConfigAccess = getConfigAccess( sRootNodePath, sal_False );
    FillItems( xConfigAccess, sRootNodePath );
    m_pPrefBox->SetUpdateMode(sal_True);
}

sal_Bool CuiAboutConfigTabPage::FillItemSet(/* SfxItemSet&*/ )
{
    sal_Bool bModified = sal_False;
    Reference< XNameAccess > xUpdateAccess = getConfigAccess( "/", sal_True );

    std::vector< boost::shared_ptr< Prop_Impl > >::iterator pIter;
    for( pIter = m_vectorOfModified.begin() ; pIter != m_vectorOfModified.end(); ++pIter )
    {
        xUpdateAccess = getConfigAccess( (*pIter)->Name , sal_True );
        Reference< XNameReplace > xNameReplace( xUpdateAccess, UNO_QUERY_THROW );

        xNameReplace->replaceByName( (*pIter)->Property, (*pIter)->Value );
        bModified = sal_True;

        Reference< util::XChangesBatch > xChangesBatch( xUpdateAccess, UNO_QUERY_THROW );
        xChangesBatch->commitChanges();
    }

    return bModified;
}

void CuiAboutConfigTabPage::FillItems(const Reference< XNameAccess >& xNameAccess, const OUString& sPath)
{
    Reference< XHierarchicalNameAccess > xHierarchicalNameAccess( xNameAccess, uno::UNO_QUERY_THROW );

    uno::Sequence< OUString > seqItems = xNameAccess->getElementNames();
    for( sal_Int16 i = 0; i < seqItems.getLength(); ++i )
    {
        // I have no idea why this is necessary, but without it, some keys will throw NoSuchElementException
        if( !xHierarchicalNameAccess->hasByHierarchicalName( seqItems[i] ))
        {
            continue;
        }

        Any aNode = xHierarchicalNameAccess->getByHierarchicalName( seqItems[i] );

        Reference< XNameAccess > xNextNameAccess( aNode, uno::UNO_QUERY );
        if( xNextNameAccess.is() )
        {
            // not leaf node
            FillItems( xNextNameAccess, sPath + "/" + seqItems[i] );
        }
        else
        {
            // leaf node
            OUString sType = aNode.getValueTypeName();

            OUString sValue;
            if( aNode.hasValue() )
            {
                switch( aNode.getValueType().getTypeClass() )
                {
                    case ::com::sun::star::uno::TypeClass_UNSIGNED_SHORT :
                    case ::com::sun::star::uno::TypeClass_SHORT :
                    case ::com::sun::star::uno::TypeClass_UNSIGNED_LONG :
                    case ::com::sun::star::uno::TypeClass_LONG :
                    //case ::com::sun::star::uno::TypeClass_INT :
                    {
                        sal_Int32 nVal = 0;
                        if(aNode >>= nVal)
                        {
                            sValue = OUString::number( nVal );
                        }
                    }
                    break;

                    case ::com::sun::star::uno::TypeClass_BOOLEAN :
                    {
                        sal_Bool bVal = sal_False;
                        if(aNode >>= bVal  )
                        {
                            sValue = OUString::boolean( bVal );
                        }
                    }
                    break;

                    case ::com::sun::star::uno::TypeClass_STRING :
                    {
                        OUString sString;
                        if(aNode >>= sString)
                        {
                            sValue = sString;
                        }

                    }
                    break;

                    case ::com::sun::star::uno::TypeClass_SEQUENCE :
                    //case ::com::sun::star::uno::TypeClass_ARRAY :
                    {
                        sValue = "";
                        if( "[]long" == sType || "[]short"== sType )
                        {
                            uno::Sequence<sal_Int32> seqLong;
                            if( aNode >>= seqLong )
                            {
                                for(int nInd=0;  nInd < seqLong.getLength(); ++nInd)
                                {
                                    sValue += OUString::number(seqLong[nInd]) + ",";
                                }
                            }
                        }

                        if( "[]string" == sType )
                        {
                            uno::Sequence< OUString > seqOUString;
                            if( aNode >>= seqOUString )
                            {
                                for( sal_Int16 nInd=0; nInd < seqOUString.getLength(); ++nInd )
                                {
                                    sValue += seqOUString[nInd] + ",";
                                }
                            }
                        }

                        if( "[]hyper" == sType )
                        {
                            uno::Sequence< sal_Int64 > seqHyp;
                            if( aNode >>= seqHyp )
                            {
                                for(int nInd = 0; nInd < seqHyp.getLength(); ++nInd)
                                {
                                    sValue += OUString::number( seqHyp[nInd] ) + ",";
                                }
                            }
                        }
                    }
                    break;

                    default:
                    {
                        if( "hyper" == sType )
                        {
                            sal_Int64 nHyp = 0;
                            if(aNode >>= nHyp)
                            {
                                sValue = OUString::number( nHyp );
                            }
                        }else
                            sValue = "";
                    }
                }
            }

            InsertEntry( sPath, seqItems[i], sType, sValue);
        }
    }
}

Reference< XNameAccess > CuiAboutConfigTabPage::getConfigAccess( OUString sNodePath, sal_Bool bUpdate )
{
    uno::Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );

    uno::Reference< lang::XMultiServiceFactory > xConfigProvider(
                com::sun::star::configuration::theDefaultProvider::get( xContext  ) );

    if( sNodePath == "" )
        sNodePath = "/";
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

void CuiAboutConfigTabPage::AddToModifiedVector( const boost::shared_ptr< Prop_Impl >& rProp )
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
    //property is not modified before
}

std::vector< OUString > CuiAboutConfigTabPage::commaStringToSequence( const OUString& rCommaSepString )
{
    std::vector<OUString> tempVector;

    sal_Int32 index = 0;
    do
    {
        OUString word = rCommaSepString.getToken(0, static_cast<sal_Unicode> (','), index);
        word = word.trim();
        if( !word.isEmpty())
            tempVector.push_back(word);
    }while( index >= 0 );
    return tempVector;
}

CuiAboutConfigValueDialog::CuiAboutConfigValueDialog( Window* pWindow,
                                                      const OUString& rValue,
                                                      int limit ) :
    ModalDialog( pWindow, "AboutConfigValueDialog", "cui/ui/aboutconfigvaluedialog.ui" ),
    m_pEDValue( get<CuiCustomMultilineEdit>("valuebox") )
{
    m_pEDValue->bNumericOnly = ( limit !=0 );
    m_pEDValue->SetMaxTextLen( limit == 0 ? EDIT_NOLIMIT : limit);
    m_pEDValue->SetText( rValue );

}

IMPL_LINK( CuiAboutConfigTabPage, HeaderSelect_Impl, HeaderBar*, /*pBar*/ )
{
    return 0;
}

IMPL_LINK_NOARG( CuiAboutConfigTabPage, ResetBtnHdl_Impl )
{
    Reset();
    return 0;
}

IMPL_LINK_NOARG( CuiAboutConfigTabPage, StandardHdl_Impl )
{
    SvTreeListEntry* pEntry = m_pPrefBox->FirstSelected();

    OUString sPropertyPath = m_pPrefBox->GetEntryText( pEntry, 0 );
    OUString sPropertyName = m_pPrefBox->GetEntryText( pEntry, 1 );
    OUString sPropertyType = m_pPrefBox->GetEntryText( pEntry, 2 );
    OUString sPropertyValue = m_pPrefBox->GetEntryText( pEntry, 3 );

    boost::shared_ptr< Prop_Impl > pProperty (new Prop_Impl( sPropertyPath, sPropertyName, makeAny( sPropertyValue ) ) );

    bool bOpenDialog;
    OUString sDialogValue;
    OUString sNewValue;

    if( sPropertyType == "boolean" )
    {
        bool bValue;
        if( sPropertyValue == "true" )
        {
            sDialogValue = "false";
            bValue = false;
        }
        else
        {
            sDialogValue = "true";
            bValue = true;
        }

        pProperty->Value = uno::makeAny( bValue );
        bOpenDialog = false;
    }
    else if ( sPropertyType == "void" )
    {
        bOpenDialog = false;
    }
    else
    {
        sDialogValue = sPropertyValue;
        bOpenDialog = true;
    }

    try
    {
        if( bOpenDialog )
        {
            //Cosmetic length limit for integer values.
            int limit=0;
            if( sPropertyType == "short" )
                limit = SHORT_LEN_LIMIT;
            else if( sPropertyType == "long" )
                limit = LONG_LEN_LIMIT;
            else if( sPropertyType == "hyper" )
                limit = HYPER_LEN_LIMIT;

            CuiAboutConfigValueDialog* pValueDialog = new CuiAboutConfigValueDialog(0, sDialogValue, limit);

            bool ret = pValueDialog->Execute();
            if( ret == RET_OK )
            {
                sNewValue = pValueDialog->getValue();
                if ( sPropertyType == "short")
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
                    if( sPropertyType == "long" )
                    {
                        sal_Int32 nLong = sNewValue.toInt32();
                        if( !( nLong==0 && sNewValue.getLength()!=1 ) && nLong < SAL_MAX_INT32 && nLong > SAL_MIN_INT32)
                            pProperty->Value = uno::makeAny( nLong );
                        else
                            throw uno::Exception();
                    }
                    else if( sPropertyType == "hyper")
                    {
                        sal_Int64 nHyper = sNewValue.toInt64();
                        if( !( nHyper==0 && sNewValue.getLength()!=1 ) && nHyper < SAL_MAX_INT32 && nHyper > SAL_MIN_INT32)
                            pProperty->Value = uno::makeAny( nHyper );
                        else
                            throw uno::Exception();
                    }
                    else if( sPropertyType == "double")
                    {
                        double nDoub = sNewValue.toDouble();
                        if( !( nDoub ==0 && sNewValue.getLength()!=1 ) && nDoub < SAL_MAX_INT32 && nDoub > SAL_MIN_INT32)
                            pProperty->Value = uno::makeAny( nDoub );
                        else
                            throw uno::Exception();
                    }
                    else if( sPropertyType == "float")
                    {
                        float nFloat = sNewValue.toFloat();
                        if( !( nFloat ==0 && sNewValue.getLength()!=1 ) && nFloat < SAL_MAX_INT32 && nFloat > SAL_MIN_INT32)
                            pProperty->Value = uno::makeAny( nFloat );
                        else
                            throw uno::Exception();
                    }
                    else if( sPropertyType == "string" )
                    {
                        pProperty->Value = uno::makeAny( sNewValue );
                    }
                    else if( sPropertyType == "[]short" )
                    {
                        //create string sequence from comma seperated string
                        //uno::Sequence< OUString > seqStr;
                        std::vector< OUString > seqStr;
                        seqStr = commaStringToSequence( sNewValue );

                        //create appropriate sequence with same size as string sequence
                        uno::Sequence< sal_Int16 > seqShort( seqStr.size() );
                        //convert all strings to appropriate type
                        for( size_t i = 0; i < seqStr.size(); ++i )
                        {
                            seqShort[i] = (sal_Int16) seqStr[i].toInt32();
                        }
                        pProperty->Value = uno::makeAny( seqShort );
                    }
                    else if( sPropertyType == "[]long" )
                    {
                        std::vector< OUString > seqStrLong;
                        seqStrLong = commaStringToSequence( sNewValue );

                        uno::Sequence< sal_Int32 > seqLong( seqStrLong.size() );
                        for( size_t i = 0; i < seqStrLong.size(); ++i )
                        {
                            seqLong[i] = seqStrLong[i].toInt32();
                        }
                        pProperty->Value = uno::makeAny( seqLong );
                    }
                    else if( sPropertyType == "[]hyper" )
                    {
                        std::vector< OUString > seqStrHyper;
                        seqStrHyper = commaStringToSequence( sNewValue );
                        uno::Sequence< sal_Int64 > seqHyper( seqStrHyper.size() );
                        for( size_t i = 0; i < seqStrHyper.size(); ++i )
                        {
                            seqHyper[i] = seqStrHyper[i].toInt64();
                        }
                        pProperty->Value = uno::makeAny( seqHyper );
                    }
                    else if( sPropertyType == "[]double" )
                    {
                        std::vector< OUString > seqStrDoub;
                        seqStrDoub = commaStringToSequence( sNewValue );
                        uno::Sequence< double > seqDoub( seqStrDoub.size() );
                        for( size_t i = 0; i < seqStrDoub.size(); ++i )
                        {
                            seqDoub[i] = seqStrDoub[i].toDouble();
                        }
                        pProperty->Value = uno::makeAny( seqDoub );
                    }
                    else if( sPropertyType == "[]float" )
                    {
                        std::vector< OUString > seqStrFloat;
                        seqStrFloat = commaStringToSequence( sNewValue );
                        uno::Sequence< sal_Int16 > seqFloat( seqStrFloat.size() );
                        for( size_t i = 0; i < seqStrFloat.size(); ++i )
                        {
                            seqFloat[i] = seqStrFloat[i].toFloat();
                        }
                        pProperty->Value = uno::makeAny( seqFloat );
                    }
                    else if( sPropertyType == "[]string" )
                    {
                        pProperty->Value = uno::makeAny( comphelper::containerToSequence( commaStringToSequence( sNewValue )));
                    }
                    else //unknown
                        throw uno::Exception();


                sDialogValue = sNewValue;
            }
        }
        AddToModifiedVector( pProperty );

        //update listbox value.
        m_pPrefBox->SetEntryText( sDialogValue,  pEntry, 3 );
    }
    catch( uno::Exception& )
    {
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
