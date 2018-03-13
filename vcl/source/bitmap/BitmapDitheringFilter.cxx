/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <BitmapDitheringFilter.hxx>
#include <bitmapwriteaccess.hxx>

const extern sal_uLong nVCLRLut[6] = { 16, 17, 18, 19, 20, 21 };
const extern sal_uLong nVCLGLut[6] = { 0, 6, 12, 18, 24, 30 };
const extern sal_uLong nVCLBLut[6] = { 0, 36, 72, 108, 144, 180 };

const extern sal_uLong nVCLDitherLut[256] =
{
       0, 49152, 12288, 61440,  3072, 52224, 15360, 64512,   768, 49920, 13056,
   62208,  3840, 52992, 16128, 65280, 32768, 16384, 45056, 28672, 35840, 19456,
   48128, 31744, 33536, 17152, 45824, 29440, 36608, 20224, 48896, 32512, 8192,
   57344,  4096, 53248, 11264, 60416,  7168, 56320,  8960, 58112,  4864, 54016,
   12032, 61184,  7936, 57088, 40960, 24576, 36864, 20480, 44032, 27648, 39936,
   23552, 41728, 25344, 37632, 21248, 44800, 28416, 40704, 24320, 2048, 51200,
   14336, 63488,  1024, 50176, 13312, 62464,  2816, 51968, 15104, 64256,  1792,
   50944, 14080, 63232, 34816, 18432, 47104, 30720, 33792, 17408, 46080, 29696,
   35584, 19200, 47872, 31488, 34560, 18176, 46848, 30464, 10240, 59392,  6144,
   55296,  9216, 58368,  5120, 54272, 11008, 60160,  6912, 56064,  9984, 59136,
    5888, 55040, 43008, 26624, 38912, 22528, 41984, 25600, 37888, 21504, 43776,
   27392, 39680, 23296, 42752, 26368, 38656, 22272,   512, 49664, 12800, 61952,
    3584, 52736, 15872, 65024,   256, 49408, 12544, 61696,  3328, 52480, 15616,
   64768, 33280, 16896, 45568, 29184, 36352, 19968, 48640, 32256, 33024, 16640,
   45312, 28928, 36096, 19712, 48384, 32000,  8704, 57856,  4608, 53760, 11776,
   60928,  7680, 56832,  8448, 57600,  4352, 53504, 11520, 60672,  7424, 56576,
   41472, 25088, 37376, 20992, 44544, 28160, 40448, 24064, 41216, 24832, 37120,
   20736, 44288, 27904, 40192, 23808,  2560, 51712, 14848, 64000,  1536, 50688,
   13824, 62976,  2304, 51456, 14592, 63744,  1280, 50432, 13568, 62720, 35328,
   18944, 47616, 31232, 34304, 17920, 46592, 30208, 35072, 18688, 47360, 30976,
   34048, 17664, 46336, 29952, 10752, 59904,  6656, 55808,  9728, 58880,  5632,
   54784, 10496, 59648,  6400, 55552,  9472, 58624,  5376, 54528, 43520, 27136,
   39424, 23040, 42496, 26112, 38400, 22016, 43264, 26880, 39168, 22784, 42240,
   25856, 38144, 21760
};

const extern sal_uLong nVCLLut[256] =
{
         0,  1286,  2572,  3858,  5144,  6430,  7716,  9002,
     10288, 11574, 12860, 14146, 15432, 16718, 18004, 19290,
     20576, 21862, 23148, 24434, 25720, 27006, 28292, 29578,
     30864, 32150, 33436, 34722, 36008, 37294, 38580, 39866,
     41152, 42438, 43724, 45010, 46296, 47582, 48868, 50154,
     51440, 52726, 54012, 55298, 56584, 57870, 59156, 60442,
     61728, 63014, 64300, 65586, 66872, 68158, 69444, 70730,
     72016, 73302, 74588, 75874, 77160, 78446, 79732, 81018,
     82304, 83590, 84876, 86162, 87448, 88734, 90020, 91306,
     92592, 93878, 95164, 96450, 97736, 99022,100308,101594,
    102880,104166,105452,106738,108024,109310,110596,111882,
    113168,114454,115740,117026,118312,119598,120884,122170,
    123456,124742,126028,127314,128600,129886,131172,132458,
    133744,135030,136316,137602,138888,140174,141460,142746,
    144032,145318,146604,147890,149176,150462,151748,153034,
    154320,155606,156892,158178,159464,160750,162036,163322,
    164608,165894,167180,168466,169752,171038,172324,173610,
    174896,176182,177468,178754,180040,181326,182612,183898,
    185184,186470,187756,189042,190328,191614,192900,194186,
    195472,196758,198044,199330,200616,201902,203188,204474,
    205760,207046,208332,209618,210904,212190,213476,214762,
    216048,217334,218620,219906,221192,222478,223764,225050,
    226336,227622,228908,230194,231480,232766,234052,235338,
    236624,237910,239196,240482,241768,243054,244340,245626,
    246912,248198,249484,250770,252056,253342,254628,255914,
    257200,258486,259772,261058,262344,263630,264916,266202,
    267488,268774,270060,271346,272632,273918,275204,276490,
    277776,279062,280348,281634,282920,284206,285492,286778,
    288064,289350,290636,291922,293208,294494,295780,297066,
    298352,299638,300924,302210,303496,304782,306068,307354,
    308640,309926,311212,312498,313784,315070,316356,317642,
    318928,320214,321500,322786,324072,325358,326644,327930
};

