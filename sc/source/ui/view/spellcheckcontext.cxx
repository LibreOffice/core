/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <spellcheckcontext.hxx>
#include <svl/sharedstring.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/unolingu.hxx>

#include <scitems.hxx>
#include <document.hxx>
#include <cellvalue.hxx>
#include <editutil.hxx>
#include <dpobject.hxx>

#include <com/sun/star/linguistic2/XSpellChecker1.hpp>

#include <o3tl/hash_combine.hxx>

#include <unordered_map>

using namespace css;

using sc::SpellCheckContext;

class SpellCheckContext::SpellCheckCache
{
    struct CellPos
    {
        struct Hash
        {
            size_t operator() (const CellPos& rPos) const
            {
                std::size_t seed = 0;
                o3tl::hash_combine(seed, rPos.mnCol);
                o3tl::hash_combine(seed, rPos.mnRow);
                return seed;
            }
        };

        SCCOL mnCol;
        SCROW mnRow;

        CellPos(SCCOL nCol, SCROW nRow) : mnCol(nCol), mnRow(nRow) {}

        bool operator== (const CellPos& r) const
        {
            return mnCol == r.mnCol && mnRow == r.mnRow;
        }

    };

    struct LangSharedString
    {
        struct Hash
        {
            size_t operator() (const LangSharedString& rKey) const
            {
                std::size_t seed = 0;
                o3tl::hash_combine(seed, rKey.meLang.get());
                o3tl::hash_combine(seed, rKey.mpString);
                return seed;
            }
        };

        LanguageType meLang;
        const rtl_uString* mpString;

        LangSharedString(LanguageType eLang, const ScRefCellValue& rCell)
            : meLang(eLang)
            , mpString(rCell.getSharedString()->getData())
        {
        }

        bool operator== (const LangSharedString& r) const
        {
            return meLang == r.meLang && mpString == r.mpString;
        }
    };

    typedef std::unordered_map<CellPos, std::unique_ptr<MisspellRangesVec>, CellPos::Hash> CellMapType;
    typedef std::unordered_map<LangSharedString, std::unique_ptr<MisspellRangesVec>, LangSharedString::Hash> SharedStringMapType;

    SharedStringMapType  maStringMisspells;
    CellMapType          maEditTextMisspells;

public:

    SpellCheckCache()
    {
    }

    bool query(SCCOL nCol, SCROW nRow, LanguageType eLang,
               const ScRefCellValue& rCell, MisspellRangesVec*& rpRanges) const
    {
        CellType eType = rCell.getType();
        if (eType == CELLTYPE_STRING)
        {
            SharedStringMapType::const_iterator it = maStringMisspells.find(LangSharedString(eLang, rCell));
            if (it == maStringMisspells.end())
                return false; // Not available

            rpRanges = it->second.get();
            return true;
        }

        if (eType == CELLTYPE_EDIT)
        {
            CellMapType::const_iterator it = maEditTextMisspells.find(CellPos(nCol, nRow));
            if (it == maEditTextMisspells.end())
                return false; // Not available

            rpRanges = it->second.get();
            return true;
        }

        rpRanges = nullptr;
        return true;
    }

    void set(SCCOL nCol, SCROW nRow, LanguageType eLang,
             const ScRefCellValue& rCell, std::unique_ptr<MisspellRangesVec> pRanges)
    {
        CellType eType = rCell.getType();
        if (eType == CELLTYPE_STRING)
        {
            maStringMisspells.insert_or_assign(LangSharedString(eLang, rCell), std::move(pRanges));
        }
        else if (eType == CELLTYPE_EDIT)
            maEditTextMisspells.insert_or_assign(CellPos(nCol, nRow), std::move(pRanges));
    }

    void clear()
    {
        maStringMisspells.clear();
        maEditTextMisspells.clear();
    }

    void clearEditTextMap()
    {
        maEditTextMisspells.clear();
    }
};

struct SpellCheckContext::SpellCheckStatus
{
    bool mbModified;

