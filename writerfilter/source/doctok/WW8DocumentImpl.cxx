#include <doctok/exceptions.hxx>
#include <WW8DocumentImpl.hxx>
#include <WW8FKPImpl.hxx>
#include <WW8PieceTableImpl.hxx>
#include <WW8BinTableImpl.hxx>
#include <WW8StreamImpl.hxx>
#include <WW8Sttbf.hxx>
#include <Dff.hxx>
#include <iterator>
#include <XNoteHelperImpl.hxx>
#include <WW8OutputWithDepth.hxx>

namespace doctok
{

using namespace ::std;
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

doctok::Reference<Properties>::Pointer_t
WW8DocumentIteratorImpl::getProperties() const
{
    return mpDocument->getProperties(mCpAndFc);
}

doctok::Reference<Stream>::Pointer_t
WW8DocumentIteratorImpl::getSubDocument() const
{
    return mpDocument->getSubDocument(mCpAndFc);
}

WW8SED * WW8DocumentIteratorImpl::getSED() const
{
    return mpDocument->getSED(mCpAndFc);
}

WW8Stream::Sequence WW8DocumentIteratorImpl::getText()
{
    return mpDocument->getText(mCpAndFc);
}

doctok::Reference<Properties>::Pointer_t
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

WW8DocumentImpl::~WW8DocumentImpl()
{
}

WW8DocumentImpl::WW8DocumentImpl(WW8Stream::Pointer_t rpStream)
: bSubDocument(false), mfcPicLoc(0), mpStream(rpStream)
{
    mpDocStream = getSubStream(::rtl::OUString::createFromAscii
                               ("WordDocument"));

    try
    {
        mpDataStream = getSubStream(::rtl::OUString::createFromAscii
                                    ("Data"));
    }
    catch (ExceptionNotFound e)
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
        mpTableStream = getSubStream(::rtl::OUString::createFromAscii
                                     ("0Table"));

        break;

    case 1:
        mpTableStream = getSubStream(::rtl::OUString::createFromAscii
                                     ("1Table"));

        break;

    default:
        break;
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

    //clog << "BinTable(CHP):" << mpBinTableCHPX->toString();

    parseBinTableCpAndFcs(*mpBinTableCHPX, PROP_CHP);

    mpSEDs = PLCF<WW8SED>::Pointer_t(new PLCF<WW8SED>
                                     (*mpTableStream,
                                      mpFib->get_fcPlcfsed(),
                                      mpFib->get_lcbPlcfsed()));

    //mpSEDs->dump(clog);

    {
        sal_uInt32 nSEDs = mpSEDs->getEntryCount();
        for (sal_uInt32 n = 0; n < nSEDs; ++n)
        {
            Cp aCp(mpSEDs->getFc(n));
            CpAndFc aCpAndFc(aCp, mpPieceTable->cp2fc(aCp), PROP_SEC);

            mCpAndFcs.insert(aCpAndFc);
        }
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

    if (mpFib->get_fcDggInfo() != 0 && mpFib->get_lcbDggInfo() > 0)
    {
        mpDffBlock = DffBlock::Pointer_t
            (new DffBlock(*mpTableStream, mpFib->get_fcDggInfo(),
                         mpFib->get_lcbDggInfo(), 1));
    }

    mpShapeHelper = ShapeHelper::Pointer_t
        (new ShapeHelper(pPlcspaMom, pPlcspaHdr, this));

    mpShapeHelper->init();

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
        if (nChar <= 16)
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
: bSubDocument(true), mfcPicLoc(0)
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
    mpDocStream = rSrc.mpDocStream;

    mpPieceTable = rSrc.mpPieceTable;

    mpBinTableCHPX = rSrc.mpBinTableCHPX;
    mpBinTablePAPX = rSrc.mpBinTablePAPX;

    mpSEDs = rSrc.mpSEDs;

    mpFib = rSrc.mpFib;

    mpHeaderOffsets = rSrc.mpHeaderOffsets;
    mpFootnoteHelper = rSrc.mpFootnoteHelper;
    mpEndnoteHelper = rSrc.mpEndnoteHelper;
    mpAnnotationHelper = rSrc.mpAnnotationHelper;

    mpBookmarkHelper = rSrc.mpBookmarkHelper;

    mpDffBlock = rSrc.mpDffBlock;

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
    //clog << "<bintable type=\"" << propertyTypeToString(eType_) << "\">" << endl;
    for (sal_uInt32 i = 0; i < rTable.getEntryCount(); i++)
    {
#if 0
        char sBuffer[255];
        snprintf(sBuffer, 255, "%ld", i);
        char sBufferPageNum[255];
        snprintf(sBufferPageNum, 255, "%ld", rTable.getPageNumber(i));
#endif
        Fc aFcFromTable(rTable.getFc(i));

        if (aFcFromTable < mpPieceTable->getFirstFc())
            aFcFromTable = mpPieceTable->getFirstFc();

        bool bComplex = mpPieceTable->isComplex(aFcFromTable);
        aFcFromTable.setComplex(bComplex);

        //clog << "<entry fc=\"" << aFcFromTable.toString() << "\">" << endl;

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

                //clog << "<fkpentry fc=\"" << aFc.toString() << "\"/>" << endl;

                try
                {
                    Cp aCp = mpPieceTable->fc2cp(aFc);

                    CpAndFc aCpAndFc(aCp, aFc, eType_);

                    mCpAndFcs.insert(aCpAndFc);

                    //clog << aCpAndFc << endl;
                }
                catch (ExceptionNotFound e)
                {
                    clog << e.getText() << endl;
                }
            }
        }
        catch (ExceptionNotFound e)
        {
            clog << e.getText() << endl;
        }

        //clog << "</entry>" << endl;
    }

