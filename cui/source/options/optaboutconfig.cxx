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
#include <vcl/svlbitm.hxx>
#include <vcl/treelistentry.hxx>
#include <vcl/fixed.hxx>
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
#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/util/SearchAlgorithms2.hpp>
#include <unotools/textsearch.hxx>
#include <sal/log.hxx>

#include <memory>
#include <vector>
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

struct UserData
{
    bool bIsPropertyPath;
    OUString sPropertyPath;
    int aLineage;
    Reference<XNameAccess> aXNameAccess;

    explicit UserData( OUString const & rPropertyPath )
        : bIsPropertyPath( true )
        , sPropertyPath(rPropertyPath)
        , aLineage(0)
    {}

    explicit UserData( Reference<XNameAccess> const & rXNameAccess, int rIndex )
        : bIsPropertyPath( false )
        , aLineage(rIndex)
        , aXNameAccess( rXNameAccess )
    {}
};

IMPL_LINK(CuiAboutConfigValueDialog, KeyInputHdl, const KeyEvent&, rKeyEvent, bool)
{
    bool bValid = false;
    bool bNonSpace = rKeyEvent.GetKeyCode().GetCode() != KEY_SPACE;
    if (m_bNumericOnly && bNonSpace )
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

    //if value return true to claim that it has been handled
    return !bValid;
}

CuiAboutConfigTabPage::CuiAboutConfigTabPage( vcl::Window* pParent/*, const SfxItemSet& rItemSet*/ ) :
    ModalDialog( pParent, "AboutConfig", "cui/ui/aboutconfigdialog.ui"),
    m_pPrefCtrl( get<SvSimpleTableContainer>("preferences") ),
    m_pResetBtn( get<PushButton>("reset") ),
    m_pEditBtn( get<PushButton>("edit") ),
    m_pSearchBtn( get<PushButton>("searchButton") ),
    m_pSearchEdit( get<Edit>("searchEntry") ),
    m_vectorOfModified(),
    m_pPrefBox( VclPtr<SvSimpleTable>::Create(*m_pPrefCtrl, WB_SCROLL | WB_HSCROLL | WB_VSCROLL ) )
{
    Size aControlSize(LogicToPixel(Size(385, 230), MapMode(MapUnit::MapAppFont)));
    m_pPrefCtrl->set_width_request(aControlSize.Width());
    m_pPrefCtrl->set_height_request(aControlSize.Height());

    m_pEditBtn->SetClickHdl( LINK( this, CuiAboutConfigTabPage, StandardHdl_Impl ) );
    m_pResetBtn->SetClickHdl( LINK( this, CuiAboutConfigTabPage, ResetBtnHdl_Impl ) );
    m_pPrefBox->SetDoubleClickHdl( LINK(this, CuiAboutConfigTabPage, DoubleClickHdl_Impl) );
    m_pPrefBox->SetExpandingHdl( LINK(this, CuiAboutConfigTabPage, ExpandingHdl_Impl) );
    m_pSearchBtn->SetClickHdl( LINK(this, CuiAboutConfigTabPage, SearchHdl_Impl) );

    m_pPrefBox->InsertHeaderEntry(get<FixedText>("preference")->GetText());
    m_pPrefBox->InsertHeaderEntry(get<FixedText>("property")->GetText());
    m_pPrefBox->InsertHeaderEntry(get<FixedText>("type")->GetText());
    m_pPrefBox->InsertHeaderEntry(get<FixedText>("value")->GetText());

    float fWidth = approximate_char_width();

    long aTabs[] = {0,0,0,0};
    aTabs[1] = fWidth * 65;
    aTabs[2] = aTabs[1] + fWidth * 20;
    aTabs[3] = aTabs[2] + fWidth * 8;

    m_options.AlgorithmType2 = util::SearchAlgorithms2::ABSOLUTE;
    m_options.transliterateFlags |= TransliterationFlags::IGNORE_CASE;
    m_options.searchFlag |= (util::SearchFlags::REG_NOT_BEGINOFLINE |
                                        util::SearchFlags::REG_NOT_ENDOFLINE);

    m_pPrefBox->SetTabs(SAL_N_ELEMENTS(aTabs), aTabs, MapUnit::MapPixel);
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
    ModalDialog::dispose();
}

