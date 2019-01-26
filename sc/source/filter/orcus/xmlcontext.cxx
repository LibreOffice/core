/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcusfiltersimpl.hxx>
#include <orcusinterface.hxx>
#include <orcusxml.hxx>
#include <document.hxx>
#include <tokenarray.hxx>

#include <vcl/treelistbox.hxx>
#include <vcl/treelistentry.hxx>
#include <ucbhelper/content.hxx>
#include <sal/log.hxx>

#include <orcus/xml_structure_tree.hpp>
#include <orcus/xml_namespace.hpp>
#include <orcus/orcus_xml.hpp>
#include <orcus/sax_parser_base.hpp>
#include <orcus/stream.hpp>

#include <com/sun/star/io/XInputStream.hpp>
#include <comphelper/processfactory.hxx>

#include <string>
#include <sstream>

namespace com { namespace sun { namespace star { namespace ucb { class XCommandEnvironment; } } } }

#define BUFFER_SIZE 4096

using namespace com::sun::star;

namespace {

ScOrcusXMLTreeParam::EntryData& setUserDataToEntry(
    SvTreeListEntry& rEntry, ScOrcusXMLTreeParam::UserDataStoreType& rStore, ScOrcusXMLTreeParam::EntryType eType)
{
    rStore.push_back(std::make_unique<ScOrcusXMLTreeParam::EntryData>(eType));
    rEntry.SetUserData(rStore.back().get());
    return *rStore.back();
}

void setEntityNameToUserData(
    ScOrcusXMLTreeParam::EntryData& rEntryData,
    const orcus::xml_structure_tree::entity_name& entity, const orcus::xml_structure_tree::walker& walker)
{
    rEntryData.mnNamespaceID = walker.get_xmlns_index(entity.ns);
}

OUString toString(const orcus::xml_structure_tree::entity_name& entity, const orcus::xml_structure_tree::walker& walker)
{
    OUStringBuffer aBuf;
    if (entity.ns)
    {
        // Namespace exists.  Use the short version of the xml namespace name for display.
        std::string aShortName = walker.get_xmlns_short_name(entity.ns);
        aBuf.appendAscii(aShortName.c_str());
        aBuf.append(':');
    }
    aBuf.append(OUString(entity.name.get(), entity.name.size(), RTL_TEXTENCODING_UTF8));
    return aBuf.makeStringAndClear();
}

void populateTree(
   SvTreeListBox& rTreeCtrl, orcus::xml_structure_tree::walker& rWalker,
   const orcus::xml_structure_tree::entity_name& rElemName, bool bRepeat,
   SvTreeListEntry* pParent, ScOrcusXMLTreeParam& rParam)
{
    SvTreeListEntry* pEntry = rTreeCtrl.InsertEntry(toString(rElemName, rWalker), pParent);
    if (!pEntry)
        // Can this ever happen!?
        return;

    ScOrcusXMLTreeParam::EntryData& rEntryData = setUserDataToEntry(
        *pEntry, rParam.m_UserDataStore,
        bRepeat ? ScOrcusXMLTreeParam::ElementRepeat : ScOrcusXMLTreeParam::ElementDefault);

    setEntityNameToUserData(rEntryData, rElemName, rWalker);

    if (bRepeat)
    {
        // Recurring elements use different icon.
        rTreeCtrl.SetExpandedEntryBmp(pEntry, rParam.maImgElementRepeat);
        rTreeCtrl.SetCollapsedEntryBmp(pEntry, rParam.maImgElementRepeat);
    }

    if (pParent)
        rTreeCtrl.Expand(pParent);

    orcus::xml_structure_tree::entity_names_type aNames;

    // Insert attributes.
    rWalker.get_attributes(aNames);
    for (const orcus::xml_structure_tree::entity_name& rAttrName : aNames)
    {
        SvTreeListEntry* pAttr = rTreeCtrl.InsertEntry(toString(rAttrName, rWalker), pEntry);

        if (!pAttr)
            continue;

        ScOrcusXMLTreeParam::EntryData& rAttrData =
            setUserDataToEntry(*pAttr, rParam.m_UserDataStore, ScOrcusXMLTreeParam::Attribute);
        setEntityNameToUserData(rAttrData, rAttrName, rWalker);

        rTreeCtrl.SetExpandedEntryBmp(pAttr, rParam.maImgAttribute);
        rTreeCtrl.SetCollapsedEntryBmp(pAttr, rParam.maImgAttribute);
    }
    rTreeCtrl.Expand(pEntry);

    rWalker.get_children(aNames);

    // Non-leaf if it has child elements, leaf otherwise.
    rEntryData.mbLeafNode = aNames.empty();

    // Insert child elements recursively.
    for (const auto& rName : aNames)
    {
        orcus::xml_structure_tree::element aElem = rWalker.descend(rName);
        populateTree(rTreeCtrl, rWalker, rName, aElem.repeat, pEntry, rParam);
        rWalker.ascend();
    }
}

class TreeUpdateSwitch
{
    SvTreeListBox& mrTreeCtrl;
public:
    explicit TreeUpdateSwitch(SvTreeListBox& rTreeCtrl) : mrTreeCtrl(rTreeCtrl)
    {
        mrTreeCtrl.SetUpdateMode(false);
    }

