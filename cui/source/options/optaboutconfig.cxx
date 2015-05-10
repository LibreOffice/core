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

#include <vcl/builderfactory.hxx>
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
#include <com/sun/star/container/XHierarchicalName.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include <unotools/textsearch.hxx>

#include <vector>
#include <boost/shared_ptr.hpp>
#include <iostream>

using namespace svx;
using namespace ::com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::container;

#define SHORT_LEN_LIMIT     7
#define LONG_LEN_LIMIT      11
#define HYPER_LEN_LIMIT     20

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

VCL_BUILDER_FACTORY_ARGS(CuiCustomMultilineEdit,WB_LEFT|WB_VCENTER|WB_BORDER|WB_3DLOOK)

void CuiCustomMultilineEdit::KeyInput( const KeyEvent& rKeyEvent )
{
    bool bValid = false;
    bool bNonSpace = rKeyEvent.GetKeyCode().GetCode() != KEY_SPACE;
    if( bNumericOnly && bNonSpace )
    {
        const vcl::KeyCode& rKeyCode = rKeyEvent.GetKeyCode();
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
            bValid = true;
    }
    else
        bValid = true;
    if( bValid )
        Edit::KeyInput( rKeyEvent );
}

Size CuiCustomMultilineEdit::GetOptimalSize() const
{
    return LogicToPixel(Size(150, GetTextHeight()), MAP_APPFONT);
}

CuiAboutConfigTabPage::CuiAboutConfigTabPage( vcl::Window* pParent/*, const SfxItemSet& rItemSet*/ ) :
    ModelessDialog( pParent, "AboutConfig", "cui/ui/aboutconfigdialog.ui"),
    m_pPrefCtrl( get<SvSimpleTableContainer>("preferences") ),
    m_pResetBtn( get<PushButton>("reset") ),
    m_pEditBtn( get<PushButton>("edit") ),
    m_pSearchBtn( get<PushButton>("searchButton") ),
    m_pSearchEdit( get<Edit>("searchEntry") ),
    m_vectorOfModified(),
    m_pPrefBox( VclPtr<SvSimpleTable>::Create(*m_pPrefCtrl, WB_SCROLL | WB_HSCROLL | WB_VSCROLL ) )
{
    Size aControlSize(LogicToPixel(Size(385, 230), MAP_APPFONT));
    m_pPrefCtrl->set_width_request(aControlSize.Width());
    m_pPrefCtrl->set_height_request(aControlSize.Height());

    m_pEditBtn->SetClickHdl( LINK( this, CuiAboutConfigTabPage, StandardHdl_Impl ) );
    m_pResetBtn->SetClickHdl( LINK( this, CuiAboutConfigTabPage, ResetBtnHdl_Impl ) );
    m_pPrefBox->SetDoubleClickHdl( LINK(this, CuiAboutConfigTabPage, StandardHdl_Impl) );
    m_pSearchBtn->SetClickHdl( LINK(this, CuiAboutConfigTabPage, SearchHdl_Impl) );

    m_pPrefBox->InsertHeaderEntry(get<FixedText>("preference")->GetText());
    m_pPrefBox->InsertHeaderEntry(get<FixedText>("property")->GetText());
    m_pPrefBox->InsertHeaderEntry(get<FixedText>("type")->GetText());
    m_pPrefBox->InsertHeaderEntry(get<FixedText>("value")->GetText());

    long aTabs[] = {4,0,0,0,0};

    float fWidth = approximate_char_width();

    aTabs[1] = 0;
    aTabs[2] = aTabs[1] + fWidth * 65;
    aTabs[3] = aTabs[2] + fWidth * 20;
    aTabs[4] = aTabs[3] + fWidth * 8;

    m_options.algorithmType = util::SearchAlgorithms_ABSOLUTE;
    m_options.transliterateFlags |= i18n::TransliterationModules_IGNORE_CASE;
    m_options.searchFlag |= (util::SearchFlags::REG_NOT_BEGINOFLINE |
                                        util::SearchFlags::REG_NOT_ENDOFLINE);

    m_pPrefBox->SetTabs(aTabs, MAP_PIXEL);
    m_pPrefBox->SetAlternatingRowColors( true );
}

CuiAboutConfigTabPage::~CuiAboutConfigTabPage()
{
    disposeOnce();
}