void CuiAboutConfigTabPage::InsertEntry(const OUString& rPropertyPath, const OUString& rProp, const OUString& rStatus,
                                        const OUString& rType, const OUString& rValue, SvTreeListEntry *pParentEntry,
                                        bool bInsertToPrefBox)
{
    SvTreeListEntry* pEntry = new SvTreeListEntry;
    pEntry->AddItem(std::make_unique<SvLBoxContextBmp>(
        Image(), Image(), false)); //It is needed, otherwise causes crash
    pEntry->AddItem(std::make_unique<SvLBoxString>(rProp));
    pEntry->AddItem(std::make_unique<SvLBoxString>(rStatus));
    pEntry->AddItem(std::make_unique<SvLBoxString>(rType));
    pEntry->AddItem(std::make_unique<SvLBoxString>(rValue));
    m_vectorUserData.push_back(std::make_unique<UserData>(rPropertyPath));
    pEntry->SetUserData(m_vectorUserData.back().get());

    if(bInsertToPrefBox)
        m_pPrefBox->Insert( pEntry, pParentEntry );
    else
        m_prefBoxEntries.push_back(std::unique_ptr<SvTreeListEntry>(pEntry));
}

void CuiAboutConfigTabPage::Reset()
{
    m_pPrefBox->Clear();

    m_vectorOfModified.clear();
    m_pPrefBox->GetModel()->SetSortMode( SortNone );
    m_prefBoxEntries.clear();
    m_modifiedPrefBoxEntries.clear();

    m_pPrefBox->SetUpdateMode(false);
    Reference< XNameAccess > xConfigAccess = getConfigAccess( "/", false );
    //Load all XNameAccess to m_prefBoxEntries
    FillItems( xConfigAccess, nullptr, 0, true );
    //Load xConfigAccess' children to m_prefBox
    FillItems( xConfigAccess );
    m_pPrefBox->SetUpdateMode(true);
}

void CuiAboutConfigTabPage::FillItemSet()
{
    std::vector< std::shared_ptr< Prop_Impl > >::iterator pIter;
    for( pIter = m_vectorOfModified.begin() ; pIter != m_vectorOfModified.end(); ++pIter )
    {
        Reference< XNameAccess > xUpdateAccess = getConfigAccess( (*pIter)->Name , true );
        Reference< XNameReplace > xNameReplace( xUpdateAccess, UNO_QUERY_THROW );

        xNameReplace->replaceByName( (*pIter)->Property, (*pIter)->Value );

        Reference< util::XChangesBatch > xChangesBatch( xUpdateAccess, UNO_QUERY_THROW );
        xChangesBatch->commitChanges();
    }
}

