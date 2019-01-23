/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <memory>

#include <revisionfragment.hxx>
#include <oox/core/relations.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <oox/core/fastparser.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <svl/sharedstringpool.hxx>
#include <sax/tools/converter.hxx>
#include <editeng/editobj.hxx>

#include <chgtrack.hxx>
#include <document.hxx>
#include <compiler.hxx>
#include <editutil.hxx>
#include <formulacell.hxx>
#include <chgviset.hxx>
#include <richstringcontext.hxx>
#include <tokenarray.hxx>

#include <com/sun/star/util/DateTime.hpp>

using namespace com::sun::star;

namespace oox { namespace xls {

namespace {

enum RevisionType
{
    REV_UNKNOWN = 0,
    REV_CELLCHANGE,
    REV_INSERTROW
};

/**
 * For nc (new cell) or oc (old cell) elements under rcc (cell content
 * revision).
 */
class RCCCellValueContext : public WorkbookContextBase
{
    sal_Int32 const mnSheetIndex;
    ScAddress& mrPos;
    ScCellValue& mrCellValue;
    sal_Int32 mnType;

    RichStringRef mxRichString;

public:
    RCCCellValueContext(
        RevisionLogFragment& rParent, sal_Int32 nSheetIndex, ScAddress& rPos, ScCellValue& rCellValue ) :
        WorkbookContextBase(rParent),
        mnSheetIndex(nSheetIndex),
        mrPos(rPos),
        mrCellValue(rCellValue),
        mnType(-1) {}

protected:
    virtual oox::core::ContextHandlerRef onCreateContext(
        sal_Int32 nElement, const AttributeList& /*rAttribs*/ ) override
    {
        if (nElement == XLS_TOKEN(is))
        {
            mxRichString.reset(new RichString(*this));
            return new RichStringContext(*this, mxRichString);
        }

        return this;
    }

    virtual void onStartElement( const AttributeList& rAttribs ) override
    {
        switch (getCurrentElement())
        {
            case XLS_TOKEN(nc):
            case XLS_TOKEN(oc):
                importCell(rAttribs);
            break;
            default:
                ;
        }
    }

    virtual void onCharacters( const OUString& rChars ) override
    {
        switch (getCurrentElement())
        {
            case XLS_TOKEN(v):
            {
                if (mnType == XML_n || mnType == XML_b)
                    mrCellValue.set(rChars.toDouble());
            }
            break;
            case XLS_TOKEN(t):
            {
                if (mnType == XML_inlineStr)
                {
                    ScDocument& rDoc = getScDocument();
                    svl::SharedStringPool& rPool = rDoc.GetSharedStringPool();
                    mrCellValue.set(rPool.intern(rChars));
                }
            }
            break;
            case XLS_TOKEN(f):
            {
                // formula string
                ScDocument& rDoc = getScDocument();
                ScCompiler aComp(&rDoc, mrPos, formula::FormulaGrammar::GRAM_OOXML);
                std::unique_ptr<ScTokenArray> pArray = aComp.CompileString(rChars);
                if (!pArray)
                    break;

                mrCellValue.set(new ScFormulaCell(&rDoc, mrPos, std::move(pArray)));
            }
            break;
            default:
                ;
        }
    }

    virtual void onEndElement() override
    {
        switch (getCurrentElement())
        {
            case XLS_TOKEN(nc):
            case XLS_TOKEN(oc):
            {
                if (mrCellValue.isEmpty() && mxRichString)
                {
                    // The value is a rich text string.
                    ScDocument& rDoc = getScDocument();
                    std::unique_ptr<EditTextObject> pTextObj = mxRichString->convert(rDoc.GetEditEngine(), nullptr);
                    if (pTextObj)
                    {
                        svl::SharedStringPool& rPool = rDoc.GetSharedStringPool();
                        pTextObj->NormalizeString(rPool);
                        mrCellValue.set(pTextObj.release());
                    }
                }
            }
            break;
            default:
                ;
        }
    }

private:
    void importCell( const AttributeList& rAttribs )
    {
        mnType = rAttribs.getToken(XML_t, XML_n);
        OUString aRefStr = rAttribs.getString(XML_r, OUString());
        if (!aRefStr.isEmpty())
        {
            mrPos.Parse(aRefStr, nullptr, formula::FormulaGrammar::CONV_XL_OOX);
            if (mnSheetIndex != -1)
                mrPos.SetTab(mnSheetIndex-1);
        }
    }
};

struct RevisionMetadata
{
    OUString maUserName;
    DateTime maDateTime;

