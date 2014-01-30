/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SW_SOURCE_FILTER_WW1_W1STRUCT_HXX
#define INCLUDED_SW_SOURCE_FILTER_WW1_W1STRUCT_HXX

// star view
#include <string.h>
#include <tools/solar.h>

class Ww1Shell;
class Ww1Manager;

struct W1_FIB /////////////////////////////////////////////////////////
{
    SVBT16 wIdent;// 0x0 int magic number
    sal_uInt16 wIdentGet() {
        return SVBT16ToShort(wIdent); }
    SVBT16 nFib;// 0x2 FIB version written
    sal_uInt16 nFibGet() {
        return SVBT16ToShort(nFib); }
    SVBT16 nProduct;// 0x4 product version written by
    sal_uInt16 nProductGet() {
        return SVBT16ToShort(nProduct); }
    SVBT16 nlocale;// 0x6 language stamp---localized version;
    sal_uInt16 nlocaleGet() {
        return SVBT16ToShort(nlocale); }
    SVBT16 pnNext;// 0x8
    sal_uInt16 pnNextGet() {
        return SVBT16ToShort(pnNext); }
    SVBT16 fFlags;
    sal_uInt16 fFlagsGet() {
        return SVBT16ToShort(fFlags); }
    // SVBT16 fDot :1;// 0xa    0001
    sal_Bool fDotGet() {
        return 0 != ((fFlagsGet() >> 0) & 1); }
    // SVBT16 fGlsy :1;//       0002
    sal_Bool fGlsyGet() {
        return 0 != ((fFlagsGet() >> 1) & 1); }
    // SVBT16 fComplex :1;//        0004 when 1, file is in complex, fast-saved format.
    sal_Bool fComplexGet() {
        return 0 != ((fFlagsGet() >> 2) & 1); }
    // SVBT16 fHasPic :1;//     0008 file contains 1 or more pictures
    sal_Bool fHasPicGet() {
        return 0 != ((fFlagsGet() >> 3) & 1); }
    // SVBT16 cQuickSaves :4;//     00F0 count of times file was quicksaved
    sal_uInt16 cQuickSavesGet() {
        return (sal_uInt16)((fFlagsGet() >> 4) & 0xf); }
    // SVBT16 u1 :8;//      FF00 unused
    sal_uInt16 u1Get() {
        return (sal_uInt16)((fFlagsGet() >> 8) & 0xff); }
    SVBT16 nFibBack;// 0xc
    sal_uInt16 nFibBackGet() {
        return SVBT16ToShort(nFibBack); }
    SVBT16 u2[5];// 0xe reserved
    SVBT32 fcMin;// 0x18 file offset of first character of text
    sal_uLong fcMinGet() {
        return SVBT32ToUInt32(fcMin); }
    SVBT32 fcMac;// 0x1c file offset of last character of text + 1
    sal_uLong fcMacGet() {
        return SVBT32ToUInt32(fcMac); }
    SVBT32 cbMac;// 0x20 file offset of last byte written to file + 1.
    sal_uLong cbMacGet() {
        return SVBT32ToUInt32(cbMac); }
    SVBT32 u4[4];// 0x24 reserved
    SVBT32 ccpText;// 0x34 length of main document text stream
    sal_uLong ccpTextGet() {
        return SVBT32ToUInt32(ccpText); }
    SVBT32 ccpFtn;// 0x38 length of footnote subdocument text stream
    sal_uLong ccpFtnGet() {
        return SVBT32ToUInt32(ccpFtn); }
    SVBT32 ccpHdd;// 0x3c length of header subdocument text stream
    sal_uLong ccpHddGet() {
        return SVBT32ToUInt32(ccpHdd); }
    SVBT32 ccpMcr;// 0x40 length of macro subdocument text stream
    sal_uLong ccpMcrGet() {
        return SVBT32ToUInt32(ccpMcr); }
    SVBT32 ccpAtn;// 0x44 length of annotation subdocument text stream
    sal_uLong ccpAtnGet() {
        return SVBT32ToUInt32(ccpAtn); }
    SVBT32 cp5[4];// 0x48
    SVBT32 fcStshfOrig;// 0x58 file offset of original allocation for STSH in file
    sal_uLong fcStshfOrigGet() {
        return SVBT32ToUInt32(fcStshfOrig); }
    SVBT16 cbStshfOrig;// 0x5c count of bytes of original STSH allocation
    sal_uInt16 cbStshfOrigGet() {
        return SVBT16ToShort(cbStshfOrig); }
    SVBT32 fcStshf;// 0x5e file offset of STSH in file.
    sal_uLong fcStshfGet() {
        return SVBT32ToUInt32(fcStshf); }
    SVBT16 cbStshf;// 0x62 count of bytes of current STSH allocation
    sal_uInt16 cbStshfGet() {
        return SVBT16ToShort(cbStshf); }
    SVBT32 fcPlcffndRef;// 0x64 file offset of footnote reference PLC.
    sal_uLong fcPlcffndRefGet() {
        return SVBT32ToUInt32(fcPlcffndRef); }
    SVBT16 cbPlcffndRef;// 0x68 count of bytes of footnote reference PLC
    sal_uInt16 cbPlcffndRefGet() {
        return SVBT16ToShort(cbPlcffndRef); }
                            // == 0 if no footnotes defined in document.

    SVBT32 fcPlcffndTxt;// 0x6a file offset of footnote text PLC.
    sal_uLong fcPlcffndTxtGet() {
        return SVBT32ToUInt32(fcPlcffndTxt); }
    SVBT16 cbPlcffndTxt;// 0x6e count of bytes of footnote text PLC.
    sal_uInt16 cbPlcffndTxtGet() {
        return SVBT16ToShort(cbPlcffndTxt); }
                            // == 0 if no footnotes defined in document

