#include <WW8Sttbf.hxx>

namespace doctok
{

WW8Sttbf::WW8Sttbf(WW8Stream & rStream, sal_uInt32 nOffset, sal_uInt32 nCount)
: WW8StructBase(rStream, nOffset, nCount)
{
    sal_uInt32 nComplexOffset = 0;
    if (getU16(0) == 0xffff)
    {
        mbComplex = true;
        nComplexOffset = 2;
    }

    mnCount = getU16(nComplexOffset);
    mnExtraDataCount = getU16(nComplexOffset + 2);

    nOffset = (mbComplex ? 2 : 0) + 4;

    for (sal_uInt32 n = 0; n < mnCount; ++n)
    {
        mEntryOffsets.push_back(nOffset);

        sal_uInt32 nStringLength = getU16(nOffset);

        nOffset += 2 + nStringLength * (mbComplex ? 2 : 1);

        mExtraOffsets.push_back(nOffset);

        nOffset += mnExtraDataCount;
    }
}

sal_uInt32 WW8Sttbf::getEntryOffset(sal_uInt32 nPos) const
{
    return mEntryOffsets[nPos];
}

sal_uInt32 WW8Sttbf::getExtraOffset(sal_uInt32 nPos) const
{
    return mExtraOffsets[nPos];
}

sal_uInt32 WW8Sttbf::getEntryCount() const
{
    return mnCount;
}

rtl::OUString WW8Sttbf::getEntry(sal_uInt32 nPos) const
{
    return getString(getEntryOffset(nPos));
}

WW8StructBase::Pointer_t WW8Sttbf::getExtraData(sal_uInt32 nPos)
{
    return WW8StructBase::Pointer_t
        (new WW8StructBase(*this, getExtraOffset(nPos), mnExtraDataCount));
}

}