void CuiAboutConfigTabPage::FillItems(const Reference< XNameAccess >& xNameAccess, SvTreeListEntry *pParentEntry,
                                      int lineage, bool bLoadAll)
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
            xNextNameAccess.set(aNode, uno::UNO_QUERY);
            bNotLeaf = xNextNameAccess.is();
        }
        catch (const RuntimeException& e)
        {
            SAL_WARN( "cui.options", "CuiAboutConfigTabPage: " << e);
        }

        if (bNotLeaf)
        {
            if(bLoadAll)
                FillItems(xNextNameAccess, nullptr, lineage + 1, true);
            else
            {
                // not leaf node
                SvTreeListEntry* pEntry = new SvTreeListEntry;
                pEntry->AddItem(std::make_unique<SvLBoxContextBmp>(
                    SvTreeListBox::GetDefaultExpandedNodeImage(),
                    SvTreeListBox::GetDefaultCollapsedNodeImage(), false));
                pEntry->AddItem(std::make_unique<SvLBoxString>(seqItems[i]));
                //It is needed, without this the selection line will be truncated.
                pEntry->AddItem(std::make_unique<SvLBoxString>(""));
                pEntry->AddItem(std::make_unique<SvLBoxString>(""));
                pEntry->AddItem(std::make_unique<SvLBoxString>(""));

                m_vectorUserData.push_back(std::make_unique<UserData>(xNextNameAccess, lineage + 1));
                pEntry->SetUserData(m_vectorUserData.back().get());
                pEntry->EnableChildrenOnDemand();
                m_pPrefBox->Insert( pEntry, pParentEntry );
            }
        }
        else
        {
            // leaf node
            OUString sPropertyName = seqItems[i];
            SvTreeListEntries::iterator it = std::find_if(m_modifiedPrefBoxEntries.begin(), m_modifiedPrefBoxEntries.end(),
              [&sPath, &sPropertyName](std::unique_ptr<SvTreeListEntry> const& pEntry) -> bool
              {
                  return static_cast<UserData*>(pEntry->GetUserData())->sPropertyPath == sPath
                      && static_cast<SvLBoxString&>(pEntry->GetItem(2)).GetText() == sPropertyName;
              }
            );

            OUString sType = aNode.getValueTypeName();
            OUStringBuffer sValue;

            if (it != m_modifiedPrefBoxEntries.end())
                sValue = static_cast< SvLBoxString& >( (*it)->GetItem(4) ).GetText();
            else
            {
                switch( aNode.getValueType().getTypeClass() )
                {
                case css::uno::TypeClass_VOID:
                    break;

                case css::uno::TypeClass_BOOLEAN:
                    sValue = OUString::boolean( aNode.get<bool>() );
                    break;

                case css::uno::TypeClass_SHORT:
                case css::uno::TypeClass_LONG:
                case css::uno::TypeClass_HYPER:
                    sValue = OUString::number( aNode.get<sal_Int64>() );
                    break;

                case css::uno::TypeClass_DOUBLE:
                    sValue = OUString::number( aNode.get<double>() );
                    break;

                case css::uno::TypeClass_STRING:
                    sValue = aNode.get<OUString>();
                    break;

                case css::uno::TypeClass_SEQUENCE:
                    if( sType == "[]boolean" )
                    {
                        uno::Sequence<sal_Bool> seq = aNode.get< uno::Sequence<sal_Bool> >();
                        for( sal_Int32 j = 0; j != seq.getLength(); ++j )
                        {
                            if( j != 0 )
                            {
                                sValue.append(",");
                            }
                            sValue.append(OUString::boolean( seq[j] ));
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
                                sValue.append("0");
                            }
                            sValue.append(s.toAsciiUpperCase());
                        }
                    }
                    else if( sType == "[][]byte" )
                    {
                        uno::Sequence< uno::Sequence<sal_Int8> > seq = aNode.get< uno::Sequence< uno::Sequence<sal_Int8> > >();
                        for( sal_Int32 j = 0; j != seq.getLength(); ++j )
                        {
                            if( j != 0 )
                            {
                                sValue.append(",");
                            }
                            for( sal_Int32 k = 0; k != seq[j].getLength(); ++k )
                            {
                                OUString s = OUString::number(
                                    static_cast<sal_uInt8>(seq[j][k]), 16 );
                                if( s.getLength() == 1 )
                                {
                                    sValue.append("0");
                                }
                                sValue.append(s.toAsciiUpperCase());
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
                                sValue.append(",");
                            }
                            sValue.append(OUString::number( seq[j] ));
                        }
                    }
                    else if( sType == "[]long" )
                    {
                        uno::Sequence<sal_Int32> seq = aNode.get< uno::Sequence<sal_Int32> >();
                        for( sal_Int32 j = 0; j != seq.getLength(); ++j )
                        {
                            if( j != 0 )
                            {
                                sValue.append(",");
                            }
                            sValue.append(OUString::number( seq[j] ));
                        }
                    }
                    else if( sType == "[]hyper" )
                    {
                        uno::Sequence<sal_Int64> seq = aNode.get< uno::Sequence<sal_Int64> >();
                        for( sal_Int32 j = 0; j != seq.getLength(); ++j )
                        {
                            if( j != 0 )
                            {
                                sValue.append(",");
                            }
                            sValue.append(OUString::number( seq[j] ));
                        }
                    }
                    else if( sType == "[]double" )
                    {
                        uno::Sequence<double> seq = aNode.get< uno::Sequence<double> >();
                        for( sal_Int32 j = 0; j != seq.getLength(); ++j )
                        {
                            if( j != 0 )
                            {
                                sValue.append(",");
                            }
                            sValue.append(OUString::number( seq[j] ));
                        }
                    }
                    else if( sType == "[]string" )
                    {
                        uno::Sequence<OUString> seq = aNode.get< uno::Sequence<OUString> >();
                        for( sal_Int32 j = 0; j != seq.getLength(); ++j )
                        {
                            if( j != 0 )
                            {
                                sValue.append(",");
                            }
                            sValue.append(seq[j]);
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
            }

            //Short name
            int index = 0;
            for(int j = 1; j < lineage; ++j)
                index = sPath.indexOf("/", index + 1);

            InsertEntry(sPath, sPath.copy(index+1), seqItems[i], sType, sValue.makeStringAndClear(), pParentEntry, !bLoadAll);
        }
    }
}