    SVBT32 fcPlcfandRef;// 0x70 file offset of annotation reference PLC.
    sal_uLong fcPlcfandRefGet() {
        return SVBT32ToUInt32(fcPlcfandRef); }
    SVBT16 cbPlcfandRef;// 0x74 count of bytes of annotation reference PLC.
    sal_uInt16 cbPlcfandRefGet() {
        return SVBT16ToShort(cbPlcfandRef); }

    SVBT32 fcPlcfandTxt;// 0x76 file offset of annotation text PLC.
    sal_uLong fcPlcfandTxtGet() {
        return SVBT32ToUInt32(fcPlcfandTxt); }
    SVBT16 cbPlcfandTxt;// 0x7a count of bytes of the annotation text PLC
    sal_uInt16 cbPlcfandTxtGet() {
        return SVBT16ToShort(cbPlcfandTxt); }

    SVBT32 fcPlcfsed;// 8x7c file offset of section descriptor PLC.
    sal_uLong fcPlcfsedGet() {
        return SVBT32ToUInt32(fcPlcfsed); }
    SVBT16 cbPlcfsed;// 0x80 count of bytes of section descriptor PLC.
    sal_uInt16 cbPlcfsedGet() {
        return SVBT16ToShort(cbPlcfsed); }

    SVBT32 fcPlcfpgd;// 0x82 file offset of paragraph descriptor PLC
    sal_uLong fcPlcfpgdGet() {
        return SVBT32ToUInt32(fcPlcfpgd); }
    SVBT16 cbPlcfpgd;// 0x86 count of bytes of paragraph descriptor PLC.
    sal_uInt16 cbPlcfpgdGet() {
        return SVBT16ToShort(cbPlcfpgd); }
                            // ==0 if file was never repaginated
                            // Should not be written by third party creators

    SVBT32 fcPlcfphe;// 0x88 file offset of PLC of paragraph heights.
    sal_uLong fcPlcfpheGet() {
        return SVBT32ToUInt32(fcPlcfphe); }
    SVBT16 cbPlcfphe;// 0x8c count of bytes of paragraph height PLC.
    sal_uInt16 cbPlcfpheGet() {
        return SVBT16ToShort(cbPlcfphe); }
                            // ==0 when file is non-complex.

    SVBT32 fcSttbfglsy;// 0x8e file offset of glossary string table.
    sal_uLong fcSttbfglsyGet() {
        return SVBT32ToUInt32(fcSttbfglsy); }
    SVBT16 cbSttbfglsy;// 0x92 count of bytes of glossary string table.
    sal_uInt16 cbSttbfglsyGet() {
        return SVBT16ToShort(cbSttbfglsy); }
                            // == 0 for non-glossary documents.
                            // !=0 for glossary documents.

    SVBT32 fcPlcfglsy;// 0x94 file offset of glossary PLC.
    sal_uLong fcPlcfglsyGet() {
        return SVBT32ToUInt32(fcPlcfglsy); }
    SVBT16 cbPlcfglsy;// 0x98 count of bytes of glossary PLC.
    sal_uInt16 cbPlcfglsyGet() {
        return SVBT16ToShort(cbPlcfglsy); }
                            // == 0 for non-glossary documents.
                            // !=0 for glossary documents.

    SVBT32 fcPlcfhdd;// 0x9a byte offset of header PLC.
    sal_uLong fcPlcfhddGet() {
        return SVBT32ToUInt32(fcPlcfhdd); }
    SVBT16 cbPlcfhdd;// 0x9e count of bytes of header PLC.
    sal_uInt16 cbPlcfhddGet() {
        return SVBT16ToShort(cbPlcfhdd); }
                            // == 0 if document contains no headers

    SVBT32 fcPlcfbteChpx;// 0xa0 file offset of character property bin table.PLC.
    sal_uLong fcPlcfbteChpxGet() {
        return SVBT32ToUInt32(fcPlcfbteChpx); }
    SVBT16 cbPlcfbteChpx;// 0xa4 count of bytes of character property bin table PLC.
    sal_uInt16 cbPlcfbteChpxGet() {
        return SVBT16ToShort(cbPlcfbteChpx); }

    SVBT32 fcPlcfbtePapx;// 0xa6 file offset of paragraph property bin table.PLC.
    sal_uLong fcPlcfbtePapxGet() {
        return SVBT32ToUInt32(fcPlcfbtePapx); }
    SVBT16 cbPlcfbtePapx;// 0xaa count of bytes of paragraph property bin table PLC.
    sal_uInt16 cbPlcfbtePapxGet() {
        return SVBT16ToShort(cbPlcfbtePapx); }

    SVBT32 fcPlcfsea;// 0xac file offset of PLC reserved for private use. The SEA is 6 bytes long.
    sal_uLong fcPlcfseaGet() {
        return SVBT32ToUInt32(fcPlcfsea); }
    SVBT16 cbPlcfsea;// 0xb0    count of bytes of private use PLC.
    sal_uInt16 cbPlcfseaGet() {
        return SVBT16ToShort(cbPlcfsea); }

    SVBT32 fcSttbfffn;// 0xb2   file offset of font information STTBF. See the FFN file structure definition.
    sal_uLong fcSttbfffnGet() {
        return SVBT32ToUInt32(fcSttbfffn); }
    SVBT16 cbSttbfffn;// 0xb6   count of bytes in sttbfffn.
    sal_uInt16 cbSttbfffnGet() {
        return SVBT16ToShort(cbSttbfffn); }

    SVBT32 fcPlcffldMom;// 0xb8 offset in doc stream to the PLC of field positions in the main document.
    sal_uLong fcPlcffldMomGet() {
        return SVBT32ToUInt32(fcPlcffldMom); }
    SVBT16 cbPlcffldMom;// 0xbc
    sal_uInt16 cbPlcffldMomGet() {
        return SVBT16ToShort(cbPlcffldMom); }

