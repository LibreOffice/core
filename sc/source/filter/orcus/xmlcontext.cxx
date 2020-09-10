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

#include <vcl/weld.hxx>
#include <ucbhelper/content.hxx>
#include <sal/log.hxx>
#include <osl/file.hxx>

#include <orcus/xml_structure_tree.hpp>
#include <orcus/xml_namespace.hpp>
#include <orcus/orcus_xml.hpp>
#include <orcus/sax_parser_base.hpp>
#include <orcus/stream.hpp>

#include <com/sun/star/io/XInputStream.hpp>
#include <comphelper/processfactory.hxx>

#include <string>
#include <sstream>

namespace com::sun::star::ucb { class XCommandEnvironment; }

#define BUFFER_SIZE 4096

using namespace com::sun::star;

namespace {

ScOrcusXMLTreeParam::EntryData& setUserDataToEntry(weld::TreeView& rControl,
    const weld::TreeIter& rEntry, ScOrcusXMLTreeParam::UserDataStoreType& rStore, ScOrcusXMLTreeParam::EntryType eType)
{
    rStore.push_back(std::make_unique<ScOrcusXMLTreeParam::EntryData>(eType));
    rControl.set_id(rEntry, OUString::number(reinterpret_cast<sal_Int64>(rStore.back().get())));
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
   weld::TreeView& rTreeCtrl, orcus::xml_structure_tree::walker& rWalker,
   const orcus::xml_structure_tree::entity_name& rElemName, bool bRepeat,
   const weld::TreeIter* pParent, ScOrcusXMLTreeParam& rParam)
{
    OUString sEntry(toString(rElemName, rWalker));
    std::unique_ptr<weld::TreeIter> xEntry(rTreeCtrl.make_iterator());
    rTreeCtrl.insert(pParent, -1, &sEntry, nullptr, nullptr, nullptr, false, xEntry.get());
    rTreeCtrl.set_image(*xEntry, rParam.maImgElementDefault, -1);

    ScOrcusXMLTreeParam::EntryData& rEntryData = setUserDataToEntry(rTreeCtrl,
        *xEntry, rParam.m_UserDataStore,
        bRepeat ? ScOrcusXMLTreeParam::ElementRepeat : ScOrcusXMLTreeParam::ElementDefault);

    setEntityNameToUserData(rEntryData, rElemName, rWalker);

    if (bRepeat)
    {
        // Recurring elements use different icon.
       rTreeCtrl.set_image(*xEntry, rParam.maImgElementRepeat, -1);
    }

    orcus::xml_structure_tree::entity_names_type aNames = rWalker.get_attributes();

    // Insert attributes.
    for (const orcus::xml_structure_tree::entity_name& rAttrName : aNames)
    {
        OUString sAttr(toString(rAttrName, rWalker));
        std::unique_ptr<weld::TreeIter> xAttr(rTreeCtrl.make_iterator());
        rTreeCtrl.insert(xEntry.get(), -1, &sAttr, nullptr, nullptr, nullptr, false, xAttr.get());

        ScOrcusXMLTreeParam::EntryData& rAttrData =
            setUserDataToEntry(rTreeCtrl, *xAttr, rParam.m_UserDataStore, ScOrcusXMLTreeParam::Attribute);
        setEntityNameToUserData(rAttrData, rAttrName, rWalker);

        rTreeCtrl.set_image(*xAttr, rParam.maImgAttribute, -1);
    }

    aNames = rWalker.get_children();

    // Non-leaf if it has child elements, leaf otherwise.
    rEntryData.mbLeafNode = aNames.empty();

    // Insert child elements recursively.
    for (const auto& rName : aNames)
    {
        orcus::xml_structure_tree::element aElem = rWalker.descend(rName);
        populateTree(rTreeCtrl, rWalker, rName, aElem.repeat, xEntry.get(), rParam);
        rWalker.ascend();
    }
}

class TreeUpdateSwitch
{
    weld::TreeView& mrTreeCtrl;
public:
    explicit TreeUpdateSwitch(weld::TreeView& rTreeCtrl) : mrTreeCtrl(rTreeCtrl)
    {
        mrTreeCtrl.freeze();
    }

    ~TreeUpdateSwitch()
    {
        mrTreeCtrl.thaw();
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

void ScOrcusXMLContextImpl::loadXMLStructure(weld::TreeView& rTreeCtrl, ScOrcusXMLTreeParam& rParam)
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
        rTreeCtrl.clear();

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

    rTreeCtrl.all_foreach([&rTreeCtrl](weld::TreeIter& rEntry){
        rTreeCtrl.expand_row(rEntry);
        return false;
    });
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

    OUString aSysPath;
    if (osl::FileBase::getSystemPathFromFileURL(maPath, aSysPath) != osl::FileBase::E_None)
        return;

    OString aOSysPath = OUStringToOString(aSysPath, RTL_TEXTENCODING_UTF8);
    const char* path = aOSysPath.getStr();

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

            std::for_each(rLink.maFieldPaths.begin(), rLink.maFieldPaths.end(),
                [&filter](const OString& rFieldPath)
                {
                    filter.append_field_link(rFieldPath.getStr(), orcus::pstring());
                }
            );

            std::for_each(rLink.maRowGroups.begin(), rLink.maRowGroups.end(),
                [&filter] (const OString& rRowGroup)
                {
                    filter.set_range_row_group(rRowGroup.getStr());
                }
            );

            filter.commit_range();
        }

        orcus::file_content content(path);
        filter.read_stream(content.data(), content.size());

        aFactory.finalize();
    }
    catch (const std::exception&)
    {
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