void CuiAboutConfigTabPage::dispose()
{
    m_pPrefBox.disposeAndClear();
    m_pPrefCtrl.clear();
    m_pResetBtn.clear();
    m_pEditBtn.clear();
    m_pSearchBtn.clear();
    m_pSearchEdit.clear();
    ModelessDialog::dispose();
}

void CuiAboutConfigTabPage::InsertEntry(const OUString& rProp, const OUString& rStatus, const OUString& rType, const OUString& rValue)
{
    SvTreeListEntry* pEntry = new SvTreeListEntry;

    pEntry->AddItem( new SvLBoxContextBmp( pEntry, 0, Image(), Image(), false)); //It is needed, otherwise causes crash
    pEntry->AddItem( new SvLBoxString( pEntry, 0, rProp));
    pEntry->AddItem( new SvLBoxString( pEntry, 0, rStatus));
    pEntry->AddItem( new SvLBoxString( pEntry, 0, rType));
    pEntry->AddItem( new SvLBoxString( pEntry, 0, rValue));

    m_pPrefBox->Insert( pEntry );

    SvTreeListEntry* pEntryClone = new SvTreeListEntry;
    pEntryClone->Clone( pEntry );
    m_prefBoxEntries.push_back( pEntryClone );
}

void CuiAboutConfigTabPage::Reset()
{
    m_pPrefBox->Clear();

    m_vectorOfModified.clear();
    m_pPrefBox->GetModel()->SetSortMode( SortNone );

    m_pPrefBox->SetUpdateMode(false);
    Reference< XNameAccess > xConfigAccess = getConfigAccess( "/", false );
    FillItems( xConfigAccess );
    m_pPrefBox->SetUpdateMode(true);
}

bool CuiAboutConfigTabPage::FillItemSet()
{
    bool bModified = false;

    std::vector< boost::shared_ptr< Prop_Impl > >::iterator pIter;
    for( pIter = m_vectorOfModified.begin() ; pIter != m_vectorOfModified.end(); ++pIter )
    {
        Reference< XNameAccess > xUpdateAccess = getConfigAccess( (*pIter)->Name , true );
        Reference< XNameReplace > xNameReplace( xUpdateAccess, UNO_QUERY_THROW );

        xNameReplace->replaceByName( (*pIter)->Property, (*pIter)->Value );
        bModified = true;

        Reference< util::XChangesBatch > xChangesBatch( xUpdateAccess, UNO_QUERY_THROW );
        xChangesBatch->commitChanges();
    }

    return bModified;
}