    SVBT32 fcPlcffldHdr;// 0xbe offset in doc stream to the PLC of field positions in the header subdocument.
    sal_uLong fcPlcffldHdrGet() {
        return SVBT32ToUInt32(fcPlcffldHdr); }
    SVBT16 cbPlcffldHdr;// 0xc2
    sal_uInt16 cbPlcffldHdrGet() {
        return SVBT16ToShort(cbPlcffldHdr); }

    SVBT32 fcPlcffldFtn;// 0xc4 offset in doc stream to the PLC of field positions in the footnote subdocument.
    sal_uLong fcPlcffldFtnGet() {
        return SVBT32ToUInt32(fcPlcffldFtn); }
    SVBT16 cbPlcffldFtn;// 0xc8
    sal_uInt16 cbPlcffldFtnGet() {
        return SVBT16ToShort(cbPlcffldFtn); }

    SVBT32 fcPlcffldAtn;// 0xca offset in doc stream to the PLC of field positions in the annotation subdocument.
    sal_uLong fcPlcffldAtnGet() {
        return SVBT32ToUInt32(fcPlcffldAtn); }
    SVBT16 cbPlcffldAtn;// 0xce
    sal_uInt16 cbPlcffldAtnGet() {
        return SVBT16ToShort(cbPlcffldAtn); }

    SVBT32 fcPlcffldMcr;// 0xd0 offset in doc stream to the PLC of field positions in the macro subdocument.
    sal_uLong fcPlcffldMcrGet() {
        return SVBT32ToUInt32(fcPlcffldMcr); }
    SVBT16 cbPlcffldMcr;// 0xd4
    sal_uInt16 cbPlcffldMcrGet() {
        return SVBT16ToShort(cbPlcffldMcr); }

    SVBT32 fcSttbfbkmk;// 0xd6 offset in document stream of the STTBF that records bookmark names in the main document
    sal_uLong fcSttbfbkmkGet() {
        return SVBT32ToUInt32(fcSttbfbkmk); }
    SVBT16 cbSttbfbkmk;// 0xda
    sal_uInt16 cbSttbfbkmkGet() {
        return SVBT16ToShort(cbSttbfbkmk); }

    SVBT32 fcPlcfbkf;// 0xdc offset in document stream of the PLCF that records the beginning CP offsets of bookmarks in the main document. See BKF
    sal_uLong fcPlcfbkfGet() {
        return SVBT32ToUInt32(fcPlcfbkf); }
    SVBT16 cbPlcfbkf;// 0xe0
    sal_uInt16 cbPlcfbkfGet() {
        return SVBT16ToShort(cbPlcfbkf); }

    SVBT32 fcPlcfbkl;// 0xe2 offset in document stream of the PLCF that records the ending CP offsets of bookmarks recorded in the main document. See the BKL structure definition.
    sal_uLong fcPlcfbklGet() {
        return SVBT32ToUInt32(fcPlcfbkl); }
    SVBT16 cbPlcfbkl;// 0xe6 SVBT16
    sal_uInt16 cbPlcfbklGet() {
        return SVBT16ToShort(cbPlcfbkl); }

    SVBT32 fcCmds;// 0xe8 FC
    sal_uLong fcCmdsGet() {
        return SVBT32ToUInt32(fcCmds); }
    SVBT16 cbCmds;// 0xec
    sal_uInt16 cbCmdsGet() {
        return SVBT16ToShort(cbCmds); }

    SVBT32 fcPlcmcr;// 0xee FC
    sal_uLong fcPlcmcrGet() {
        return SVBT32ToUInt32(fcPlcmcr); }
    SVBT16 cbPlcmcr;// 0xf2
    sal_uInt16 cbPlcmcrGet() {
        return SVBT16ToShort(cbPlcmcr); }

    SVBT32 fcSttbfmcr;// 0xf4 FC
    sal_uLong fcSttbfmcrGet() {
        return SVBT32ToUInt32(fcSttbfmcr); }
    SVBT16 cbSttbfmcr;// 0xf8
    sal_uInt16 cbSttbfmcrGet() {
        return SVBT16ToShort(cbSttbfmcr); }

    SVBT32 fcPrEnv;// 0xfa
    sal_uLong fcPrEnvGet() {
        return SVBT32ToUInt32(fcPrEnv); }
    SVBT16 cbPrEnv;// 0xfe
    sal_uInt16 cbPrEnvGet() {
        return SVBT16ToShort(cbPrEnv); }

    SVBT32 fcWss;// 0x100 file offset of Window Save State data structure. See WSS.
    sal_uLong fcWssGet() {
        return SVBT32ToUInt32(fcWss); }
    SVBT16 cbWss;// 0x100 count of bytes of WSS. ==0 if unable to store the window state.
    sal_uInt16 cbWssGet() {
        return SVBT16ToShort(cbWss); }

    SVBT32 fcDop;// 0x106 file offset of document property data structure.
    sal_uLong fcDopGet() {
        return SVBT32ToUInt32(fcDop); }
    SVBT16 cbDop;// 0x10a count of bytes of document properties.
    sal_uInt16 cbDopGet() {
        return SVBT16ToShort(cbDop); }


    SVBT32 fcSttbfAssoc;// 0x10c offset to STTBF of associated strings. See STTBFASSOC.
    sal_uLong fcSttbfAssocGet() {
        return SVBT32ToUInt32(fcSttbfAssoc); }
    SVBT16 cbSttbfAssoc;// 0x110
    sal_uInt16 cbSttbfAssocGet() {
        return SVBT16ToShort(cbSttbfAssoc); }

