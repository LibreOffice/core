/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <resourcemodel/exceptions.hxx>
#include <resourcemodel/QNameToString.hxx>
#include <WW8DocumentImpl.hxx>
#include <WW8FKPImpl.hxx>
#include <WW8PieceTableImpl.hxx>
#include <WW8BinTableImpl.hxx>
#include <WW8StreamImpl.hxx>
#include <WW8Sttbf.hxx>
#include <Dff.hxx>
#include <iterator>
#include <XNoteHelperImpl.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <doctokLoggers.hxx>

namespace writerfilter {
namespace doctok
{

using namespace ::std;

template <class T>
struct PLCFHelper
{
    static void processPLCFCpAndFcs(WW8DocumentImpl & rDoc,
                                    WW8PieceTable::Pointer_t pPieceTable,
                                    typename PLCF<T>::Pointer_t pPLCF,
                                    PropertyType eType,
                                    sal_uInt32 nOffset)
    {
        sal_uInt32 nCount = pPLCF->getEntryCount();
        for (sal_uInt32 n = 0; n < nCount; ++n)
        {
            Cp aCp(pPLCF->getFc(n) + nOffset);
            CpAndFc aCpAndFc = pPieceTable->createCpAndFc(aCp, eType);

            rDoc.insertCpAndFc(aCpAndFc);
        }
    }
};

// WW8DocumentIteratorImpl
bool operator == (const WW8DocumentIterator & rA,
                  const WW8DocumentIterator & rB)
{
    return rA.equal(rB);
}

WW8DocumentIterator::~WW8DocumentIterator()
{
}

WW8DocumentIteratorImpl::~WW8DocumentIteratorImpl()
{
}

WW8DocumentIterator & WW8DocumentIteratorImpl::operator++()
{
    mCpAndFc = mpDocument->getNextCp(mCpAndFc);

    return *this;
}

WW8DocumentIterator & WW8DocumentIteratorImpl::operator--()
{
    mCpAndFc = mpDocument->getPrevCp(mCpAndFc);

    return *this;
}

bool WW8DocumentIteratorImpl::equal(const WW8DocumentIterator & rIt_) const
{
    const WW8DocumentIteratorImpl & rIt =
        dynamic_cast<const WW8DocumentIteratorImpl &>(rIt_);

    return mCpAndFc == rIt.mCpAndFc && mpDocument == rIt.mpDocument;
}

writerfilter::Reference<Properties>::Pointer_t
WW8DocumentIteratorImpl::getProperties() const
{
    return mpDocument->getProperties(mCpAndFc);
}

writerfilter::Reference<Stream>::Pointer_t
WW8DocumentIteratorImpl::getSubDocument() const
{
    return mpDocument->getSubDocument(mCpAndFc);
}

WW8Stream::Sequence WW8DocumentIteratorImpl::getText()
{
    return mpDocument->getText(mCpAndFc);
}

writerfilter::Reference<Properties>::Pointer_t
WW8DocumentIteratorImpl::getShape() const
{
    return mpDocument->getShape(mCpAndFc);
}

PropertyType WW8DocumentIteratorImpl::getPropertyType() const
{
    return mCpAndFc.getType();
}

bool WW8DocumentIteratorImpl::isComplex() const
{
    return mCpAndFc.isComplex();
}

void WW8DocumentIteratorImpl::dump(ostream & o) const
{
    o << mCpAndFc;
}

string WW8DocumentIteratorImpl::toString() const
{
    return mCpAndFc.toString();
}

// WW8DocumentImpl

WW8Document::~WW8Document()
{
}

#if OSL_DEBUG_LEVEL > 1
class WW8IdToString : public IdToString
{
public:
    WW8IdToString() : IdToString() {}
    virtual ~WW8IdToString() {}

