/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <spellcheckcontext.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <svl/sharedstring.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/unolingu.hxx>

#include <scitems.hxx>
#include <attarray.hxx>
#include <document.hxx>
#include <cellvalue.hxx>
#include <editutil.hxx>
#include <dpobject.hxx>

#include <com/sun/star/linguistic2/XSpellChecker1.hpp>

#include <boost/functional/hash.hpp>

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
                boost::hash_combine(seed, rPos.mnCol);
                boost::hash_combine(seed, rPos.mnRow);
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

    typedef std::vector<editeng::MisspellRanges> MisspellType;
    typedef std::unordered_map<CellPos, std::unique_ptr<MisspellType>, CellPos::Hash> CellMapType;
    typedef std::unordered_map<const rtl_uString*, std::unique_ptr<MisspellType>> SharedStringMapType;
    typedef std::unordered_map<CellPos, LanguageType, CellPos::Hash> CellLangMapType;

    SharedStringMapType  maStringMisspells;
    CellMapType          maEditTextMisspells;
    CellLangMapType      maCellLanguages;
    LanguageType         meDefCellLanguage;

public:

    SpellCheckCache(LanguageType eDefaultCellLanguage) : meDefCellLanguage(eDefaultCellLanguage)
    {
    }

    bool query(SCCOL nCol, SCROW nRow, const ScRefCellValue& rCell, MisspellType*& rpRanges) const
    {
        CellType eType = rCell.meType;
        if (eType == CELLTYPE_STRING)
        {
            SharedStringMapType::const_iterator it = maStringMisspells.find(rCell.mpString->getData());
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

    void set(SCCOL nCol, SCROW nRow, const ScRefCellValue& rCell, std::unique_ptr<MisspellType> pRanges)
    {
        CellType eType = rCell.meType;
        if (eType == CELLTYPE_STRING)
            maStringMisspells.insert_or_assign(rCell.mpString->getData(), std::move(pRanges));
        else if (eType == CELLTYPE_EDIT)
            maEditTextMisspells.insert_or_assign(CellPos(nCol, nRow), std::move(pRanges));
    }

    LanguageType getLanguage(SCCOL nCol, SCROW nRow) const
    {
        CellLangMapType::const_iterator it = maCellLanguages.find(CellPos(nCol, nRow));
        if (it == maCellLanguages.end())
            return meDefCellLanguage;

        return it->second;
    }

    void setLanguage(LanguageType eCellLang, SCCOL nCol, SCROW nRow)
    {
        if (eCellLang == meDefCellLanguage)
            maCellLanguages.erase(CellPos(nCol, nRow));
        else
            maCellLanguages.insert_or_assign(CellPos(nCol, nRow), eCellLang);
    }

    void clear(LanguageType eDefaultCellLanguage)
    {
        maStringMisspells.clear();
        maEditTextMisspells.clear();
        maCellLanguages.clear();
        meDefCellLanguage = eDefaultCellLanguage;
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
    const std::vector<editeng::MisspellRanges>* pRanges;

    SpellCheckResult() : mnCol(-1), mnRow(-1), pRanges(nullptr) {}

    void set(SCCOL nCol, SCROW nRow, const std::vector<editeng::MisspellRanges>* pMisspells)
    {
        mnCol = nCol;
        mnRow = nRow;
        pRanges = pMisspells;
    }

    const std::vector<editeng::MisspellRanges>* query(SCCOL nCol, SCROW nRow) const
    {
        assert(mnCol == nCol);
        assert(mnRow == nRow);
        (void)nCol;
        (void)nRow;
        return pRanges;
    }

    void clear()
    {
        mnCol = -1;
        mnRow = -1;
        pRanges = nullptr;
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

bool SpellCheckContext::isMisspelled(SCCOL nCol, SCROW nRow) const
{
    const_cast<SpellCheckContext*>(this)->ensureResults(nCol, nRow);
    return mpResult->query(nCol, nRow);
}

const std::vector<editeng::MisspellRanges>* SpellCheckContext::getMisspellRanges(
    SCCOL nCol, SCROW nRow ) const
{
    const_cast<SpellCheckContext*>(this)->ensureResults(nCol, nRow);
    return mpResult->query(nCol, nRow);
}

void SpellCheckContext::setMisspellRanges(
    SCCOL nCol, SCROW nRow, const std::vector<editeng::MisspellRanges>* pRanges )
{
    if (!mpEngine || !mpCache)
        reset();

    ScRefCellValue aCell(*pDoc, ScAddress(nCol, nRow, mnTab));
    CellType eType = aCell.meType;

    if (eType != CELLTYPE_STRING && eType != CELLTYPE_EDIT)
        return;

    typedef std::vector<editeng::MisspellRanges> MisspellType;
    std::unique_ptr<MisspellType> pMisspells(pRanges ? new MisspellType(*pRanges) : nullptr);
    mpCache->set(nCol, nRow, aCell, std::move(pMisspells));
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
            if (aPivotRanges.In(ScAddress(nCol, nRow, mnTab))) // Don't spell check within pivot tables
            {
                mpResult->set(nCol, nRow, nullptr);
                return;
            }
        }
    }

    ScRefCellValue aCell(*pDoc, ScAddress(nCol, nRow, mnTab));
    CellType eType = aCell.meType;

    if (eType != CELLTYPE_STRING && eType != CELLTYPE_EDIT)
    {
        // No spell-check required.
        mpResult->set(nCol, nRow, nullptr);
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
        mpResult->set(nCol, nRow, nullptr); // No need to spell check this cell.
        return;
    }

    typedef std::vector<editeng::MisspellRanges> MisspellType;

    LanguageType eCachedCellLang = mpCache->getLanguage(nCol, nRow);

    if (eCellLang != eCachedCellLang)
        mpCache->setLanguage(eCellLang, nCol, nRow);

    else
    {
        MisspellType* pRanges = nullptr;
        bool bFound = mpCache->query(nCol, nRow, aCell, pRanges);
        if (bFound)
        {
            // Cache hit.
            mpResult->set(nCol, nRow, pRanges);
            return;
        }
    }

    // Cache miss, the cell needs spell-check..
    mpEngine->SetDefaultItem(SvxLanguageItem(eCellLang, EE_CHAR_LANGUAGE));
    if (eType == CELLTYPE_STRING)
        mpEngine->SetText(aCell.mpString->getString());
    else
        mpEngine->SetText(*aCell.mpEditText);

    mpStatus->mbModified = false;
    mpEngine->CompleteOnlineSpelling();
    std::unique_ptr<MisspellType> pRanges;
    if (mpStatus->mbModified)
    {
        pRanges.reset(new MisspellType);
        mpEngine->GetAllMisspellRanges(*pRanges);

        if (pRanges->empty())
            pRanges.reset(nullptr);
    }
    // else : No change in status for EditStatusFlags::WRONGWORDCHANGED => no spell errors (which is the default status).

    mpResult->set(nCol, nRow, pRanges.get());
    mpCache->set(nCol, nRow, aCell, std::move(pRanges));
}

void SpellCheckContext::resetCache(bool bContentChangeOnly)
{
    if (!mpResult)
        mpResult.reset(new SpellCheckResult());
    else
        mpResult->clear();

    if (!mpCache)
        mpCache.reset(new SpellCheckCache(meLanguage));
    else if (bContentChangeOnly)
        mpCache->clearEditTextMap();
    else
        mpCache->clear(meLanguage);
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