    SVBT32 fcClx;// 0x112 file offset of beginning of information for complex files.
    sal_uLong fcClxGet() {
        return SVBT32ToUInt32(fcClx); }
    SVBT16 cbClx;// 0x116 count of bytes of complex file information. 0 if file is non-complex.
    sal_uInt16 cbClxGet() {
        return SVBT16ToShort(cbClx); }

    SVBT32 fcPlcfpgdFtn;// 0x118 file offset of page descriptor PLC for footnote subdocument.
    sal_uLong fcPlcfpgdFtnGet() {
        return SVBT32ToUInt32(fcPlcfpgdFtn); }
    SVBT16 cbPlcfpgdFtn;// 0x11C count of bytes of page descriptor PLC for footnote subdocument.
    sal_uInt16 cbPlcfpgdFtnGet() {
        return SVBT16ToShort(cbPlcfpgdFtn); }
                            // ==0 if document has not been paginated. The length of the PGD is 8 bytes.

    SVBT32 fcSpare1;// 0x11e file offset of the name of the original file.
    sal_uLong fcSpare1Get() {
        return SVBT32ToUInt32(fcSpare1); }
    SVBT16 cbSpare1;// 0x122 count of bytes of the name of the original file.
    sal_uInt16 cbSpare1Get() {
        return SVBT16ToShort(cbSpare1); }
    SVBT32 fcSpare2;// 0x124 file offset of the name of the original file.
    sal_uLong fcSpare2Get() {
        return SVBT32ToUInt32(fcSpare2); }
    SVBT16 cbSpare2;// 0x128 count of bytes of the name of the original file.
    sal_uInt16 cbSpare2Get() {
        return SVBT16ToShort(cbSpare2); }
    SVBT32 fcSpare3;// 0x12a file offset of the name of the original file.
    sal_uLong fcSpare3Get() {
        return SVBT32ToUInt32(fcSpare3); }
    SVBT16 cbSpare3;// 0x12e count of bytes of the name of the original file.
    sal_uInt16 cbSpare3Get() {
        return SVBT16ToShort(cbSpare3); }
    SVBT32 fcSpare4;// 0x130 file offset of the name of the original file.
    sal_uLong fcSpare4Get() {
        return SVBT32ToUInt32(fcSpare4); }
    SVBT16 cbSpare4;// 0x134 count of bytes of the name of the original file.
    sal_uInt16 cbSpare4Get() {
        return SVBT16ToShort(cbSpare4); }

    SVBT16 cpnBteChp;// 0x18E count of CHPX FKPs recorded in file. In non-complex files if the number of
    sal_uInt16 cpnBteChpGet() {
        return SVBT16ToShort(cpnBteChp); }
                            // entries in the plcfbteChpx is less than this, the plcfbteChpx is incomplete.
    SVBT16 cpnBtePap;// 0x190 count of PAPX FKPs recorded in file. In non-complex files if the number of
    sal_uInt16 cpnBtePapGet() {
        return SVBT16ToShort(cpnBtePap); }
                            // entries in the plcfbtePapx is less than this, the plcfbtePapx is incomplete.
};