    virtual string toString(const Id & rId) const
    {
        string s((*SprmIdToString::Instance())(rId));

        if (s.empty())
            s = (*QNameToString::Instance())(rId);

        return s;
    }
};
#endif

WW8DocumentImpl::~WW8DocumentImpl()
{
}

WW8DocumentImpl::WW8DocumentImpl(WW8Stream::Pointer_t rpStream)
: bSubDocument(false), mfcPicLoc(0), mbPicIsData(false), mpStream(rpStream),
mbInSection(false), mbInParagraphGroup(false), mbInCharacterGroup(false)
{
    mpDocStream = getSubStream("WordDocument");

    mpSummaryInformationStream = getSubStream("\5SummaryInformation");

    try
    {
        mpDataStream = getSubStream("Data");
    }
    catch (const ExceptionNotFound &)
    {
    }

    try
    {
        mpCompObjStream = getSubStream("\1CompObj");
    }
    catch (const ExceptionNotFound &)
    {
    }

    mpCHPFKPCache =
        WW8FKPCache::Pointer_t(new WW8CHPFKPCacheImpl(mpDocStream, 5));
    mpPAPFKPCache =
        WW8FKPCache::Pointer_t(new WW8PAPFKPCacheImpl(mpDocStream, 5));

    mpFib = WW8Fib::Pointer_t(new WW8Fib(*mpDocStream));

    switch (mpFib->get_fWhichTblStm())
    {
    case 0:
        mpTableStream = getSubStream("0Table");

        break;

    case 1:
        mpTableStream = getSubStream("1Table");

        break;

    default:
        break;
    }

    if (mpFib->get_nFib() >= 0xD9)
    {
        mpFibRgFcLcb2000.reset(new WW8FibRgFcLcb2000(*mpFib));
    }

    if (mpTableStream.get() == NULL)
        throw ExceptionNotFound("Table stream not found.");

    mpPieceTable =
        WW8PieceTable::Pointer_t
        (new WW8PieceTableImpl(*mpTableStream, mpFib->get_fcClx(),
                               mpFib->get_lcbClx()));

    {
        Cp aCp(mpPieceTable->getLastCp());
        Fc aFc(mpPieceTable->getLastFc());
        CpAndFc aCpAndFc(aCp, aFc, PROP_DOC);
        mCpAndFcs.insert(aCpAndFc);
    }

    {
        Cp aCp(mpFib->get_ccpText());

        mDocumentEndCpAndFc = CpAndFc(aCp, mpPieceTable->cp2fc(aCp),
                                      PROP_DOC);
        mCpAndFcs.insert(mDocumentEndCpAndFc);

        aCp += mpFib->get_ccpFtn();
        mFootnoteEndCpAndFc = CpAndFc(aCp, mpPieceTable->cp2fc(aCp),
                                      PROP_DOC);
        mCpAndFcs.insert(mFootnoteEndCpAndFc);

        aCp += mpFib->get_ccpHdd();
        mHeaderEndCpAndFc = CpAndFc(aCp, mpPieceTable->cp2fc(aCp),
                                    PROP_DOC);
        mCpAndFcs.insert(mHeaderEndCpAndFc);

        aCp += mpFib->get_ccpAtn();
        mAnnotationEndCpAndFc = CpAndFc(aCp, mpPieceTable->cp2fc(aCp),
                                        PROP_DOC);
        mCpAndFcs.insert(mAnnotationEndCpAndFc);

        aCp += mpFib->get_ccpEdn();
        mEndnoteEndCpAndFc = CpAndFc(aCp, mpPieceTable->cp2fc(aCp),
                                     PROP_DOC);
        mCpAndFcs.insert(mEndnoteEndCpAndFc);

        aCp += mpFib->get_ccpTxbx();
        mTextboxEndCpAndFc = CpAndFc(aCp, mpPieceTable->cp2fc(aCp),
                                     PROP_DOC);
        mCpAndFcs.insert(mTextboxEndCpAndFc);

        aCp += mpFib->get_ccpHdrTxbx();
        mTextboxHeaderEndCpAndFc = CpAndFc(aCp, mpPieceTable->cp2fc(aCp),
                                           PROP_DOC);
        mCpAndFcs.insert(mTextboxHeaderEndCpAndFc);
    }

    mpBinTablePAPX =
        WW8BinTable::Pointer_t(new WW8BinTableImpl
                               (*mpTableStream,
                                mpFib->get_fcPlcfbtePapx(),
                                mpFib->get_lcbPlcfbtePapx()));

    //clog << "BinTable(PAP):" << mpBinTablePAPX->toString();

    parseBinTableCpAndFcs(*mpBinTablePAPX, PROP_PAP);

    mpBinTableCHPX =
        WW8BinTable::Pointer_t(new WW8BinTableImpl
                               (*mpTableStream,
                                mpFib->get_fcPlcfbteChpx(),
                                mpFib->get_lcbPlcfbteChpx()));

    parseBinTableCpAndFcs(*mpBinTableCHPX, PROP_CHP);

    mpSEDs = PLCF<WW8SED>::Pointer_t(new PLCF<WW8SED>
                                     (*mpTableStream,
                                      mpFib->get_fcPlcfsed(),
                                      mpFib->get_lcbPlcfsed()));

    {
        PLCFHelper<WW8SED>::processPLCFCpAndFcs
            (*this, mpPieceTable, mpSEDs, PROP_SEC, 0);
    }

    sal_uInt32 nHeaders = getHeaderCount();

    if (nHeaders > 0)
    {
        mpHeaderOffsets = WW8StructBase::Pointer_t
            (new WW8StructBase(*mpTableStream,
                               mpFib->get_fcPlcfhdd(),
                               mpFib->get_lcbPlcfhdd()));

        {
            for (sal_uInt32 n = 0; n <= nHeaders; ++n)
            {
                CpAndFc aCpAndFc(getHeaderCpAndFc(n));

                mCpAndFcs.insert(aCpAndFc);
            }
        }
    }

    if (mpFib->get_lcbPlcffndTxt() > 0)
    {
        WW8StructBase::Pointer_t pCps
            (new WW8StructBase(*mpTableStream,
                               mpFib->get_fcPlcffndTxt(),
                               mpFib->get_lcbPlcffndTxt()));

        PLCF<WW8FRD>::Pointer_t pRefs
            (new PLCF<WW8FRD>(*mpTableStream,
                               mpFib->get_fcPlcffndRef(),
                               mpFib->get_lcbPlcffndRef()));

        mpFootnoteHelper = XNoteHelper<WW8FRD>::Pointer_t
            (new XNoteHelper<WW8FRD>(pCps, pRefs, mpPieceTable, this,
                             PROP_FOOTNOTE, getDocumentEndCp()));

        mpFootnoteHelper->init();
    }

    if (mpFib->get_lcbPlcfendTxt() > 0)
    {
        WW8StructBase::Pointer_t pCps
            (new WW8StructBase(*mpTableStream,
                               mpFib->get_fcPlcfendTxt(),
                               mpFib->get_lcbPlcfendTxt()));

        PLCF<WW8FRD>::Pointer_t pRefs
            (new PLCF<WW8FRD>(*mpTableStream,
                               mpFib->get_fcPlcfendRef(),
                               mpFib->get_lcbPlcfendRef()));

        mpEndnoteHelper = XNoteHelper<WW8FRD>::Pointer_t
            (new XNoteHelper<WW8FRD>(pCps, pRefs, mpPieceTable, this,
                             PROP_ENDNOTE, getAnnotationEndCp()));

        mpEndnoteHelper->init();
    }

    if (mpFib->get_lcbPlcfandTxt() > 0)
    {
        WW8StructBase::Pointer_t pCps
            (new WW8StructBase(*mpTableStream,
                               mpFib->get_fcPlcfandTxt(),
                               mpFib->get_lcbPlcfandTxt()));

        PLCF<WW8ATRD>::Pointer_t pRefs
            (new PLCF<WW8ATRD>(*mpTableStream,
                               mpFib->get_fcPlcfandRef(),
                               mpFib->get_lcbPlcfandRef()));

        mpAnnotationHelper = XNoteHelper<WW8ATRD>::Pointer_t
            (new XNoteHelper<WW8ATRD>(pCps, pRefs, mpPieceTable, this,
                                   PROP_ANNOTATION, getHeaderEndCp()));

        mpAnnotationHelper->init();
    }

    if (mpFib->get_lcbSttbfbkmk() > 0)
    {
        PLCF<WW8BKF>::Pointer_t pStartCps
            (new PLCF<WW8BKF>(*mpTableStream, mpFib->get_fcPlcfbkf(),
                              mpFib->get_lcbPlcfbkf()));

        WW8StructBase::Pointer_t pEndCps
            (new WW8StructBase(*mpTableStream, mpFib->get_fcPlcfbkl(),
                               mpFib->get_lcbPlcfbkl()));

        WW8Sttbf::Pointer_t pNames
            (new WW8Sttbf(*mpTableStream, mpFib->get_fcSttbfbkmk(),
                          mpFib->get_lcbSttbfbkmk()));

        mpBookmarkHelper = BookmarkHelper::Pointer_t
            (new BookmarkHelper(pStartCps, pEndCps, pNames, mpPieceTable, this));

        mpBookmarkHelper->init();
    }

    {
        PLCF<WW8FLD>::Pointer_t pPlcffldMom;

        if (mpFib->get_lcbPlcffldMom() > 0)
        {
            pPlcffldMom = PLCF<WW8FLD>::Pointer_t
                (new PLCF<WW8FLD>(*mpTableStream,
                                  mpFib->get_fcPlcffldMom(),
                                  mpFib->get_lcbPlcffldMom()));

            mpFieldHelper = FieldHelper::Pointer_t
                (new FieldHelper(pPlcffldMom,
                                 this));

            mpFieldHelper->init();
        }
    }

    PLCF<WW8FSPA>::Pointer_t pPlcspaMom;
    if (mpFib->get_lcbPlcspaMom() > 0)
    {
        pPlcspaMom = PLCF<WW8FSPA>::Pointer_t
            (new PLCF<WW8FSPA>
             (*mpTableStream, mpFib->get_fcPlcspaMom(),
              mpFib->get_lcbPlcspaMom()));
    }

    PLCF<WW8FSPA>::Pointer_t pPlcspaHdr;
    if (mpFib->get_lcbPlcspaHdr() > 0)
    {
        pPlcspaHdr = PLCF<WW8FSPA>::Pointer_t
            (new PLCF<WW8FSPA>
             (*mpTableStream, mpFib->get_fcPlcspaHdr(),
              mpFib->get_lcbPlcspaHdr()));
    }

    mpShapeHelper = ShapeHelper::Pointer_t
        (new ShapeHelper(pPlcspaMom, pPlcspaHdr, this));

    mpShapeHelper->init();

    PLCF<WW8BKD>::Pointer_t pPlcbkdMother;
    if (mpFib->get_fcBkdMother() > 0 && mpFib->get_lcbBkdMother() > 0)
    {
        pPlcbkdMother = PLCF<WW8BKD>::Pointer_t
            (new PLCF<WW8BKD>
             (*mpTableStream, mpFib->get_fcBkdMother(),
              mpFib->get_lcbBkdMother()));
    }

    mpBreakHelper = BreakHelper::Pointer_t
        (new BreakHelper(pPlcbkdMother, this));

    mpBreakHelper->init();

    if (mpFib->get_fcDggInfo() != 0 && mpFib->get_lcbDggInfo() > 0)
    {
        mpDffBlock = DffBlock::Pointer_t
            (new DffBlock(*mpTableStream, mpFib->get_fcDggInfo(),
                         mpFib->get_lcbDggInfo(), 1));

        mpDffBlock->setDocument(this);
    }

    if (mpFib->get_lcbPlcftxbxTxt() > 0)
    {
        mpTextBoxStories = PLCF<WW8FTXBXS>::Pointer_t
            (new PLCF<WW8FTXBXS>(*mpTableStream,
                                 mpFib->get_fcPlcftxbxTxt(),
                                 mpFib->get_lcbPlcftxbxTxt()));

        PLCFHelper<WW8FTXBXS>::processPLCFCpAndFcs
            (*this, mpPieceTable, mpTextBoxStories, PROP_DOC,
             mEndnoteEndCpAndFc.getCp().get());
    }

    if (mCpAndFcs.size() > 0)
    {
        mCpAndFcStart = *mCpAndFcs.begin();
        mCpAndFcEnd = getDocumentEndCp();
    }
}

bool WW8DocumentImpl::isSpecial(sal_uInt32 nChar)
{
    bool bResult = false;

    if (nChar <= 8)
        bResult = true;
    else if (nChar >= 10)
    {
        if (nChar == 12)
            bResult= true;
        else if (nChar <= 16)
            bResult = true;
        else if (nChar >= 22)
        {
            if (nChar <= 30)
                bResult = true;
            else if (nChar >= 33)
            {
                if (nChar <= 39)
                    bResult = true;
                else if (nChar == 41)
                    bResult = true;
            }
        }
    }

    return bResult;
}

WW8DocumentImpl::WW8DocumentImpl(const WW8DocumentImpl & rSrc,
                                 const CpAndFc & rStart, const CpAndFc & rEnd)
: bSubDocument(true), mfcPicLoc(0), mbPicIsData(false)
{
    Assign(rSrc);

    mCpAndFcStart = rStart;
    mCpAndFcEnd = rEnd;
}

WW8DocumentImpl & WW8DocumentImpl::Assign(const WW8DocumentImpl & rSrc)
{
    mCpAndFcs = rSrc.mCpAndFcs;

    mpCHPFKPCache = rSrc.mpCHPFKPCache;
    mpPAPFKPCache = rSrc.mpPAPFKPCache;

    mpStream = rSrc.mpStream;
    mpTableStream = rSrc.mpTableStream;
    mpDataStream = rSrc.mpDataStream;
    mpDocStream = rSrc.mpDocStream;
    mpCompObjStream = rSrc.mpCompObjStream;

    mpPieceTable = rSrc.mpPieceTable;

    mpBinTableCHPX = rSrc.mpBinTableCHPX;
    mpBinTablePAPX = rSrc.mpBinTablePAPX;

    mpSEDs = rSrc.mpSEDs;

    mpFib = rSrc.mpFib;

    mpHeaderOffsets = rSrc.mpHeaderOffsets;
    mpFootnoteHelper = rSrc.mpFootnoteHelper;
    mpEndnoteHelper = rSrc.mpEndnoteHelper;
    mpAnnotationHelper = rSrc.mpAnnotationHelper;
    mpShapeHelper = rSrc.mpShapeHelper;
    mpBreakHelper = rSrc.mpBreakHelper;

    mpBookmarkHelper = rSrc.mpBookmarkHelper;

    mpDffBlock = rSrc.mpDffBlock;
    mpTextBoxStories = rSrc.mpTextBoxStories;

    mDocumentEndCpAndFc = rSrc.mDocumentEndCpAndFc;
    mFootnoteEndCpAndFc = rSrc.mFootnoteEndCpAndFc;

    return *this;
}

string WW8DocumentImpl::getType() const
{
    return "WW8DocumentImpl";
}

void WW8DocumentImpl::parseBinTableCpAndFcs(WW8BinTable & rTable,
                                            PropertyType eType_)
{
    for (sal_uInt32 i = 0; i < rTable.getEntryCount(); i++)
    {
        Fc aFcFromTable(rTable.getFc(i));

        if (aFcFromTable < mpPieceTable->getFirstFc())
            aFcFromTable = mpPieceTable->getFirstFc();

        bool bComplex = mpPieceTable->isComplex(aFcFromTable);
        aFcFromTable.setComplex(bComplex);

        try
        {
            Cp aCpFromTable(mpPieceTable->fc2cp(aFcFromTable));
            CpAndFc aCpAndFcFromTable(aCpFromTable, aFcFromTable, eType_);

            mCpAndFcs.insert(aCpAndFcFromTable);

            WW8FKP::Pointer_t pFKP;

            switch (eType_)
            {
            case PROP_CHP:
                pFKP = getFKPCHPX(rTable.getPageNumber(i),
                                  aCpAndFcFromTable.isComplex());

                break;

            case PROP_PAP:
                pFKP = getFKPPAPX(rTable.getPageNumber(i),
                                  aCpAndFcFromTable.isComplex());

                break;
            default:
                break;
            }

            for (sal_uInt32 n = 0; n < pFKP->getEntryCount(); n++)
            {
                Fc aFc = pFKP->getFc(n);

                if (aFc < mpPieceTable->getFirstFc())
                    aFc = mpPieceTable->getFirstFc();

                bool bComplexFKP = mpPieceTable->isComplex(aFc);
                aFc.setComplex(bComplexFKP);

                try
                {
                    Cp aCp = mpPieceTable->fc2cp(aFc);

                    CpAndFc aCpAndFc(aCp, aFc, eType_);

                    mCpAndFcs.insert(aCpAndFc);
                }
                catch (const ExceptionNotFound &e)
                {
                    clog << e.getText() << endl;
                }
            }
        }
        catch (const ExceptionNotFound &e)
        {
            clog << e.getText() << endl;
        }
    }
}

WW8Stream::Pointer_t WW8DocumentImpl::getSubStream
(const OUString & sId) const
{
    return mpStream->getSubStream(sId);
}

WW8Document::Pointer_t WW8DocumentImpl::getSubDocument(SubDocumentId /*nId*/)
{
    return WW8Document::Pointer_t(new WW8DocumentImpl(*this));
}

WW8DocumentIterator::Pointer_t
WW8DocumentImpl::getIterator(const CpAndFc & rCpAndFc)
{
    return WW8DocumentIterator::Pointer_t
        (new WW8DocumentIteratorImpl(this, rCpAndFc));
}

WW8DocumentIterator::Pointer_t WW8DocumentImpl::begin()
{
    return getIterator(getFirstCp());
}

WW8DocumentIterator::Pointer_t WW8DocumentImpl::end()
{
    return getIterator(getLastCp());
}

WW8Stream::Pointer_t WW8DocumentImpl::getDocStream() const
{
    return mpDocStream;
}

WW8Stream::Pointer_t WW8DocumentImpl::getDataStream() const
{
    return mpDataStream;
}

sal_uInt32 WW8DocumentImpl::getByteLength(const CpAndFc & rCpAndFc) const
{
    CpAndFc aEnd = getNextCp(rCpAndFc);

    sal_uInt32 nResult = 3;

    if (rCpAndFc < aEnd)
        nResult = (aEnd - rCpAndFc) *
            (mpPieceTable->isComplex(rCpAndFc.getCp()) ? 1 : 2);

    return nResult;
}

WW8Stream::Sequence
WW8DocumentImpl::getText(const CpAndFc & rStart)
{
    return mpDocStream->get(rStart.getFc().get(), getByteLength(rStart));
}

const CpAndFc & WW8DocumentImpl::getFirstCp() const
{
    return mCpAndFcStart;
}

const CpAndFc & WW8DocumentImpl::getLastCp() const
{
    return mCpAndFcEnd;
}

CpAndFc WW8DocumentImpl::getDocumentEndCp() const
{
    return mDocumentEndCpAndFc;
}

CpAndFc WW8DocumentImpl::getFootnodeEndCp() const
{
    return mFootnoteEndCpAndFc;
}

CpAndFc WW8DocumentImpl::getHeaderEndCp() const
{
    return mHeaderEndCpAndFc;
}

CpAndFc WW8DocumentImpl::getAnnotationEndCp() const
{
    return mAnnotationEndCpAndFc;
}

CpAndFc WW8DocumentImpl::getEndnoteEndCp() const
{
    return mEndnoteEndCpAndFc;
}

CpAndFc WW8DocumentImpl::getNextCp(const CpAndFc & rCpAndFc) const
{
    CpAndFc aResult = mCpAndFcEnd;
    CpAndFcs::const_iterator aIt = mCpAndFcs.find(rCpAndFc);

    if (aIt != mCpAndFcs.end())
    {
        ++aIt;

        if (aIt != mCpAndFcs.end())
            aResult = *aIt;
    }
    else
        throw ExceptionNotFound("getNextCp: " + rCpAndFc.toString());

    return aResult;
}

CpAndFc WW8DocumentImpl::getPrevCp(const CpAndFc & rCpAndFc) const
{
    CpAndFc aResult = mCpAndFcStart;

    CpAndFcs::const_iterator aIt = mCpAndFcs.find(CpAndFc(rCpAndFc));

    if (aIt != mCpAndFcs.end() && aIt != mCpAndFcs.begin())
    {
        --aIt;

        aResult = *aIt;
    }
    else
        throw ExceptionNotFound("getPrevCp: " + rCpAndFc.toString());

    return aResult;
}

WW8FKP::Pointer_t WW8DocumentImpl::getFKP(const CpAndFc & rCpAndFc)
{
    WW8FKP::Pointer_t pResult;

    sal_uInt32 nPageNumber = 0;

    switch (rCpAndFc.getType())
    {
    case PROP_PAP:
        {
            nPageNumber =
                mpBinTablePAPX->getPageNumber(rCpAndFc.getFc());

            pResult = getFKPPAPX(nPageNumber, rCpAndFc.isComplex());
        }
        break;
    case PROP_CHP:
        {
            nPageNumber =
                mpBinTableCHPX->getPageNumber(rCpAndFc.getFc());

            pResult = getFKPCHPX(nPageNumber, rCpAndFc.isComplex());
        }
        break;
    default:
        break;
    }

    if (pResult.get() != NULL)
        pResult->setDocument(this);

    return pResult;
}

WW8FKP::Pointer_t WW8DocumentImpl::getFKPCHPX(sal_uInt32 nIndex,
                                              bool bComplex)
{
    return mpCHPFKPCache->get(nIndex, bComplex);
}

WW8FKP::Pointer_t WW8DocumentImpl::getFKPPAPX(sal_uInt32 nIndex,
                                              bool bComplex)
{
    return mpPAPFKPCache->get(nIndex, bComplex);
}

writerfilter::Reference<Properties>::Pointer_t WW8DocumentImpl::getProperties
(const CpAndFc & rCpAndFc)
{
    writerfilter::Reference<Properties>::Pointer_t pResult;

    switch (rCpAndFc.getType())
    {
    case PROP_CHP:
    case PROP_PAP:
        {
            try
            {
                WW8FKP::Pointer_t pFKP = getFKP(rCpAndFc);

                pResult = pFKP->getProperties(rCpAndFc.getFc());
            }
            catch (const ExceptionOutOfBounds &)
            {
            }
        }

        break;

    case PROP_SEC:
        {
            pResult = writerfilter::Reference<Properties>::Pointer_t
                (getSED(rCpAndFc));
        }

        break;

    case PROP_FOOTNOTE:
        {
            pResult = writerfilter::Reference<Properties>::Pointer_t
                (mpFootnoteHelper->getRef(rCpAndFc));
        }
        break;

    case PROP_ENDNOTE:
        {
            pResult = writerfilter::Reference<Properties>::Pointer_t
                (mpEndnoteHelper->getRef(rCpAndFc));
        }
        break;

    case PROP_ANNOTATION:
        {
            pResult = writerfilter::Reference<Properties>::Pointer_t
                (mpAnnotationHelper->getRef(rCpAndFc));
        }
        break;

    case PROP_BOOKMARKSTART:
    case PROP_BOOKMARKEND:
        {
            pResult = getBookmark(rCpAndFc);
        }

        break;
    case PROP_FLD:
        {
            pResult = getField(rCpAndFc);

            mpFLD = mpFieldHelper->getWW8FLD(rCpAndFc);
        }

        break;
    case PROP_SHP:
        {
            pResult = getShape(rCpAndFc);
        }
        break;
    case PROP_BRK:
        {
            pResult = getBreak(rCpAndFc);
        }
        break;
    default:
        break;
    }

    return pResult;
}

writerfilter::Reference<Stream>::Pointer_t
WW8DocumentImpl::getSubDocument(const CpAndFc & rCpAndFc)
{
    writerfilter::Reference<Stream>::Pointer_t pResult;

    switch (rCpAndFc.getType())
    {
    case PROP_FOOTNOTE:
        pResult = getFootnote(rCpAndFc);
        break;

    case PROP_ENDNOTE:
        pResult = getEndnote(rCpAndFc);
        break;

    case PROP_ANNOTATION:
        pResult = getAnnotation(rCpAndFc);
        break;

    default:
        break;
    }

    return pResult;
}

WW8SED * WW8DocumentImpl::getSED(const CpAndFc & rCpAndFc) const
{
    WW8SED * pResult = mpSEDs->getEntryByFc(rCpAndFc.getCp().get());

    pResult->setDoc(const_cast<const WW8DocumentImpl *>(this));

    return pResult;
}

writerfilter::Reference<Table>::Pointer_t WW8DocumentImpl::getListTplcs() const
{
    writerfilter::Reference<Table>::Pointer_t pResult;

    if (mpFibRgFcLcb2000.get() != NULL &&
        mpFibRgFcLcb2000->get_fcSttbRgtplc() != 0 &&
        mpFibRgFcLcb2000->get_lcbSttbRgtplc() != 0)
    {
        WW8SttbRgtplc * pSttbRgtplc =
        new WW8SttbRgtplc(*mpTableStream,
                          mpFibRgFcLcb2000->get_fcSttbRgtplc(),
                          mpFibRgFcLcb2000->get_lcbSttbRgtplc());

        pResult = writerfilter::Reference<Table>::Pointer_t(pSttbRgtplc);
    }

    return pResult;
}

writerfilter::Reference<Table>::Pointer_t WW8DocumentImpl::getListTable() const
{
    writerfilter::Reference<Table>::Pointer_t pResult;

    if (mpFib->get_fcPlcfLst() != 0 && mpFib->get_lcbPlcfLst() > 0)
    {
        try
        {
            WW8ListTable * pList = new WW8ListTable(*mpTableStream,
                                                    mpFib->get_fcPlcfLst(),
                                                    mpFib->get_fcPlfLfo() -
                                                    mpFib->get_fcPlcfLst());

            pList->setPayloadOffset(mpFib->get_lcbPlcfLst());
            pList->initPayload();

            pResult = writerfilter::Reference<Table>::Pointer_t(pList);
        }
        catch (const ExceptionOutOfBounds &) {
        }
    }

    return pResult;
}

writerfilter::Reference<Table>::Pointer_t WW8DocumentImpl::getLFOTable() const
{
    writerfilter::Reference<Table>::Pointer_t pResult;

    if (mpFib->get_fcPlfLfo() != 0 && mpFib->get_lcbPlfLfo() > 0)
    {
        try
        {
            WW8LFOTable * pLFOs = new WW8LFOTable(*mpTableStream,
                                                  mpFib->get_fcPlfLfo(),
                                                  mpFib->get_lcbPlfLfo());

            pLFOs->setPayloadOffset(mpFib->get_lcbPlcfLst());
            pLFOs->initPayload();

            pResult = writerfilter::Reference<Table>::Pointer_t(pLFOs);
        }
        catch (const Exception &e)
        {
            clog << e.getText() << endl;
        }
    }

    return pResult;
}

writerfilter::Reference<Table>::Pointer_t WW8DocumentImpl::getFontTable() const
{
    writerfilter::Reference<Table>::Pointer_t pResult;

    if (mpFib->get_fcSttbfffn() != 0 && mpFib->get_lcbSttbfffn() > 0)
    {
        WW8FontTable * pFonts = new WW8FontTable(*mpTableStream,
                                                 mpFib->get_fcSttbfffn(),
                                                 mpFib->get_lcbSttbfffn());

        pFonts->initPayload();

        pResult = writerfilter::Reference<Table>::Pointer_t(pFonts);
    }

    return pResult;
}

writerfilter::Reference<Table>::Pointer_t WW8DocumentImpl::getStyleSheet() const
{
    writerfilter::Reference<Table>::Pointer_t pResult;

    if (mpFib->get_lcbStshf() > 0)
    {
        WW8StyleSheet * pStyles = new WW8StyleSheet(*mpTableStream,
                                                    mpFib->get_fcStshf(),
                                                    mpFib->get_lcbStshf());

        pStyles->initPayload();

        pResult = writerfilter::Reference<Table>::Pointer_t(pStyles);
    }

    return pResult;
}

writerfilter::Reference<Table>::Pointer_t WW8DocumentImpl::getAssocTable() const
{
    writerfilter::Reference<Table>::Pointer_t pResult;

    if (mpFib->get_lcbSttbfAssoc() > 0)
    {
        WW8Sttbf::Pointer_t pSttbfAssoc
            (new WW8Sttbf(*mpTableStream,
                          mpFib->get_fcSttbfAssoc(),
                          mpFib->get_lcbSttbfAssoc()));

        pResult = writerfilter::Reference<Table>::Pointer_t
            (new WW8SttbTableResource(pSttbfAssoc));
    }

    return pResult;
}

sal_uInt32 WW8DocumentImpl::getHeaderCount() const
{
    sal_uInt32 nResult = 0;
    sal_uInt32 nLcbPlcfhdd = mpFib->get_lcbPlcfhdd();

    if (nLcbPlcfhdd > 4)
        nResult = (nLcbPlcfhdd / 4) - 1;

    return nResult;
}

CpAndFc WW8DocumentImpl::getHeaderCpAndFc(sal_uInt32 nPos)
{
    sal_uInt32 nCount = getHeaderCount();

    // There are getHeaderCount() + 1 entries in mpHeaderOffsets => greater
    if (nPos > nCount)
        throw ExceptionNotFound("getHeaderCpAndFc");

    if (nPos == nCount)
        return mHeaderEndCpAndFc;
    else
    {
        Cp aCp(getFootnodeEndCp().getCp() + mpHeaderOffsets->getU32(nPos * 4));
        Fc aFc(mpPieceTable->cp2fc(aCp));
        CpAndFc aCpAndFc(aCp, aFc, PROP_DOC);

        return aCpAndFc;
    }

}

writerfilter::Reference<Stream>::Pointer_t WW8DocumentImpl::getHeader(sal_uInt32 nPos)
{
    // There are getHeaderCount() headers => greater or equal
    if (nPos >= getHeaderCount())
        throw ExceptionNotFound("getHeader");

    writerfilter::Reference<Stream>::Pointer_t pResult;

    CpAndFc aCpAndFcStart(getHeaderCpAndFc(nPos));
    CpAndFc aCpAndFcEnd(getHeaderCpAndFc(nPos + 1));

    if (aCpAndFcStart < aCpAndFcEnd)
        pResult = writerfilter::Reference<Stream>::Pointer_t
            (new WW8DocumentImpl(*this, aCpAndFcStart, aCpAndFcEnd));

    return pResult;
}

writerfilter::Reference<Stream>::Pointer_t
WW8DocumentImpl::getFootnote(const CpAndFc & rCpAndFc)
{
    writerfilter::Reference<Stream>::Pointer_t pResult;

    if (! bSubDocument)
        pResult = mpFootnoteHelper->get(rCpAndFc);

    return pResult;
}

writerfilter::Reference<Stream>::Pointer_t
WW8DocumentImpl::getEndnote(const CpAndFc & rCpAndFc)
{
    writerfilter::Reference<Stream>::Pointer_t pResult;

    if (! bSubDocument)
        pResult = mpEndnoteHelper->get(rCpAndFc);

    return pResult;
}


writerfilter::Reference<Stream>::Pointer_t
WW8DocumentImpl::getAnnotation(const CpAndFc & rCpAndFc)
{
    writerfilter::Reference<Stream>::Pointer_t pResult;

    if (! bSubDocument)
        pResult = mpAnnotationHelper->get(rCpAndFc);

    return pResult;
}

writerfilter::Reference<Properties>::Pointer_t
WW8DocumentImpl::getBookmark(const CpAndFc & rCpAndFc) const
{
    return mpBookmarkHelper->getBookmark(rCpAndFc);
}

writerfilter::Reference<Properties>::Pointer_t
WW8DocumentImpl::getShape(const CpAndFc & rCpAndFc) const
{
    return mpShapeHelper->getShape(rCpAndFc);
}

writerfilter::Reference<Properties>::Pointer_t
WW8DocumentImpl::getShape(sal_uInt32 nSpid)
{
    writerfilter::Reference<Properties>::Pointer_t pResult;
    DffRecord::Pointer_t pShape = mpDffBlock->getShape(nSpid);

    if (pShape.get() != NULL)
    {
        DffSpContainer * pTmp = new DffSpContainer(*pShape);
        pTmp->setDocument(this);

        pResult = writerfilter::Reference<Properties>::Pointer_t(pTmp);
    }

    return pResult;
}

writerfilter::Reference<Properties>::Pointer_t
WW8DocumentImpl::getBreak(const CpAndFc & rCpAndFc) const
{
    return mpBreakHelper->getBreak(rCpAndFc);
}

writerfilter::Reference<Properties>::Pointer_t
WW8DocumentImpl::getBlip(sal_uInt32 nBid)
{
    writerfilter::Reference<Properties>::Pointer_t pResult;

    if (mpDffBlock != 0)
    {
        DffRecord::Pointer_t pDffRecord(mpDffBlock->getBlip(nBid));

        if (pDffRecord.get() != NULL)
        {
            DffBSE * pBlip = new DffBSE(*pDffRecord);

            if (pBlip != NULL)
            pResult = writerfilter::Reference<Properties>::Pointer_t(pBlip);
        }
    }

    return pResult;
}

writerfilter::Reference<Properties>::Pointer_t
WW8DocumentImpl::getField(const CpAndFc & rCpAndFc) const
{
    return mpFieldHelper->getField(rCpAndFc);
}

writerfilter::Reference<Properties>::Pointer_t
WW8DocumentImpl::getDocumentProperties() const
{
    writerfilter::Reference<Properties>::Pointer_t pResult;

    if (mpFib->get_lcbDop() > 0)
    {
        pResult.reset(new WW8DopBase(*mpTableStream, mpFib->get_fcDop(), mpFib->get_lcbDop()));
    }

    return pResult;
}

WW8FLD::Pointer_t WW8DocumentImpl::getCurrentFLD() const
{
    return mpFLD;
}

void WW8DocumentImpl::setPicLocation(sal_uInt32 fcPicLoc)
{
    mfcPicLoc = fcPicLoc;
}

bool WW8DocumentImpl::isPicData()
{
    return mbPicIsData;
}

void WW8DocumentImpl::setPicIsData(bool bPicIsData)
{
    mbPicIsData = bPicIsData;
}

writerfilter::Reference<Stream>::Pointer_t
WW8DocumentImpl::getTextboxText(sal_uInt32 nShpId) const
{
    writerfilter::Reference<Stream>::Pointer_t pResult;

    if (mpTextBoxStories.get() != NULL)
    {
        sal_uInt32 nCount = mpTextBoxStories->getEntryCount();

        sal_uInt32 n = 0;
        while (n < nCount)
        {
            WW8FTXBXS * pTextboxStory = mpTextBoxStories->getEntryPointer(n);

            if (pTextboxStory->get_lid() == nShpId)
                break;

            ++n;
        }

        if (n < nCount)
        {
            Cp aCpStart(mpTextBoxStories->getFc(n));
            aCpStart += getEndnoteEndCp().getCp().get();
            CpAndFc aCpAndFcStart =
                mpPieceTable->createCpAndFc(aCpStart, PROP_DOC);
            Cp aCpEnd(mpTextBoxStories->getFc(n + 1));
            aCpEnd += getEndnoteEndCp().getCp().get();
            CpAndFc aCpAndFcEnd = mpPieceTable->createCpAndFc(aCpEnd, PROP_DOC);

            pResult = writerfilter::Reference<Stream>::Pointer_t
                (new WW8DocumentImpl(*this, aCpAndFcStart, aCpAndFcEnd));
        }
    }

    return pResult;
}

Id lcl_headerQName(sal_uInt32 nIndex)
{
    Id qName = NS_rtf::LN_header;

    if (nIndex > 5)
    {
        switch ((nIndex - 6) % 6)
        {
        case 0:
            qName = NS_rtf::LN_headerl;

            break;
        case 1:
            qName = NS_rtf::LN_headerr;

            break;
        case 2:
            qName = NS_rtf::LN_footerl;

            break;
        case 3:
            qName = NS_rtf::LN_footerr;

            break;
        case 4:
            qName = NS_rtf::LN_headerf;

            break;
        case 5:
            qName = NS_rtf::LN_footerf;

            break;
        }
    }

    return qName;
}

Fc WW8DocumentImpl::cp2fc(const Cp & cp) const
{
    return mpPieceTable->cp2fc(cp);
}

CpAndFc WW8DocumentImpl::getCpAndFc(const Cp & cp, PropertyType type) const
{
    Fc aFc = cp2fc(cp);

    return CpAndFc(cp, aFc, type);
}

void WW8DocumentImpl::resolvePicture(Stream & rStream)
{
    WW8Stream::Pointer_t pStream = getDataStream();

    if (pStream.get() != NULL)
    {
        WW8StructBase aStruct(*pStream, mfcPicLoc, 4);
        sal_uInt32 nCount = aStruct.getU32(0);

        {
            WW8PICF * pPicf = new WW8PICF(*pStream, mfcPicLoc, nCount);
            pPicf->setDocument(this);

            writerfilter::Reference<Properties>::Pointer_t pProps(pPicf);

            rStream.props(pProps);
        }
    }
}

void WW8DocumentImpl::resolveSpecialChar(sal_uInt32 nChar, Stream & rStream)
{
    switch (nChar)
    {
    case 0x1:
        resolvePicture(rStream);
        break;
    default:
        break;
    }
}

void WW8DocumentImpl::text(Stream & rStream, const sal_uInt8 * data, size_t len)
{
#ifdef DEBUG_ELEMENT
    OUString sText( (const sal_Char*) data, len, RTL_TEXTENCODING_MS_1252 );
    debug_logger->startElement("text");
    debug_logger->chars(OUStringToOString(sText, RTL_TEXTENCODING_ASCII_US).getStr());
    debug_logger->endElement();
#endif
    rStream.text(data, len);
}

void WW8DocumentImpl::utext(Stream & rStream, const sal_uInt8 * data, size_t len)
{
#ifdef DEBUG_ELEMENT
    debug_logger->startElement("utext");

    OUString sText;
    OUStringBuffer aBuffer = ::rtl:: OUStringBuffer(len);
    aBuffer.append( (const sal_Unicode *) data, len);
    sText = aBuffer.makeStringAndClear();

    debug_logger->chars(OUStringToOString(sText, RTL_TEXTENCODING_ASCII_US).getStr());
    debug_logger->endElement();
#endif
    rStream.utext(data, len);
}


void WW8DocumentImpl::resolveText(WW8DocumentIterator::Pointer_t pIt,
                                  Stream & rStream)
{
    WW8Stream::Sequence aSeq = pIt->getText();

    sal_uInt32 nCount = aSeq.getCount();
    bool bComplex = pIt->isComplex();

    /*
      Assumption: Special characters are always at the beginning or end of a
      run.
     */
    if (nCount > 0)
    {
        if (nCount == 1)
            bComplex = true;

        if (bComplex)
        {
            sal_uInt32 nStartIndex = 0;
            sal_uInt32 nEndIndex = nCount - 1;

            sal_uInt32 nCharFirst = aSeq[0];
            sal_uInt32 nCharLast = aSeq[nEndIndex];

            if (isSpecial(nCharFirst))
            {
                nStartIndex += 1;
                resolveSpecialChar(nCharFirst, rStream);
                text(rStream, &aSeq[0], 1);
            }

            if (!isSpecial(nCharLast))
                nEndIndex += 1;

            if (nStartIndex < nEndIndex)
            {
                sal_uInt32 nChars = nEndIndex - nStartIndex;
                text(rStream, &aSeq[nStartIndex], nChars);

                if (isSpecial(nCharLast))
                {
                    resolveSpecialChar(nCharLast, rStream);
                    text(rStream, &aSeq[nEndIndex], 1);
                }
            }
        }
        else
        {
            sal_uInt32 nStartIndex = 0;
            sal_uInt32 nEndIndex = nCount - 2;

            sal_uInt32 nCharFirst = aSeq[0] + (aSeq[1] << 8);
            sal_uInt32 nCharLast = aSeq[nEndIndex] + (aSeq[nEndIndex + 1]);

            if (isSpecial(nCharFirst))
            {
                nStartIndex += 2;
                resolveSpecialChar(nCharFirst, rStream);
                utext(rStream, &aSeq[0], 1);
            }

            if (!isSpecial(nCharLast))
                nEndIndex += 2;

            if (nStartIndex < nEndIndex)
            {
                sal_uInt32 nChars = (nEndIndex - nStartIndex) / 2;
                utext(rStream, &aSeq[nStartIndex], nChars);

                if (isSpecial(nCharLast))
                {
                    resolveSpecialChar(nCharLast, rStream);
                    utext(rStream, &aSeq[nEndIndex], 1);
                }
            }
        }
    }
}

void WW8DocumentImpl::startCharacterGroup(Stream & rStream)
{
    if (mbInCharacterGroup)
        endCharacterGroup(rStream);

#ifdef DEBUG_ELEMENT
    debug_logger->startElement("charactergroup");
#endif

    rStream.startCharacterGroup();
    mbInCharacterGroup = true;
}

void WW8DocumentImpl::endCharacterGroup(Stream & rStream)
{
#ifdef DEBUG_ELEMENT
    debug_logger->endElement();
#endif

    rStream.endCharacterGroup();
    mbInCharacterGroup = false;
}

void WW8DocumentImpl::startParagraphGroup(Stream & rStream)
{
    if (mbInParagraphGroup)
        endParagraphGroup(rStream);

#ifdef DEBUG_ELEMENT
    debug_logger->startElement("paragraphgroup");
#endif

    rStream.startParagraphGroup();
    mbInParagraphGroup = true;
}

void WW8DocumentImpl::endParagraphGroup(Stream & rStream)
{
    if (mbInCharacterGroup)
        endCharacterGroup(rStream);

#ifdef DEBUG_ELEMENT
    debug_logger->endElement();
#endif
    rStream.endParagraphGroup();
    mbInParagraphGroup = false;
}

void WW8DocumentImpl::startSectionGroup(Stream & rStream)
{
    if (mbInSection)
        endSectionGroup(rStream);

#ifdef DEBUG_ELEMENT
    debug_logger->startElement("sectiongroup");
#endif

    rStream.startSectionGroup();
    mbInSection = true;
}

void WW8DocumentImpl::endSectionGroup(Stream & rStream)
{
    if (mbInParagraphGroup)
        endParagraphGroup(rStream);

#ifdef DEBUG_ELEMENT
    debug_logger->endElement();
#endif
    rStream.endSectionGroup();
    mbInSection = false;
}

void WW8DocumentImpl::resolve(Stream & rStream)
{
    if (! bSubDocument)
    {
        output.addItem("<substream-names>");
        output.addItem(mpStream->getSubStreamNames());
        output.addItem("</substream-names>");

        if (mpDocStream.get() != NULL)
        {
            mpDocStream->dump(output);
        }

        if (mpSummaryInformationStream.get() != NULL)
        {
            mpSummaryInformationStream->dump(output);
        }

        writerfilter::Reference<Properties>::Pointer_t pFib
            (new WW8Fib(*mpFib));
        rStream.props(pFib);

        if (mpFibRgFcLcb2000.get() != NULL)
        {
            writerfilter::Reference<Properties>::Pointer_t pFibRgFcLcb2000
            (new WW8FibRgFcLcb2000(*mpFibRgFcLcb2000));
            rStream.props(pFibRgFcLcb2000);
        }

        if (mpFib->get_lcbPlcftxbxBkd() > 0)
        {
            PLCF<WW8BKD> aPLCF(*mpTableStream,
                               mpFib->get_fcPlcftxbxBkd(),
                               mpFib->get_lcbPlcftxbxBkd());
        }

        if (mpDffBlock.get() != NULL)
        {
            DffBlock * pTmp = new DffBlock(*mpDffBlock);
            writerfilter::Reference<Properties>::Pointer_t pDffBlock =
                writerfilter::Reference<Properties>::Pointer_t(pTmp);

            rStream.props(pDffBlock);
        }

        {
            rStream.info("headers");
            sal_uInt32 nHeaderCount = getHeaderCount();
            for (sal_uInt32 n = 0; n < nHeaderCount; ++n)
            {
                rStream.info(getHeaderCpAndFc(n).toString());
            }
            rStream.info("/headers");
        }

        writerfilter::Reference<Table>::Pointer_t pSttbRgtplc = getListTplcs();

        if (pSttbRgtplc.get() != NULL)
            rStream.table(NS_rtf::LN_SttbRgtplc, pSttbRgtplc);

        writerfilter::Reference<Table>::Pointer_t pFontTable = getFontTable();

        if (pFontTable.get() != NULL)
            rStream.table(NS_rtf::LN_FONTTABLE, pFontTable);

        try
        {
            writerfilter::Reference<Table>::Pointer_t pStyleSheet = getStyleSheet();

            if (pStyleSheet.get() != NULL)
                rStream.table(NS_rtf::LN_STYLESHEET, pStyleSheet);
        }
        catch (const Exception &e)
        {
            clog << e.getText() << endl;
        }

        writerfilter::Reference<Table>::Pointer_t pAssocTable = getAssocTable();

        if (pAssocTable.get() != NULL)
            rStream.table(NS_rtf::LN_SttbAssoc, pAssocTable);

        writerfilter::Reference<Table>::Pointer_t pListTable = getListTable();

        if (pListTable.get() != NULL)
            rStream.table(NS_rtf::LN_LISTTABLE, pListTable);

        writerfilter::Reference<Table>::Pointer_t pLFOTable = getLFOTable();

        if (pLFOTable.get() != NULL)
            rStream.table(NS_rtf::LN_LFOTABLE, pLFOTable);
    }

    WW8DocumentIterator::Pointer_t pIt = begin();
    WW8DocumentIterator::Pointer_t pItEnd = end();

    mbInParagraphGroup = false;
    mbInCharacterGroup = false;
    mbInSection = false;

    sal_uInt32 nSectionIndex = 0;

    rStream.info(pIt->toString());
    rStream.info(pItEnd->toString());

    while (! pIt->equal(*pItEnd))
    {
        writerfilter::Reference<Properties>::Pointer_t
            pProperties(pIt->getProperties());

        switch (pIt->getPropertyType())
        {
        case PROP_FOOTNOTE:
            {
                rStream.info(pIt->toString());
                writerfilter::Reference<Stream>::Pointer_t
                    pFootnote(pIt->getSubDocument());

                if (pFootnote.get() != NULL)
                {
#ifdef DEBUG_ELEMENT
                    debug_logger->startElement("substream");
#endif
                    rStream.substream(NS_rtf::LN_footnote, pFootnote);
#ifdef DEBUG_ELEMENT
                    debug_logger->endElement();
#endif
                }
            }
            break;
        case PROP_ENDNOTE:
            {
                rStream.info(pIt->toString());
                writerfilter::Reference<Stream>::Pointer_t
                    pEndnote(pIt->getSubDocument());

                if (pEndnote.get() != NULL)
                {
#ifdef DEBUG_ELEMENT
                    debug_logger->startElement("substream");
#endif
                    rStream.substream(NS_rtf::LN_endnote, pEndnote);
#ifdef DEBUG_ELEMENT
                    debug_logger->endElement();
#endif
                }
            }
            break;
        case PROP_ANNOTATION:
            {
                rStream.info(pIt->toString());
                writerfilter::Reference<Stream>::Pointer_t
                    pAnnotation(pIt->getSubDocument());

                if (pAnnotation.get() != NULL)
                {
#ifdef DEBUG_ELEMENT
                    debug_logger->startElement("substream");
#endif
                    rStream.substream(NS_rtf::LN_annotation, pAnnotation);
#ifdef DEBUG_ELEMENT
                    debug_logger->endElement();
#endif
                }
            }
            break;
        case PROP_CHP:
            {
                startCharacterGroup(rStream);
            }

            break;
        case PROP_PAP:
            {
                startParagraphGroup(rStream);
                rStream.info(pIt->toString());
            }

            break;
        case PROP_SEC:
            {
                startSectionGroup(rStream);
                rStream.info(pIt->toString());

                if (nSectionIndex == 0)
                    rStream.props(getDocumentProperties());

                sal_uInt32 nHeaderStartIndex = 6 + nSectionIndex * 6;
                sal_uInt32 nHeaderEndIndex = nHeaderStartIndex + 6;

                if (nHeaderStartIndex >= getHeaderCount())
                    nHeaderStartIndex = getHeaderCount();

                if (nHeaderEndIndex >= getHeaderCount())
                    nHeaderEndIndex = getHeaderCount();

                for (sal_uInt32 n = nHeaderStartIndex; n < nHeaderEndIndex; ++n)
                {
                    writerfilter::Reference<Stream>::Pointer_t
                        pHeader(getHeader(n));

                    Id qName = lcl_headerQName(n);

                    if (pHeader.get() != NULL)
                        rStream.substream(qName, pHeader);
                }

                ++nSectionIndex;
            }

            break;
        default:
            rStream.info(pIt->toString());
        }

        if (pProperties.get() != NULL)
        {
#ifdef DEBUG_PROPERTIES
            debug_logger->propertySet(pProperties,
                    IdToString::Pointer_t(new WW8IdToString()));
#endif

            rStream.props(pProperties);
        }

        if (pIt->getPropertyType() == PROP_PAP)
        {
            startCharacterGroup(rStream);
        }

        resolveText(pIt, rStream);

        ++(*pIt);
    }

    if (mbInCharacterGroup)
        endCharacterGroup(rStream);

    if (mbInParagraphGroup)
        endParagraphGroup(rStream);

    if (mbInSection)
        endSectionGroup(rStream);

}

writerfilter::Reference<Properties>::Pointer_t
WW8SED::get_sepx()
{
    writerfilter::Reference<Properties>::Pointer_t pResult;

    if (get_fcSepx() != 0xffffffff)
    {
        WW8StructBase aTmp(*mpDoc->getDocStream(), get_fcSepx(), 2);
        pResult = writerfilter::Reference<Properties>::Pointer_t
            (new WW8PropertySetImpl
             (*mpDoc->getDocStream(), get_fcSepx() + 2,
              (sal_uInt32) aTmp.getU16(0), false));
    }

    return pResult;
}

void WW8DocumentImpl::insertCpAndFc(const CpAndFc & rCpAndFc)
{
    mCpAndFcs.insert(rCpAndFc);
}

string propertyTypeToString(PropertyType nType)
{
    string result;

    switch (nType)
    {
    case PROP_SHP:
        result = "SHP";

        break;
    case PROP_FLD:
        result = "FLD";

        break;
    case PROP_BOOKMARKSTART:
        result = "BOOKMARKSTART";

        break;
    case PROP_BOOKMARKEND:
        result = "BOOKMARKEND";

        break;
    case PROP_ENDNOTE:
        result = "ENDNOTE";

        break;
    case PROP_FOOTNOTE:
        result = "FOOTNOTE";

        break;
    case PROP_ANNOTATION:
        result = "ANNOTATION";

        break;
    case PROP_DOC:
        result = "DOC";

        break;

    case PROP_SEC:
        result = "SEC";

        break;

    case PROP_PAP:
        result = "PAP";

        break;

    case PROP_CHP:
        result = "CHP";

        break;
    default:
        break;
    }

    return result;
}

string CpAndFc::toString() const
{
    string result;

    result += "(";
    result += getCp().toString();
    result += ", ";
    result += getFc().toString();
    result += ", ";

    result += propertyTypeToString(getType());

    result += ")";

    return result;
}


// Bookmark

Bookmark::Bookmark(writerfilter::Reference<Properties>::Pointer_t pBKF,
                   OUString & rName)
: mpBKF(pBKF), mName(rName)
{
}

void Bookmark::resolve(Properties & rHandler)
{
    mpBKF->resolve(rHandler);

    WW8Value::Pointer_t pValue = createValue(mName);
    rHandler.attribute(NS_rtf::LN_BOOKMARKNAME, *pValue);
}

string Bookmark::getType() const
{
    return "Bookmark";
}

// BookmarkHelper

CpAndFc BookmarkHelper::getStartCpAndFc(sal_uInt32 nPos)
{
    Cp aCp(mpStartCps->getFc(nPos));
    Fc aFc(mpPieceTable->cp2fc(aCp));
    CpAndFc aCpAndFc(aCp, aFc, PROP_BOOKMARKSTART);

    return aCpAndFc;
}

CpAndFc BookmarkHelper::getEndCpAndFc(sal_uInt32 nPos)
{
    Cp aCp(mpEndCps->getU32(nPos * 4));
    Fc aFc(mpPieceTable->cp2fc(aCp));
    CpAndFc aCpAndFc(aCp, aFc, PROP_BOOKMARKEND);

    return aCpAndFc;
}

OUString BookmarkHelper::getName(sal_uInt32 nPos)
{
    return mpNames->getEntry(nPos);
}

sal_uInt32 BookmarkHelper::getIndex(const CpAndFc & rCpAndFc)
{
    sal_uInt32 nResult = mpStartCps->getEntryCount();

    sal_uInt32 nCp = rCpAndFc.getCp().get();

    sal_uInt32 n;
    switch (rCpAndFc.getType())
    {
    case PROP_BOOKMARKSTART:
        {
            sal_uInt32 nStartsCount = mpStartCps->getEntryCount();

            for (n = 0; n < nStartsCount; ++n)
            {
                if (nCp == mpStartCps->getFc(n))
                {
                    nResult = n;

                    break;
                }
            }

            if (n == nStartsCount)
                throw ExceptionNotFound("BookmarkHelper::getIndex");
        }

        break;

    case PROP_BOOKMARKEND:
        {
            sal_uInt32 nEndsCount = mpEndCps->getCount() / 4;
            sal_uInt32 nIndex = nEndsCount;

            for (n = 0; n < nEndsCount; ++n)
            {
                if (nCp == mpEndCps->getU16(n * 4))
                {
                    nIndex = n;

                    break;
                }
            }

            if (n == nEndsCount)
                throw ExceptionNotFound("BookmarkHelper::getIndex");

            {
                {
                    sal_uInt32 nStartsCount = mpStartCps->getEntryCount();
                    for (n = 0; n < nStartsCount; ++n)
                    {
                        WW8BKF::Pointer_t pBKF(mpStartCps->getEntry(n));

                        if (pBKF->get_ibkl() ==
                            sal::static_int_cast<sal_Int32>(nIndex))
                        {
                            nResult = n;

                            break;
                        }
                    }

                    if (n == nStartsCount)
                        throw ExceptionNotFound("BookmarkHelper::getIndex");
                }
            }
        }

        break;
    default:
        break;
    }

    return nResult;
}

void BookmarkHelper::init()
{
    {
        sal_uInt32 nStartsCount = mpStartCps->getEntryCount();

        for (sal_uInt32 n = 0; n < nStartsCount; ++n)
            mpDoc->insertCpAndFc(getStartCpAndFc(n));
    }

    {
        sal_uInt32 nEndsCount = mpEndCps->getCount() / 4;

        for (sal_uInt32 n = 0; n < nEndsCount; ++n)
            mpDoc->insertCpAndFc(getEndCpAndFc(n));
    }
}

writerfilter::Reference<Properties>::Pointer_t
BookmarkHelper::getBKF(const CpAndFc & rCpAndFc)
{
    sal_uInt32 nIndex = getIndex(rCpAndFc);

    return writerfilter::Reference<Properties>::Pointer_t
        (mpStartCps->getEntryPointer(nIndex));
}

writerfilter::Reference<Properties>::Pointer_t
BookmarkHelper::getBookmark(const CpAndFc & rCpAndFc)
{
    writerfilter::Reference<Properties>::Pointer_t pResult;

    try
    {
        OUString aName = getName(rCpAndFc);

        pResult = writerfilter::Reference<Properties>::Pointer_t
            (new Bookmark(getBKF(rCpAndFc), aName));
    }
    catch (const ExceptionNotFound &e)
    {
        clog << e.getText() << endl;
    }

    return pResult;
}

OUString BookmarkHelper::getName(const CpAndFc & rCpAndFc)
{
    OUString sResult;

    sal_uInt32 nIndex = getIndex(rCpAndFc);

    sResult = getName(nIndex);

    return sResult;
}

template <class T, class Helper>
struct ProcessPLCF2Map
{
    void process(typename PLCF<T>::Pointer_t pPlcf,
                 typename Helper::Map_t & rMap,
                 PropertyType type,
                 WW8DocumentImpl * pDoc)
    {
        if (pPlcf.get() != NULL)
        {
            sal_uInt32 nCount = pPlcf->getEntryCount();

            for (sal_uInt32 n = 0; n < nCount; n++)
            {
                Cp aCp(pPlcf->getFc(n));
                CpAndFc aCpAndFc(pDoc->getCpAndFc(aCp, type));
                typename T::Pointer_t pT = pPlcf->getEntry(n);

                rMap[aCpAndFc] = pT;
            }
        }
    }
};

FieldHelper::FieldHelper(PLCF<WW8FLD>::Pointer_t pPlcffldMom,
                         WW8DocumentImpl * pDoc)
: mpDoc(pDoc)
{
    ProcessPLCF2Map<WW8FLD, FieldHelper> process;
    process.process(pPlcffldMom, mMap, PROP_FLD, pDoc);
}

void FieldHelper::init()
{
    Map_t::iterator aIt;

    for (aIt = mMap.begin(); aIt != mMap.end(); ++aIt)
    {
        mpDoc->insertCpAndFc(aIt->first);
    }
}

WW8FLD::Pointer_t FieldHelper::getWW8FLD(const CpAndFc & rCpAndFc)
{
    WW8FLD::Pointer_t pFld = mMap[rCpAndFc];

    return pFld;
}

writerfilter::Reference<Properties>::Pointer_t
FieldHelper::getField(const CpAndFc & rCpAndFc)
{
    WW8FLD::Pointer_t pFLD = getWW8FLD(rCpAndFc);

    return writerfilter::Reference<Properties>::Pointer_t
        (new WW8FLD(*pFLD));
}

ShapeHelper::ShapeHelper(PLCF<WW8FSPA>::Pointer_t pPlcspaMom,
                         PLCF<WW8FSPA>::Pointer_t pPlcspaHdr,
                         WW8DocumentImpl * pDoc)
: mpDoc(pDoc)
{
    ProcessPLCF2Map<WW8FSPA, ShapeHelper> process;
    process.process(pPlcspaMom, mMap, PROP_SHP, pDoc);
    process.process(pPlcspaHdr, mMap, PROP_SHP, pDoc);
}

void ShapeHelper::init()
{
    Map_t::iterator aIt;

    for (aIt = mMap.begin(); aIt != mMap.end(); ++aIt)
    {
        mpDoc->insertCpAndFc(aIt->first);
        aIt->second->setDocument(mpDoc);
    }
}

writerfilter::Reference<Properties>::Pointer_t
ShapeHelper::getShape(const CpAndFc & rCpAndFc)
{
    WW8FSPA::Pointer_t pFSPA = mMap[rCpAndFc];

    return writerfilter::Reference<Properties>::Pointer_t
        (new WW8FSPA(*pFSPA));
}

BreakHelper::BreakHelper(PLCF<WW8BKD>::Pointer_t pPlcfbkdMom,
                         WW8DocumentImpl * pDoc)
: mpDoc(pDoc)
{
    ProcessPLCF2Map<WW8BKD, BreakHelper> process;
    process.process(pPlcfbkdMom, mMap, PROP_BRK, pDoc);
}

void BreakHelper::init()
{
    Map_t::iterator aIt;

    for (aIt = mMap.begin(); aIt != mMap.end(); ++aIt)
    {
        mpDoc->insertCpAndFc(aIt->first);
    }
}

writerfilter::Reference<Properties>::Pointer_t
BreakHelper::getBreak(const CpAndFc & rCpAndFc)
{
    WW8BKD::Pointer_t pBKD = mMap[rCpAndFc];

    return writerfilter::Reference<Properties>::Pointer_t
        (new WW8BKD(*pBKD));
}


}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
