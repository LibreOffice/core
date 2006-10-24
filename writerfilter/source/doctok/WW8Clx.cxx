#include <com/sun/star/io/XSeekable.hpp>
#include <WW8Clx.hxx>
#include <resources.hxx>

namespace doctok
{
WW8Clx::WW8Clx(WW8Stream & rStream,
               sal_uInt32 nOffset, sal_uInt32 nCount)
: WW8StructBase(rStream, nOffset, nCount), nOffsetPieceTable(0)
{
    while (getU8(nOffsetPieceTable) != 2)
    {
        nOffsetPieceTable += getU16(nOffsetPieceTable + 1) + 3;
    }
}

sal_uInt32 WW8Clx::getPieceCount() const
{
    return (getU32(nOffsetPieceTable + 1) - 4) / 12;
}

sal_uInt32 WW8Clx::getCp(sal_uInt32 nIndex) const
{
    return getU32(nOffsetPieceTable + 5 + nIndex * 4);
}

sal_uInt32 WW8Clx::getFc(sal_uInt32 nIndex) const
{
    sal_uInt32 nResult = getU32(nOffsetPieceTable + 5 +
                                (getPieceCount() + 1) * 4 +
                                nIndex * 8 + 2);

    if (nResult & 0x40000000)
        nResult = (nResult & ~0x40000000) / 2;

    return nResult;
}

sal_Bool WW8Clx::isComplexFc(sal_uInt32 nIndex) const
{
    sal_Bool bResult = sal_False;
    sal_uInt32 nTmp = getU32(nOffsetPieceTable + 5 +
                             (getPieceCount() + 1) * 4 +
                             nIndex * 8 + 2);
    if (nTmp & 0x40000000)
        bResult = sal_True;

    return bResult;
}

void WW8Clx::dump(WW8OutputWithDepth & o) const
{
    WW8StructBase::dump(o);
}

}