    //clog << "</bintable>" << endl;
}

WW8Stream::Pointer_t WW8DocumentImpl::getSubStream
(const ::rtl::OUString & sId) const
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

CpAndFc WW8DocumentImpl::getTextboxEndCp() const
{
    return mTextboxEndCpAndFc;
}

CpAndFc WW8DocumentImpl::getTextboxHeaderEndCp() const
{
    return mTextboxHeaderEndCpAndFc;
}

CpAndFc WW8DocumentImpl::getNextCp(const CpAndFc & rCpAndFc) const
{
    CpAndFc aResult = mCpAndFcEnd;
    CpAndFcs::const_iterator aIt = mCpAndFcs.find(rCpAndFc);

    if (aIt != mCpAndFcs.end())
    {
        aIt++;

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
        aIt--;

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

doctok::Reference<Properties>::Pointer_t WW8DocumentImpl::getProperties
(const CpAndFc & rCpAndFc)
{
    doctok::Reference<Properties>::Pointer_t pResult;

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
            catch (ExceptionOutOfBounds e)
            {
            }
        }

        break;

    case PROP_SEC:
        {
            pResult = doctok::Reference<Properties>::Pointer_t
                (getSED(rCpAndFc));
        }

        break;

    case PROP_FOOTNOTE:
        {
            pResult = doctok::Reference<Properties>::Pointer_t
                (mpFootnoteHelper->getRef(rCpAndFc));
        }
        break;

    case PROP_ENDNOTE:
        {
            pResult = doctok::Reference<Properties>::Pointer_t
                (mpEndnoteHelper->getRef(rCpAndFc));
        }
        break;

    case PROP_ANNOTATION:
        {
            pResult = doctok::Reference<Properties>::Pointer_t
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
        }

        break;
    case PROP_SHP:
        {
            pResult = getShape(rCpAndFc);
        }
    default:
        break;
    }

    return pResult;
}