    ~TreeUpdateSwitch()
    {
        mrTreeCtrl.SetUpdateMode(true);
    }
};

class InsertFieldPath
{
    orcus::orcus_xml& mrFilter;
public:
    explicit InsertFieldPath(orcus::orcus_xml& rFilter) : mrFilter(rFilter) {}
    void operator() (const OString& rPath)
    {
        mrFilter.append_field_link(rPath.getStr());
    }
};

void loadContentFromURL(const OUString& rURL, std::string& rStrm)
{
    ucbhelper::Content aContent(
        rURL, uno::Reference<ucb::XCommandEnvironment>(), comphelper::getProcessComponentContext());
    uno::Reference<io::XInputStream> xStrm = aContent.openStream();

    std::ostringstream aStrmBuf;
    uno::Sequence<sal_Int8> aBytes;
    size_t nBytesRead = 0;
    do
    {
        nBytesRead = xStrm->readBytes(aBytes, BUFFER_SIZE);
        const sal_Int8* p = aBytes.getConstArray();
        aStrmBuf << std::string(p, p + nBytesRead);
    }
    while (nBytesRead == BUFFER_SIZE);

    rStrm = aStrmBuf.str();
}

}

ScOrcusXMLContextImpl::ScOrcusXMLContextImpl(ScDocument& rDoc, const OUString& rPath) :
    ScOrcusXMLContext(), mrDoc(rDoc), maPath(rPath) {}

ScOrcusXMLContextImpl::~ScOrcusXMLContextImpl() {}

void ScOrcusXMLContextImpl::loadXMLStructure(SvTreeListBox& rTreeCtrl, ScOrcusXMLTreeParam& rParam)
{
    rParam.m_UserDataStore.clear();

    std::string aStrm;
    loadContentFromURL(maPath, aStrm);

    if (aStrm.empty())
        return;

    orcus::xmlns_context cxt = maNsRepo.create_context();
    orcus::xml_structure_tree aXmlTree(cxt);
    try
    {
        aXmlTree.parse(&aStrm[0], aStrm.size());

        TreeUpdateSwitch aSwitch(rTreeCtrl);
        rTreeCtrl.Clear();
        rTreeCtrl.SetDefaultCollapsedEntryBmp(rParam.maImgElementDefault);
        rTreeCtrl.SetDefaultExpandedEntryBmp(rParam.maImgElementDefault);

        orcus::xml_structure_tree::walker aWalker = aXmlTree.get_walker();

        // Root element.
        orcus::xml_structure_tree::element aElem = aWalker.root();
        populateTree(rTreeCtrl, aWalker, aElem.name, aElem.repeat, nullptr, rParam);
    }
    catch (const orcus::sax::malformed_xml_error& e)
    {
        SAL_WARN("sc.orcus", "Malformed XML error: " << e.what());
    }
    catch (const std::exception& e)
    {
        SAL_WARN("sc.orcus", "parsing failed with an unknown error " << e.what());
    }
}

namespace {

class SetNamespaceAlias
{
    orcus::orcus_xml& mrFilter;
    orcus::xmlns_repository& mrNsRepo;
public:
    SetNamespaceAlias(orcus::orcus_xml& filter, orcus::xmlns_repository& repo) :
        mrFilter(filter), mrNsRepo(repo) {}

    void operator() (size_t index)
    {
        orcus::xmlns_id_t nsid = mrNsRepo.get_identifier(index);
        if (nsid == orcus::XMLNS_UNKNOWN_ID)
            return;

        std::string alias = mrNsRepo.get_short_name(index);
        mrFilter.set_namespace_alias(alias.c_str(), nsid);
    }
};

}

void ScOrcusXMLContextImpl::importXML(const ScOrcusImportXMLParam& rParam)
{
    ScOrcusFactory aFactory(mrDoc, true);
    OString aSysPath = ScOrcusFiltersImpl::toSystemPath(maPath);
    const char* path = aSysPath.getStr();
    try
    {
        orcus::orcus_xml filter(maNsRepo, &aFactory, nullptr);

        // Define all used namespaces.
        std::for_each(rParam.maNamespaces.begin(), rParam.maNamespaces.end(), SetNamespaceAlias(filter, maNsRepo));

        // Set cell links.
        for (const ScOrcusImportXMLParam::CellLink& rLink : rParam.maCellLinks)
        {
            OUString aTabName;
            mrDoc.GetName(rLink.maPos.Tab(), aTabName);
            filter.set_cell_link(
                rLink.maPath.getStr(),
                OUStringToOString(aTabName, RTL_TEXTENCODING_UTF8).getStr(),
                rLink.maPos.Row(), rLink.maPos.Col());
        }

        // Set range links.
        for (const ScOrcusImportXMLParam::RangeLink& rLink : rParam.maRangeLinks)
        {
            OUString aTabName;
            mrDoc.GetName(rLink.maPos.Tab(), aTabName);
            filter.start_range(
                OUStringToOString(aTabName, RTL_TEXTENCODING_UTF8).getStr(),
                rLink.maPos.Row(), rLink.maPos.Col());

            std::for_each(rLink.maFieldPaths.begin(), rLink.maFieldPaths.end(), InsertFieldPath(filter));

            filter.commit_range();
        }

        std::string content = orcus::load_file_content(path);
        filter.read_stream(content.data(), content.size());

        aFactory.finalize();
    }
    catch (const std::exception&)
    {
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
