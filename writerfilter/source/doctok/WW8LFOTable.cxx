/*
   $RCSfile: WW8LFOTable.cxx,v $

   $Revision: 1.1 $

*/

#include "resources.hxx"

namespace doctok {

void WW8LFOTable::initPayload()
{
    sal_uInt32 nCount = getEntryCount();

    sal_uInt32 nOffset = 4;
    sal_uInt32 nOffsetLevel = mnPlcfPayloadOffset;

    for (sal_uInt32 n = 0; n < nCount; ++n)
    {
        WW8LFO aLFO(this, nOffset);

        entryOffsets.push_back(nOffset);
        payloadIndices.push_back(payloadOffsets.size());
        nOffset += WW8LFO::getSize();

        sal_uInt32 nLvlCount = aLFO.get_lfolevel_count();

        for (sal_uInt32 i = 0; i < nLvlCount; ++i)
        {
            WW8LFOLevel aLevel(this, nOffsetLevel);

            payloadOffsets.push_back(nOffsetLevel);

            nOffsetLevel += aLevel.calcSize();
        }
    }

    entryOffsets.push_back(nOffset);
    payloadOffsets.push_back(nOffsetLevel);
}

sal_uInt32 WW8LFOTable::calcPayloadOffset()
{
    sal_uInt32 nResult = 4 + getEntryCount() * WW8LFO::getSize();

    while (getU32(nResult) == 0xffffffff)
        nResult += 4;

    return nResult;
}

sal_uInt32 WW8LFOTable::getEntryCount()
{
    return getU32(0);
}

doctok::Reference<Properties>::Pointer_t
WW8LFOTable::getEntry(sal_uInt32 nIndex)
{
    WW8LFO * pLFO = new WW8LFO(this, entryOffsets[nIndex]);

    pLFO->setIndex(nIndex);

    return doctok::Reference<Properties>::Pointer_t(pLFO);
}

sal_uInt32 WW8LFO::get_lfolevel_count()
{
    return get_clfolvl();
}

doctok::Reference<Properties>::Pointer_t
WW8LFO::get_lfolevel(sal_uInt32 nIndex)
{
    WW8LFOTable * pLFOTable = dynamic_cast<WW8LFOTable *>(mpParent);
    sal_uInt32 nPayloadIndex = pLFOTable->getPayloadIndex(mnIndex) + nIndex;
    sal_uInt32 nPayloadOffset = pLFOTable->getPayloadOffset(nPayloadIndex);
    sal_uInt32 nPayloadSize = pLFOTable->getPayloadSize(nPayloadIndex);

    return doctok::Reference<Properties>::Pointer_t
        (new WW8LFOLevel(mpParent, nPayloadOffset, nPayloadSize));
}

void WW8LFOLevel::resolveNoAuto(Properties & /*rHandler*/)
{

}

sal_uInt32 WW8LFOLevel::calcSize()
{
    sal_uInt32 nResult = WW8LFOLevel::getSize();

    if (get_fFormatting())
    {
        WW8ListLevel aLevel(mpParent, mnOffsetInParent + nResult);

        nResult += aLevel.calcSize();

        sal_uInt32 nXstSize = mpParent->getU16(mnOffsetInParent + nResult);

        nResult += (nXstSize + 1) * 2;
    }

    return nResult;
}

}