    SpellCheckStatus() : mbModified(false) {};

    DECL_LINK( EventHdl, EditStatus&, void );
};

IMPL_LINK(SpellCheckContext::SpellCheckStatus, EventHdl, EditStatus&, rStatus, void)
{
    EditStatusFlags nStatus = rStatus.GetStatusWord();
    if (nStatus & EditStatusFlags::WRONGWORDCHANGED)
        mbModified = true;
}

struct SpellCheckContext::SpellCheckResult
{
    SCCOL mnCol;
    SCROW mnRow;
    MisspellRangeResult maRanges;

    SpellCheckResult() : mnCol(-1), mnRow(-1) {}

    void set(SCCOL nCol, SCROW nRow, const MisspellRangeResult& rMisspells)
    {
        mnCol = nCol;
        mnRow = nRow;
        maRanges = rMisspells;
    }

    MisspellRangeResult query(SCCOL nCol, SCROW nRow) const
    {
        assert(mnCol == nCol);
        assert(mnRow == nRow);
        (void)nCol;
        (void)nRow;
        return maRanges;
    }

    void clear()
    {
        mnCol = -1;
        mnRow = -1;
        maRanges = {};
    }
};

SpellCheckContext::SpellCheckContext(ScDocument* pDocument, SCTAB nTab) :
    pDoc(pDocument),
    mnTab(nTab),
    meLanguage(ScGlobal::GetEditDefaultLanguage())
{
    // defer init of engine and cache till the first query/set
}

SpellCheckContext::~SpellCheckContext()
{
}

void SpellCheckContext::dispose()
{
    mpEngine.reset();
    mpCache.reset();
    pDoc = nullptr;
}

void SpellCheckContext::setTabNo(SCTAB nTab)
{
    if (mnTab == nTab)
        return;
    mnTab = nTab;
    reset();
}

bool SpellCheckContext::isMisspelled(SCCOL nCol, SCROW nRow) const
{
    const_cast<SpellCheckContext*>(this)->ensureResults(nCol, nRow);
    return mpResult->query(nCol, nRow).mpRanges;
}

sc::MisspellRangeResult SpellCheckContext::getMisspellRanges(
    SCCOL nCol, SCROW nRow ) const
{
    const_cast<SpellCheckContext*>(this)->ensureResults(nCol, nRow);
    return mpResult->query(nCol, nRow);
}

void SpellCheckContext::setMisspellRanges(
    SCCOL nCol, SCROW nRow, const sc::MisspellRangeResult& rRangeResult )
{
    if (!mpEngine || !mpCache)
        reset();

    ScRefCellValue aCell(*pDoc, ScAddress(nCol, nRow, mnTab));
    CellType eType = aCell.getType();

    if (eType != CELLTYPE_STRING && eType != CELLTYPE_EDIT)
        return;

    const MisspellRangesVec* pRanges = rRangeResult.mpRanges;
    std::unique_ptr<MisspellRangesVec> pMisspells(pRanges ? new MisspellRangesVec(*pRanges) : nullptr);
    mpCache->set(nCol, nRow, rRangeResult.meCellLang, aCell, std::move(pMisspells));
}

void SpellCheckContext::reset()
{
    meLanguage = ScGlobal::GetEditDefaultLanguage();
    resetCache();
    mpEngine.reset();
    mpStatus.reset();
}

void SpellCheckContext::resetForContentChange()
{
    resetCache(true /* bContentChangeOnly */);
}