struct W1_DOP ///////////////////////////////////// Document Properties
{
    SVBT16 fFlags;
    sal_uInt16 fFlagsGet() {
        return SVBT16ToShort(fFlags); }
    // SVBT16 fFacingPages : 1;// 1 when facing pages should be printed
    sal_Bool fFacingPagesGet() {
        return 0 != ((fFlagsGet() >> 0) & 1); }
    // SVBT16 fWidowControl : 1;// 1 when widow control is in effect. 0 when widow control disabled.
    sal_Bool fWidowControlGet() {
        return 0 != ((fFlagsGet() >> 1) & 1); }
    // SVBT16 : 3;// unused
    // SVBT16 fpc : 2;// 1 footnote position code: 0 as endnotes, 1 at bottom of page, 2 immediately beneath text
    sal_uInt16 fpcGet() {
        return (sal_uInt16)((fFlagsGet() >> 5) & 3); }
    // SVBT16 fWide : 1;// Landscape
    sal_Bool fWideGet() {
        return 0 != ((fFlagsGet() >> 7) & 1); }
    // SVBT16 grpfIhdt : 8;// 0 specification of document headers and footers. See explanation under Headers and Footers topic.
    sal_uInt16 grpfIhdtGet() {
        return (sal_uInt16)((fFlagsGet() >> 8) & 0xff); }
    SVBT16 fFtnFlags;
    sal_uInt16 fFtnFlagsGet() {
        return SVBT16ToShort(fFtnFlags); }
    // SVBT16 fFtnRestart : 1;
    sal_Bool fFtnRestartGet() {
        return 0 != ((fFtnFlagsGet() >> 0) & 1); }
    // SVBT16 nFtn : 15;// 1 initial footnote number for document
    sal_uInt16 nFtnGet() {
        return (sal_uInt16)((fFtnFlagsGet() >> 1) & 0x7fff); }
    SVBT16 fRvsFlags;
    sal_uInt16 fRvsFlagsGet() {
        return SVBT16ToShort(fRvsFlags); }
    // SVBT16 irmBar : 8;
    sal_uInt16 irmBarGet() {
        return (sal_uInt16)((fRvsFlagsGet() >> 0) & 0xff); }
    // SVBT16 irmProps : 7;
    sal_uInt16 irmPropsGet() {
        return (sal_uInt16)((fRvsFlagsGet() >> 8) & 0x7f); }
    // SVBT16 fRevMarking   : 1;//   when 1, Word will mark revisions as the document is edited
    sal_Bool fRevMarkingGet() {
        return 0 != ((fRvsFlagsGet() >> 15) & 1); }
    SVBT16 fSveFlags;
    sal_uInt16 fSveFlagsGet() {
        return SVBT16ToShort(fSveFlags); }
    // SVBT16 fBackup : 1;//     always make backup when document saved when 1.
    sal_Bool fBackupGet() {
        return 0 != ((fSveFlagsGet() >> 0) & 1); }
    // SVBT16 fExactCWords : 1;
    sal_Bool fExactCWordsGet() {
        return 0 != ((fSveFlagsGet() >> 1) & 1); }
    // SVBT16 fPagHidden : 1;//
    sal_Bool fPagHiddenGet() {
        return 0 != ((fSveFlagsGet() >> 2) & 1); }
    // SVBT16 fPagResults : 1;
    sal_Bool fPagResultsGet() {
        return 0 != ((fSveFlagsGet() >> 3) & 1); }
    // SVBT16 fLockAtn : 1;//    when 1, annotations are locked for editing
    sal_Bool fLockAtnGet() {
        return 0 != ((fSveFlagsGet() >> 4) & 1); }
    // SVBT16 fMirrorMargins : 1;//  swap margins on left/right pages when 1.
    sal_Bool fMirrorMarginsGet() {
        return 0 != ((fSveFlagsGet() >> 5) & 1); }
    // SVBT16 : 10;// unused
    SVBT16 fSpares;
    sal_uInt16 fSparesGet() {
        return SVBT16ToShort(fSpares); }
    SVBT16 yaPage;
    sal_uInt16 yaPageGet() {
        return SVBT16ToShort(yaPage); }
    SVBT16 xaPage;
    sal_uInt16 xaPageGet() {
        return SVBT16ToShort(xaPage); }
    SVBT16 dyaTop;
    sal_uInt16 dyaTopGet() {
        return SVBT16ToShort(dyaTop); }
    SVBT16 dxaLeft;
    sal_uInt16 dxaLeftGet() {
        return SVBT16ToShort(dxaLeft); }
    SVBT16 dyaBottom;
    sal_uInt16 dyaBottomGet() {
        return SVBT16ToShort(dyaBottom); }
    SVBT16 dxaRight;
    sal_uInt16 dxaRightGet() {
        return SVBT16ToShort(dxaRight); }
    SVBT16 dxaGutter;
    sal_uInt16 dxaGutterGet() {
        return SVBT16ToShort(dxaGutter); }
    SVBT16  dxaTab;// 720 twips default tab width
    sal_uInt16 dxaTabGet() {
        return SVBT16ToShort(dxaTab); }
    SVBT16  wSpare;//
    sal_uInt16 wSpareGet() {
        return SVBT16ToShort(wSpare); }
    SVBT16  dxaHotZ;// width of hyphenation hot zone measured in twips
    sal_uInt16 dxaHotZGet() {
        return SVBT16ToShort(dxaHotZ); }
    SVBT16  rgwSpare[2];// reserved
    SVBT32  dttmCreated;// DTTM date and time document was created
    sal_uLong dttmCreatedGet() {
        return SVBT32ToUInt32(dttmCreated); }
    SVBT32  dttmRevised;// DTTM date and time document was last revised
    sal_uLong dttmRevisedGet() {
        return SVBT32ToUInt32(dttmRevised); }
    SVBT32  dttmLastPrint;// DTTM date and time document was last printed
    sal_uLong dttmLastPrintGet() {
        return SVBT32ToUInt32(dttmLastPrint); }
    SVBT16  nRevision;// number of times document has been revised since its creation
    sal_uInt16 nRevisionGet() {
        return SVBT16ToShort(nRevision); }
    SVBT32  tmEdited;// time document was last edited
    sal_uLong tmEditedGet() {
        return SVBT32ToUInt32(tmEdited); }
    SVBT32  cWords;// count of words tallied by last Word Count execution
    sal_uLong cWordsGet() {
        return SVBT32ToUInt32(cWords); }
    SVBT32  cCh;// count of characters tallied by last Word Count execution
    sal_uLong cChGet() {
        return SVBT32ToUInt32(cCh); }
    SVBT16  cPg;// count of pages tallied by last Word Count execution
    sal_uInt16 cPgGet() {
        return SVBT16ToShort(cPg); }
    SVBT16  rgwSpareDocSum[2];
};
// cbDOP is 66.

struct W1_CHP /////////////////////////////////////////////////////////
{
    SVBT16  fChar;
    SVBT16  ftc;// Font Code
    sal_uInt8   hps;// Font size in half points
    sal_uInt8   hpsPos;// Sub/Superscript ( signed number, 0 = normal )
    SVBT16  fText;
    SVBT32  fcPic;// not stored in File
    sal_uInt8   fnPic;// internal
    SVBT16  hpsLargeChp;// ???

    W1_CHP() { memset( this, 0, sizeof( *this)); }