    RevisionMetadata() : maDateTime(DateTime::EMPTY) {}
    RevisionMetadata( const RevisionMetadata& r ) :
        maUserName(r.maUserName), maDateTime(r.maDateTime) {}
};

}

typedef std::map<OUString, RevisionMetadata> RevDataType;

struct RevisionHeadersFragment::Impl
{
    RevDataType maRevData;

    Impl() {}
};

RevisionHeadersFragment::RevisionHeadersFragment(
    const WorkbookHelper& rHelper, const OUString& rFragmentPath ) :
    WorkbookFragmentBase(rHelper, rFragmentPath),
    mpImpl(new Impl) {}

RevisionHeadersFragment::~RevisionHeadersFragment()
{
}

oox::core::ContextHandlerRef RevisionHeadersFragment::onCreateContext(
    sal_Int32 /*nElement*/, const AttributeList& /*rAttribs*/ )
{
    return this;
}

void RevisionHeadersFragment::onStartElement( const AttributeList& rAttribs )
{
    switch (getCurrentElement())
    {
        case XLS_TOKEN(headers):
        break;
        case XLS_TOKEN(header):
            importHeader(rAttribs);
        break;
        case XLS_TOKEN(sheetIdMap):
        break;
        case XLS_TOKEN(sheetId):
        break;
        default:
            ;
    }
}

void RevisionHeadersFragment::onCharacters( const OUString& /*rChars*/ ) {}

void RevisionHeadersFragment::onEndElement()
{
    switch (getCurrentElement())
    {
        case XLS_TOKEN(headers):
        break;
        case XLS_TOKEN(header):
        break;
        case XLS_TOKEN(sheetIdMap):
        break;
        case XLS_TOKEN(sheetId):
        break;
        default:
            ;
    }
}

void RevisionHeadersFragment::finalizeImport()
{
    ScDocument& rDoc = getScDocument();
    std::unique_ptr<ScChangeTrack> pCT(new ScChangeTrack(&rDoc));
    OUString aSelfUser = pCT->GetUser(); // owner of this document.
    pCT->SetUseFixDateTime(true);

    const oox::core::Relations& rRels = getRelations();
    for (const auto& [rRelId, rData] : mpImpl->maRevData)
    {
        OUString aPath = rRels.getFragmentPathFromRelId(rRelId);
        if (aPath.isEmpty())
            continue;

        // Parse each revision log fragment.
        pCT->SetUser(rData.maUserName);
        pCT->SetFixDateTimeLocal(rData.maDateTime);
        std::unique_ptr<oox::core::FastParser> xParser(oox::core::XmlFilterBase::createParser());
        rtl::Reference<oox::core::FragmentHandler> xFragment(new RevisionLogFragment(*this, aPath, *pCT));
        importOoxFragment(xFragment, *xParser);
    }

    pCT->SetUser(aSelfUser); // set the default user to the document owner.
    pCT->SetUseFixDateTime(false);
    rDoc.SetChangeTrack(std::move(pCT));

    // Turn on visibility of tracked changes.
    ScChangeViewSettings aSettings;
    aSettings.SetShowChanges(true);
    rDoc.SetChangeViewSettings(aSettings);
}

void RevisionHeadersFragment::importHeader( const AttributeList& rAttribs )
{
    OUString aRId = rAttribs.getString(R_TOKEN(id), OUString());
    if (aRId.isEmpty())
        // All bets are off if we don't have a relation ID.
        return;

    RevisionMetadata aMetadata;
    OUString aDateTimeStr = rAttribs.getString(XML_dateTime, OUString());
    if (!aDateTimeStr.isEmpty())
    {
        util::DateTime aDateTime;
        sax::Converter::parseDateTime(aDateTime, aDateTimeStr);
        Date aDate(aDateTime);
        tools::Time aTime(aDateTime);
        aMetadata.maDateTime.SetDate(aDate.GetDate());
        aMetadata.maDateTime.SetTime(aTime.GetTime());
    }

    aMetadata.maUserName = rAttribs.getString(XML_userName, OUString());

    mpImpl->maRevData.emplace(aRId, aMetadata);
}

struct RevisionLogFragment::Impl
{
    ScChangeTrack& mrChangeTrack;

    sal_Int32 mnSheetIndex;

    RevisionType meType;