void SpellCheckContext::ensureResults(SCCOL nCol, SCROW nRow)
{
    if (!mpEngine || !mpCache ||
        ScGlobal::GetEditDefaultLanguage() != meLanguage)
    {
        reset();
        setup();
    }

    // perhaps compute the pivot rangelist once in some pivot-table change handler ?
    if (pDoc->HasPivotTable())
    {
        if (ScDPCollection* pDPs = pDoc->GetDPCollection())
        {
            ScRangeList aPivotRanges = pDPs->GetAllTableRanges(mnTab);
            if (aPivotRanges.Contains(ScRange(ScAddress(nCol, nRow, mnTab)))) // Don't spell check within pivot tables
            {
                mpResult->set(nCol, nRow, {});
                return;
            }
        }
    }

    ScRefCellValue aCell(*pDoc, ScAddress(nCol, nRow, mnTab));
    CellType eType = aCell.getType();

    if (eType != CELLTYPE_STRING && eType != CELLTYPE_EDIT)
    {
        // No spell-check required.
        mpResult->set(nCol, nRow, {});
        return;
    }


    // Cell content is either shared-string or EditTextObject

    // For spell-checking, we currently only use the primary
    // language; not CJK nor CTL.
    const ScPatternAttr* pPattern = pDoc->GetPattern(nCol, nRow, mnTab);
    LanguageType eCellLang = pPattern->GetItem(ATTR_FONT_LANGUAGE).GetValue();

    if (eCellLang == LANGUAGE_SYSTEM)
        eCellLang = meLanguage;   // never use SYSTEM for spelling

    if (eCellLang == LANGUAGE_NONE)
    {
        mpResult->set(nCol, nRow, {}); // No need to spell check this cell.
        return;
    }

    MisspellRangesVec* pCacheRanges = nullptr;
    bool bFound = mpCache->query(nCol, nRow, eCellLang, aCell, pCacheRanges);
    if (bFound)
    {
        // Cache hit.
        mpResult->set(nCol, nRow, MisspellRangeResult(pCacheRanges, eCellLang));
        return;
    }

    // Cache miss, the cell needs spell-check..
    if (eType == CELLTYPE_STRING)
        mpEngine->SetText(aCell.getSharedString()->getString());
    else
        mpEngine->SetText(*aCell.getEditText());

    // it has to happen after we set text
    mpEngine->SetDefaultItem(SvxLanguageItem(eCellLang, EE_CHAR_LANGUAGE));

    mpStatus->mbModified = false;
    mpEngine->CompleteOnlineSpelling();
    std::unique_ptr<MisspellRangesVec> pRanges;
    if (mpStatus->mbModified)
    {
        pRanges.reset(new MisspellRangesVec);
        mpEngine->GetAllMisspellRanges(*pRanges);

        if (pRanges->empty())
            pRanges.reset(nullptr);
    }
    // else : No change in status for EditStatusFlags::WRONGWORDCHANGED => no spell errors (which is the default status).

    mpResult->set(nCol, nRow, MisspellRangeResult(pRanges.get(), eCellLang));
    mpCache->set(nCol, nRow, eCellLang, aCell, std::move(pRanges));
}

void SpellCheckContext::resetCache(bool bContentChangeOnly)
{
    if (!mpResult)
        mpResult.reset(new SpellCheckResult());
    else
        mpResult->clear();

    if (!mpCache)
        mpCache.reset(new SpellCheckCache);
    else if (bContentChangeOnly)
        mpCache->clearEditTextMap();
    else
        mpCache->clear();
}

void SpellCheckContext::setup()
{
    mpEngine.reset(new ScTabEditEngine(pDoc));
    mpStatus.reset(new SpellCheckStatus());

    mpEngine->SetControlWord(
        mpEngine->GetControlWord() | (EEControlBits::ONLINESPELLING | EEControlBits::ALLOWBIGOBJS));
    mpEngine->SetStatusEventHdl(LINK(mpStatus.get(), SpellCheckStatus, EventHdl));
    //  Delimiters here like in inputhdl.cxx !!!
    mpEngine->SetWordDelimiters(
                ScEditUtil::ModifyDelimiters(mpEngine->GetWordDelimiters()));

    uno::Reference<linguistic2::XSpellChecker1> xXSpellChecker1(LinguMgr::GetSpellChecker());
    mpEngine->SetSpeller(xXSpellChecker1);
    mpEngine->SetDefaultLanguage(meLanguage);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