    sal_uInt16 fCharGet()       { return SVBT16ToShort(fChar); }
    void fCharSet(sal_uInt16 n) { ShortToSVBT16(n, fChar); }
    sal_Bool fBoldGet()         { return 0 != ((fCharGet() >> 0) & 1); }
    void fBoldSet(sal_Bool b)   { fCharSet( ( fCharGet() & 0xfffe ) | ( b << 0 ) ); }
    sal_Bool fItalicGet()       { return 0 != ((fCharGet() >> 1) & 1); }
    void fItalicSet(sal_Bool b) { fCharSet( ( fCharGet() & 0xfffd ) | ( b << 1 ) ); }
    sal_Bool fStrikeGet()       { return 0 != ((fCharGet() >> 2) & 1); }
    sal_Bool fOutlineGet()      { return 0 != ((fCharGet() >> 3) & 1); }
    sal_Bool fFldVanishGet()    { return 0 != ((fCharGet() >> 4) & 1); }
    sal_Bool fSmallCapsGet()    { return 0 != ((fCharGet() >> 5) & 1); }
    sal_Bool fCapsGet()         { return 0 != ((fCharGet() >> 6) & 1); }
    sal_Bool fVanishGet()       { return 0 != ((fCharGet() >> 7) & 1); }
    sal_Bool fRMarkGet()        { return 0 != ((fCharGet() >> 8) & 1); }
    sal_Bool fSpecGet()         { return 0 != ((fCharGet() >> 9) & 1); }
    sal_Bool fsIcoGet()         { return 0 != ((fCharGet() >> 10) & 1); }
    sal_Bool fsFtcGet()         { return 0 != ((fCharGet() >> 11) & 1); }
    void fsFtcSet(sal_Bool b)   { fCharSet( ( fCharGet() & 0xf7ff ) | ( b << 11 ) ); }
    sal_Bool fsHpsGet()         { return 0 != ((fCharGet() >> 12) & 1); }
    void fsHpsSet(sal_Bool b)   { fCharSet( ( fCharGet() & 0xefff ) | ( b << 12 ) ); }
    sal_Bool fsKulGet()         { return 0 != ((fCharGet() >> 13) & 1); }
    void fsKulSet(sal_Bool b)   { fCharSet( ( fCharGet() & 0xdfff ) | ( b << 13 ) ); }
    sal_Bool fsPosGet()         { return 0 != ((fCharGet() >> 14) & 1); }
    sal_Bool fsSpaceGet()       { return 0 != ((fCharGet() >> 15) & 1); }
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

    sal_uInt16 ftcGet()         { return SVBT16ToShort(ftc); }
    void ftcSet(sal_uInt16 n)   { ShortToSVBT16(n, ftc); }
    void hpsSet(sal_uInt8 n)    { hps = n; }
    sal_uInt8 hpsGet()          { return hps; }

    sal_uInt8 hpsPosGet()       { return hpsPos; }
    sal_uInt16 fTextGet()       { return SVBT16ToShort(fText); }
    void fTextSet(sal_uInt16 n) { ShortToSVBT16(n, fText); }
    sal_uInt16 qpsSpaceGet()    { return (sal_uInt16)((fTextGet() >> 0) & 0x3f); }
    sal_uInt16 wSpare2Get()     { return (sal_uInt16)((fTextGet() >> 6) & 3); }
    sal_uInt16 icoGet()         { return (sal_uInt16)((fTextGet() >> 8) & 0xf); }
    sal_uInt16 kulGet()         { return (sal_uInt16)((fTextGet() >> 12) & 7); }
    void kulSet(sal_uInt16 n)   { fTextSet( ( fTextGet() & 0x8fff ) | ( ( n & 7 ) << 12 ) ); }
    sal_Bool fSysVanishGet()    { return 0 != ((fTextGet() >> 15) & 1); }
    // SVBT16 qpsSpace :6;// Char Spacing, -7 .. 56; 57 = -7, 63 = -1
    // SVBT16 wSpare2 : 2;// reserved
    // SVBT16 ico :4;// color of Text: 0=black, 1=blue, 2=cyan, 3=green, 4=magenta, 5=red, 6=yellow, 7=white
    // SVBT16 kul: 3;// Underline code: 0=none, 1=single, 2=by word, 3=double, 4=dotted
    // SVBT16 fSysVanish: 1;// used internally

    sal_uLong fcPicGet()        { return SVBT32ToUInt32(fcPic); }
    sal_uInt16 fnPicGet()       { return fnPic; }
    sal_uInt16 hpsLargeChpGet() { return SVBT16ToShort(hpsLargeChp); }

    void Out(Ww1Shell&, Ww1Manager&);
};

struct W1_FFN ///////////////////////////////////////// Font Descriptor
{
    sal_uInt8 cbFfnM1;// 0x0    total length of FFN - 1.
    sal_uInt16 cbFfnM1Get() {
        return static_cast<sal_uInt8>(cbFfnM1); }
    sal_uInt8 fFlags;
    sal_uInt16 fFlagsGet() {
        return static_cast<sal_uInt8>(fFlags); }
    // sal_uInt8 prg : 2;// 0x1:03  pitch request
    sal_uInt16 prgGet() {
        return (sal_uInt16)((fFlagsGet() >> 0) & 3); }
    // sal_uInt8 fTrueType : 1;//   0x1:04  when 1, font is a TrueType font
    sal_Bool fTrueTypeGet() {
        return 0 != ((fFlagsGet() >> 2) & 1); }
    // sal_uInt8 : 1;// 0x1:08  reserved
    // sal_uInt8 ff : 3;//  0x1:70  font family id
    sal_uInt16 ffGet() {
        return (sal_uInt16)((fFlagsGet() >> 4) & 7); }
    // sal_uInt8 : 1;// 0x1:80  reserved
    sal_uInt8 szFfn[65];// 0x6      zero terminated string that records name of font.
                            // Vorsicht: Dieses Array kann auch kleiner sein!!!
                            // Possibly followed by a second sz which records the name of an
                            // alternate font to use if the first named font does not exist
                            // on this system. Maximal size of szFfn is 65 characters.
    sal_uInt8* szFfnGet() { return szFfn; }
};

struct W1_PHE /////////////////////////////////////// Paragraph Height
{
    SVBT16 fFlags;
    sal_uInt16 fFlagsGet() {
        return SVBT16ToShort(fFlags); }
    sal_Bool fSpareGet() {
        return 0 != (fFlagsGet() & 1); }
    sal_Bool fUnkGet() {
        return 0 != ((fFlagsGet() >> 1) & 1); }
    sal_Bool fDiffLinesGet() {
        return 0 != ((fFlagsGet() >> 2) & 1); }
    sal_uInt8 clMacGet() {
        return sal::static_int_cast< sal_uInt8, sal_uInt16 >((fFlagsGet() >> 8) & 0xff); }
    SVBT16 dxaCol;
    sal_uInt16 dxaColGet() {
        return SVBT16ToShort(dxaCol); }
    SVBT16 xxx; // beachte die dreifachnutzung.... siehe doc.
    sal_uInt16 dylHeightGet() {
        return SVBT16ToShort(xxx); }
    sal_uInt16 dylLineGet() {
        return SVBT16ToShort(xxx); }
    sal_uInt16 fStyleDirtyGet() {
        return SVBT16ToShort(xxx); }
};

