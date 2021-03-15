/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "optaboutconfig.hxx"

#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/container/XHierarchicalName.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/util/SearchAlgorithms2.hpp>
#include <rtl/ustrbuf.hxx>
#include <unotools/textsearch.hxx>
#include <vcl/event.hxx>
#include <sal/log.hxx>
#include <tools/diagnose_ex.h>

#include <memory>
#include <vector>
#include <iostream>

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

CuiAboutConfigTabPage::CuiAboutConfigTabPage(weld::Window* pParent)
    : GenericDialogController(pParent, "cui/ui/aboutconfigdialog.ui", "AboutConfig")
    , m_xResetBtn(m_xBuilder->weld_button("reset"))
    , m_xEditBtn(m_xBuilder->weld_button("edit"))
    , m_xSearchBtn(m_xBuilder->weld_button("searchButton"))
    , m_xSearchEdit(m_xBuilder->weld_entry("searchEntry"))
    , m_xPrefBox(m_xBuilder->weld_tree_view("preferences"))
    , m_xScratchIter(m_xPrefBox->make_iterator())
    , m_vectorOfModified()
    , m_bSorted(false)
{
    m_xPrefBox->set_size_request(m_xPrefBox->get_approximate_digit_width() * 100,
                                 m_xPrefBox->get_height_rows(23));
    m_xPrefBox->connect_column_clicked(LINK(this, CuiAboutConfigTabPage, HeaderBarClick));

    m_xEditBtn->connect_clicked(LINK( this, CuiAboutConfigTabPage, StandardHdl_Impl));
    m_xResetBtn->connect_clicked(LINK( this, CuiAboutConfigTabPage, ResetBtnHdl_Impl));
    m_xPrefBox->connect_row_activated(LINK(this, CuiAboutConfigTabPage, DoubleClickHdl_Impl));
    m_xPrefBox->connect_expanding(LINK(this, CuiAboutConfigTabPage, ExpandingHdl_Impl));
    m_xSearchBtn->connect_clicked(LINK(this, CuiAboutConfigTabPage, SearchHdl_Impl));

    m_options.AlgorithmType2 = util::SearchAlgorithms2::ABSOLUTE;
    m_options.transliterateFlags |= TransliterationFlags::IGNORE_CASE;
    m_options.searchFlag |= (util::SearchFlags::REG_NOT_BEGINOFLINE |
                                        util::SearchFlags::REG_NOT_ENDOFLINE);

    float fWidth = m_xPrefBox->get_approximate_digit_width();
    std::vector<int> aWidths;
    aWidths.push_back(fWidth * 65);
    aWidths.push_back(fWidth * 20);
    aWidths.push_back(fWidth * 8);
    m_xPrefBox->set_column_fixed_widths(aWidths);
}

IMPL_LINK(CuiAboutConfigTabPage, HeaderBarClick, int, nColumn, void)
{
    if (!m_bSorted)
    {
        m_xPrefBox->make_sorted();
        m_bSorted = true;
    }

    bool bSortAtoZ = m_xPrefBox->get_sort_order();

    //set new arrow positions in headerbar
    if (nColumn == m_xPrefBox->get_sort_column())
    {
        bSortAtoZ = !bSortAtoZ;
        m_xPrefBox->set_sort_order(bSortAtoZ);
    }
    else
    {
        int nOldSortColumn = m_xPrefBox->get_sort_column();
        if (nOldSortColumn != -1)
            m_xPrefBox->set_sort_indicator(TRISTATE_INDET, nOldSortColumn);
        m_xPrefBox->set_sort_column(nColumn);
    }

    if (nColumn != -1)
    {
        //sort lists
        m_xPrefBox->set_sort_indicator(bSortAtoZ ? TRISTATE_TRUE : TRISTATE_FALSE, nColumn);
    }
}

CuiAboutConfigTabPage::~CuiAboutConfigTabPage()
{
}