void CuiAboutConfigTabPage::FillItems(const Reference< XNameAccess >& xNameAccess)
{
    OUString sPath = Reference< XHierarchicalName >(
        xNameAccess, uno::UNO_QUERY_THROW )->getHierarchicalName();
    uno::Sequence< OUString > seqItems = xNameAccess->getElementNames();
    for( sal_Int32 i = 0; i < seqItems.getLength(); ++i )
    {
        Any aNode = xNameAccess->getByName( seqItems[i] );

        bool bNotLeaf = false;

        Reference< XNameAccess > xNextNameAccess;
        try
        {
            xNextNameAccess = Reference< XNameAccess >(aNode, uno::UNO_QUERY);
            bNotLeaf = xNextNameAccess.is();
        }
        catch (const RuntimeException& e)
        {
            SAL_WARN( "cui.options", "CuiAboutConfigTabPage: exception " << e.Message);
        }

        if (bNotLeaf)
        {
            // not leaf node
            FillItems( xNextNameAccess );
        }
        else
        {
            // leaf node
            OUString sType = aNode.getValueTypeName();

            OUString sValue;
            switch( aNode.getValueType().getTypeClass() )
            {
            case ::com::sun::star::uno::TypeClass_VOID:
                break;

            case ::com::sun::star::uno::TypeClass_BOOLEAN:
                sValue = OUString::boolean( aNode.get<bool>() );
                break;

            case ::com::sun::star::uno::TypeClass_SHORT:
            case ::com::sun::star::uno::TypeClass_LONG:
            case ::com::sun::star::uno::TypeClass_HYPER:
                sValue = OUString::number( aNode.get<sal_Int64>() );
                break;

            case ::com::sun::star::uno::TypeClass_DOUBLE:
                sValue = OUString::number( aNode.get<double>() );
                break;

            case ::com::sun::star::uno::TypeClass_STRING:
                sValue = aNode.get<OUString>();
                break;

            case ::com::sun::star::uno::TypeClass_SEQUENCE:
                if( sType == "[]boolean" )
                {
                    uno::Sequence<sal_Bool> seq = aNode.get< uno::Sequence<sal_Bool> >();
                    for( sal_Int32 j = 0; j != seq.getLength(); ++j )
                    {
                        if( j != 0 )
                        {
                            sValue += ",";
                        }
                        sValue += OUString::boolean( seq[j] );
                    }
                }
                else if( sType == "[]byte" )
                {
                    uno::Sequence<sal_Int8> seq = aNode.get< uno::Sequence<sal_Int8> >();
                    for( sal_Int32 j = 0; j != seq.getLength(); ++j )
                    {
                        OUString s = OUString::number(
                            static_cast<sal_uInt8>(seq[j]), 16 );
                        if( s.getLength() == 1 )
                        {
                            sValue += "0";
                        }
                        sValue += s.toAsciiUpperCase();
                    }
                }
                else if( sType == "[][]byte" )
                {
                    uno::Sequence< uno::Sequence<sal_Int8> > seq = aNode.get< uno::Sequence< uno::Sequence<sal_Int8> > >();
                    for( sal_Int32 j = 0; j != seq.getLength(); ++j )
                    {
                        if( j != 0 )
                        {
                            sValue += ",";
                        }
                        for( sal_Int32 k = 0; k != seq[j].getLength(); ++k )
                        {
                            OUString s = OUString::number(
                                static_cast<sal_uInt8>(seq[j][k]), 16 );
                            if( s.getLength() == 1 )
                            {
                                sValue += "0";
                            }
                            sValue += s.toAsciiUpperCase();
                        }
                    }
                }
                else if( sType == "[]short" )
                {
                    uno::Sequence<sal_Int16> seq = aNode.get< uno::Sequence<sal_Int16> >();
                    for( sal_Int32 j = 0; j != seq.getLength(); ++j )
                    {
                        if( j != 0 )
                        {
                            sValue += ",";
                        }
                        sValue += OUString::number( seq[j] );
                    }
                }
                else if( sType == "[]long" )
                {
                    uno::Sequence<sal_Int32> seq = aNode.get< uno::Sequence<sal_Int32> >();
                    for( sal_Int32 j = 0; j != seq.getLength(); ++j )
                    {
                        if( j != 0 )
                        {
                            sValue += ",";
                        }
                        sValue += OUString::number( seq[j] );
                    }
                }
                else if( sType == "[]hyper" )
                {
                    uno::Sequence<sal_Int64> seq = aNode.get< uno::Sequence<sal_Int64> >();
                    for( sal_Int32 j = 0; j != seq.getLength(); ++j )
                    {
                        if( j != 0 )
                        {
                            sValue += ",";
                        }
                        sValue += OUString::number( seq[j] );
                    }
                }
                else if( sType == "[]double" )
                {
                    uno::Sequence<double> seq = aNode.get< uno::Sequence<double> >();
                    for( sal_Int32 j = 0; j != seq.getLength(); ++j )
                    {
                        if( j != 0 )
                        {
                            sValue += ",";
                        }
                        sValue += OUString::number( seq[j] );
                    }
                }
                else if( sType == "[]string" )
                {
                    uno::Sequence<OUString> seq = aNode.get< uno::Sequence<OUString> >();
                    for( sal_Int32 j = 0; j != seq.getLength(); ++j )
                    {
                        if( j != 0 )
                        {
                            sValue += ",";
                        }
                        sValue += seq[j];
                    }
                }
                else
                {
                    SAL_WARN(
                        "cui.options",
                        "path \"" << sPath << "\" member " << seqItems[i]
                            << " of unsupported type " << sType);
                }
                break;

            default:
                SAL_WARN(
                    "cui.options",
                    "path \"" << sPath << "\" member " << seqItems[i]
                        << " of unsupported type " << sType);
                break;
            }

            InsertEntry( sPath, seqItems[i], sType, sValue);
        }
    }
}

Reference< XNameAccess > CuiAboutConfigTabPage::getConfigAccess( const OUString& sNodePath, bool bUpdate )
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

