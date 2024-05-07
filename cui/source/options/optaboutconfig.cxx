/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "optaboutconfig.hxx"
#include <o3tl/safeint.hxx>
#include <o3tl/string_view.hxx>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/configuration/ReadWriteAccess.hpp>
#include <com/sun/star/configuration/XDocumentation.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XHierarchicalName.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/uno/TypeClass.hpp>
#include <com/sun/star/util/InvalidStateException.hpp>
#include <com/sun/star/util/SearchAlgorithms2.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <comphelper/diagnose_ex.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <cppu/unotype.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <unotools/textsearch.hxx>
#include <utility>
#include <vcl/event.hxx>

#include <dialmgr.hxx>
#include <strings.hrc>

#include <algorithm>
#include <memory>
#include <vector>

using namespace ::com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::container;

struct Prop_Impl
{
    OUString Name;
    OUString Property;
    Any Value;

    Prop_Impl(OUString sName, OUString sProperty, Any aValue)
        : Name(std::move(sName))
        , Property(std::move(sProperty))
        , Value(std::move(aValue))
    {
    }
};

struct UserData
{
    bool bIsPropertyPath;
    bool bIsReadOnly;
    bool bWasModified;
    OUString sPropertyPath;
    Any aPropertyValue;
    OUString sTooltip;
    int aLineage;
    Reference<XNameAccess> aXNameAccess;

    explicit UserData(OUString aPropertyPath, Any aPropValue, OUString aTooltip, bool isReadOnly,
                      bool wasModified)
        : bIsPropertyPath(true)
        , bIsReadOnly(isReadOnly)
        , bWasModified(wasModified)
        , sPropertyPath(std::move(aPropertyPath))
        , aPropertyValue(aPropValue)
        , sTooltip(std::move(aTooltip))
        , aLineage(0)
    {
    }

    explicit UserData(Reference<XNameAccess> const& rXNameAccess, int rIndex)
        : bIsPropertyPath(false)
        , bIsReadOnly(false)
        , bWasModified(false)
        , aLineage(rIndex)
        , aXNameAccess(rXNameAccess)
    {
    }
};

CuiAboutConfigTabPage::CuiAboutConfigTabPage(weld::Window* pParent)
    : GenericDialogController(pParent, u"cui/ui/aboutconfigdialog.ui"_ustr, u"AboutConfig"_ustr)
    , m_xResetBtn(m_xBuilder->weld_button(u"reset"_ustr))
    , m_xEditBtn(m_xBuilder->weld_button(u"edit"_ustr))
    , m_xSearchBtn(m_xBuilder->weld_button(u"searchButton"_ustr))
    , m_xModifiedCheckBtn(m_xBuilder->weld_check_button(u"modifiedButton"_ustr))
    , m_xSearchEdit(m_xBuilder->weld_entry(u"searchEntry"_ustr))
    , m_xPrefBox(m_xBuilder->weld_tree_view(u"preferences"_ustr))
    , m_xScratchIter(m_xPrefBox->make_iterator())
    , m_bSorted(false)
{
    m_xPrefBox->set_size_request(m_xPrefBox->get_approximate_digit_width() * 100,
                                 m_xPrefBox->get_height_rows(23));
    m_xPrefBox->connect_column_clicked(LINK(this, CuiAboutConfigTabPage, HeaderBarClick));

    m_xEditBtn->connect_clicked(LINK(this, CuiAboutConfigTabPage, StandardHdl_Impl));
    m_xResetBtn->connect_clicked(LINK(this, CuiAboutConfigTabPage, ResetBtnHdl_Impl));
    m_xPrefBox->connect_row_activated(LINK(this, CuiAboutConfigTabPage, DoubleClickHdl_Impl));
    m_xPrefBox->connect_expanding(LINK(this, CuiAboutConfigTabPage, ExpandingHdl_Impl));
    m_xSearchBtn->connect_clicked(LINK(this, CuiAboutConfigTabPage, SearchHdl_Impl));
    m_xModifiedCheckBtn->connect_toggled(LINK(this, CuiAboutConfigTabPage, ModifiedHdl_Impl));

    m_options.AlgorithmType2 = util::SearchAlgorithms2::ABSOLUTE;
    m_options.transliterateFlags |= TransliterationFlags::IGNORE_CASE;
    m_options.searchFlag
        |= (util::SearchFlags::REG_NOT_BEGINOFLINE | util::SearchFlags::REG_NOT_ENDOFLINE);

    float fWidth = m_xPrefBox->get_approximate_digit_width();
    std::vector<int> aWidths{ o3tl::narrowing<int>(fWidth * 65), o3tl::narrowing<int>(fWidth * 20),
                              o3tl::narrowing<int>(fWidth * 8) };
    m_xPrefBox->set_column_fixed_widths(aWidths);

    m_xPrefBox->connect_query_tooltip(LINK(this, CuiAboutConfigTabPage, QueryTooltip));
}

