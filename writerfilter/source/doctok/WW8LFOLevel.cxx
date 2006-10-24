/**
   $RCSfile: WW8LFOLevel.cxx,v $

   $Revision: 1.1 $

 */
#include "resources.hxx"

sal_uInt32 WW8LFOLevel::calcSize()
{
    sal_uInt32 nResult = WW8LFOLevel::getSize();

    if (get_fFormatting())
    {
        WW8ListLevel aLevel(parent, mnOffsetInParent + nResult)
            nResult += aLevel.calcSize();

        sal_uInt32 nXstSize = parent->getU16(nOffset);

        nResult += (nXstSize + 1) * 2;
    }

    return nResult;
}