Reference< XNameAccess > CuiAboutConfigTabPage::getConfigAccess( const OUString& sNodePath, bool bUpdate )
{
    uno::Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );

    uno::Reference< lang::XMultiServiceFactory > xConfigProvider(
                css::configuration::theDefaultProvider::get( xContext  ) );

    beans::NamedValue aProperty;
    aProperty.Name = "nodepath";
    aProperty.Value <<= sNodePath;

    uno::Sequence< uno::Any > aArgumentList( 1 );
    aArgumentList[0] <<= aProperty;

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

void CuiAboutConfigTabPage::AddToModifiedVector( const std::shared_ptr< Prop_Impl >& rProp )
{
    bool isModifiedBefore = false;
    //Check if value modified before
    for(std::shared_ptr<Prop_Impl> & nInd : m_vectorOfModified)
    {
        if( rProp->Name == nInd->Name && rProp->Property == nInd->Property )
        {
            //property modified before. Assign reference to the modified value
            //do your changes on this object. They will be saved later.
            nInd = rProp;
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
        OUString word = rCommaSepString.getToken(0, u',', index);
        word = word.trim();
        if( !word.isEmpty())
            tempVector.push_back(word);
    }while( index >= 0 );
    return tempVector;
}

CuiAboutConfigValueDialog::CuiAboutConfigValueDialog(weld::Window* pWindow,
                                                     const OUString& rValue,
                                                     int limit)
    : GenericDialogController(pWindow, "cui/ui/aboutconfigvaluedialog.ui", "AboutConfigValueDialog")
    , m_bNumericOnly(limit != 0)
    , m_xEDValue(m_xBuilder->weld_entry("valuebox"))
{
    if (limit)
        m_xEDValue->set_max_length(limit);
    m_xEDValue->set_text(rValue);
    m_xEDValue->connect_key_press(LINK(this, CuiAboutConfigValueDialog, KeyInputHdl));
}

CuiAboutConfigValueDialog::~CuiAboutConfigValueDialog()
{
}

IMPL_LINK_NOARG( CuiAboutConfigTabPage, ResetBtnHdl_Impl, Button*, void )
{
    Reset();
}

IMPL_LINK_NOARG( CuiAboutConfigTabPage, DoubleClickHdl_Impl, SvTreeListBox*, bool )
{
    StandardHdl_Impl(nullptr);
    return true;
}

IMPL_LINK_NOARG( CuiAboutConfigTabPage, StandardHdl_Impl, Button*, void )
{
    SvTreeListEntry* pEntry = m_pPrefBox->GetHdlEntry();
    if(pEntry == nullptr)
        return;

    UserData *pUserData = static_cast<UserData*>(pEntry->GetUserData());
    if(pUserData && pUserData->bIsPropertyPath)
    {
        //if selection is a node
        OUString sPropertyName = SvTabListBox::GetEntryText( pEntry, 1 );
        OUString sPropertyType = SvTabListBox::GetEntryText( pEntry, 2 );
        OUString sPropertyValue = SvTabListBox::GetEntryText( pEntry, 3 );

        std::shared_ptr< Prop_Impl > pProperty (new Prop_Impl( pUserData->sPropertyPath, sPropertyName, Any( sPropertyValue ) ) );
        bool bSaveChanges = false;

        bool bOpenDialog = true;
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

            pProperty->Value <<= bValue;
            bOpenDialog = false;
            bSaveChanges = true;
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

                CuiAboutConfigValueDialog aValueDialog(GetFrameWeld(), sDialogValue, limit);

                if (aValueDialog.run() == RET_OK )
                {
                    sNewValue = aValueDialog.getValue();
                    bSaveChanges = true;
                    if ( sPropertyType == "short")
                    {
                        sal_Int16 nShort;
                        sal_Int32 nNumb = sNewValue.toInt32();

                        //if the value is 0 and length is not 1, there is something wrong
                        if( ( nNumb==0 && sNewValue.getLength()!=1 ) || nNumb >= SAL_MAX_INT16 || nNumb <= SAL_MIN_INT16)
                            throw uno::Exception("out of range short", nullptr);
                        nShort = static_cast<sal_Int16>(nNumb);
                        pProperty->Value <<= nShort;
                    }
                    else if( sPropertyType == "long" )
                    {
                        sal_Int32 nLong = sNewValue.toInt32();
                        if( ( nLong==0 && sNewValue.getLength()!=1 ) || nLong >= SAL_MAX_INT32 || nLong <= SAL_MIN_INT32)
                            throw uno::Exception("out of range long", nullptr);
                        pProperty->Value <<= nLong;
                    }
                    else if( sPropertyType == "hyper")
                    {
                        sal_Int64 nHyper = sNewValue.toInt64();
                        if( ( nHyper==0 && sNewValue.getLength()!=1 ) || nHyper >= SAL_MAX_INT32 || nHyper <= SAL_MIN_INT32)
                            throw uno::Exception("out of range hyper", nullptr);
                        pProperty->Value <<= nHyper;
                    }
                    else if( sPropertyType == "double")
                    {
                        double nDoub = sNewValue.toDouble();
                        if( ( nDoub ==0 && sNewValue.getLength()!=1 ) || nDoub >= SAL_MAX_INT32 || nDoub <= SAL_MIN_INT32)
                            throw uno::Exception("out of range double", nullptr);
                        pProperty->Value <<= nDoub;
                    }
                    else if( sPropertyType == "float")
                    {
                        float nFloat = sNewValue.toFloat();
                        if( ( nFloat ==0 && sNewValue.getLength()!=1 ) || nFloat >= SAL_MAX_INT32 || nFloat <= SAL_MIN_INT32)
                            throw uno::Exception("out of range float", nullptr);
                        pProperty->Value <<= nFloat;
                    }
                    else if( sPropertyType == "string" )
                    {
                        pProperty->Value <<= sNewValue;
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
                            seqShort[i] = static_cast<sal_Int16>(seqStr[i].toInt32());
                        }
                        pProperty->Value <<= seqShort;
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
                        pProperty->Value <<= seqLong;
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
                        pProperty->Value <<= seqHyper;
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
                        pProperty->Value <<= seqDoub;
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
                        pProperty->Value <<= seqFloat;
                    }
                    else if( sPropertyType == "[]string" )
                    {
                        pProperty->Value <<= comphelper::containerToSequence( commaStringToSequence( sNewValue ));
                    }
                    else //unknown
                        throw uno::Exception("unknown property type " + sPropertyType, nullptr);

                    sDialogValue = sNewValue;
                }
            }

            if(bSaveChanges)
            {
                AddToModifiedVector( pProperty );

                //update listbox value.
                m_pPrefBox->SetEntryText( sDialogValue,  pEntry, 3 );
                //update m_prefBoxEntries
                SvTreeListEntries::iterator it = std::find_if(m_prefBoxEntries.begin(), m_prefBoxEntries.end(),
                  [&pUserData, &sPropertyName](std::unique_ptr<SvTreeListEntry> const& rpEntry) -> bool
                  {
                      return static_cast<UserData*>(rpEntry->GetUserData())->sPropertyPath == pUserData->sPropertyPath
                          && static_cast<SvLBoxString&>(rpEntry->GetItem(2)).GetText() == sPropertyName;
                  }
                );
                if (it != m_prefBoxEntries.end())
                {
                    (*it)->ReplaceItem(std::make_unique<SvLBoxString>(sDialogValue), 4);

                    SvTreeListEntries::iterator modifiedIt = std::find_if(
                                m_modifiedPrefBoxEntries.begin(), m_modifiedPrefBoxEntries.end(),
                                [&pUserData, &sPropertyName](std::unique_ptr<SvTreeListEntry> const& rpEntry) -> bool
                                {
                                    return static_cast<UserData*>(rpEntry->GetUserData())->sPropertyPath == pUserData->sPropertyPath
                                        && static_cast<SvLBoxString&>(rpEntry->GetItem(2)).GetText() == sPropertyName;
                                }
                    );

                    if( modifiedIt != m_modifiedPrefBoxEntries.end())
                    {
                        (*modifiedIt)->ReplaceItem(std::make_unique<SvLBoxString>(sDialogValue), 4);
                    }
                    else
                    {
                        std::unique_ptr<SvTreeListEntry> pCloneEntry(
                                new SvTreeListEntry);
                        pCloneEntry->Clone((*it).get());
                        m_modifiedPrefBoxEntries.push_back(std::move(pCloneEntry));
                    }
                }
            }
        }
        catch( uno::Exception& )
        {
        }
    }
}

