#include <WW8StructBase.hxx>
#include <util.hxx>

namespace doctok {
using namespace ::com::sun::star;

WW8StructBase::WW8StructBase(const WW8StructBase & rParent,
              sal_uInt32 nOffset, sal_uInt32 nCount)
: mSequence(rParent.mSequence, nOffset, nCount), mpParent(0),
  mpDocument(rParent.getDocument())
{
    if (nOffset + nCount > rParent.getCount())
    {
        throw ExceptionOutOfBounds("WW8StructBase");
    }
}

WW8StructBase & WW8StructBase::Assign(const WW8StructBase & rSrc)
{
    mSequence = rSrc.mSequence;
    mpDocument = rSrc.mpDocument;

    return *this;
}

void WW8StructBase::setDocument(WW8DocumentImpl * pDocument)
{
    mpDocument = pDocument;
}

WW8DocumentImpl * WW8StructBase::getDocument() const
{
    return mpDocument;
}

sal_uInt8 WW8StructBase::getU8(sal_uInt32 nOffset) const
{
    return doctok::getU8(mSequence, nOffset);
}

sal_uInt16 WW8StructBase::getU16(sal_uInt32 nOffset) const
{
    return doctok::getU16(mSequence, nOffset);
}

sal_uInt32 WW8StructBase::getU32(sal_uInt32 nOffset) const
{
    return doctok::getU32(mSequence, nOffset);
}

sal_Bool WW8StructBase::getBit(sal_uInt32 nValue, sal_uInt16 nBit) const
{
    return (nValue & (1 << nBit)) != 0;
}

sal_uInt8 WW8StructBase::getNibble(sal_uInt32 nValue,
                                   sal_uInt16 nShift) const
{
    return sal::static_int_cast<sal_uInt8>((nValue >> nShift) & 0xf);
}

sal_uInt8 getU8(const WW8StructBase::Sequence & rSeq,
                sal_uInt32 nOffset)
{
    return rSeq[nOffset];
}

sal_uInt16 getU16(const WW8StructBase::Sequence & rSeq,
                  sal_uInt32 nOffset)
{
    return getU8(rSeq, nOffset) | (getU8(rSeq, nOffset + 1) << 8);
}

sal_uInt32 getU32(const WW8StructBase::Sequence & rSeq,
                  sal_uInt32 nOffset)
{
    sal_uInt32 nResult = getU8(rSeq, nOffset);
    nResult |= (getU8(rSeq, nOffset + 1) << 8);
    nResult |=  (getU8(rSeq, nOffset + 2) << 16);
    nResult |= (getU8(rSeq, nOffset + 3) << 24);

    return nResult;
}

rtl::OUString WW8StructBase::getString(sal_uInt32 nOffset, sal_uInt32 nCount)
    const
{
    rtl::OUString aResult;

    Sequence aSeq(mSequence, nOffset, nCount * 2);

    if (nCount > 0)
    {
        rtl_uString * pNew = 0;
        rtl_uString_newFromStr_WithLength
            (&pNew, reinterpret_cast<const sal_Unicode *>(&aSeq[0]),
             nCount);

        aResult = rtl::OUString(pNew);
    }

    return aResult;
}

rtl::OUString WW8StructBase::getString(sal_uInt32 nOffset) const
{
    sal_uInt32 nCount = getU16(nOffset);

    return getString(nOffset + 2, nCount);
}

WW8StructBaseTmpOffset::WW8StructBaseTmpOffset
(WW8StructBase * pStructBase)
: mnOffset(0), mpStructBase(pStructBase)
{
}

sal_uInt32 WW8StructBaseTmpOffset::set(sal_uInt32 nOffset)
{
    if (nOffset >= mpStructBase->getCount())
        throw ExceptionOutOfBounds("WW8StructBaseTmpOffset::set");

    mnOffset = nOffset;

    return mnOffset;
}

sal_uInt32 WW8StructBaseTmpOffset::get() const
{
    return mnOffset;
}

sal_uInt32 WW8StructBaseTmpOffset::inc(sal_uInt32 nOffset)
{
    if (mpStructBase->getCount() - mnOffset < nOffset)
        throw ExceptionOutOfBounds("WW8StructBaseTmpOffset::inc");

    mnOffset += nOffset;

    return mnOffset;
}

WW8StructBaseTmpOffset::operator sal_uInt32() const
{
    return mnOffset;
}

}