void CuiAboutConfigTabPage::InsertEntry(const OUString& rPropertyPath, const OUString& rProp, const OUString& rStatus,
                                        const OUString& rType, const OUString& rValue, const weld::TreeIter* pParentEntry,
                                        bool bInsertToPrefBox)
{
    m_vectorUserData.push_back(std::make_unique<UserData>(rPropertyPath));
    if (bInsertToPrefBox)
    {
        OUString sId(OUString::number(reinterpret_cast<sal_Int64>(m_vectorUserData.back().get())));
        m_xPrefBox->insert(pParentEntry, -1, &rProp, &sId, nullptr, nullptr, false, m_xScratchIter.get());
        m_xPrefBox->set_text(*m_xScratchIter, rStatus, 1);
        m_xPrefBox->set_text(*m_xScratchIter, rType, 2);
        m_xPrefBox->set_text(*m_xScratchIter, rValue, 3);
    }
    else
    {
        m_prefBoxEntries.push_back({rProp, rStatus, rType, rValue, m_vectorUserData.back().get()});
    }
}

void CuiAboutConfigTabPage::Reset()
{
    weld::WaitObject aWait(m_xDialog.get());

    m_xPrefBox->clear();
    m_vectorOfModified.clear();
    if (m_bSorted)
    {
        m_xPrefBox->set_sort_indicator(TRISTATE_INDET, m_xPrefBox->get_sort_column());
        m_xPrefBox->make_unsorted();
        m_bSorted = false;
    }
    m_prefBoxEntries.clear();
    m_modifiedPrefBoxEntries.clear();

    m_xPrefBox->freeze();
    Reference< XNameAccess > xConfigAccess = getConfigAccess( "/", false );
    //Load all XNameAccess to m_prefBoxEntries
    FillItems( xConfigAccess, nullptr, 0, true );
    //Load xConfigAccess' children to m_prefBox
    FillItems( xConfigAccess );
    m_xPrefBox->thaw();
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

void CuiAboutConfigTabPage::FillItems(const Reference< XNameAccess >& xNameAccess, const weld::TreeIter* pParentEntry,
                                      int lineage, bool bLoadAll)
{
    OUString sPath = Reference< XHierarchicalName >(
        xNameAccess, uno::UNO_QUERY_THROW )->getHierarchicalName();
    const uno::Sequence< OUString > seqItems = xNameAccess->getElementNames();
    for( const OUString& item : seqItems )
    {
        Any aNode = xNameAccess->getByName( item );

        bool bNotLeaf = false;

        Reference< XNameAccess > xNextNameAccess;
        try
        {
            xNextNameAccess.set(aNode, uno::UNO_QUERY);
            bNotLeaf = xNextNameAccess.is();
        }
        catch (const RuntimeException&)
        {
            TOOLS_WARN_EXCEPTION( "cui.options", "CuiAboutConfigTabPage");
        }

        if (bNotLeaf)
        {
            if(bLoadAll)
                FillItems(xNextNameAccess, nullptr, lineage + 1, true);
            else
            {
                // not leaf node
                m_vectorUserData.push_back(std::make_unique<UserData>(xNextNameAccess, lineage + 1));
                OUString sId(OUString::number(reinterpret_cast<sal_Int64>(m_vectorUserData.back().get())));

                m_xPrefBox->insert(pParentEntry, -1, &item, &sId, nullptr, nullptr, true, m_xScratchIter.get());
                //It is needed, without this the selection line will be truncated.
                m_xPrefBox->set_text(*m_xScratchIter, "", 1);
                m_xPrefBox->set_text(*m_xScratchIter, "", 2);
                m_xPrefBox->set_text(*m_xScratchIter, "", 3);
            }
        }
        else
        {
            // leaf node
            OUString sPropertyName = item;
            auto it = std::find_if(m_modifiedPrefBoxEntries.begin(), m_modifiedPrefBoxEntries.end(),
              [&sPath, &sPropertyName](const prefBoxEntry& rEntry) -> bool
              {
                  return rEntry.pUserData->sPropertyPath == sPath
                      && rEntry.sStatus == sPropertyName;
              }
            );

            OUString sType = aNode.getValueTypeName();
            OUStringBuffer sValue;

            if (it != m_modifiedPrefBoxEntries.end())
                sValue = it->sValue;
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
                        for( sal_Int8 j : seq )
                        {
                            OUString s = OUString::number(
                                static_cast<sal_uInt8>(j), 16 );
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
                            for( sal_Int8 k : seq[j] )
                            {
                                OUString s = OUString::number(
                                    static_cast<sal_uInt8>(k), 16 );
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
                            "path \"" << sPath << "\" member " << item
                                << " of unsupported type " << sType);
                    }
                    break;

                default:
                    SAL_WARN(
                        "cui.options",
                        "path \"" << sPath << "\" member " << item
                            << " of unsupported type " << sType);
                    break;
                }
            }

            //Short name
            int index = 0;
            for(int j = 1; j < lineage; ++j)
                index = sPath.indexOf("/", index + 1);

            InsertEntry(sPath, sPath.copy(index+1), item, sType, sValue.makeStringAndClear(), pParentEntry, !bLoadAll);
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

IMPL_LINK_NOARG( CuiAboutConfigTabPage, ResetBtnHdl_Impl, weld::Button&, void )
{
    Reset();
}

IMPL_LINK_NOARG(CuiAboutConfigTabPage, DoubleClickHdl_Impl, weld::TreeView&, bool)
{
    StandardHdl_Impl(*m_xEditBtn);
    return true;
}

IMPL_LINK_NOARG( CuiAboutConfigTabPage, StandardHdl_Impl, weld::Button&, void )
{
    if (!m_xPrefBox->get_selected(m_xScratchIter.get()))
        return;

    UserData *pUserData = reinterpret_cast<UserData*>(m_xPrefBox->get_id(*m_xScratchIter).toInt64());
    if (!(pUserData && pUserData->bIsPropertyPath))
        return;

    //if selection is a node
    OUString sPropertyName = m_xPrefBox->get_text(*m_xScratchIter, 1);
    OUString sPropertyType = m_xPrefBox->get_text(*m_xScratchIter, 2);
    OUString sPropertyValue = m_xPrefBox->get_text(*m_xScratchIter, 3);

    auto pProperty  = std::make_shared<Prop_Impl>( pUserData->sPropertyPath, sPropertyName, Any( sPropertyValue ) );
    bool bSaveChanges = false;

    bool bOpenDialog = true;
    OUString sDialogValue;

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

            CuiAboutConfigValueDialog aValueDialog(m_xDialog.get(), sDialogValue, limit);

            if (aValueDialog.run() == RET_OK )
            {
                OUString sNewValue = aValueDialog.getValue();
                bSaveChanges = true;
                if ( sPropertyType == "short")
                {
                    sal_Int16 nShort;
                    sal_Int32 nNumb = sNewValue.toInt32();

                    //if the value is 0 and length is not 1, there is something wrong
                    if( ( nNumb==0 && sNewValue.getLength()!=1 ) || nNumb > SAL_MAX_INT16 || nNumb < SAL_MIN_INT16)
                        throw uno::Exception("out of range short", nullptr);
                    nShort = static_cast<sal_Int16>(nNumb);
                    pProperty->Value <<= nShort;
                }
                else if( sPropertyType == "long" )
                {
                    sal_Int32 nLong = sNewValue.toInt32();
                    if( nLong==0 && sNewValue.getLength()!=1)
                        throw uno::Exception("out of range long", nullptr);
                    pProperty->Value <<= nLong;
                }
                else if( sPropertyType == "hyper")
                {
                    sal_Int64 nHyper = sNewValue.toInt64();
                    if( nHyper==0 && sNewValue.getLength()!=1)
                        throw uno::Exception("out of range hyper", nullptr);
                    pProperty->Value <<= nHyper;
                }
                else if( sPropertyType == "double")
                {
                    double nDoub = sNewValue.toDouble();
                    if( nDoub ==0 && sNewValue.getLength()!=1)
                        throw uno::Exception("out of range double", nullptr);
                    pProperty->Value <<= nDoub;
                }
                else if( sPropertyType == "float")
                {
                    float nFloat = sNewValue.toFloat();
                    if( nFloat ==0 && sNewValue.getLength()!=1)
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
                    std::vector< OUString > seqStr = commaStringToSequence( sNewValue );

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
                    std::vector< OUString > seqStrLong = commaStringToSequence( sNewValue );

                    uno::Sequence< sal_Int32 > seqLong( seqStrLong.size() );
                    for( size_t i = 0; i < seqStrLong.size(); ++i )
                    {
                        seqLong[i] = seqStrLong[i].toInt32();
                    }
                    pProperty->Value <<= seqLong;
                }
                else if( sPropertyType == "[]hyper" )
                {
                    std::vector< OUString > seqStrHyper = commaStringToSequence( sNewValue );
                    uno::Sequence< sal_Int64 > seqHyper( seqStrHyper.size() );
                    for( size_t i = 0; i < seqStrHyper.size(); ++i )
                    {
                        seqHyper[i] = seqStrHyper[i].toInt64();
                    }
                    pProperty->Value <<= seqHyper;
                }
                else if( sPropertyType == "[]double" )
                {
                    std::vector< OUString > seqStrDoub = commaStringToSequence( sNewValue );
                    uno::Sequence< double > seqDoub( seqStrDoub.size() );
                    for( size_t i = 0; i < seqStrDoub.size(); ++i )
                    {
                        seqDoub[i] = seqStrDoub[i].toDouble();
                    }
                    pProperty->Value <<= seqDoub;
                }
                else if( sPropertyType == "[]float" )
                {
                    std::vector< OUString > seqStrFloat = commaStringToSequence( sNewValue );
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
            m_xPrefBox->set_text(*m_xScratchIter, sDialogValue, 3);
            //update m_prefBoxEntries
            auto it = std::find_if(m_prefBoxEntries.begin(), m_prefBoxEntries.end(),
              [&pUserData, &sPropertyName](const prefBoxEntry& rEntry) -> bool
              {
                  return rEntry.pUserData->sPropertyPath == pUserData->sPropertyPath
                      && rEntry.sStatus == sPropertyName;
              }
            );
            if (it != m_prefBoxEntries.end())
            {
                it->sValue = sDialogValue;

                auto modifiedIt = std::find_if(
                            m_modifiedPrefBoxEntries.begin(), m_modifiedPrefBoxEntries.end(),
                            [&pUserData, &sPropertyName](const prefBoxEntry& rEntry) -> bool
                            {
                                return rEntry.pUserData->sPropertyPath == pUserData->sPropertyPath
                                    && rEntry.sStatus == sPropertyName;
                            }
                );

                if (modifiedIt != m_modifiedPrefBoxEntries.end())
                {
                    modifiedIt->sValue = sDialogValue;
                }
                else
                {
                    m_modifiedPrefBoxEntries.push_back(*it);
                }
            }
        }
    }
    catch( uno::Exception& )
    {
    }
}

IMPL_LINK_NOARG( CuiAboutConfigTabPage, SearchHdl_Impl, weld::Button&, void)
{
    weld::WaitObject aWait(m_xDialog.get());

    m_xPrefBox->hide();
    m_xPrefBox->clear();
    m_xPrefBox->freeze();

    if (m_bSorted)
        m_xPrefBox->make_unsorted();

    if (m_xSearchEdit->get_text().isEmpty())
    {
        m_xPrefBox->clear();
        Reference< XNameAccess > xConfigAccess = getConfigAccess( "/", false );
        FillItems( xConfigAccess );
    }
    else
    {
        m_options.searchString = m_xSearchEdit->get_text();
        utl::TextSearch textSearch( m_options );
        for (auto const& it : m_prefBoxEntries)
        {
            sal_Int32 endPos, startPos = 0;

            for(size_t i = 0; i < 5; ++i)
            {
                OUString scrTxt;

                if (i == 0)
                    scrTxt = it.pUserData->sPropertyPath;
                else if (i == 1)
                    scrTxt = it.sProp;
                else if (i == 2)
                    scrTxt = it.sStatus;
                else if (i == 3)
                    scrTxt = it.sType;
                else if (i == 4)
                    scrTxt = it.sValue;

                endPos = scrTxt.getLength();
                if (textSearch.SearchForward(scrTxt, &startPos, &endPos))
                {
                    InsertEntry(it);
                    break;
                }
            }
        }
    }

    m_xPrefBox->thaw();
    if (m_bSorted)
        m_xPrefBox->make_sorted();

    m_xPrefBox->all_foreach([this](weld::TreeIter& rEntry) {
        m_xPrefBox->expand_row(rEntry);
        return false;
    });
    m_xPrefBox->show();
}

void CuiAboutConfigTabPage::InsertEntry(const prefBoxEntry& rEntry)
{
    OUString sPathWithProperty = rEntry.pUserData->sPropertyPath;
    sal_Int32 index = sPathWithProperty.lastIndexOf(rEntry.sProp);
    OUString sPath = sPathWithProperty.copy(0, index);
    index = 0;
    std::unique_ptr<weld::TreeIter> xParentEntry(m_xPrefBox->make_iterator());
    std::unique_ptr<weld::TreeIter> xGrandParentEntry;

    do
    {
        int prevIndex = index;
        index = sPath.indexOf("/", index+1);
        // deal with no parent case (tdf#107811)
        if (index < 0)
        {
            OUString sId(OUString::number(reinterpret_cast<sal_Int64>(rEntry.pUserData)));
            m_xPrefBox->insert(nullptr, -1, &rEntry.sProp, &sId, nullptr, nullptr, false, m_xScratchIter.get());
            m_xPrefBox->set_text(*m_xScratchIter, rEntry.sStatus, 1);
            m_xPrefBox->set_text(*m_xScratchIter, rEntry.sType, 2);
            m_xPrefBox->set_text(*m_xScratchIter, rEntry.sValue, 3);
            return;
        }
        OUString sParentName = sPath.copy(prevIndex+1, index - prevIndex - 1);

        bool hasEntry = false;
        bool bStartOk;

        if (!xGrandParentEntry)
            bStartOk = m_xPrefBox->get_iter_first(*xParentEntry);
        else
        {
            m_xPrefBox->copy_iterator(*xGrandParentEntry, *xParentEntry);
            bStartOk = m_xPrefBox->iter_children(*xParentEntry);
        }

        if (bStartOk)
        {
            do
            {
                if (m_xPrefBox->get_text(*xParentEntry, 0) == sParentName)
                {
                    hasEntry = true;
                    break;
                }
            } while (m_xPrefBox->iter_next_sibling(*xParentEntry));
        }

        if (!hasEntry)
        {
            m_xPrefBox->insert(xGrandParentEntry.get(), -1, &sParentName, nullptr, nullptr, nullptr, false, xParentEntry.get());
            //It is needed, without this the selection line will be truncated.
            m_xPrefBox->set_text(*xParentEntry, "", 1);
            m_xPrefBox->set_text(*xParentEntry, "", 2);
            m_xPrefBox->set_text(*xParentEntry, "", 3);
        }

        xGrandParentEntry = m_xPrefBox->make_iterator(xParentEntry.get());
    } while(index < sPath.getLength() - 1);

    OUString sId(OUString::number(reinterpret_cast<sal_Int64>(rEntry.pUserData)));
    m_xPrefBox->insert(xParentEntry.get(), -1, &rEntry.sProp, &sId, nullptr, nullptr, false, m_xScratchIter.get());
    m_xPrefBox->set_text(*m_xScratchIter, rEntry.sStatus, 1);
    m_xPrefBox->set_text(*m_xScratchIter, rEntry.sType, 2);
    m_xPrefBox->set_text(*m_xScratchIter, rEntry.sValue, 3);
}

IMPL_LINK(CuiAboutConfigTabPage, ExpandingHdl_Impl, const weld::TreeIter&, rEntry, bool)
{
    if (m_xPrefBox->iter_has_child(rEntry))
        return true;
    UserData *pUserData = reinterpret_cast<UserData*>(m_xPrefBox->get_id(rEntry).toInt64());
    if (pUserData && !pUserData->bIsPropertyPath)
    {
        assert(pUserData->aXNameAccess.is());
        FillItems(pUserData->aXNameAccess, &rEntry, pUserData->aLineage);
    }
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
