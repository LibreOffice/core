#include <resources.hxx>

namespace doctok {

void WW8ListTable::initPayload()
{
    sal_uInt32 nCount = getEntryCount();

    sal_uInt32 nOffset = 2;
    sal_uInt32 nOffsetLevel = mnPlcfPayloadOffset;
    for (sal_uInt32 n = 0; n < nCount; ++n)
    {
        WW8List aList(this, nOffset);

        entryOffsets.push_back(nOffset);
        payloadIndices.push_back(payloadOffsets.size());
        nOffset += WW8List::getSize();

        sal_uInt32 nLvlCount = aList.get_fSimpleList() ? 1 : 9;

        for (sal_uInt32 i = 0; i < nLvlCount; ++i)
        {
            WW8ListLevel aLevel(this, nOffsetLevel);

            payloadOffsets.push_back(nOffsetLevel);

            nOffsetLevel += aLevel.calcSize();

            sal_uInt32 nXstSize = getU16(nOffsetLevel);

            nOffsetLevel += 2 + nXstSize * 2;
        }

        if (nOffsetLevel > getCount())
        {
            nOffsetLevel = getCount();

            break;
        }
    }

    payloadOffsets.push_back(nOffsetLevel);
    entryOffsets.push_back(nOffset);
}

sal_uInt32 WW8ListTable::calcPayloadOffset()
{
    return 0;
}

sal_uInt32 WW8ListTable::getEntryCount()
{
    return getU16(0);
}

doctok::Reference<Properties>::Pointer_t
WW8ListTable::getEntry(sal_uInt32 nIndex)
{
    WW8List * pList = new WW8List(this, entryOffsets[nIndex]);

    pList->setIndex(nIndex);

    return doctok::Reference<Properties>::Pointer_t
        (pList);
}

sal_uInt32 WW8List::get_listlevel_count()
{
    if (get_fSimpleList())
        return 1;

    return 9;
}

doctok::Reference<Properties>::Pointer_t
WW8List::get_listlevel(sal_uInt32 nIndex)
{
    WW8ListTable * pListTable = dynamic_cast<WW8ListTable *>(mpParent);
    sal_uInt32 nPayloadIndex = pListTable->getPayloadIndex(mnIndex) + nIndex;
    sal_uInt32 nPayloadOffset = pListTable->getPayloadOffset(nPayloadIndex);
    sal_uInt32 nPayloadSize = pListTable->getPayloadSize(nPayloadIndex);

    return doctok::Reference<Properties>::Pointer_t
        (new WW8ListLevel(mpParent, nPayloadOffset, nPayloadSize));
}

void WW8ListLevel::resolveNoAuto(Properties & rHandler)
{
    sal_uInt32 nOffset = getSize();

    {
        WW8PropertySet::Pointer_t pSet
            (new WW8PropertySetImpl(*this, nOffset, get_cbGrpprlPapx()));

        WW8PropertiesReference aRef(pSet);
        aRef.resolve(rHandler);
    }

    nOffset += get_cbGrpprlPapx();

    {
        WW8PropertySet::Pointer_t pSet
            (new WW8PropertySetImpl(*this, nOffset, get_cbGrpprlChpx()));

        WW8PropertiesReference aRef(pSet);
        aRef.resolve(rHandler);
    }

    nOffset += get_cbGrpprlChpx();

    WW8StringValue aVal(getString(nOffset));

    rHandler.attribute(0, aVal);
}

sal_uInt32 WW8ListLevel::calcSize()
{
    sal_uInt32 nResult = WW8ListLevel::getSize();

    nResult += get_cbGrpprlPapx();
    nResult += get_cbGrpprlChpx();

    return nResult;
}
}