    // rcc
    ScAddress maOldCellPos;
    ScAddress maNewCellPos;
    ScCellValue maOldCellValue;
    ScCellValue maNewCellValue;

    // rrc
    ScRange maRange;

    bool mbEndOfList;

    explicit Impl( ScChangeTrack& rChangeTrack ) :
        mrChangeTrack(rChangeTrack),
        mnSheetIndex(-1),
        meType(REV_UNKNOWN),
        mbEndOfList(false) {}
};

RevisionLogFragment::RevisionLogFragment(
    const WorkbookHelper& rHelper, const OUString& rFragmentPath, ScChangeTrack& rChangeTrack ) :
    WorkbookFragmentBase(rHelper, rFragmentPath),
    mpImpl(new Impl(rChangeTrack)) {}

RevisionLogFragment::~RevisionLogFragment()
{
}

oox::core::ContextHandlerRef RevisionLogFragment::onCreateContext(
    sal_Int32 nElement, const AttributeList& /*rAttribs*/ )
{
    switch (nElement)
    {
        case XLS_TOKEN(nc):
            return new RCCCellValueContext(*this, mpImpl->mnSheetIndex, mpImpl->maNewCellPos, mpImpl->maNewCellValue);
        case XLS_TOKEN(oc):
            return new RCCCellValueContext(*this, mpImpl->mnSheetIndex, mpImpl->maOldCellPos, mpImpl->maOldCellValue);
        default:
            ;
    }
    return this;
}

void RevisionLogFragment::onStartElement( const AttributeList& rAttribs )
{
    switch (getCurrentElement())
    {
        case XLS_TOKEN(rcc):
            mpImpl->maNewCellPos.SetInvalid();
            mpImpl->maOldCellPos.SetInvalid();
            mpImpl->maNewCellValue.clear();
            mpImpl->maOldCellValue.clear();
            importRcc(rAttribs);
        break;
        case XLS_TOKEN(rrc):
            importRrc(rAttribs);
        break;
        default:
            ;
    }
}

void RevisionLogFragment::onCharacters( const OUString& /*rChars*/ ) {}

void RevisionLogFragment::onEndElement()
{
    switch (getCurrentElement())
    {
        case XLS_TOKEN(rcc):
        case XLS_TOKEN(rrc):
            pushRevision();
        break;
        default:
            ;
    }
}

void RevisionLogFragment::finalizeImport() {}

void RevisionLogFragment::importCommon( const AttributeList& rAttribs )
{
    mpImpl->mnSheetIndex = rAttribs.getInteger(XML_sId, -1);
}

void RevisionLogFragment::importRcc( const AttributeList& rAttribs )
{
    importCommon(rAttribs);

    mpImpl->meType = REV_CELLCHANGE;
}

void RevisionLogFragment::importRrc( const AttributeList& rAttribs )
{
    importCommon(rAttribs);

    if (mpImpl->mnSheetIndex == -1)
        // invalid sheet index, or sheet index not given.
        return;

    mpImpl->meType = REV_UNKNOWN;
    sal_Int32 nAction = rAttribs.getToken(XML_action, -1);
    if (nAction == -1)
        return;

    OUString aRefStr = rAttribs.getString(XML_ref, OUString());
    mpImpl->maRange.Parse(aRefStr, &getScDocument(), formula::FormulaGrammar::CONV_XL_OOX);
    if (!mpImpl->maRange.IsValid())
        return;

    switch (nAction)
    {
        case XML_insertRow:
            mpImpl->meType = REV_INSERTROW;
            mpImpl->maRange.aEnd.SetCol(MAXCOL);
            mpImpl->maRange.aStart.SetTab(mpImpl->mnSheetIndex-1);
            mpImpl->maRange.aEnd.SetTab(mpImpl->mnSheetIndex-1);
        break;
        default:
            // Unknown action type.  Ignore it.
            return;
    }

    mpImpl->mbEndOfList = rAttribs.getBool(XML_eol, false);
}

void RevisionLogFragment::pushRevision()
{
    switch (mpImpl->meType)
    {
        case REV_CELLCHANGE:
            mpImpl->mrChangeTrack.AppendContentOnTheFly(
                mpImpl->maNewCellPos, mpImpl->maOldCellValue, mpImpl->maNewCellValue);
        break;
        case REV_INSERTROW:
            mpImpl->mrChangeTrack.AppendInsert(mpImpl->maRange, mpImpl->mbEndOfList);
        break;
        default:
            ;
    }
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