BitmapEx BitmapDitheringFilter::execute(BitmapEx const& rBitmapEx)
{
    Bitmap aBitmap = rBitmapEx.GetBitmap();

    Bitmap::ScopedReadAccess pReadAcc(aBitmap);
    Bitmap aNewBmp(aBitmap.GetSizePixel(), 8);
    BitmapScopedWriteAccess pWriteAcc(aNewBmp);
    bool bRet = false;

    if (pReadAcc && pWriteAcc)
    {
        const sal_uLong nWidth = pReadAcc->Width();
        const sal_uLong nHeight = pReadAcc->Height();
        BitmapColor aIndex(sal_uInt8(0));

        if (pReadAcc->HasPalette())
        {
            for (sal_uLong nY = 0; nY < nHeight; nY++)
            {
                Scanline pScanline = pWriteAcc->GetScanline(nY);
                Scanline pScanlineRead = pReadAcc->GetScanline(nY);
                for (sal_uLong nX = 0, nModY = (nY & 0x0FUL) << 4; nX < nWidth; nX++)
                {
                    const BitmapColor aCol(
                        pReadAcc->GetPaletteColor(pReadAcc->GetIndexFromData(pScanlineRead, nX)));
                    const sal_uLong nD = nVCLDitherLut[nModY + (nX & 0x0FUL)];
                    const sal_uLong nR = (nVCLLut[aCol.GetRed()] + nD) >> 16;
                    const sal_uLong nG = (nVCLLut[aCol.GetGreen()] + nD) >> 16;
                    const sal_uLong nB = (nVCLLut[aCol.GetBlue()] + nD) >> 16;

                    aIndex.SetIndex(
                        static_cast<sal_uInt8>(nVCLRLut[nR] + nVCLGLut[nG] + nVCLBLut[nB]));
                    pWriteAcc->SetPixelOnData(pScanline, nX, aIndex);
                }
            }
        }
        else
        {
            for (sal_uLong nY = 0; nY < nHeight; nY++)
            {
                Scanline pScanline = pWriteAcc->GetScanline(nY);
                Scanline pScanlineRead = pReadAcc->GetScanline(nY);
                for (sal_uLong nX = 0, nModY = (nY & 0x0FUL) << 4; nX < nWidth; nX++)
                {
                    const BitmapColor aCol(pReadAcc->GetPixelFromData(pScanlineRead, nX));
                    const sal_uLong nD = nVCLDitherLut[nModY + (nX & 0x0FUL)];
                    const sal_uLong nR = (nVCLLut[aCol.GetRed()] + nD) >> 16;
                    const sal_uLong nG = (nVCLLut[aCol.GetGreen()] + nD) >> 16;
                    const sal_uLong nB = (nVCLLut[aCol.GetBlue()] + nD) >> 16;

                    aIndex.SetIndex(
                        static_cast<sal_uInt8>(nVCLRLut[nR] + nVCLGLut[nG] + nVCLBLut[nB]));
                    pWriteAcc->SetPixelOnData(pScanline, nX, aIndex);
                }
            }
        }

        bRet = true;
    }

    pReadAcc.reset();
    pWriteAcc.reset();

    if (bRet)
    {
        const MapMode aMap(aBitmap.GetPrefMapMode());
        const Size aSize(aBitmap.GetPrefSize());

        aBitmap = aNewBmp;

        aBitmap.SetPrefMapMode(aMap);
        aBitmap.SetPrefSize(aSize);
    }

    if (bRet)
        return BitmapEx(aBitmap);

    return BitmapEx();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