IMPL_LINK(CuiAboutConfigTabPage, QueryTooltip, const weld::TreeIter&, rIter, OUString)
{
    UserData* pUserData = weld::fromId<UserData*>(m_xPrefBox->get_id(rIter));
    OUStringBuffer ret;
    if (pUserData && pUserData->bIsReadOnly)
    {
        ret.append(CuiResId(RID_CUISTR_OPT_READONLY));
    }
    if (pUserData && !pUserData->sTooltip.isEmpty())
    {
        if (pUserData->bIsReadOnly)
            ret.append("\n\n");
        ret.append(pUserData->sTooltip);
    }

    return ret.makeStringAndClear();
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

IMPL_STATIC_LINK_NOARG(CuiAboutConfigTabPage, ValidNameHdl, SvxNameDialog&, bool)
{
    // Allow empty value
    return true;
}

CuiAboutConfigTabPage::~CuiAboutConfigTabPage() {}

void CuiAboutConfigTabPage::InsertEntry(const OUString& rPropertyPath, Any aPropertyValue,
                                        const OUString& rProp, const OUString& rStatus,
                                        const OUString& rType, const OUString& rValue,
                                        const OUString& rTooltip,
                                        const weld::TreeIter* pParentEntry, bool bInsertToPrefBox,
                                        bool bIsReadOnly, bool bWasModified)
{
    bool bOnlyModified = m_xModifiedCheckBtn->get_active();
    if (bOnlyModified && !bWasModified)
        return;

    m_vectorUserData.push_back(std::make_unique<UserData>(rPropertyPath, aPropertyValue, rTooltip,
                                                          bIsReadOnly, bWasModified));
    if (bInsertToPrefBox)
    {
        OUString sId(weld::toId(m_vectorUserData.back().get()));
        m_xPrefBox->insert(pParentEntry, -1, &rProp, &sId, nullptr, nullptr, false,
                           m_xScratchIter.get());
        m_xPrefBox->set_text(*m_xScratchIter, rStatus, 1);
        m_xPrefBox->set_text(*m_xScratchIter, rType, 2);
        m_xPrefBox->set_text(*m_xScratchIter, rValue, 3);
        m_xPrefBox->set_text_emphasis(*m_xScratchIter, bWasModified, -1);
        m_xPrefBox->set_sensitive(*m_xScratchIter, !bIsReadOnly, -1);
    }
    else
    {
        m_prefBoxEntries.push_back(
            { rProp, rStatus, rType, rValue, m_vectorUserData.back().get() });
    }
}

void CuiAboutConfigTabPage::InputChanged()
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
        Reference<XNameAccess> xConfigAccess = getConfigAccess(u"/"_ustr, false);
        FillItems(xConfigAccess);
    }
    else
    {
        m_options.searchString = m_xSearchEdit->get_text();
        utl::TextSearch textSearch(m_options);
        for (auto const& it : m_prefBoxEntries)
        {
            sal_Int32 endPos, startPos = 0;

            for (size_t i = 0; i < 5; ++i)
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
    Reference<XNameAccess> xConfigAccess = getConfigAccess(u"/"_ustr, false);
    //Load all XNameAccess to m_prefBoxEntries
    FillItems(xConfigAccess, nullptr, 0, true);
    //Load xConfigAccess' children to m_prefBox
    FillItems(xConfigAccess);
    m_xPrefBox->thaw();
}

void CuiAboutConfigTabPage::FillItemSet()
{
    std::vector<std::shared_ptr<Prop_Impl>>::iterator pIter;
    for (pIter = m_vectorOfModified.begin(); pIter != m_vectorOfModified.end(); ++pIter)
    {
        Reference<XNameAccess> xUpdateAccess = getConfigAccess((*pIter)->Name, true);
        Reference<XNameReplace> xNameReplace(xUpdateAccess, UNO_QUERY_THROW);

        xNameReplace->replaceByName((*pIter)->Property, (*pIter)->Value);

        Reference<util::XChangesBatch> xChangesBatch(xUpdateAccess, UNO_QUERY_THROW);
        xChangesBatch->commitChanges();
    }
}

namespace
{
OUString lcl_StringListToString(const uno::Sequence<OUString>& seq)
{
    OUStringBuffer sBuffer;
    for (sal_Int32 i = 0; i != seq.getLength(); ++i)
    {
        if (i != 0)
            sBuffer.append(",");
        sBuffer.append(seq[i]);
    }
    return sBuffer.makeStringAndClear();
}

OUString lcl_IntListToString(const uno::Sequence<sal_Int16>& seq)
{
    OUStringBuffer sBuffer;
    for (sal_Int32 i = 0; i != seq.getLength(); ++i)
    {
        if (i != 0)
            sBuffer.append(",");
        sBuffer.append(OUString::number(seq[i]));
    }
    return sBuffer.makeStringAndClear();
}

OUString lcl_IntListToString(const uno::Sequence<sal_Int32>& seq)
{
    OUStringBuffer sBuffer;
    for (sal_Int32 i = 0; i != seq.getLength(); ++i)
    {
        if (i != 0)
            sBuffer.append(",");
        sBuffer.append(OUString::number(seq[i]));
    }
    return sBuffer.makeStringAndClear();
}

OUString lcl_IntListToString(const uno::Sequence<sal_Int64>& seq)
{
    OUStringBuffer sBuffer;
    for (sal_Int32 i = 0; i != seq.getLength(); ++i)
    {
        if (i != 0)
            sBuffer.append(",");
        sBuffer.append(OUString::number(seq[i]));
    }
    return sBuffer.makeStringAndClear();
}

OUString lcl_DoubleListToString(const uno::Sequence<double>& seq)
{
    OUStringBuffer sBuffer;
    for (sal_Int32 i = 0; i != seq.getLength(); ++i)
    {
        if (i != 0)
            sBuffer.append(",");
        sBuffer.append(OUString::number(seq[i]));
    }
    return sBuffer.makeStringAndClear();
}
}

void CuiAboutConfigTabPage::FillItems(const Reference<XNameAccess>& xNameAccess,
                                      const weld::TreeIter* pParentEntry, int lineage,
                                      bool bLoadAll)
{
    OUString sPath
        = Reference<XHierarchicalName>(xNameAccess, uno::UNO_QUERY_THROW)->getHierarchicalName();
    const uno::Sequence<OUString> seqItems = xNameAccess->getElementNames();
    for (const OUString& item : seqItems)
    {
        Any aNode = xNameAccess->getByName(item);

        bool bNotLeaf = false;

        Reference<XNameAccess> xNextNameAccess;
        try
        {
            xNextNameAccess.set(aNode, uno::UNO_QUERY);
            bNotLeaf = xNextNameAccess.is();
        }
        catch (const RuntimeException&)
        {
            TOOLS_WARN_EXCEPTION("cui.options", "CuiAboutConfigTabPage");
        }

        if (bNotLeaf)
        {
            if (bLoadAll)
                FillItems(xNextNameAccess, nullptr, lineage + 1, true);
            else
            {
                // not leaf node
                m_vectorUserData.push_back(
                    std::make_unique<UserData>(xNextNameAccess, lineage + 1));
                OUString sId(weld::toId(m_vectorUserData.back().get()));

                m_xPrefBox->insert(pParentEntry, -1, &item, &sId, nullptr, nullptr, true,
                                   m_xScratchIter.get());
                // Necessary, without this the selection line will be truncated.
                m_xPrefBox->set_text(*m_xScratchIter, u""_ustr, 1);
                m_xPrefBox->set_text(*m_xScratchIter, u""_ustr, 2);
                m_xPrefBox->set_text(*m_xScratchIter, u""_ustr, 3);
                m_xPrefBox->set_sensitive(*m_xScratchIter, true);
            }
        }
        else
        {
            // leaf node
            OUString sPropertyName = item;
            auto it = std::find_if(m_modifiedPrefBoxEntries.begin(), m_modifiedPrefBoxEntries.end(),
                                   [&sPath, &sPropertyName](const prefBoxEntry& rEntry) -> bool {
                                       return rEntry.pUserData->sPropertyPath == sPath
                                              && rEntry.sStatus == sPropertyName;
                                   });

            css::uno::Reference<css::configuration::XReadWriteAccess> m_xReadWriteAccess;
            m_xReadWriteAccess = css::configuration::ReadWriteAccess::create(
                ::comphelper::getProcessComponentContext(), u"*"_ustr);
            beans::Property aProperty;
            bool bReadOnly = false;
            OUString sFullPath(sPath + "/" + sPropertyName);
            try
            {
                aProperty = m_xReadWriteAccess->getPropertyByHierarchicalName(sFullPath);
                bReadOnly = (aProperty.Attributes & beans::PropertyAttribute::READONLY) != 0;
            }
            catch (css::beans::UnknownPropertyException)
            {
                SAL_WARN("cui.options", "unknown property: " << sFullPath);
            }

            OUString sTooltip;
            OUString sType;
            bool bWasModified = false;
            css::uno::Type aType = cppu::UnoType<void>::get();
            OUString sDynamicType = aNode.getValueTypeName();
            try
            {
                Reference<configuration::XDocumentation> xDocumentation(xNameAccess,
                                                                        UNO_QUERY_THROW);
                sTooltip
                    = xDocumentation->getDescriptionByHierarchicalName(sPath + "/" + sPropertyName);
                aType = xDocumentation->getTypeByHierarchicalName(sFullPath);
                bWasModified = xDocumentation->getModifiedByHierarchicalName(sFullPath);
            }
            catch (css::container::NoSuchElementException)
            {
            }
            catch (css::util::InvalidStateException)
            {
            }

            OUStringBuffer sValue;

            // Fall back to dynamic type when this is empty
            if (aType == cppu::UnoType<void>::get() && sDynamicType != "void")
            {
                if (sDynamicType == "boolean")
                    aType = cppu::UnoType<sal_Bool>::get();
                else if (sDynamicType == "short")
                    aType = cppu::UnoType<sal_Int16>::get();
                else if (sDynamicType == "long")
                    aType = cppu::UnoType<sal_Int32>::get();
                else if (sDynamicType == "hyper")
                    aType = cppu::UnoType<sal_Int64>::get();
                else if (sDynamicType == "double")
                    aType = cppu::UnoType<double>::get();
                else if (sDynamicType == "string")
                    aType = cppu::UnoType<OUString>::get();
                else if (sDynamicType == "[]byte")
                    aType = cppu::UnoType<css::uno::Sequence<sal_Int8>>::get();
                else if (sDynamicType == "[]boolean")
                    aType = cppu::UnoType<css::uno::Sequence<sal_Bool>>::get();
                else if (sDynamicType == "[]short")
                    aType = cppu::UnoType<css::uno::Sequence<sal_Int16>>::get();
                else if (sDynamicType == "[]long")
                    aType = cppu::UnoType<css::uno::Sequence<sal_Int32>>::get();
                else if (sDynamicType == "[]hyper")
                    aType = cppu::UnoType<css::uno::Sequence<sal_Int64>>::get();
                else if (sDynamicType == "[]double")
                    aType = cppu::UnoType<css::uno::Sequence<double>>::get();
                else if (sDynamicType == "[]string")
                    aType = cppu::UnoType<css::uno::Sequence<OUString>>::get();
                else if (sDynamicType == "[][]byte")
                    aType = cppu::UnoType<css::uno::Sequence<css::uno::Sequence<sal_Int8>>>::get();
            }

            if (it != m_modifiedPrefBoxEntries.end())
                sValue = it->sValue;
            else
            {
                bool bHasValue = sDynamicType != "void";
                if (aType == cppu::UnoType<sal_Bool>::get())
                {
                    if (bHasValue)
                        sValue = OUString::boolean(aNode.get<bool>());
                    sType = "boolean";
                }
                else if (aType == cppu::UnoType<sal_Int16>::get())
                {
                    if (bHasValue)
                        sValue = OUString::number(aNode.get<sal_Int16>());
                    sType = "short";
                }
                else if (aType == cppu::UnoType<sal_Int32>::get())
                {
                    if (bHasValue)
                        sValue = OUString::number(aNode.get<sal_Int32>());
                    sType = "int";
                }
                else if (aType == cppu::UnoType<sal_Int64>::get())
                {
                    if (bHasValue)
                        sValue = OUString::number(aNode.get<sal_Int64>());
                    sType = "long";
                }
                else if (aType == cppu::UnoType<double>::get())
                {
                    if (bHasValue)
                        sValue = OUString::number(aNode.get<double>());
                    sType = "double";
                }
                else if (aType == cppu::UnoType<OUString>::get())
                {
                    if (bHasValue)
                        sValue = aNode.get<OUString>();
                    sType = "string";
                }
                else if (aType == cppu::UnoType<css::uno::Sequence<sal_Int8>>::get())
                {
                    if (bHasValue)
                    {
                        const uno::Sequence<sal_Int8> seq = aNode.get<uno::Sequence<sal_Int8>>();
                        for (sal_Int8 j : seq)
                        {
                            OUString s = OUString::number(static_cast<sal_uInt8>(j), 16);
                            if (s.getLength() == 1)
                            {
                                sValue.append("0");
                            }
                            sValue.append(s.toAsciiUpperCase());
                        }
                    }
                    sType = "hexBinary";
                }
                else if (aType == cppu::UnoType<css::uno::Sequence<sal_Bool>>::get())
                {
                    if (bHasValue)
                    {
                        uno::Sequence<sal_Bool> seq = aNode.get<uno::Sequence<sal_Bool>>();
                        for (sal_Int32 j = 0; j != seq.getLength(); ++j)
                        {
                            if (j != 0)
                            {
                                sValue.append(",");
                            }
                            sValue.append(OUString::boolean(seq[j]));
                        }
                    }
                    sType = "boolean-list";
                }
                else if (aType == cppu::UnoType<css::uno::Sequence<sal_Int16>>::get())
                {
                    if (bHasValue)
                    {
                        uno::Sequence<sal_Int16> seq = aNode.get<uno::Sequence<sal_Int16>>();
                        for (sal_Int32 j = 0; j != seq.getLength(); ++j)
                        {
                            if (j != 0)
                            {
                                sValue.append(",");
                            }
                            sValue.append(static_cast<sal_Int32>(seq[j]));
                        }
                    }
                    sType = "short-list";
                }
                else if (aType == cppu::UnoType<css::uno::Sequence<sal_Int32>>::get())
                {
                    if (bHasValue)
                    {
                        uno::Sequence<sal_Int32> seq = aNode.get<uno::Sequence<sal_Int32>>();
                        for (sal_Int32 j = 0; j != seq.getLength(); ++j)
                        {
                            if (j != 0)
                            {
                                sValue.append(",");
                            }
                            sValue.append(seq[j]);
                        }
                    }
                    sType = "int-list";
                }
                else if (aType == cppu::UnoType<css::uno::Sequence<sal_Int64>>::get())
                {
                    if (bHasValue)
                    {
                        uno::Sequence<sal_Int64> seq = aNode.get<uno::Sequence<sal_Int64>>();
                        for (sal_Int32 j = 0; j != seq.getLength(); ++j)
                        {
                            if (j != 0)
                            {
                                sValue.append(",");
                            }
                            sValue.append(seq[j]);
                        }
                    }
                    sType = "long-list";
                }
                else if (aType == cppu::UnoType<css::uno::Sequence<double>>::get())
                {
                    if (bHasValue)
                    {
                        uno::Sequence<double> seq = aNode.get<uno::Sequence<double>>();
                        for (sal_Int32 j = 0; j != seq.getLength(); ++j)
                        {
                            if (j != 0)
                            {
                                sValue.append(",");
                            }
                            sValue.append(seq[j]);
                        }
                    }
                    sType = "double-list";
                }
                else if (aType == cppu::UnoType<css::uno::Sequence<OUString>>::get())
                {
                    if (bHasValue)
                        sValue = lcl_StringListToString(aNode.get<uno::Sequence<OUString>>());
                    sType = "string-list";
                }
                else if (aType
                         == cppu::UnoType<css::uno::Sequence<css::uno::Sequence<sal_Int8>>>::get())
                {
                    if (bHasValue)
                    {
                        const uno::Sequence<uno::Sequence<sal_Int8>> seq
                            = aNode.get<uno::Sequence<uno::Sequence<sal_Int8>>>();
                        for (sal_Int32 j = 0; j != seq.getLength(); ++j)
                        {
                            if (j != 0)
                            {
                                sValue.append(",");
                            }
                            for (sal_Int8 k : seq[j])
                            {
                                OUString s = OUString::number(static_cast<sal_uInt8>(k), 16);
                                if (s.getLength() == 1)
                                {
                                    sValue.append("0");
                                }
                                sValue.append(s.toAsciiUpperCase());
                            }
                        }
                    }
                    sType = "hexBinary-list";
                }
                else
                {
                    SAL_INFO("cui.options", "path \"" << sPath << "\" member " << item
                                                      << " of unsupported type " << sType);
                    continue;
                }
            }

            //Short name
            int index = 0;
            for (int j = 1; j < lineage; ++j)
                index = sPath.indexOf("/", index + 1);

            InsertEntry(sPath, aNode, sPath.copy(index + 1), item, sType,
                        sValue.makeStringAndClear(), sTooltip, pParentEntry, !bLoadAll, bReadOnly,
                        bWasModified);
        }
    }
}

Reference<XNameAccess> CuiAboutConfigTabPage::getConfigAccess(const OUString& sNodePath,
                                                              bool bUpdate)
{
    uno::Reference<uno::XComponentContext> xContext(::comphelper::getProcessComponentContext());

    uno::Reference<lang::XMultiServiceFactory> xConfigProvider(
        css::configuration::theDefaultProvider::get(xContext));

    beans::NamedValue aProperty;
    aProperty.Name = "nodepath";
    aProperty.Value <<= sNodePath;

    uno::Sequence<uno::Any> aArgumentList{ uno::Any(aProperty) };

    OUString sAccessString;

    if (bUpdate)
        sAccessString = "com.sun.star.configuration.ConfigurationUpdateAccess";
    else
        sAccessString = "com.sun.star.configuration.ConfigurationAccess";

    uno::Reference<container::XNameAccess> xNameAccess(
        xConfigProvider->createInstanceWithArguments(sAccessString, aArgumentList),
        uno::UNO_QUERY_THROW);

    return xNameAccess;
}

void CuiAboutConfigTabPage::AddToModifiedVector(const std::shared_ptr<Prop_Impl>& rProp)
{
    bool isModifiedBefore = false;
    //Check if value modified before
    for (std::shared_ptr<Prop_Impl>& nInd : m_vectorOfModified)
    {
        if (rProp->Name == nInd->Name && rProp->Property == nInd->Property)
        {
            //property modified before. Assign reference to the modified value
            //do your changes on this object. They will be saved later.
            nInd = rProp;
            isModifiedBefore = true;
            break;
        }
    }

    if (!isModifiedBefore)
        m_vectorOfModified.push_back(rProp);
    //property is not modified before
}

std::vector<OUString>
CuiAboutConfigTabPage::commaStringToSequence(std::u16string_view rCommaSepString)
{
    std::vector<OUString> tempVector;

    sal_Int32 index = 0;
    do
    {
        OUString word(o3tl::getToken(rCommaSepString, 0, u',', index));
        word = word.trim();
        if (!word.isEmpty())
            tempVector.push_back(word);
    } while (index >= 0);
    return tempVector;
}

IMPL_LINK_NOARG(CuiAboutConfigTabPage, ResetBtnHdl_Impl, weld::Button&, void) { Reset(); }

IMPL_LINK_NOARG(CuiAboutConfigTabPage, DoubleClickHdl_Impl, weld::TreeView&, bool)
{
    StandardHdl_Impl(*m_xEditBtn);
    return true;
}

IMPL_LINK_NOARG(CuiAboutConfigTabPage, StandardHdl_Impl, weld::Button&, void)
{
    if (!m_xPrefBox->get_selected(m_xScratchIter.get()))
        return;

    UserData* pUserData = weld::fromId<UserData*>(m_xPrefBox->get_id(*m_xScratchIter));
    if (!pUserData || !pUserData->bIsPropertyPath || pUserData->bIsReadOnly)
        return;

    //if selection is a node
    OUString sPropertyName = m_xPrefBox->get_text(*m_xScratchIter, 1);
    OUString sPropertyType = m_xPrefBox->get_text(*m_xScratchIter, 2);
    OUString sPropertyValue = m_xPrefBox->get_text(*m_xScratchIter, 3);

    auto pProperty
        = std::make_shared<Prop_Impl>(pUserData->sPropertyPath, sPropertyName, Any(sPropertyValue));
    bool bSaveChanges = false;

    bool bOpenDialog = true;
    OUString sDialogValue;

    if (sPropertyType == "boolean")
    {
        bool bValue;
        if (sPropertyValue == "true")
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
    else
    {
        sDialogValue = sPropertyValue;
        bOpenDialog = true;
    }

    try
    {
        if (bOpenDialog)
        {
            if (sPropertyType == "short" || sPropertyType == "int" || sPropertyType == "long")
            {
                sal_Int64 nMin = sPropertyType == "short"
                                     ? SAL_MIN_INT16
                                     : sPropertyType == "int" ? SAL_MIN_INT32 : SAL_MIN_INT64;
                sal_Int64 nMax = sPropertyType == "short"
                                     ? SAL_MAX_INT16
                                     : sPropertyType == "int" ? SAL_MAX_INT32 : SAL_MAX_INT64;
                SvxNumberDialog aNumberDialog(m_xDialog.get(), sPropertyName,
                                              sDialogValue.toInt64(), nMin, nMax);
                if (aNumberDialog.run() == RET_OK)
                {
                    sal_Int64 nNewValue = aNumberDialog.GetNumber();
                    if (sPropertyType == "short")
                    {
                        pProperty->Value <<= static_cast<sal_Int16>(nNewValue);
                    }
                    else if (sPropertyType == "int")
                    {
                        pProperty->Value <<= static_cast<sal_Int32>(nNewValue);
                    }
                    else if (sPropertyType == "long")
                    {
                        pProperty->Value <<= nNewValue;
                    }
                    bSaveChanges = true;
                    sDialogValue = OUString::number(nNewValue);
                }
            }
            else if (sPropertyType == "double")
            {
                SvxDecimalNumberDialog aNumberDialog(m_xDialog.get(), sPropertyName,
                                                     sDialogValue.toDouble());
                if (aNumberDialog.run() == RET_OK)
                {
                    double fNewValue = aNumberDialog.GetNumber();
                    pProperty->Value <<= fNewValue;
                    bSaveChanges = true;
                    sDialogValue = OUString::number(fNewValue);
                }
            }
            else if (sPropertyType == "string")
            {
                SvxNameDialog aNameDialog(m_xDialog.get(), sDialogValue, sPropertyName);
                aNameDialog.SetCheckNameHdl(LINK(this, CuiAboutConfigTabPage, ValidNameHdl));
                if (aNameDialog.run() == RET_OK)
                {
                    sDialogValue = aNameDialog.GetName();
                    pProperty->Value <<= sDialogValue;
                    bSaveChanges = true;
                }
            }
            else if (sPropertyType == "short-list")
            {
                SvxListDialog aListDialog(m_xDialog.get());
                aListDialog.SetEntries(commaStringToSequence(sDialogValue));
                aListDialog.SetMode(ListMode::Int16);
                if (aListDialog.run() == RET_OK)
                {
                    std::vector<OUString> seqStr = aListDialog.GetEntries();
                    uno::Sequence<sal_Int16> seqShort(seqStr.size());
                    std::transform(
                        seqStr.begin(), seqStr.end(), seqShort.getArray(),
                        [](const auto& str) { return static_cast<sal_Int16>(str.toInt32()); });
                    pProperty->Value <<= seqShort;
                    sDialogValue = lcl_IntListToString(seqShort);
                    bSaveChanges = true;
                }
            }
            else if (sPropertyType == "int-list")
            {
                SvxListDialog aListDialog(m_xDialog.get());
                aListDialog.SetEntries(commaStringToSequence(sDialogValue));
                aListDialog.SetMode(ListMode::Int32);
                if (aListDialog.run() == RET_OK)
                {
                    std::vector<OUString> seqStr = aListDialog.GetEntries();
                    uno::Sequence<sal_Int32> seq(seqStr.size());
                    std::transform(
                        seqStr.begin(), seqStr.end(), seq.getArray(),
                        [](const auto& str) { return static_cast<sal_Int32>(str.toInt32()); });
                    pProperty->Value <<= seq;
                    sDialogValue = lcl_IntListToString(seq);
                    bSaveChanges = true;
                }
            }
            else if (sPropertyType == "long-list")
            {
                SvxListDialog aListDialog(m_xDialog.get());
                aListDialog.SetEntries(commaStringToSequence(sDialogValue));
                aListDialog.SetMode(ListMode::Int64);
                if (aListDialog.run() == RET_OK)
                {
                    std::vector<OUString> seqStr = aListDialog.GetEntries();
                    uno::Sequence<sal_Int64> seq(seqStr.size());
                    std::transform(
                        seqStr.begin(), seqStr.end(), seq.getArray(),
                        [](const auto& str) { return static_cast<sal_Int64>(str.toInt32()); });
                    pProperty->Value <<= seq;
                    sDialogValue = lcl_IntListToString(seq);
                    bSaveChanges = true;
                }
            }
            else if (sPropertyType == "double-list")
            {
                SvxListDialog aListDialog(m_xDialog.get());
                aListDialog.SetEntries(commaStringToSequence(sDialogValue));
                aListDialog.SetMode(ListMode::Double);
                if (aListDialog.run() == RET_OK)
                {
                    std::vector<OUString> seqStr = aListDialog.GetEntries();
                    uno::Sequence<double> seq(seqStr.size());
                    std::transform(
                        seqStr.begin(), seqStr.end(), seq.getArray(),
                        [](const auto& str) { return static_cast<double>(str.toDouble()); });
                    pProperty->Value <<= seq;
                    sDialogValue = lcl_DoubleListToString(seq);
                    bSaveChanges = true;
                }
            }
            else if (sPropertyType == "string-list")
            {
                SvxListDialog aListDialog(m_xDialog.get());
                uno::Sequence<OUString> aList
                    = pUserData->aPropertyValue.get<uno::Sequence<OUString>>();
                aListDialog.SetEntries(
                    comphelper::sequenceToContainer<std::vector<OUString>>(aList));
                aListDialog.SetMode(ListMode::String);
                if (aListDialog.run() == RET_OK)
                {
                    auto seq = comphelper::containerToSequence(aListDialog.GetEntries());
                    sDialogValue = lcl_StringListToString(seq);
                    pProperty->Value <<= seq;
                    bSaveChanges = true;
                }
            }
            else //unknown
                throw uno::Exception("unknown property type " + sPropertyType, nullptr);
        }

        if (bSaveChanges)
        {
            AddToModifiedVector(pProperty);
            pUserData->aPropertyValue = pProperty->Value;

            //update listbox value.
            m_xPrefBox->set_text(*m_xScratchIter, sPropertyType, 2);
            m_xPrefBox->set_text(*m_xScratchIter, sDialogValue, 3);
            m_xPrefBox->set_text_emphasis(*m_xScratchIter, true, -1);
            //update m_prefBoxEntries
            auto it = std::find_if(
                m_prefBoxEntries.begin(), m_prefBoxEntries.end(),
                [&pUserData, &sPropertyName](const prefBoxEntry& rEntry) -> bool {
                    return rEntry.pUserData->sPropertyPath == pUserData->sPropertyPath
                           && rEntry.sStatus == sPropertyName;
                });
            if (it != m_prefBoxEntries.end())
            {
                it->sValue = sDialogValue;
                it->pUserData->bWasModified = true;

                auto modifiedIt = std::find_if(
                    m_modifiedPrefBoxEntries.begin(), m_modifiedPrefBoxEntries.end(),
                    [&pUserData, &sPropertyName](const prefBoxEntry& rEntry) -> bool {
                        return rEntry.pUserData->sPropertyPath == pUserData->sPropertyPath
                               && rEntry.sStatus == sPropertyName;
                    });

                if (modifiedIt != m_modifiedPrefBoxEntries.end())
                {
                    modifiedIt->sValue = sDialogValue;
                    modifiedIt->pUserData->bWasModified = true;
                }
                else
                {
                    m_modifiedPrefBoxEntries.push_back(*it);
                }
            }
        }
    }
    catch (uno::Exception&)
    {
    }
}

IMPL_LINK_NOARG(CuiAboutConfigTabPage, SearchHdl_Impl, weld::Button&, void) { InputChanged(); }

IMPL_LINK_NOARG(CuiAboutConfigTabPage, ModifiedHdl_Impl, weld::Toggleable&, void)
{
    InputChanged();
}

void CuiAboutConfigTabPage::InsertEntry(const prefBoxEntry& rEntry)
{
    bool bOnlyModified = m_xModifiedCheckBtn->get_active();
    if (bOnlyModified && !rEntry.pUserData->bWasModified)
        return;

    OUString sPathWithProperty = rEntry.pUserData->sPropertyPath;
    sal_Int32 index = sPathWithProperty.lastIndexOf(rEntry.sProp);
    OUString sPath = sPathWithProperty.copy(0, index);
    index = 0;
    std::unique_ptr<weld::TreeIter> xParentEntry(m_xPrefBox->make_iterator());
    std::unique_ptr<weld::TreeIter> xGrandParentEntry;

    do
    {
        int prevIndex = index;
        index = sPath.indexOf("/", index + 1);
        // deal with no parent case (tdf#107811)
        if (index < 0)
        {
            OUString sId(weld::toId(rEntry.pUserData));
            m_xPrefBox->insert(nullptr, -1, &rEntry.sProp, &sId, nullptr, nullptr, false,
                               m_xScratchIter.get());
            m_xPrefBox->set_text(*m_xScratchIter, rEntry.sStatus, 1);
            m_xPrefBox->set_text(*m_xScratchIter, rEntry.sType, 2);
            m_xPrefBox->set_text(*m_xScratchIter, rEntry.sValue, 3);
            m_xPrefBox->set_text_emphasis(*m_xScratchIter, rEntry.pUserData->bWasModified, -1);
            m_xPrefBox->set_sensitive(*m_xScratchIter, !rEntry.pUserData->bIsReadOnly);
            return;
        }
        OUString sParentName = sPath.copy(prevIndex + 1, index - prevIndex - 1);

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
            m_xPrefBox->insert(xGrandParentEntry.get(), -1, &sParentName, nullptr, nullptr, nullptr,
                               false, xParentEntry.get());
            //It is needed, without this the selection line will be truncated.
            m_xPrefBox->set_text(*xParentEntry, u""_ustr, 1);
            m_xPrefBox->set_text(*xParentEntry, u""_ustr, 2);
            m_xPrefBox->set_text(*xParentEntry, u""_ustr, 3);
            m_xPrefBox->set_sensitive(*xParentEntry, true);
        }

        xGrandParentEntry = m_xPrefBox->make_iterator(xParentEntry.get());
    } while (index < sPath.getLength() - 1);

    OUString sId(weld::toId(rEntry.pUserData));
    m_xPrefBox->insert(xParentEntry.get(), -1, &rEntry.sProp, &sId, nullptr, nullptr, false,
                       m_xScratchIter.get());
    m_xPrefBox->set_text(*m_xScratchIter, rEntry.sStatus, 1);
    m_xPrefBox->set_text(*m_xScratchIter, rEntry.sType, 2);
    m_xPrefBox->set_text(*m_xScratchIter, rEntry.sValue, 3);
    m_xPrefBox->set_text_emphasis(*m_xScratchIter, rEntry.pUserData->bWasModified, -1);
    m_xPrefBox->set_sensitive(*m_xScratchIter, !rEntry.pUserData->bIsReadOnly);
}

IMPL_LINK(CuiAboutConfigTabPage, ExpandingHdl_Impl, const weld::TreeIter&, rEntry, bool)
{
    if (m_xPrefBox->iter_has_child(rEntry))
        return true;
    UserData* pUserData = weld::fromId<UserData*>(m_xPrefBox->get_id(rEntry));
    if (pUserData && !pUserData->bIsPropertyPath)
    {
        assert(pUserData->aXNameAccess.is());
        FillItems(pUserData->aXNameAccess, &rEntry, pUserData->aLineage);
    }
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