IMPL_LINK_NOARG( CuiAboutConfigTabPage, SearchHdl_Impl, Button*, void)
{
    m_pPrefBox->Clear();
    m_pPrefBox->SetUpdateMode( false );

    SvSortMode sortMode = m_pPrefBox->GetModel()->GetSortMode();
    sal_uInt16 sortedCol = m_pPrefBox->GetSortedCol();

    if( sortMode != SortNone )
        m_pPrefBox->SortByCol( 0xFFFF );

    if( m_pSearchEdit->GetText().isEmpty() )
    {
        m_pPrefBox->Clear();
        Reference< XNameAccess > xConfigAccess = getConfigAccess( "/", false );
        FillItems( xConfigAccess );
    }
    else
    {
        m_options.searchString = m_pSearchEdit->GetText();
        utl::TextSearch textSearch( m_options );
        for (auto const& it : m_prefBoxEntries)
        {
            sal_Int32 endPos, startPos = 0;

            for(size_t i = 1; i < it->ItemCount(); ++i)
            {
                OUString scrTxt;
                if(i == 1)
                    scrTxt = static_cast< UserData* >( it->GetUserData() )->sPropertyPath;
                else
                    scrTxt = static_cast< SvLBoxString& >( it->GetItem(i) ).GetText();
                endPos = scrTxt.getLength();
                if( textSearch.SearchForward( scrTxt, &startPos, &endPos ) )
                {
                    SvTreeListEntry* pEntry = new SvTreeListEntry;
                    pEntry->Clone( it.get() );
                    InsertEntry( pEntry );
                    break;
                }
            }
        }
    }

    if( sortMode != SortNone )
        m_pPrefBox->SortByCol(sortedCol, sortMode == SortAscending);

    m_pPrefBox->SetUpdateMode( true );
}