struct W1_PAPX ///////////////////////// Paragraph Property Difference
{
    sal_uInt8 stc;
    sal_uInt8 stcGet() { return stc; }
    W1_PHE phe;
    sal_uInt8 grpprl[1];
    sal_uInt8* grpprlGet() {
        return grpprl; }
};

struct W1_BRC //////////////////////////////////////////// Border Code
{
    SVBT16 aBits;
    sal_uInt16 aBitsGet() {
        return SVBT16ToShort(aBits); }
    // SVBT16 dxpLineWidth : 3;// 0007 When dxpLineWidth is 0, 1, 2, 3, 4, or 5,
                                // this field is the width of a single line of border
                                // in units of 0.75 points Must be nonzero when brcType
                                // is nonzero. 6 == dotted, 7 == dashed.
    sal_uInt8 dxpLineWidthGet() {
        return sal::static_int_cast< sal_uInt8, sal_uInt16 >((aBitsGet() >> 0) & 0x0007); }
    // SVBT16 brcType : 2;// 0018 border type code: 0 == none, 1 == single, 2 == thick,
                            // 3 == double
    sal_uInt8 brcTypeGet() {
        return sal::static_int_cast< sal_uInt8, sal_uInt16 >((aBitsGet() >> 3) & 0x0003); }
    // SVBT16 fShadow : 1;// 0020   when 1, border is drawn with shadow. Must be 0
                            // when BRC is a substructure of the TC
    sal_uInt8 fShadowGet() {
        return sal::static_int_cast< sal_uInt8, sal_uInt16 >((aBitsGet() >> 5) & 0x0001); }
    // SVBT16 ico : 5;// 07C0 color code (see chp.ico)
    sal_uInt8 icoGet() {
        return sal::static_int_cast< sal_uInt8, sal_uInt16 >((aBitsGet() >> 6) & 0x001f); }
    // SVBT16 dxpSpace : 5;// F800  width of space to maintain between border and
                                // text within border. Must be 0 when BRC is a
                                // substructure of the TC. Stored in points for Windows.
    sal_uInt8 dxpSpaceGet() {
        return sal::static_int_cast< sal_uInt8, sal_uInt16 >((aBitsGet() >> 11) & 0x001f); }
};

struct W1_BRC10 ///////////////////////////////// Border Code Word 1.0
{
    SVBT16 aBits;
    sal_uInt16 aBitsGet() {
        return SVBT16ToShort(aBits); }
    sal_uInt8 dxpLine2WidthGet() {
        return sal::static_int_cast< sal_uInt8, sal_uInt16 >((aBitsGet() >> 0) & 0x0007); }
    sal_uInt8 dxpSpaceBetweenGet() {
        return sal::static_int_cast< sal_uInt8, sal_uInt16 >((aBitsGet() >> 3) & 0x0007); }
    sal_uInt8 dxpLine1WidthGet() {
        return sal::static_int_cast< sal_uInt8, sal_uInt16 >((aBitsGet() >> 6) & 0x0007); }
    sal_uInt8 dxpSpaceGet() {
        return sal::static_int_cast< sal_uInt8, sal_uInt16 >((aBitsGet() >> 9) & 0x001f); }
    sal_uInt8 fShadowGet() {
        return sal::static_int_cast< sal_uInt8, sal_uInt16 >((aBitsGet() >> 14) & 0x0001); }
    sal_uInt8 fSpareGet() {
        return sal::static_int_cast< sal_uInt8, sal_uInt16 >((aBitsGet() >> 15) & 0x0001); }
};

struct W1_FLD //////////////////////////////////////// FieldDescriptor
{
    sal_uInt8 ch; // boundary-type (begin(19), separator (20), end (21))
    sal_uInt8 chGet() { return ch; }
    sal_uInt8 flt; // field type / flags
    sal_uInt8 fltGet() { return flt; }
    // variant, when ch==21:
    sal_Bool fDifferGet() {
        return (fltGet() >> 0) & 1; }
    sal_Bool fResultDirtyGet() {
        return (fltGet() >> 2) & 1; }
    sal_Bool ResultEditedGet() {
        return (fltGet() >> 3) & 1; }
    sal_Bool fLockedGet() {
        return (fltGet() >> 4) & 1; }
    sal_Bool fPrivateResultGet() {
        return (fltGet() >> 5) & 1; }
    sal_Bool fNestedGet() {
        return (fltGet() >> 6) & 1; }
};

