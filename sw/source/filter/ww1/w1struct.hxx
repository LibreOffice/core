/*************************************************************************
 *
 *  $RCSfile: w1struct.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:58 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
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
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef W1STRUCT_HXX
#define W1STRUCT_HXX

// star view
#include <string.h>

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

class Ww1Shell;
class Ww1Manager;

struct W1_FIB /////////////////////////////////////////////////////////
{
    SVBT16 wIdent;// 0x0 int magic number
    USHORT wIdentGet() {
        return SVBT16ToShort(wIdent); }
    SVBT16 nFib;// 0x2 FIB version written
    USHORT nFibGet() {
        return SVBT16ToShort(nFib); }
    SVBT16 nProduct;// 0x4 product version written by
    USHORT nProductGet() {
        return SVBT16ToShort(nProduct); }
    SVBT16 nlocale;// 0x6 language stamp---localized version;
    USHORT nlocaleGet() {
        return SVBT16ToShort(nlocale); }
    SVBT16 pnNext;// 0x8
    USHORT pnNextGet() {
        return SVBT16ToShort(pnNext); }
    SVBT16 fFlags;
    USHORT fFlagsGet() {
        return SVBT16ToShort(fFlags); }
    // SVBT16 fDot :1;// 0xa    0001
    BOOL fDotGet() {
        return ((fFlagsGet() >> 0) & 1); }
    // SVBT16 fGlsy :1;//       0002
    BOOL fGlsyGet() {
        return ((fFlagsGet() >> 1) & 1); }
    // SVBT16 fComplex :1;//        0004 when 1, file is in complex, fast-saved format.
    BOOL fComplexGet() {
        return ((fFlagsGet() >> 2) & 1); }
    // SVBT16 fHasPic :1;//     0008 file contains 1 or more pictures
    BOOL fHasPicGet() {
        return ((fFlagsGet() >> 3) & 1); }
    // SVBT16 cQuickSaves :4;//     00F0 count of times file was quicksaved
    USHORT cQuickSavesGet() {
        return (USHORT)((fFlagsGet() >> 4) & 0xf); }
    // SVBT16 u1 :8;//      FF00 unused
    USHORT u1Get() {
        return (USHORT)((fFlagsGet() >> 8) & 0xff); }
    SVBT16 nFibBack;// 0xc
    USHORT nFibBackGet() {
        return SVBT16ToShort(nFibBack); }
    SVBT16 u2[5];// 0xe reserved
    SVBT32 fcMin;// 0x18 file offset of first character of text
    ULONG fcMinGet() {
        return SVBT32ToLong(fcMin); }
    SVBT32 fcMac;// 0x1c file offset of last character of text + 1
    ULONG fcMacGet() {
        return SVBT32ToLong(fcMac); }
    SVBT32 cbMac;// 0x20 file offset of last byte written to file + 1.
    ULONG cbMacGet() {
        return SVBT32ToLong(cbMac); }
    SVBT32 u4[4];// 0x24 reserved
    SVBT32 ccpText;// 0x34 length of main document text stream
    ULONG ccpTextGet() {
        return SVBT32ToLong(ccpText); }
    SVBT32 ccpFtn;// 0x38 length of footnote subdocument text stream
    ULONG ccpFtnGet() {
        return SVBT32ToLong(ccpFtn); }
    SVBT32 ccpHdd;// 0x3c length of header subdocument text stream
    ULONG ccpHddGet() {
        return SVBT32ToLong(ccpHdd); }
    SVBT32 ccpMcr;// 0x40 length of macro subdocument text stream
    ULONG ccpMcrGet() {
        return SVBT32ToLong(ccpMcr); }
    SVBT32 ccpAtn;// 0x44 length of annotation subdocument text stream
    ULONG ccpAtnGet() {
        return SVBT32ToLong(ccpAtn); }
    SVBT32 cp5[4];// 0x48
    SVBT32 fcStshfOrig;// 0x58 file offset of original allocation for STSH in file
    ULONG fcStshfOrigGet() {
        return SVBT32ToLong(fcStshfOrig); }
    SVBT16 cbStshfOrig;// 0x5c count of bytes of original STSH allocation
    USHORT cbStshfOrigGet() {
        return SVBT16ToShort(cbStshfOrig); }
    SVBT32 fcStshf;// 0x5e file offset of STSH in file.
    ULONG fcStshfGet() {
        return SVBT32ToLong(fcStshf); }
    SVBT16 cbStshf;// 0x62 count of bytes of current STSH allocation
    USHORT cbStshfGet() {
        return SVBT16ToShort(cbStshf); }
    SVBT32 fcPlcffndRef;// 0x64 file offset of footnote reference PLC.
    ULONG fcPlcffndRefGet() {
        return SVBT32ToLong(fcPlcffndRef); }
    SVBT16 cbPlcffndRef;// 0x68 count of bytes of footnote reference PLC
    USHORT cbPlcffndRefGet() {
        return SVBT16ToShort(cbPlcffndRef); }
                            // == 0 if no footnotes defined in document.

    SVBT32 fcPlcffndTxt;// 0x6a file offset of footnote text PLC.
    ULONG fcPlcffndTxtGet() {
        return SVBT32ToLong(fcPlcffndTxt); }
    SVBT16 cbPlcffndTxt;// 0x6e count of bytes of footnote text PLC.
    USHORT cbPlcffndTxtGet() {
        return SVBT16ToShort(cbPlcffndTxt); }
                            // == 0 if no footnotes defined in document

    SVBT32 fcPlcfandRef;// 0x70 file offset of annotation reference PLC.
    ULONG fcPlcfandRefGet() {
        return SVBT32ToLong(fcPlcfandRef); }
    SVBT16 cbPlcfandRef;// 0x74 count of bytes of annotation reference PLC.
    USHORT cbPlcfandRefGet() {
        return SVBT16ToShort(cbPlcfandRef); }

    SVBT32 fcPlcfandTxt;// 0x76 file offset of annotation text PLC.
    ULONG fcPlcfandTxtGet() {
        return SVBT32ToLong(fcPlcfandTxt); }
    SVBT16 cbPlcfandTxt;// 0x7a count of bytes of the annotation text PLC
    USHORT cbPlcfandTxtGet() {
        return SVBT16ToShort(cbPlcfandTxt); }

    SVBT32 fcPlcfsed;// 8x7c file offset of section descriptor PLC.
    ULONG fcPlcfsedGet() {
        return SVBT32ToLong(fcPlcfsed); }
    SVBT16 cbPlcfsed;// 0x80 count of bytes of section descriptor PLC.
    USHORT cbPlcfsedGet() {
        return SVBT16ToShort(cbPlcfsed); }

    SVBT32 fcPlcfpgd;// 0x82 file offset of paragraph descriptor PLC
    ULONG fcPlcfpgdGet() {
        return SVBT32ToLong(fcPlcfpgd); }
    SVBT16 cbPlcfpgd;// 0x86 count of bytes of paragraph descriptor PLC.
    USHORT cbPlcfpgdGet() {
        return SVBT16ToShort(cbPlcfpgd); }
                            // ==0 if file was never repaginated
                            // Should not be written by third party creators

    SVBT32 fcPlcfphe;// 0x88 file offset of PLC of paragraph heights.
    ULONG fcPlcfpheGet() {
        return SVBT32ToLong(fcPlcfphe); }
    SVBT16 cbPlcfphe;// 0x8c count of bytes of paragraph height PLC.
    USHORT cbPlcfpheGet() {
        return SVBT16ToShort(cbPlcfphe); }
                            // ==0 when file is non-complex.

    SVBT32 fcSttbfglsy;// 0x8e file offset of glossary string table.
    ULONG fcSttbfglsyGet() {
        return SVBT32ToLong(fcSttbfglsy); }
    SVBT16 cbSttbfglsy;// 0x92 count of bytes of glossary string table.
    USHORT cbSttbfglsyGet() {
        return SVBT16ToShort(cbSttbfglsy); }
                            // == 0 for non-glossary documents.
                            // !=0 for glossary documents.

    SVBT32 fcPlcfglsy;// 0x94 file offset of glossary PLC.
    ULONG fcPlcfglsyGet() {
        return SVBT32ToLong(fcPlcfglsy); }
    SVBT16 cbPlcfglsy;// 0x98 count of bytes of glossary PLC.
    USHORT cbPlcfglsyGet() {
        return SVBT16ToShort(cbPlcfglsy); }
                            // == 0 for non-glossary documents.
                            // !=0 for glossary documents.

    SVBT32 fcPlcfhdd;// 0x9a byte offset of header PLC.
    ULONG fcPlcfhddGet() {
        return SVBT32ToLong(fcPlcfhdd); }
    SVBT16 cbPlcfhdd;// 0x9e count of bytes of header PLC.
    USHORT cbPlcfhddGet() {
        return SVBT16ToShort(cbPlcfhdd); }
                            // == 0 if document contains no headers

    SVBT32 fcPlcfbteChpx;// 0xa0 file offset of character property bin table.PLC.
    ULONG fcPlcfbteChpxGet() {
        return SVBT32ToLong(fcPlcfbteChpx); }
    SVBT16 cbPlcfbteChpx;// 0xa4 count of bytes of character property bin table PLC.
    USHORT cbPlcfbteChpxGet() {
        return SVBT16ToShort(cbPlcfbteChpx); }

    SVBT32 fcPlcfbtePapx;// 0xa6 file offset of paragraph property bin table.PLC.
    ULONG fcPlcfbtePapxGet() {
        return SVBT32ToLong(fcPlcfbtePapx); }
    SVBT16 cbPlcfbtePapx;// 0xaa count of bytes of paragraph property bin table PLC.
    USHORT cbPlcfbtePapxGet() {
        return SVBT16ToShort(cbPlcfbtePapx); }

    SVBT32 fcPlcfsea;// 0xac file offset of PLC reserved for private use. The SEA is 6 bytes long.
    ULONG fcPlcfseaGet() {
        return SVBT32ToLong(fcPlcfsea); }
    SVBT16 cbPlcfsea;// 0xb0    count of bytes of private use PLC.
    USHORT cbPlcfseaGet() {
        return SVBT16ToShort(cbPlcfsea); }

    SVBT32 fcSttbfffn;// 0xb2   file offset of font information STTBF. See the FFN file structure definition.
    ULONG fcSttbfffnGet() {
        return SVBT32ToLong(fcSttbfffn); }
    SVBT16 cbSttbfffn;// 0xb6   count of bytes in sttbfffn.
    USHORT cbSttbfffnGet() {
        return SVBT16ToShort(cbSttbfffn); }

    SVBT32 fcPlcffldMom;// 0xb8 offset in doc stream to the PLC of field positions in the main document.
    ULONG fcPlcffldMomGet() {
        return SVBT32ToLong(fcPlcffldMom); }
    SVBT16 cbPlcffldMom;// 0xbc
    USHORT cbPlcffldMomGet() {
        return SVBT16ToShort(cbPlcffldMom); }

    SVBT32 fcPlcffldHdr;// 0xbe offset in doc stream to the PLC of field positions in the header subdocument.
    ULONG fcPlcffldHdrGet() {
        return SVBT32ToLong(fcPlcffldHdr); }
    SVBT16 cbPlcffldHdr;// 0xc2
    USHORT cbPlcffldHdrGet() {
        return SVBT16ToShort(cbPlcffldHdr); }

    SVBT32 fcPlcffldFtn;// 0xc4 offset in doc stream to the PLC of field positions in the footnote subdocument.
    ULONG fcPlcffldFtnGet() {
        return SVBT32ToLong(fcPlcffldFtn); }
    SVBT16 cbPlcffldFtn;// 0xc8
    USHORT cbPlcffldFtnGet() {
        return SVBT16ToShort(cbPlcffldFtn); }

    SVBT32 fcPlcffldAtn;// 0xca offset in doc stream to the PLC of field positions in the annotation subdocument.
    ULONG fcPlcffldAtnGet() {
        return SVBT32ToLong(fcPlcffldAtn); }
    SVBT16 cbPlcffldAtn;// 0xce
    USHORT cbPlcffldAtnGet() {
        return SVBT16ToShort(cbPlcffldAtn); }

    SVBT32 fcPlcffldMcr;// 0xd0 offset in doc stream to the PLC of field positions in the macro subdocument.
    ULONG fcPlcffldMcrGet() {
        return SVBT32ToLong(fcPlcffldMcr); }
    SVBT16 cbPlcffldMcr;// 0xd4
    USHORT cbPlcffldMcrGet() {
        return SVBT16ToShort(cbPlcffldMcr); }

    SVBT32 fcSttbfbkmk;// 0xd6 offset in document stream of the STTBF that records bookmark names in the main document
    ULONG fcSttbfbkmkGet() {
        return SVBT32ToLong(fcSttbfbkmk); }
    SVBT16 cbSttbfbkmk;// 0xda
    USHORT cbSttbfbkmkGet() {
        return SVBT16ToShort(cbSttbfbkmk); }

    SVBT32 fcPlcfbkf;// 0xdc offset in document stream of the PLCF that records the beginning CP offsets of bookmarks in the main document. See BKF
    ULONG fcPlcfbkfGet() {
        return SVBT32ToLong(fcPlcfbkf); }
    SVBT16 cbPlcfbkf;// 0xe0
    USHORT cbPlcfbkfGet() {
        return SVBT16ToShort(cbPlcfbkf); }

    SVBT32 fcPlcfbkl;// 0xe2 offset in document stream of the PLCF that records the ending CP offsets of bookmarks recorded in the main document. See the BKL structure definition.
    ULONG fcPlcfbklGet() {
        return SVBT32ToLong(fcPlcfbkl); }
    SVBT16 cbPlcfbkl;// 0xe6 SVBT16
    USHORT cbPlcfbklGet() {
        return SVBT16ToShort(cbPlcfbkl); }

    SVBT32 fcCmds;// 0xe8 FC
    ULONG fcCmdsGet() {
        return SVBT32ToLong(fcCmds); }
    SVBT16 cbCmds;// 0xec
    USHORT cbCmdsGet() {
        return SVBT16ToShort(cbCmds); }

    SVBT32 fcPlcmcr;// 0xee FC
    ULONG fcPlcmcrGet() {
        return SVBT32ToLong(fcPlcmcr); }
    SVBT16 cbPlcmcr;// 0xf2
    USHORT cbPlcmcrGet() {
        return SVBT16ToShort(cbPlcmcr); }

    SVBT32 fcSttbfmcr;// 0xf4 FC
    ULONG fcSttbfmcrGet() {
        return SVBT32ToLong(fcSttbfmcr); }
    SVBT16 cbSttbfmcr;// 0xf8
    USHORT cbSttbfmcrGet() {
        return SVBT16ToShort(cbSttbfmcr); }

    SVBT32 fcPrEnv;// 0xfa
    ULONG fcPrEnvGet() {
        return SVBT32ToLong(fcPrEnv); }
    SVBT16 cbPrEnv;// 0xfe
    USHORT cbPrEnvGet() {
        return SVBT16ToShort(cbPrEnv); }

    SVBT32 fcWss;// 0x100 file offset of Window Save State data structure. See WSS.
    ULONG fcWssGet() {
        return SVBT32ToLong(fcWss); }
    SVBT16 cbWss;// 0x100 count of bytes of WSS. ==0 if unable to store the window state.
    USHORT cbWssGet() {
        return SVBT16ToShort(cbWss); }

    SVBT32 fcDop;// 0x106 file offset of document property data structure.
    ULONG fcDopGet() {
        return SVBT32ToLong(fcDop); }
    SVBT16 cbDop;// 0x10a count of bytes of document properties.
    USHORT cbDopGet() {
        return SVBT16ToShort(cbDop); }


    SVBT32 fcSttbfAssoc;// 0x10c offset to STTBF of associated strings. See STTBFASSOC.
    ULONG fcSttbfAssocGet() {
        return SVBT32ToLong(fcSttbfAssoc); }
    SVBT16 cbSttbfAssoc;// 0x110
    USHORT cbSttbfAssocGet() {
        return SVBT16ToShort(cbSttbfAssoc); }

    SVBT32 fcClx;// 0x112 file offset of beginning of information for complex files.
    ULONG fcClxGet() {
        return SVBT32ToLong(fcClx); }
    SVBT16 cbClx;// 0x116 count of bytes of complex file information. 0 if file is non-complex.
    USHORT cbClxGet() {
        return SVBT16ToShort(cbClx); }

    SVBT32 fcPlcfpgdFtn;// 0x118 file offset of page descriptor PLC for footnote subdocument.
    ULONG fcPlcfpgdFtnGet() {
        return SVBT32ToLong(fcPlcfpgdFtn); }
    SVBT16 cbPlcfpgdFtn;// 0x11C count of bytes of page descriptor PLC for footnote subdocument.
    USHORT cbPlcfpgdFtnGet() {
        return SVBT16ToShort(cbPlcfpgdFtn); }
                            // ==0 if document has not been paginated. The length of the PGD is 8 bytes.

    SVBT32 fcSpare1;// 0x11e file offset of the name of the original file.
    ULONG fcSpare1Get() {
        return SVBT32ToLong(fcSpare1); }
    SVBT16 cbSpare1;// 0x122 count of bytes of the name of the original file.
    USHORT cbSpare1Get() {
        return SVBT16ToShort(cbSpare1); }
    SVBT32 fcSpare2;// 0x124 file offset of the name of the original file.
    ULONG fcSpare2Get() {
        return SVBT32ToLong(fcSpare2); }
    SVBT16 cbSpare2;// 0x128 count of bytes of the name of the original file.
    USHORT cbSpare2Get() {
        return SVBT16ToShort(cbSpare2); }
    SVBT32 fcSpare3;// 0x12a file offset of the name of the original file.
    ULONG fcSpare3Get() {
        return SVBT32ToLong(fcSpare3); }
    SVBT16 cbSpare3;// 0x12e count of bytes of the name of the original file.
    USHORT cbSpare3Get() {
        return SVBT16ToShort(cbSpare3); }
    SVBT32 fcSpare4;// 0x130 file offset of the name of the original file.
    ULONG fcSpare4Get() {
        return SVBT32ToLong(fcSpare4); }
    SVBT16 cbSpare4;// 0x134 count of bytes of the name of the original file.
    USHORT cbSpare4Get() {
        return SVBT16ToShort(cbSpare4); }

    SVBT16 cpnBteChp;// 0x18E count of CHPX FKPs recorded in file. In non-complex files if the number of
    USHORT cpnBteChpGet() {
        return SVBT16ToShort(cpnBteChp); }
                            // entries in the plcfbteChpx is less than this, the plcfbteChpx is incomplete.
    SVBT16 cpnBtePap;// 0x190 count of PAPX FKPs recorded in file. In non-complex files if the number of
    USHORT cpnBtePapGet() {
        return SVBT16ToShort(cpnBtePap); }
                            // entries in the plcfbtePapx is less than this, the plcfbtePapx is incomplete.
};

struct W1_DOP ///////////////////////////////////// Document Properties
{
    SVBT16 fFlags;
    USHORT fFlagsGet() {
        return SVBT16ToShort(fFlags); }
    // SVBT16 fFacingPages : 1;// 1 when facing pages should be printed
    BOOL fFacingPagesGet() {
        return ((fFlagsGet() >> 0) & 1); }
    // SVBT16 fWidowControl : 1;// 1 when widow control is in effect. 0 when widow control disabled.
    BOOL fWidowControlGet() {
        return ((fFlagsGet() >> 1) & 1); }
    // SVBT16 : 3;// unused
    // SVBT16 fpc : 2;// 1 footnote position code: 0 as endnotes, 1 at bottom of page, 2 immediately beneath text
    USHORT fpcGet() {
        return (USHORT)((fFlagsGet() >> 5) & 3); }
    // SVBT16 fWide : 1;// Landscape
    BOOL fWideGet() {
        return ((fFlagsGet() >> 7) & 1); }
    // SVBT16 grpfIhdt : 8;// 0 specification of document headers and footers. See explanation under Headers and Footers topic.
    USHORT grpfIhdtGet() {
        return (USHORT)((fFlagsGet() >> 8) & 0xff); }
    SVBT16 fFtnFlags;
    USHORT fFtnFlagsGet() {
        return SVBT16ToShort(fFtnFlags); }
    // SVBT16 fFtnRestart : 1;
    BOOL fFtnRestartGet() {
        return ((fFtnFlagsGet() >> 0) & 1); }
    // SVBT16 nFtn : 15;// 1 initial footnote number for document
    USHORT nFtnGet() {
        return (USHORT)((fFtnFlagsGet() >> 1) & 0x7fff); }
    SVBT16 fRvsFlags;
    USHORT fRvsFlagsGet() {
        return SVBT16ToShort(fRvsFlags); }
    // SVBT16 irmBar : 8;
    USHORT irmBarGet() {
        return (USHORT)((fRvsFlagsGet() >> 0) & 0xff); }
    // SVBT16 irmProps : 7;
    USHORT irmPropsGet() {
        return (USHORT)((fRvsFlagsGet() >> 8) & 0x7f); }
    // SVBT16 fRevMarking   : 1;//   when 1, Word will mark revisions as the document is edited
    BOOL fRevMarkingGet() {
        return ((fRvsFlagsGet() >> 15) & 1); }
    SVBT16 fSveFlags;
    USHORT fSveFlagsGet() {
        return SVBT16ToShort(fSveFlags); }
    // SVBT16 fBackup : 1;//     always make backup when document saved when 1.
    BOOL fBackupGet() {
        return ((fSveFlagsGet() >> 0) & 1); }
    // SVBT16 fExactCWords : 1;
    BOOL fExactCWordsGet() {
        return ((fSveFlagsGet() >> 1) & 1); }
    // SVBT16 fPagHidden : 1;//
    BOOL fPagHiddenGet() {
        return ((fSveFlagsGet() >> 2) & 1); }
    // SVBT16 fPagResults : 1;
    BOOL fPagResultsGet() {
        return ((fSveFlagsGet() >> 3) & 1); }
    // SVBT16 fLockAtn : 1;//    when 1, annotations are locked for editing
    BOOL fLockAtnGet() {
        return ((fSveFlagsGet() >> 4) & 1); }
    // SVBT16 fMirrorMargins : 1;//  swap margins on left/right pages when 1.
    BOOL fMirrorMarginsGet() {
        return ((fSveFlagsGet() >> 5) & 1); }
    // SVBT16 : 10;// unused
    SVBT16 fSpares;
    USHORT fSparesGet() {
        return SVBT16ToShort(fSpares); }
    SVBT16 yaPage;
    USHORT yaPageGet() {
        return SVBT16ToShort(yaPage); }
    SVBT16 xaPage;
    USHORT xaPageGet() {
        return SVBT16ToShort(xaPage); }
    SVBT16 dyaTop;
    USHORT dyaTopGet() {
        return SVBT16ToShort(dyaTop); }
    SVBT16 dxaLeft;
    USHORT dxaLeftGet() {
        return SVBT16ToShort(dxaLeft); }
    SVBT16 dyaBottom;
    USHORT dyaBottomGet() {
        return SVBT16ToShort(dyaBottom); }
    SVBT16 dxaRight;
    USHORT dxaRightGet() {
        return SVBT16ToShort(dxaRight); }
    SVBT16 dxaGutter;
    USHORT dxaGutterGet() {
        return SVBT16ToShort(dxaGutter); }
    SVBT16  dxaTab;// 720 twips default tab width
    USHORT dxaTabGet() {
        return SVBT16ToShort(dxaTab); }
    SVBT16  wSpare;//
    USHORT wSpareGet() {
        return SVBT16ToShort(wSpare); }
    SVBT16  dxaHotZ;// width of hyphenation hot zone measured in twips
    USHORT dxaHotZGet() {
        return SVBT16ToShort(dxaHotZ); }
    SVBT16  rgwSpare[2];// reserved
    SVBT32  dttmCreated;// DTTM date and time document was created
    ULONG dttmCreatedGet() {
        return SVBT32ToLong(dttmCreated); }
    SVBT32  dttmRevised;// DTTM date and time document was last revised
    ULONG dttmRevisedGet() {
        return SVBT32ToLong(dttmRevised); }
    SVBT32  dttmLastPrint;// DTTM date and time document was last printed
    ULONG dttmLastPrintGet() {
        return SVBT32ToLong(dttmLastPrint); }
    SVBT16  nRevision;// number of times document has been revised since its creation
    USHORT nRevisionGet() {
        return SVBT16ToShort(nRevision); }
    SVBT32  tmEdited;// time document was last edited
    ULONG tmEditedGet() {
        return SVBT32ToLong(tmEdited); }
    SVBT32  cWords;// count of words tallied by last Word Count execution
    ULONG cWordsGet() {
        return SVBT32ToLong(cWords); }
    SVBT32  cCh;// count of characters tallied by last Word Count execution
    ULONG cChGet() {
        return SVBT32ToLong(cCh); }
    SVBT16  cPg;// count of pages tallied by last Word Count execution
    USHORT cPgGet() {
        return SVBT16ToShort(cPg); }
    SVBT16  rgwSpareDocSum[2];
};
// cbDOP is 66.

struct W1_CHP /////////////////////////////////////////////////////////
{
    SVBT16  fChar;
    SVBT16  ftc;// Font Code
    SVBT8   hps;// Font size in half points
    SVBT8   hpsPos;// Sub/Superscript ( signed number, 0 = normal )
    SVBT16  fText;
    SVBT32  fcPic;// not stored in File
    SVBT8   fnPic;// internal
    SVBT16  hpsLargeChp;// ???

    W1_CHP() { memset( this, 0, sizeof( *this)); }

    USHORT fCharGet()       { return SVBT16ToShort(fChar); }
    void fCharSet(USHORT n) { ShortToSVBT16(n, fChar); }
    BOOL fBoldGet()         { return ((fCharGet() >> 0) & 1); }
    void fBoldSet(BOOL b)   { fCharSet( ( fCharGet() & 0xfffe ) | ( b << 0 ) ); }
    BOOL fItalicGet()       { return ((fCharGet() >> 1) & 1); }
    void fItalicSet(BOOL b) { fCharSet( ( fCharGet() & 0xfffd ) | ( b << 1 ) ); }
    BOOL fStrikeGet()       { return ((fCharGet() >> 2) & 1); }
    BOOL fOutlineGet()      { return ((fCharGet() >> 3) & 1); }
    BOOL fFldVanishGet()    { return ((fCharGet() >> 4) & 1); }
    BOOL fSmallCapsGet()    { return ((fCharGet() >> 5) & 1); }
    BOOL fCapsGet()         { return ((fCharGet() >> 6) & 1); }
    BOOL fVanishGet()       { return ((fCharGet() >> 7) & 1); }
    BOOL fRMarkGet()        { return ((fCharGet() >> 8) & 1); }
    BOOL fSpecGet()         { return ((fCharGet() >> 9) & 1); }
    BOOL fsIcoGet()         { return ((fCharGet() >> 10) & 1); }
    BOOL fsFtcGet()         { return ((fCharGet() >> 11) & 1); }
    void fsFtcSet(BOOL b)   { fCharSet( ( fCharGet() & 0xf7ff ) | ( b << 11 ) ); }
    BOOL fsHpsGet()         { return ((fCharGet() >> 12) & 1); }
    void fsHpsSet(BOOL b)   { fCharSet( ( fCharGet() & 0xefff ) | ( b << 12 ) ); }
    BOOL fsKulGet()         { return ((fCharGet() >> 13) & 1); }
    void fsKulSet(BOOL b)   { fCharSet( ( fCharGet() & 0xdfff ) | ( b << 13 ) ); }
    BOOL fsPosGet()         { return ((fCharGet() >> 14) & 1); }
    BOOL fsSpaceGet()       { return ((fCharGet() >> 15) & 1); }
    // SVBT16 fBold :1;// 1 == opposite boldness of style
    // SVBT16 fItalic :1;// 1 == opposite of style
    // SVBT16 fStrike :1;// 1 == opposite of style
    // SVBT16 fOutline :1;// 1 == opposite of style
    // SVBT16 fFldVanish :1;// 1 == opposite of style
    // SVBT16 fSmallCaps :1;// 1 == opposite of style
    // SVBT16 fCaps :1;// 1 == opposite of style
    // SVBT16 fVanish :1;// 1 == opposite of style
    // SVBT16 fRMark :1;// ???
    // SVBT16 fSpec :1;// 1 == opposite of style
    // SVBT16 fsIco :1;// 1 == Color (ico) different to style
    // SVBT16 fsFtc :1;// 1 == FontCode (ftc) different to style
    // SVBT16 fsHps :1;// 1 == FontSize (hps) different to style
    // SVBT16 fsKul :1;// 1 == Underline Code (kul) different to style
    // SVBT16 fsPos :1;// 1 == Char position (hpsPos) different to style
    // SVBT16 fsSpace :1;// 1 == Char Spacing (qpsSpace) different to style

    USHORT ftcGet()         { return SVBT16ToShort(ftc); }
    void ftcSet(USHORT n)   { ShortToSVBT16(n, ftc); }
    void hpsSet(BYTE n)     { ByteToSVBT8(n, hps); }
    BYTE hpsGet()           { return SVBT8ToByte(hps); }

    BYTE hpsPosGet()        { return SVBT8ToByte(hpsPos); }
    USHORT fTextGet()       { return SVBT16ToShort(fText); }
    void fTextSet(USHORT n) { ShortToSVBT16(n, fText); }
    USHORT qpsSpaceGet()    { return (USHORT)((fTextGet() >> 0) & 0x3f); }
    USHORT wSpare2Get()     { return (USHORT)((fTextGet() >> 6) & 3); }
    USHORT icoGet()         { return (USHORT)((fTextGet() >> 8) & 0xf); }
    USHORT kulGet()         { return (USHORT)((fTextGet() >> 12) & 7); }
    void kulSet(USHORT n)   { fTextSet( ( fTextGet() & 0x8fff ) | ( ( n & 7 ) << 12 ) ); }
    BOOL fSysVanishGet()    { return ((fTextGet() >> 15) & 1); }
    // SVBT16 qpsSpace :6;// Char Spacing, -7 .. 56; 57 = -7, 63 = -1
    // SVBT16 wSpare2 : 2;// reserved
    // SVBT16 ico :4;// color of Text: 0=black, 1=blue, 2=cyan, 3=green, 4=magenta, 5=red, 6=yellow, 7=white
    // SVBT16 kul: 3;// Underline code: 0=none, 1=single, 2=by word, 3=double, 4=dotted
    // SVBT16 fSysVanish: 1;// used internally

    ULONG fcPicGet()        { return SVBT32ToLong(fcPic); }
    USHORT fnPicGet()       { return SVBT8ToByte(fnPic); }
    USHORT hpsLargeChpGet() { return SVBT16ToShort(hpsLargeChp); }

    void Out(Ww1Shell&, Ww1Manager&);
};

struct W1_FFN ///////////////////////////////////////// Font Descriptor
{
    SVBT8 cbFfnM1;// 0x0    total length of FFN - 1.
    USHORT cbFfnM1Get() {
        return SVBT8ToByte(cbFfnM1); }
    SVBT8 fFlags;
    USHORT fFlagsGet() {
        return SVBT8ToByte(fFlags); }
    // SVBT8 prg : 2;// 0x1:03  pitch request
    USHORT prgGet() {
        return (USHORT)((fFlagsGet() >> 0) & 3); }
    // SVBT8 fTrueType : 1;//   0x1:04  when 1, font is a TrueType font
    BOOL fTrueTypeGet() {
        return ((fFlagsGet() >> 2) & 1); }
    // SVBT8 : 1;// 0x1:08  reserved
    // SVBT8 ff : 3;//  0x1:70  font family id
    USHORT ffGet() {
        return (USHORT)((fFlagsGet() >> 4) & 7); }
    // SVBT8 : 1;// 0x1:80  reserved
    BYTE szFfn[65];// 0x6       zero terminated string that records name of font.
                            // Vorsicht: Dieses Array kann auch kleiner sein!!!
                            // Possibly followed by a second sz which records the name of an
                            // alternate font to use if the first named font does not exist
                            // on this system. Maximal size of szFfn is 65 characters.
    BYTE* szFfnGet() { return szFfn; }
};

struct W1_PHE /////////////////////////////////////// Paragraph Height
{
    SVBT16 fFlags;
    USHORT fFlagsGet() {
        return SVBT16ToShort(fFlags); }
    BOOL fSpareGet() {
        return fFlagsGet() & 1; }
    BOOL fUnkGet() {
        return (fFlagsGet() >> 1) & 1; }
    BOOL fDiffLinesGet() {
        return (fFlagsGet() >> 2) & 1; }
    BYTE clMacGet() {
        return (fFlagsGet() >> 8) & 0xff; }
    SVBT16 dxaCol;
    USHORT dxaColGet() {
        return SVBT16ToShort(dxaCol); }
    SVBT16 xxx; // beachte die dreifachnutzung.... siehe doc.
    USHORT dylHeightGet() {
        return SVBT16ToShort(xxx); }
    USHORT dylLineGet() {
        return SVBT16ToShort(xxx); }
    USHORT fStyleDirtyGet() {
        return SVBT16ToShort(xxx); }
};

struct W1_PAPX ///////////////////////// Paragraph Property Difference
{
    SVBT8 stc;
    BYTE stcGet() {
        return SVBT8ToByte(stc); }
    W1_PHE phe;
    BYTE grpprl[1];
    BYTE* grpprlGet() {
        return grpprl; }
};

struct W1_BRC //////////////////////////////////////////// Border Code
{
    SVBT16 aBits;
    USHORT aBitsGet() {
        return SVBT16ToShort(aBits); }
    // SVBT16 dxpLineWidth : 3;// 0007 When dxpLineWidth is 0, 1, 2, 3, 4, or 5,
                                // this field is the width of a single line of border
                                // in units of 0.75 points Must be nonzero when brcType
                                // is nonzero. 6 == dotted, 7 == dashed.
    BYTE dxpLineWidthGet() {
        return (aBitsGet() >> 0) & 0x0007; }
    // SVBT16 brcType : 2;// 0018 border type code: 0 == none, 1 == single, 2 == thick,
                            // 3 == double
    BYTE brcTypeGet() {
        return (aBitsGet() >> 3) & 0x0003; }
    // SVBT16 fShadow : 1;// 0020   when 1, border is drawn with shadow. Must be 0
                            // when BRC is a substructure of the TC
    BYTE fShadowGet() {
        return (aBitsGet() >> 5) & 0x0001; }
    // SVBT16 ico : 5;// 07C0 color code (see chp.ico)
    BYTE icoGet() {
        return (aBitsGet() >> 6) & 0x001f; }
    // SVBT16 dxpSpace : 5;// F800  width of space to maintain between border and
                                // text within border. Must be 0 when BRC is a
                                // substructure of the TC. Stored in points for Windows.
    BYTE dxpSpaceGet() {
        return (aBitsGet() >> 11) & 0x001f; }
};

struct W1_BRC10 ///////////////////////////////// Border Code Word 1.0
{
    SVBT16 aBits;
    USHORT aBitsGet() {
        return SVBT16ToShort(aBits); }
    BYTE dxpLine2WidthGet() {
        return (aBitsGet() >> 0) & 0x0007; }
    BYTE dxpSpaceBetweenGet() {
        return (aBitsGet() >> 3) & 0x0007; }
    BYTE dxpLine1WidthGet() {
        return (aBitsGet() >> 6) & 0x0007; }
    BYTE dxpSpaceGet() {
        return (aBitsGet() >> 9) & 0x001f; }
    BYTE fShadowGet() {
        return (aBitsGet() >> 14) & 0x0001; }
    BYTE fSpareGet() {
        return (aBitsGet() >> 15) & 0x0001; }
};

struct W1_FLD //////////////////////////////////////// FieldDescriptor
{
    SVBT8 ch; // boundary-type (begin(19), separator (20), end (21))
    BYTE chGet() {
        return SVBT8ToByte(ch); }
    SVBT8 flt; // field type / flags
    BYTE fltGet() {
        return SVBT8ToByte(flt); }
    // variant, when ch==21:
    BOOL fDifferGet() {
        return (fltGet() >> 0) & 1; }
    BOOL fResultDirtyGet() {
        return (fltGet() >> 2) & 1; }
    BOOL ResultEditedGet() {
        return (fltGet() >> 3) & 1; }
    BOOL fLockedGet() {
        return (fltGet() >> 4) & 1; }
    BOOL fPrivateResultGet() {
        return (fltGet() >> 5) & 1; }
    BOOL fNestedGet() {
        return (fltGet() >> 6) & 1; }
};

struct W1_PIC /////////////////////////////////////// PictureStructure
{
    SVBT32 lcb;// 0x0 number of bytes in the PIC structure plus size of following picture data which may be a Window's metafile, a bitmap, or the filename of a TIFF file.
    ULONG lcbGet() {
        return SVBT32ToLong(lcb); }
    SVBT16 cbHeader;// 0x4 number of bytes in the PIC (to allow for future expansion).
    USHORT cbHeaderGet() {
        return SVBT16ToShort(cbHeader); }
    struct MFP {
        SVBT16 mm;// 0x6    int
        USHORT mmGet() {
            return SVBT16ToShort(mm); }
        SVBT16 xExt;// 0x8  int
        USHORT xExtGet() {
            return SVBT16ToShort(xExt); }
        SVBT16 yExt;// 0xa  int
        USHORT yExtGet() {
            return SVBT16ToShort(yExt); }
        SVBT16 hMF;// 0xc   int
        USHORT hMFGet() {
            return SVBT16ToShort(hMF); }
    } mfp;
    union W1_MFP_BMP {
        SVBT8 bm[14];// 0xe BITMAP(14 bytes)    Window's bitmap structure when PIC describes a BITMAP.
        SVBT8 rcWinMF[14];// 0xe    rc (rectangle - 8 bytes) rect for window origin and extents when metafile is stored -- ignored if 0
    } MFP_BMP;
    SVBT16 dxaGoal;// 0x1c  horizontal measurement in twips of the rectangle the picture should be imaged within.
    USHORT dxaGoalGet() {
        return SVBT16ToShort(dxaGoal); }
    SVBT16 dyaGoal;// 0x1e  vertical measurement in twips of the rectangle the picture should be imaged within.
    USHORT dyaGoalGet() {
        return SVBT16ToShort(dyaGoal); }
    SVBT16 mx;// 0x20   horizontal scaling factor supplied by user in .1% units.
    USHORT mxGet() {
        return SVBT16ToShort(mx); }
    SVBT16 my;// 0x22   vertical scaling factor supplied by user in .1% units.
    USHORT myGet() {
        return SVBT16ToShort(my); }
    SVBT16 dxaCropLeft;// 0x24  the amount the picture has been cropped on the left in twips.
    USHORT dxaCropLeftGet() {
        return SVBT16ToShort(dxaCropLeft); }
    SVBT16 dyaCropTop;// 0x26   the amount the picture has been cropped on the top in twips.
    USHORT dyaCropTopGet() {
        return SVBT16ToShort(dyaCropTop); }
    SVBT16 dxaCropRight;// 0x28 the amount the picture has been cropped on the right in twips.
    USHORT dxaCropRightGet() {
        return SVBT16ToShort(dxaCropRight); }
    SVBT16 dyaCropBottom;// 0x2a    the amount the picture has been cropped on the bottom in twips.
    USHORT dyaCropBottomGet() {
        return SVBT16ToShort(dyaCropBottom); }
    SVBT16 flags;
    USHORT flagsGet() {
        return SVBT16ToShort(flags); }
//  brcl : 4;// 000F    Obsolete, superseded by brcTop, etc. In
    BYTE brclGet() {
        return flagsGet() & 0xf; }
//  fFrameEmpty : 1;// 0010 picture consists of a single frame
    BOOL fFrameEmptyGet() {
        return (flagsGet() >> 4) & 1; }
// win6 stuff:
//  fBitmap : 1;// 0020 ==1, when picture is just a bitmap
//  BOOL fBitmapGet() {
//      return (flagsGet() >> 5) & 1; }
//  fDrawHatch : 1;// 0040  ==1, when picture is an active OLE object
//  BOOL fDrawHatchGet() {
//      return (flagsGet() >> 6) & 1; }
//  fError : 1;// 0080  ==1, when picture is just an error message
//  BOOL fErrorGet() {
//      return (flagsGet() >> 7) & 1; }
//  bpp : 8;// FF00 bits per pixel, 0 = unknown
//  BYTE bppGet() {
//      return (flagsGet() >> 8) & 0xff; }
//  SVBT16 rgbrc[4];
//  USHORT rgbrcGet(USHORT nIndex) {
//      return SVBT16ToShort(rgbrc[nIndex]); }
//  W1_BRC brcTop;// 0x2e   specification for border above picture
//  W1_BRC brcLeft;// 0x30 specification for border to the left
//  W1_BRC brcBottom;// 0x32    specification for border below picture
//  W1_BRC brcRight;// 0x34 specification for border to the right
//  SVBT16 dxaOrigin;// 0x36    horizontal offset of hand annotation origin
//  USHORT dxaOriginGet() {
//      return SVBT16ToShort(dxaOrigin); }
//  SVBT16 dyaOrigin;// 0x38    vertical offset of hand annotation origin
//  USHORT dyaOriginGet() {
//      return SVBT16ToShort(dyaOrigin); }
    SVBT8 rgb;// 0x3a   variable array of bytes containing Window's metafile, bitmap or TIFF file filename.
    BYTE* rgbGet() {
        return rgb; }
};

struct W1_TBD /////////////////////////////////////////////////////////
{
    SVBT8 aBits1;
    BYTE aBits1Get() {
        return SVBT8ToByte(aBits1); }
// SVBT8 jc : 3;// 0x07 justification code: 0=left tab, 1=centered tab, 2=right tab, 3=decimal tab, 4=bar
    BYTE jcGet() {
        return aBits1Get() & 0x07; }
// SVBT8 tlc : 3;// 0x38    tab leader code: 0=no leader, 1=dotted leader,
                        // 2=hyphenated leader, 3=single line leader, 4=heavy line leader
    BYTE tlcGet() {
        return (aBits1Get() >> 3 ) & 0x07; }
// *    int :2  C0  reserved
};

struct W1_TC //////////////////////////////////////////////////////////
{
    SVBT8 aBits1;
    BYTE aBits1Get() {
        return SVBT8ToByte(aBits1); }
    BYTE fFirstMergedGet() {
        return aBits1Get() & 0x01; }
    BYTE fMergedGet() {
        return (aBits1Get() >> 1 ) & 0x01; }
    SVBT8 aBits2;
// SVBT16 fFirstMerged : 1;// 0001  set to 1 when cell is first cell of a range of cells that have been merged.
// SVBT16 fMerged : 1;// 0002   set to 1 when cell has been merged with preceding cell.
// SVBT16 fUnused : 14;// FFFC  reserved
    W1_BRC10 rgbrc[4];// notational convenience for referring to brcTop, brcLeft, etc fields.
    W1_BRC10* rgbrcGet() {
        return rgbrc; }
// BRC brcTop;// specification of the top border of a table cell
// BRC brcLeft;// specification of left border of table row
// BRC brcBottom;// specification of bottom border of table row
// BRC brcRight;// specification of right border of table row.
};
// cbTC (count of bytes of a TC) is 10(decimal), A(hex).

#if 0
struct W1_SHD ////////////////////////////////////////////// struct SHD
{
    SVBT16 aBits1;
    // SVBT16 nFore : 5;// 0x001f ForegroundColor
    USHORT GetFore() { return SVBT16ToShort(aBits1) & 0x01; };
    void SetFore( short nVal ){
        ShortToSVBT16((SVBT16ToShort(aBits1)&0xffe0)|(nVal&0x1f), aBits1); };
    // SVBT16 nBack : 5;// 0x03e0 BackgroundColor
    W1_SHD() { ShortToSVBT16( 0, aBits1 ); };
    USHORT GetBack() { return (SVBT16ToShort(aBits1) >> 5 & 0x1f ); };
    void SetBack( short nVal ){
        ShortToSVBT16((SVBT16ToShort(aBits1)&0xfc1f)|(nVal&0x1f)<<5, aBits1); };
    // SVBT16 nStyle : 5;// 0x7c00 Percentage and Style
    USHORT GetStyle() { return (SVBT16ToShort(aBits1) >> 10 & 0x1f ); };
    void SetStyle( short nVal ){
        ShortToSVBT16((SVBT16ToShort(aBits1)&0x83ff)|(nVal&0x1f)<<10, aBits1); };
    // SVBT16 nDontKnow : 1;// 0x8000 ???
};

struct W1_ANLV ////////////////////////////////////////////////////////
{
    SVBT8 nfc;// 0 number format code, 0=Arabic, 1=Upper case Roman, 2=Lower case Roman
                        // 3=Upper case Letter, 4=Lower case letter, 5=Ordinal
    SVBT8 cbTextBefore;// 1 offset into anld.rgch limit of prefix text
    SVBT8 cbTextAfter;// 2
    SVBT8 aBits1;
// SVBT8 jc : 2;// 3 : 0x03 justification code, 0=left, 1=center, 2=right, 3=left and right justify
// SVBT8 fPrev : 1;// 0x04  when ==1, include previous levels
// SVBT8 fHang : 1;// 0x08  when ==1, number will be displayed using a hanging indent
// SVBT8 fSetBold : 1;// 0x10   when ==1, boldness of number will be determined by anld.fBold.
// SVBT8 fSetItalic : 1;// 0x20 when ==1, italicness of number will be determined by anld.fItalic
// SVBT8 fSetSmallCaps : 1;// 0x40  when ==1, anld.fSmallCaps will determine whether number will be displayed in small caps or not.
// SVBT8 fSetCaps : 1;// 0x80   when ==1, anld.fCaps will determine whether number will be displayed capitalized or not
    SVBT8 aBits2;
// SVBT8 fSetStrike : 1;// 4 : 0x01 when ==1, anld.fStrike will determine whether the number will be displayed using strikethrough or not.
// SVBT8 fSetKul : 1;// 0x02 when ==1, anld.kul will determine the underlining state of the autonumber.
// SVBT8 fPrevSpace : 1;// 0x04 when ==1, autonumber will be displayed with a single prefixing space character
// SVBT8 fBold : 1;// 0x08  determines boldness of autonumber when anld.fSetBold == 1.
// SVBT8 fItalic : 1;// 0x10    determines italicness of autonumber when anld.fSetItalic == 1.
// SVBT8 fSmallCaps : 1;// 0x20 determines whether autonumber will be displayed using small caps when anld.fSetSmallCaps == 1.
// SVBT8 fCaps : 1;// 0x40  determines whether autonumber will be displayed using caps when anld.fSetCaps == 1.
// SVBT8 fStrike : 1;// 0x80    determines whether autonumber will be displayed using caps when anld.fSetStrike == 1.
    SVBT8 aBits3;
// SVBT8 kul : 3;// 5 : 0x07    determines whether autonumber will be displayed with underlining when anld.fSetKul == 1.
// SVBT8 ico : 5;// 0xF1    color of autonumber
    SVBT16 ftc;// 6 font code of autonumber
    SVBT16 hps;// 8 font half point size (or 0=auto)
    SVBT16 iStartAt;// 0x0a starting value (0 to 65535)
    SVBT16 dxaIndent;// 0x0c *short?* *USHORT?* width of prefix text (same as indent)
    SVBT16 dxaSpace;// 0x0e     minimum space between number and paragraph
};
// *cbANLV (count of bytes of ANLV) is 16 (decimal), 10(hex).

struct W1_ANLD ////////////////////////////////////////////////////////
{
    W1_ANLV eAnlv;// 0
    SVBT8 fNumber1;// 0x10      number only 1 item per table cell
    SVBT8 fNumberAcross;// 0x11     number across cells in table rows(instead of down)
    SVBT8 fRestartHdn;// 0x12       restart heading number on section boundary
    SVBT8 fSpareX;// 0x13       unused( should be 0)
    sal_Char rgchAnld[32];// 0x14   characters displayed before/after autonumber
};


struct W1_OLST ////////////////////////////////////////////////////////
{
    W1_ANLV rganlv[9];// 0 an array of 9 ANLV structures (heading levels)
    SVBT8 fRestartHdr;// 0x90 when ==1, restart heading on section break
    SVBT8 fSpareOlst2;// 0x91 reserved
    SVBT8 fSpareOlst3;// 0x92 reserved
    SVBT8 fSpareOlst4;// 0x93 reserved
    sal_Char rgch[64];// 0x94 array of 64 chars     text before/after number
};
// cbOLST is 212(decimal), D4(hex).

#endif
#endif