doctok::Reference<Stream>::Pointer_t
WW8DocumentImpl::getSubDocument(const CpAndFc & rCpAndFc)
{
    doctok::Reference<Stream>::Pointer_t pResult;

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

doctok::Reference<Table>::Pointer_t WW8DocumentImpl::getListTable() const
{
    doctok::Reference<Table>::Pointer_t pResult;

    if (mpFib->get_fcPlcfLst() != 0 && mpFib->get_lcbPlcfLst() > 0)
    {
        WW8ListTable * pList = new WW8ListTable(*mpTableStream,
                                                mpFib->get_fcPlcfLst(),
                                                mpFib->get_fcPlfLfo() -
                                                mpFib->get_fcPlcfLst());

        pList->setPayloadOffset(mpFib->get_lcbPlcfLst());
        pList->initPayload();

        pResult = doctok::Reference<Table>::Pointer_t(pList);
    }

    return pResult;
}

doctok::Reference<Table>::Pointer_t WW8DocumentImpl::getLFOTable() const
{
    doctok::Reference<Table>::Pointer_t pResult;

    if (mpFib->get_fcPlfLfo() != 0 && mpFib->get_lcbPlfLfo() > 0)
    {
        try
        {
            WW8LFOTable * pLFOs = new WW8LFOTable(*mpTableStream,
                                                  mpFib->get_fcPlfLfo(),
                                                  mpFib->get_lcbPlfLfo());

            pLFOs->setPayloadOffset(mpFib->get_lcbPlcfLst());
            pLFOs->initPayload();

            pResult = doctok::Reference<Table>::Pointer_t(pLFOs);
        }
        catch (Exception e)
        {
            clog << e.getText() << endl;
        }
    }

    return pResult;
}

doctok::Reference<Table>::Pointer_t WW8DocumentImpl::getFontTable() const
{
    doctok::Reference<Table>::Pointer_t pResult;

    if (mpFib->get_fcSttbfffn() != 0 && mpFib->get_lcbSttbfffn() > 0)
    {
        WW8FontTable * pFonts = new WW8FontTable(*mpTableStream,
                                                 mpFib->get_fcSttbfffn(),
                                                 mpFib->get_lcbSttbfffn());

        pFonts->initPayload();

        pResult = doctok::Reference<Table>::Pointer_t(pFonts);
    }

    return pResult;
}

doctok::Reference<Table>::Pointer_t WW8DocumentImpl::getStyleSheet() const
{
    doctok::Reference<Table>::Pointer_t pResult;

    if (mpFib->get_lcbStshf() > 0)
    {
        WW8StyleSheet * pStyles = new WW8StyleSheet(*mpTableStream,
                                                    mpFib->get_fcStshf(),
                                                    mpFib->get_lcbStshf());

        pStyles->initPayload();

        pResult = doctok::Reference<Table>::Pointer_t(pStyles);
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

    // never reached
    return CpAndFc();
}

doctok::Reference<Stream>::Pointer_t WW8DocumentImpl::getHeader(sal_uInt32 nPos)
{
    // There are getHeaderCount() headers => greater or equal
    if (nPos >= getHeaderCount())
        throw ExceptionNotFound("getHeader");

    doctok::Reference<Stream>::Pointer_t pResult;

    CpAndFc aCpAndFcStart(getHeaderCpAndFc(nPos));
    CpAndFc aCpAndFcEnd(getHeaderCpAndFc(nPos + 1));

    sal_uInt32 nEquals = 1;
    while (aCpAndFcEnd == aCpAndFcStart && nPos + nEquals < getHeaderCount())
    {
        ++nEquals;

        aCpAndFcEnd = getHeaderCpAndFc(nPos + nEquals);
    }

    if (aCpAndFcStart < aCpAndFcEnd)
        pResult = doctok::Reference<Stream>::Pointer_t
            (new WW8DocumentImpl(*this, aCpAndFcStart, aCpAndFcEnd));

    return pResult;
}

sal_uInt32 WW8DocumentImpl::getFootnoteCount() const
{
    return (mpFootnoteHelper.get() != NULL) ? mpFootnoteHelper->getCount() : 0;
}

doctok::Reference<Stream>::Pointer_t
WW8DocumentImpl::getFootnote(sal_uInt32 nPos)
{
    doctok::Reference<Stream>::Pointer_t pResult;

    if (! bSubDocument)
        pResult = mpFootnoteHelper->get(nPos);

    return pResult;
}

doctok::Reference<Stream>::Pointer_t
WW8DocumentImpl::getFootnote(const CpAndFc & rCpAndFc)
{
    doctok::Reference<Stream>::Pointer_t pResult;

    if (! bSubDocument)
        pResult = mpFootnoteHelper->get(rCpAndFc);

    return pResult;
}

sal_uInt32 WW8DocumentImpl::getEndnoteCount() const
{
    return mpEndnoteHelper.get() != NULL ? mpEndnoteHelper->getCount() : 0;
}

doctok::Reference<Stream>::Pointer_t
WW8DocumentImpl::getEndnote(sal_uInt32 nPos)
{
    doctok::Reference<Stream>::Pointer_t pResult;

    if (! bSubDocument)
        pResult = mpEndnoteHelper->get(nPos);

    return pResult;
}

doctok::Reference<Stream>::Pointer_t
WW8DocumentImpl::getEndnote(const CpAndFc & rCpAndFc)
{
    doctok::Reference<Stream>::Pointer_t pResult;

    if (! bSubDocument)
        pResult = mpEndnoteHelper->get(rCpAndFc);

    return pResult;
}

sal_uInt32 WW8DocumentImpl::getAnnotationCount() const
{
    return mpAnnotationHelper.get() != NULL ?
        mpAnnotationHelper->getCount() : 0;
}

doctok::Reference<Stream>::Pointer_t
WW8DocumentImpl::getAnnotation(sal_uInt32 nPos)
{
    doctok::Reference<Stream>::Pointer_t pResult;

    if (! bSubDocument)
        pResult = mpAnnotationHelper->get(nPos);

    return pResult;
}

doctok::Reference<Stream>::Pointer_t
WW8DocumentImpl::getAnnotation(const CpAndFc & rCpAndFc)
{
    doctok::Reference<Stream>::Pointer_t pResult;

    if (! bSubDocument)
        pResult = mpAnnotationHelper->get(rCpAndFc);

    return pResult;
}

doctok::Reference<Properties>::Pointer_t
WW8DocumentImpl::getBookmark(const CpAndFc & rCpAndFc) const
{
    return mpBookmarkHelper->getBookmark(rCpAndFc);
}

doctok::Reference<Properties>::Pointer_t
WW8DocumentImpl::getShape(const CpAndFc & rCpAndFc) const
{
    return mpShapeHelper->getShape(rCpAndFc);
}

doctok::Reference<Properties>::Pointer_t
WW8DocumentImpl::getShape(sal_uInt32 nSpid)
{
    DffRecord * pTmp = new DffRecord(*mpDffBlock->getShape(nSpid));
    return doctok::Reference<Properties>::Pointer_t(pTmp);
}

doctok::Reference<Properties>::Pointer_t
WW8DocumentImpl::getField(const CpAndFc & rCpAndFc) const
{
    return mpFieldHelper->getField(rCpAndFc);
}

sal_uInt32 WW8DocumentImpl::getPicLocation() const
{
    return mfcPicLoc;
}

void WW8DocumentImpl::setPicLocation(sal_uInt32 fcPicLoc)
{
    mfcPicLoc = fcPicLoc;
}

QName_t lcl_headerQName(sal_uInt32 nIndex)
{
    QName_t qName = NS_rtf::LN_header;

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

Cp WW8DocumentImpl::fc2cp(const Fc & fc) const
{
    return mpPieceTable->fc2cp(fc);
}

CpAndFc WW8DocumentImpl::getCpAndFc(const Cp & cp, PropertyType type) const
{
    Fc aFc = cp2fc(cp);

    return CpAndFc(cp, aFc, type);
}

CpAndFc WW8DocumentImpl::getCpAndFc(const Fc & fc, PropertyType type) const
{
    Cp aCp = fc2cp(fc);

    return CpAndFc(aCp, fc, type);
}

void WW8DocumentImpl::resolvePicture(Stream & rStream)
{
    WW8Stream::Pointer_t pStream = getDataStream();

    if (pStream.get() != NULL)
    {
        WW8StructBase aStruct(*pStream, mfcPicLoc, 4);
        sal_uInt32 nCount = aStruct.getU32(0);

        doctok::Reference<Properties>::Pointer_t pPicf
            (new WW8PICF(*pStream, mfcPicLoc, nCount));

        rStream.props(pPicf);
    }
}

void WW8DocumentImpl::resolveText(WW8DocumentIterator::Pointer_t pIt,
                                  Stream & rStream)
{
    WW8Stream::Sequence aSeq = pIt->getText();

    sal_uInt32 nCount = aSeq.getCount();
    bool bComplex = pIt->isComplex();

#if 1
    /*
      Assumption: Special characters are always at the end of a run.
     */
    if (nCount > 0)
    {
        if (nCount == 1)
            bComplex = true;

        sal_uInt32 nIndex = nCount - (bComplex ? 1 : 2);
        sal_uInt32 nChar = aSeq[nIndex];

        if (! bComplex)
            nChar += aSeq[nIndex + 1] << 8;

        if (isSpecial(nChar))
        {
            if (bComplex)
            {
                rStream.text(&aSeq[0], nIndex);
                rStream.text(&aSeq[nIndex], 1);
            }
            else
            {
                rStream.utext(&aSeq[0], nIndex / 2);
                rStream.utext(&aSeq[nIndex], 1);
            }

            switch (nChar)
            {
            case 0x1:
                resolvePicture(rStream);
                break;
            default:
                break;
            }
        }
        else
        {
            if (bComplex)
                rStream.text(&aSeq[0], nCount);
            else
                rStream.utext(&aSeq[0], nCount/2);

        }
    }
#else
    sal_uInt32 nIndex = 0;

    while (nIndex < nCount)
    {
        sal_uInt32 nChar = aSeq[nIndex];

        if (! bComplex)
            nChar += aSeq[nIndex + 1] << 8;

        if (isSpecial(nChar))
        {
            if (bComplex)
            {
                if (nStart < nIndex)
                    rStream.text(&aSeq[nStart], nIndex - nStart);
                rStream.text(&aSeq[nIndex], 1);
            }
            else
            {
                if (nStart < nIndex)
                    rStream.utext(&aSeq[nStart], (nIndex - nStart) / 2);

                rStream.utext(&aSeq[nIndex], 1);
            }

            nStart = nIndex + (bComplex ? 1 : 2);
        }

        nIndex += bComplex ? 1 : 2;
    }

    if (nStart < nCount)
    {
        if (bComplex)
            rStream.text(&aSeq[nStart], nCount - nStart);
        else
            rStream.utext(&aSeq[nStart], (nCount - nStart) / 2);
    }
#endif
}

void WW8DocumentImpl::resolve(Stream & rStream)
{
    if (! bSubDocument)
    {
        //mpPieceTable->dump(clog);

        //copy(mCpAndFcs.begin(), mCpAndFcs.end(), ostream_iterator<CpAndFc>(clog, ", "));

        doctok::Reference<Properties>::Pointer_t pFib
            (new WW8Fib(*mpFib));
        rStream.props(pFib);

        if (mpFib->get_lcbPlcftxbxTxt() > 0)
        {
            WW8StructBase aStructBase(*mpTableStream, mpFib->get_fcPlcftxbxTxt(),
                                      mpFib->get_lcbPlcftxbxTxt());

            WW8OutputWithDepth output;
            output.addItem("<textboxes>");
            aStructBase.dump(output);
            output.addItem("</textboxes>");
        }

        if (mpFib->get_lcbPlcftxbxBkd() > 0)
        {
            PLCF<WW8BKD> aPLCF(*mpTableStream,
                               mpFib->get_fcPlcftxbxBkd(),
                               mpFib->get_lcbPlcftxbxBkd());

            WW8OutputWithDepth output;
            output.addItem("<textboxes.breaks>");
            aPLCF.dump(output);
            output.addItem("</textboxes.breaks>");
        }

        if (mpDffBlock.get() != NULL)
        {
            DffBlock * pTmp = new DffBlock(*mpDffBlock);
            //pTmp->dump(clog);
            doctok::Reference<Properties>::Pointer_t pDffBlock =
                doctok::Reference<Properties>::Pointer_t(pTmp);

            rStream.props(pDffBlock);
        }

#if 0
        {
            sal_uInt32 nHeaderCount = getHeaderCount();
            for (sal_uInt32 n = 0; n < nHeaderCount; ++n)
            {
#if 0
                clog << "<header num=\"" << n << "\" cp=\""
                     << getHeaderCpAndFc(n) << "\"/>" << endl;
#endif
                doctok::Reference<Stream>::Pointer_t pHeader(getHeader(n));

                QName_t qName = lcl_headerQName(n);

                if (pHeader.get() != NULL)
                    rStream.substream(qName, pHeader);
            }
        }

        {
            sal_uInt32 nFootnoteCount = getFootnoteCount();
            for (sal_uInt32 n = 0; n < nFootnoteCount; ++n)
            {
                //clog << "<footnote num=\"" << n << "\"/>" << endl;

                doctok::Reference<Stream>::Pointer_t pFootnote(getFootnote(n));

                if (pFootnote.get() != NULL)
                    rStream.substream(NS_rtf::LN_footnote, pFootnote);
            }
        }
        {
            sal_uInt32 nEndnoteCount = getEndnoteCount();
            for (sal_uInt32 n = 0; n < nEndnoteCount; ++n)
            {
                //clog << "<endnote num=\"" << n << "\"/>" << endl;

                doctok::Reference<Stream>::Pointer_t pEndnote(getEndnote(n));

                if (pEndnote.get() != NULL)
                    rStream.substream(NS_rtf::LN_endnote, pEndnote);
            }
        }
#endif


        doctok::Reference<Table>::Pointer_t pListTable = getListTable();

        if (pListTable.get() != NULL)
            rStream.table(NS_rtf::LN_LISTTABLE, pListTable);

        doctok::Reference<Table>::Pointer_t pLFOTable = getLFOTable();

        if (pLFOTable.get() != NULL)
            rStream.table(NS_rtf::LN_LFOTABLE, pLFOTable);

        doctok::Reference<Table>::Pointer_t pFontTable = getFontTable();

        if (pFontTable.get() != NULL)
            rStream.table(NS_rtf::LN_FONTTABLE, pFontTable);

        try
        {
            doctok::Reference<Table>::Pointer_t pStyleSheet = getStyleSheet();

            if (pStyleSheet.get() != NULL)
                rStream.table(NS_rtf::LN_STYLESHEET, pStyleSheet);
        }
        catch (Exception e)
        {
            clog << e.getText() << endl;
        }
    }

    WW8DocumentIterator::Pointer_t pIt = begin();
    WW8DocumentIterator::Pointer_t pItEnd = end();

    bool bInParagraphGroup = false;
    bool bInCharacterGroup = false;
    bool bInSection = false;

    sal_uInt32 nSectionIndex = 0;

    rStream.info(pIt->toString());
    rStream.info(pItEnd->toString());

    while (! pIt->equal(*pItEnd))
    {
        Reference<doctok::Properties>::Pointer_t
            pProperties(pIt->getProperties());

        switch (pIt->getPropertyType())
        {
        case PROP_FOOTNOTE:
            {
                rStream.info(pIt->toString());
                doctok::Reference<Stream>::Pointer_t
                    pFootnote(pIt->getSubDocument());

                if (pFootnote.get() != NULL)
                    rStream.substream(NS_rtf::LN_footnote, pFootnote);
            }
            break;
        case PROP_ENDNOTE:
            {
                rStream.info(pIt->toString());
                doctok::Reference<Stream>::Pointer_t
                    pEndnote(pIt->getSubDocument());

                if (pEndnote.get() != NULL)
                    rStream.substream(NS_rtf::LN_endnote, pEndnote);
            }
            break;
        case PROP_ANNOTATION:
            {
                rStream.info(pIt->toString());
                doctok::Reference<Stream>::Pointer_t
                    pAnnotation(pIt->getSubDocument());

                if (pAnnotation.get() != NULL)
                    rStream.substream(NS_rtf::LN_annotation, pAnnotation);
            }
            break;
        case PROP_CHP:
            {
                if (bInCharacterGroup)
                    rStream.endCharacterGroup();

                rStream.info(pIt->toString());
                rStream.startCharacterGroup();

                bInCharacterGroup = true;
            }

            break;
        case PROP_PAP:
            {
                if (bInCharacterGroup)
                {
                    rStream.endCharacterGroup();
                    bInCharacterGroup = false;
                }

                if (bInParagraphGroup)
                    rStream.endParagraphGroup();

                rStream.info(pIt->toString());
                rStream.startParagraphGroup();

                bInParagraphGroup = true;
            }

            break;
        case PROP_SEC:
            {
                if (bInCharacterGroup)
                {
                    rStream.endCharacterGroup();
                    bInCharacterGroup = false;
                }

                if (bInParagraphGroup)
                {
                    rStream.endParagraphGroup();
                    bInParagraphGroup = false;
                }

                if (bInSection)
                    rStream.endSectionGroup();

                rStream.startSectionGroup();
                rStream.info(pIt->toString());

                bInSection = true;

                sal_uInt32 nHeaderStartIndex = 6 + nSectionIndex * 6;
                sal_uInt32 nHeaderEndIndex = nHeaderStartIndex + 6;

                if (nHeaderStartIndex >= getHeaderCount())
                    nHeaderStartIndex = getHeaderCount();

                if (nHeaderEndIndex >= getHeaderCount())
                    nHeaderEndIndex = getHeaderCount();

                for (sal_uInt32 n = nHeaderStartIndex; n < nHeaderEndIndex; ++n)
                {
                    doctok::Reference<Stream>::Pointer_t
                        pHeader(getHeader(n));

                    QName_t qName = lcl_headerQName(n);

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
            rStream.props(pProperties);
        }

        resolveText(pIt, rStream);

        ++(*pIt);
    }

    if (bInCharacterGroup)
        rStream.endCharacterGroup();

    if (bInParagraphGroup)
        rStream.endParagraphGroup();

    if (bInSection)
        rStream.endSectionGroup();

}

WW8Stream::Pointer_t
WW8DocumentFactory::createStream(uno::Reference<uno::XComponentContext> rContext,
                                 uno::Reference<io::XInputStream> rStream)
{
    return WW8Stream::Pointer_t(new WW8StreamImpl(rContext, rStream));
}

WW8Document::Pointer_t
WW8DocumentFactory::createDocument(WW8Stream::Pointer_t rpStream)
{
    return WW8Document::Pointer_t(new WW8DocumentImpl(rpStream));
}

doctok::Reference<Properties>::Pointer_t
WW8SED::get_sepx()
{
    doctok::Reference<Properties>::Pointer_t pResult;

    if (get_fcSepx() != 0xffffffff)
    {
        WW8StructBase aTmp(*mpDoc->getDocStream(), get_fcSepx(), 2);
        pResult = doctok::Reference<Properties>::Pointer_t
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

Bookmark::Bookmark(doctok::Reference<Properties>::Pointer_t pBKF,
                   rtl::OUString & rName)
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

rtl::OUString BookmarkHelper::getName(sal_uInt32 nPos)
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

doctok::Reference<Properties>::Pointer_t
BookmarkHelper::getBKF(const CpAndFc & rCpAndFc)
{
    sal_uInt32 nIndex = getIndex(rCpAndFc);

    return doctok::Reference<Properties>::Pointer_t
        (mpStartCps->getEntryPointer(nIndex));
}

doctok::Reference<Properties>::Pointer_t
BookmarkHelper::getBookmark(const CpAndFc & rCpAndFc)
{
    doctok::Reference<Properties>::Pointer_t pResult;

    try
    {
        rtl::OUString aName = getName(rCpAndFc);

        pResult = doctok::Reference<Properties>::Pointer_t
            (new Bookmark(getBKF(rCpAndFc), aName));
    }
    catch (ExceptionNotFound e)
    {
        clog << e.getText() << endl;
    }

    return pResult;
}

rtl::OUString BookmarkHelper::getName(const CpAndFc & rCpAndFc)
{
    rtl::OUString sResult;

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

    for (aIt = mMap.begin(); aIt != mMap.end(); aIt++)
    {
        mpDoc->insertCpAndFc(aIt->first);
    }
}

doctok::Reference<Properties>::Pointer_t
FieldHelper::getField(const CpAndFc & rCpAndFc)
{
    WW8FLD::Pointer_t pFLD = mMap[rCpAndFc];

    return doctok::Reference<Properties>::Pointer_t
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

    for (aIt = mMap.begin(); aIt != mMap.end(); aIt++)
    {
        mpDoc->insertCpAndFc(aIt->first);
        aIt->second->setDocument(mpDoc);
    }
}

doctok::Reference<Properties>::Pointer_t
ShapeHelper::getShape(const CpAndFc & rCpAndFc)
{
    WW8FSPA::Pointer_t pFSPA = mMap[rCpAndFc];

    return doctok::Reference<Properties>::Pointer_t
        (new WW8FSPA(*pFSPA));
}


}