struct W1_PIC /////////////////////////////////////// PictureStructure
{
    SVBT32 lcb;// 0x0 number of bytes in the PIC structure plus size of following picture data which may be a Window's metafile, a bitmap, or the filename of a TIFF file.
    sal_uLong lcbGet() {
        return SVBT32ToUInt32(lcb); }
    SVBT16 cbHeader;// 0x4 number of bytes in the PIC (to allow for future expansion).
    sal_uInt16 cbHeaderGet() {
        return SVBT16ToShort(cbHeader); }
    struct MFP {
        SVBT16 mm;// 0x6    int
        sal_uInt16 mmGet() {
            return SVBT16ToShort(mm); }
        SVBT16 xExt;// 0x8  int
        sal_uInt16 xExtGet() {
            return SVBT16ToShort(xExt); }
        SVBT16 yExt;// 0xa  int
        sal_uInt16 yExtGet() {
            return SVBT16ToShort(yExt); }
        SVBT16 hMF;// 0xc   int
        sal_uInt16 hMFGet() {
            return SVBT16ToShort(hMF); }
    } mfp;
    union W1_MFP_BMP {
        sal_uInt8 bm[14];// 0xe BITMAP(14 bytes)    Window's bitmap structure when PIC describes a BITMAP.
        sal_uInt8 rcWinMF[14];// 0xe    rc (rectangle - 8 bytes) rect for window origin and extents when metafile is stored -- ignored if 0
    } MFP_BMP;
    SVBT16 dxaGoal;// 0x1c  horizontal measurement in twips of the rectangle the picture should be imaged within.
    sal_uInt16 dxaGoalGet() {
        return SVBT16ToShort(dxaGoal); }
    SVBT16 dyaGoal;// 0x1e  vertical measurement in twips of the rectangle the picture should be imaged within.
    sal_uInt16 dyaGoalGet() {
        return SVBT16ToShort(dyaGoal); }
    SVBT16 mx;// 0x20   horizontal scaling factor supplied by user in .1% units.
    sal_uInt16 mxGet() {
        return SVBT16ToShort(mx); }
    SVBT16 my;// 0x22   vertical scaling factor supplied by user in .1% units.
    sal_uInt16 myGet() {
        return SVBT16ToShort(my); }
    SVBT16 dxaCropLeft;// 0x24  the amount the picture has been cropped on the left in twips.
    sal_uInt16 dxaCropLeftGet() {
        return SVBT16ToShort(dxaCropLeft); }
    SVBT16 dyaCropTop;// 0x26   the amount the picture has been cropped on the top in twips.
    sal_uInt16 dyaCropTopGet() {
        return SVBT16ToShort(dyaCropTop); }
    SVBT16 dxaCropRight;// 0x28 the amount the picture has been cropped on the right in twips.
    sal_uInt16 dxaCropRightGet() {
        return SVBT16ToShort(dxaCropRight); }
    SVBT16 dyaCropBottom;// 0x2a    the amount the picture has been cropped on the bottom in twips.
    sal_uInt16 dyaCropBottomGet() {
        return SVBT16ToShort(dyaCropBottom); }
    SVBT16 flags;
    sal_uInt16 flagsGet() {
        return SVBT16ToShort(flags); }
//  brcl : 4;// 000F    Obsolete, superseded by brcTop, etc. In
    sal_uInt8 brclGet() {
        return sal::static_int_cast< sal_uInt8, sal_uInt16 >(flagsGet() & 0xf); }
//  fFrameEmpty : 1;// 0010 picture consists of a single frame
    sal_Bool fFrameEmptyGet() {
        return sal::static_int_cast< sal_uInt8, sal_uInt16 >((flagsGet() >> 4) & 1); }
// win6 stuff:
//  fBitmap : 1;// 0020 ==1, when picture is just a bitmap
//  sal_Bool fBitmapGet() {
//      return (flagsGet() >> 5) & 1; }
//  fDrawHatch : 1;// 0040  ==1, when picture is an active OLE object
//  sal_Bool fDrawHatchGet() {
//      return (flagsGet() >> 6) & 1; }
//  fError : 1;// 0080  ==1, when picture is just an error message
//  sal_Bool fErrorGet() {
//      return (flagsGet() >> 7) & 1; }
//  bpp : 8;// FF00 bits per pixel, 0 = unknown
//  sal_uInt8 bppGet() {
//      return (flagsGet() >> 8) & 0xff; }
//  SVBT16 rgbrc[4];
//  sal_uInt16 rgbrcGet(sal_uInt16 nIndex) {
//      return SVBT16ToShort(rgbrc[nIndex]); }
//  W1_BRC brcTop;// 0x2e   specification for border above picture
//  W1_BRC brcLeft;// 0x30 specification for border to the left
//  W1_BRC brcBottom;// 0x32    specification for border below picture
//  W1_BRC brcRight;// 0x34 specification for border to the right
//  SVBT16 dxaOrigin;// 0x36    horizontal offset of hand annotation origin
//  sal_uInt16 dxaOriginGet() {
//      return SVBT16ToShort(dxaOrigin); }
//  SVBT16 dyaOrigin;// 0x38    vertical offset of hand annotation origin
//  sal_uInt16 dyaOriginGet() {
//      return SVBT16ToShort(dyaOrigin); }
    sal_uInt8 rgb;// 0x3a   variable array of bytes containing Window's metafile, bitmap or TIFF file filename.
    sal_uInt8* rgbGet() { return &rgb; }
};

struct W1_TBD /////////////////////////////////////////////////////////
{
    sal_uInt8 aBits1;
    sal_uInt8 aBits1Get() { return aBits1; }
// sal_uInt8 jc : 3;// 0x07 justification code: 0=left tab, 1=centered tab, 2=right tab, 3=decimal tab, 4=bar
    sal_uInt8 jcGet() {
        return aBits1Get() & 0x07; }
// sal_uInt8 tlc : 3;// 0x38    tab leader code: 0=no leader, 1=dotted leader,
                        // 2=hyphenated leader, 3=single line leader, 4=heavy line leader
    sal_uInt8 tlcGet() {
        return (aBits1Get() >> 3 ) & 0x07; }
// *    int :2  C0  reserved
};

struct W1_TC //////////////////////////////////////////////////////////
{
    sal_uInt8 aBits1;
    sal_uInt8 aBits1Get() { return aBits1; }
    sal_uInt8 fFirstMergedGet() {
        return aBits1Get() & 0x01; }
    sal_uInt8 fMergedGet() {
        return (aBits1Get() >> 1 ) & 0x01; }
    sal_uInt8 aBits2;
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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
