/************************************************************************
 *
 *  SVMReader.java
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *  Copyright: 2004 by Urban Widmark
 *
 *  All Rights Reserved.
 *
 *  Version 0.3.3b (2004-02-26)
 *
 */

package writer2latex.office;

/** This class contains a static method to extract EPS data from an SVM file */
public final class SVMReader {

    /* Notes on SVM file format:

     Relevant OO 1.1.0 Source Files include:
       tools/source/stream/vcompat.cxx
       vcl/source/gdi/gdimtf.cxx
       vcl/inc/gdimtf.hxx
       vcl/source/gdi/metaact.cxx
       vcl/inc/metaact.hxx
       vcl/source/gdi/mapmod.cxx
       vcl/inc/mapmod.hxx
       tools/source/generic/fract.cxx
       tools/inc/fract.hxx
       tools/source/generic/gen.cxx
       tools/inc/gen.hxx

     VersionCompat      (6 bytes)
       version  - 2 bytes
       totalsize    - 4 bytes

     Point (Pair)           (8 bytes)
       X - 4 bytes (long)
       Y - 4 bytes (long)

     Fraction           (8 bytes)
       nNumerator - 4 bytes (long)
       nDenominator - 4 bytes (long)

     MapMode            (6 + 2 + 8 + 8 + 8 + 1 = 33 bytes)
       VersionCompat
       meUnit   - UINT16
       maOrigin - Point
       maScaleX - Fraction
       maScaleY - Fraction
       mbSimple - BOOL (1 byte)

     Size (Pair)            (8 bytes)
       width - 4 bytes (long)
       height - 4 bytes (long)

     GfxLink            (16 or 57 bytes)
       VersionCompat
       nType    - 2 bytes
       nSize    - 4 bytes
       nUserId  - 4 bytes
       aSize    - Size (version >=2)
       aMapMode - MapMode (version >=2)

     MetaEPSAction
       VersionCompat
       maGfxLink                - GfxLink
       data[maGfxLink.nSize]    - bytes
       maPoint                  - Point
       maSize                   - Size
       maSubst                  - GDIMetaFile (alternative image?)

     SVM file
       "VCLMTF"
       Compat           - VersionCompat
       nStmCompressMode - UINT32
       aPrefMapMode     - MapMode
       aPrefSize        - Size
       count            - UINT32
       action[count]    - MetaAction


    Example header from an EPS image included in a Writer document:
    00000000: 5643 4c4d 5446 0100 3100 0000 0000 0000  VCLMTF..1.......
    00000010: 0100 1b00 0000 0800 0000 0000 0000 0000  ................
    00000020: 0100 0000 0100 0000 0100 0000 0100 0000  ................
    00000030: 0169 0100 00fd 0000 0001 0000 008f 0001  .i..............
    00000040: 0096 3200 0002 0033 0000 0001 003f 3000  ..2....3.....?0.
    00000050: 0000 0000 0000 0000 0000 0000 0001 001b  ................
    00000060: 0000 000a 0000 0000 0000 0000 0001 0000  ................
    00000070: 0001 0000 0001 0000 0001 0000 0001 2521  ..............%!

    5643 4c4d 5446  "VCLMTF"                    0

    0100            version                     6
    3100 0000       totalsize

    0000 0000       compress                    12

    0100            aPrefMapMode.version        16
    1b00 0000       aPrefMapMode.totalsize
    0800            aPrefMapMode.meUnit
    0000 0000       aPrefMapMode.maOrigin
    0000 0000
    0100 0000       aPrefMapMode.maScaleX
    0100 0000
    0100 0000       aPrefMapMode.maScaleY
    0100 0000
    01              aPrefMapMode.mbSimple

    6901 0000       aPrefSize                   49
    fd00 0000

    0100 0000       nCount                      57

    8f00            type == META_EPS_ACTION     61

    0100            version
    9632 0000       totalsize

    0200            version
    3300 0000       totalsize

    0100            nType
    3f30 0000       nSize                       75
    0000 0000       nUserId

    0000 0000       aSize
    0000 0000

    0100
    1b00 0000
    0a00

    0000 0000
    0000 0000
    0100 0000
    0100 0000
    0100 0000
    0100 0000
    01

    Beginning of EPS data:

    2521

    Note that maPoint/maSize/maSubst are all after the EPS file.

    */

    /** Determine if this SVM contains an EPS document and retrieve start and
      * end positions if so.
      *
      * @param blob        byte array containing SVM file
      * @param offlen      integer array to retrieve the offset into the SVM file
      *                    (offlen[0]) and the length (offlen[1]) of the EPS
      *                    document. If the method returns false, the array
      *                    will be unchanged.
      *
      * @return returns true if the SVM contains an EPS document
      */
    public static final boolean readSVM(byte[] blob, int[] offlen) {
        int pos = 57;
        int nCount = getInt(blob, pos);
        pos += 4;

        for (int i=0; i<nCount; i++) {
            int type = getShort(blob, pos);
            pos += 2;

            // We only understand META_EPS_ACTION
            if (type != 143)
            return false;

            pos += 6;
            int version = getShort(blob, pos);
            pos += 6;       // version + totalsize
            pos += 2;       // nType

            // This is the size of the EPS data.
            int size = getInt(blob, pos);
            pos += 8;

            if (version >= 2)
            pos += 41;

            for (int j=0; j<MIMETypes.EPS_SIG.length; j++)
            if (MIMETypes.EPS_SIG[j] != blob[pos + j])
               return false;

            offlen[0] = pos;
            offlen[1] = size;

            // For now we only understand files where the EPS entry is
            // the first MetaAction
            break;
        }

        return true;
    }

    private static int getInt(byte[] blob, int pos)
    {
     return
        ((blob[pos+0] & 0xff) << 0)  +
        ((blob[pos+1] & 0xff) << 8)  +
        ((blob[pos+2] & 0xff) << 16) +
        ((blob[pos+3] & 0xff) << 24);
    }

    private static int getShort(byte[] blob, int pos)
    {
     return
        ((blob[pos+0] & 0xff) << 0)  +
        ((blob[pos+1] & 0xff) << 8);
    }

}