void CuiAboutConfigTabPage::AddToModifiedVector( const boost::shared_ptr< Prop_Impl >& rProp )
{
    bool isModifiedBefore = false;
    //Check if value modified before
    for( size_t nInd = 0; nInd < m_vectorOfModified.size() ; ++nInd )
    {
        if( rProp->Name == m_vectorOfModified[nInd]->Name && rProp->Property == m_vectorOfModified[nInd]->Property )
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

CuiAboutConfigValueDialog::CuiAboutConfigValueDialog( vcl::Window* pWindow,
                                                      const OUString& rValue,
                                                      int limit ) :
    ModalDialog( pWindow, "AboutConfigValueDialog", "cui/ui/aboutconfigvaluedialog.ui" ),
    m_pEDValue( get<CuiCustomMultilineEdit>("valuebox") )
{
    m_pEDValue->bNumericOnly = ( limit !=0 );
    m_pEDValue->SetMaxTextLen( limit == 0 ? EDIT_NOLIMIT : limit);
    m_pEDValue->SetText( rValue );

}

CuiAboutConfigValueDialog::~CuiAboutConfigValueDialog()
{
    disposeOnce();
}

void CuiAboutConfigValueDialog::dispose()
{
    m_pEDValue.clear();
    ModalDialog::dispose();
}

IMPL_LINK_NOARG( CuiAboutConfigTabPage, ResetBtnHdl_Impl )
{
    Reset();
    return 0;
}

IMPL_LINK_NOARG( CuiAboutConfigTabPage, StandardHdl_Impl )
{
    SvTreeListEntry* pEntry = m_pPrefBox->FirstSelected();

    OUString sPropertyPath = SvTabListBox::GetEntryText( pEntry, 0 );
    OUString sPropertyName = SvTabListBox::GetEntryText( pEntry, 1 );
    OUString sPropertyType = SvTabListBox::GetEntryText( pEntry, 2 );
    OUString sPropertyValue = SvTabListBox::GetEntryText( pEntry, 3 );

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

            VclPtrInstance<CuiAboutConfigValueDialog> pValueDialog(nullptr, sDialogValue, limit);

            if( pValueDialog->Execute() == RET_OK )
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
                        //create string sequence from comma separated string
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
        //update m_prefBoxEntries
        SvTreeListEntries::iterator it = std::find_if(m_prefBoxEntries.begin(), m_prefBoxEntries.end(),
          [&sPropertyPath, &sPropertyName](SvTreeListEntry &entry) -> bool
          {
              return static_cast< SvLBoxString* >( entry.GetItem(1) )->GetText().equals( sPropertyPath ) &&
                      static_cast< SvLBoxString* >( entry.GetItem(2) )->GetText().equals( sPropertyName );
          }
        );
        it->ReplaceItem( new SvLBoxString( &(*it), 0, sDialogValue ), 4 );
    }
    catch( uno::Exception& )
    {
    }

    return 0;
}

IMPL_LINK_NOARG( CuiAboutConfigTabPage, SearchHdl_Impl)
{
    m_pPrefBox->Clear();
    m_pPrefBox->SetUpdateMode( false );

    SvSortMode sortMode = m_pPrefBox->GetModel()->GetSortMode();
    sal_uInt16 sortedCol = m_pPrefBox->GetSortedCol();

    if( sortMode != SortNone )
        m_pPrefBox->SortByCol( 0xFFFF );

    if( m_pSearchEdit->GetText().isEmpty() )
    {
        for( auto it = m_prefBoxEntries.begin(); it != m_prefBoxEntries.end(); ++it )
        {
            SvTreeListEntry* pEntry = new SvTreeListEntry;
            pEntry->Clone( &(*it) ) ;
            m_pPrefBox->Insert( pEntry );
        }
    }
    else
    {
        m_options.searchString = m_pSearchEdit->GetText();
        utl::TextSearch textSearch( m_options );

        for(auto it = m_prefBoxEntries.begin(); it != m_prefBoxEntries.end(); ++it)
        {
            sal_Int32 endPos, startPos = 0;

            for(size_t i = 1; i < it->ItemCount(); ++i)
            {
                OUString scrTxt = static_cast< SvLBoxString* >( it->GetItem(i) )->GetText();
                endPos = scrTxt.getLength();
                if( textSearch.SearchForward( scrTxt, &startPos, &endPos ) )
                {
                    SvTreeListEntry* pEntry = new SvTreeListEntry;
                    pEntry->Clone( &(*it) ) ;
                    m_pPrefBox->Insert( pEntry );
                    break;
                }
            }
        }
    }

    if( sortMode != SortNone )
        m_pPrefBox->SortByCol(sortedCol, sortMode == SortAscending);

    m_pPrefBox->SetUpdateMode( true );

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