void CuiAboutConfigTabPage::InsertEntry( SvTreeListEntry *pEntry)
{
    OUString sPathWithProperty = static_cast< UserData* >(pEntry->GetUserData())->sPropertyPath;
    sal_Int32 index = sPathWithProperty.lastIndexOf(static_cast< SvLBoxString& >(pEntry->GetItem(1)).GetText());
    OUString sPath = sPathWithProperty.copy(0, index);
    index = 0;
    SvTreeListEntry* pParentEntry;
    SvTreeListEntry* pGrandParentEntry = nullptr;

    do
    {
        int prevIndex = index;
        index = sPath.indexOf("/", index+1);
        // deal with no parent case (tdf#107811)
        if (index < 0)
        {
            m_pPrefBox->Insert( pEntry, nullptr);
            return;
        }
        OUString sParentName = sPath.copy(prevIndex+1, index - prevIndex - 1);

        bool hasEntry = false;
        for(pParentEntry = m_pPrefBox->FirstChild(pGrandParentEntry); pParentEntry != nullptr; pParentEntry = pParentEntry->NextSibling())
            if(static_cast< SvLBoxString& >(pParentEntry->GetItem(1)).GetText() == sParentName)
            {
                hasEntry = true;
                break;
            }

        if(!hasEntry)
        {
            pParentEntry = new SvTreeListEntry;
            pParentEntry->AddItem(std::make_unique<SvLBoxContextBmp>(
                   SvTreeListBox::GetDefaultExpandedNodeImage(),
                   SvTreeListBox::GetDefaultCollapsedNodeImage(), false));
            pParentEntry->AddItem(std::make_unique<SvLBoxString>(sParentName));
            //It is needed, without this the selection line will be truncated.
            pParentEntry->AddItem(std::make_unique<SvLBoxString>(""));
            pParentEntry->AddItem(std::make_unique<SvLBoxString>(""));
            pParentEntry->AddItem(std::make_unique<SvLBoxString>(""));
            pParentEntry->EnableChildrenOnDemand(false);
            m_pPrefBox->Insert( pParentEntry, pGrandParentEntry );
        }

        if(pGrandParentEntry)
            m_pPrefBox->Expand( pGrandParentEntry );
        pGrandParentEntry = pParentEntry;
    } while(index < sPath.getLength() - 1);

    m_pPrefBox->Insert( pEntry, pParentEntry );
    m_pPrefBox->Expand( pParentEntry );
}

IMPL_LINK_NOARG( CuiAboutConfigTabPage, ExpandingHdl_Impl, SvTreeListBox*, bool )
{
    SvTreeListEntry* pEntry = m_pPrefBox->GetHdlEntry();

    if(pEntry != nullptr && pEntry->HasChildrenOnDemand())
    {
        pEntry->EnableChildrenOnDemand(false);
        SvTreeListEntry *pFirstChild = m_pPrefBox->FirstChild(pEntry);
        if(pFirstChild)
            m_pPrefBox->RemoveEntry(pFirstChild);

        if(pEntry->GetUserData() != nullptr)
        {
            UserData *pUserData = static_cast<UserData*>(pEntry->GetUserData());
            FillItems( pUserData->aXNameAccess, pEntry, pUserData->aLineage );
        }
    }

    return pEntry && pEntry->HasChildren();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
