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


#include "ccidecom.hxx"

//=============================== Huffman tables ========================

//---------------------------- White-Run ------------------------------

#define CCIWhiteTableSize 105

const CCIHuffmanTableEntry CCIWhiteTable[CCIWhiteTableSize]={
    {    0, 0x0035,  8 },
    {    1, 0x0007,  6 },
    {    2, 0x0007,  4 },
    {    3, 0x0008,  4 },
    {    4, 0x000b,  4 },
    {    5, 0x000c,  4 },
    {    6, 0x000e,  4 },
    {    7, 0x000f,  4 },
    {    8, 0x0013,  5 },
    {    9, 0x0014,  5 },
    {   10, 0x0007,  5 },
    {   11, 0x0008,  5 },
    {   12, 0x0008,  6 },
    {   13, 0x0003,  6 },
    {   14, 0x0034,  6 },
    {   15, 0x0035,  6 },
    {   16, 0x002a,  6 },
    {   17, 0x002b,  6 },
    {   18, 0x0027,  7 },
    {   19, 0x000c,  7 },
    {   20, 0x0008,  7 },
    {   21, 0x0017,  7 },
    {   22, 0x0003,  7 },
    {   23, 0x0004,  7 },
    {   24, 0x0028,  7 },
    {   25, 0x002b,  7 },
    {   26, 0x0013,  7 },
    {   27, 0x0024,  7 },
    {   28, 0x0018,  7 },
    {   29, 0x0002,  8 },
    {   30, 0x0003,  8 },
    {   31, 0x001a,  8 },
    {   32, 0x001b,  8 },
    {   33, 0x0012,  8 },
    {   34, 0x0013,  8 },
    {   35, 0x0014,  8 },
    {   36, 0x0015,  8 },
    {   37, 0x0016,  8 },
    {   38, 0x0017,  8 },
    {   39, 0x0028,  8 },
    {   40, 0x0029,  8 },
    {   41, 0x002a,  8 },
    {   42, 0x002b,  8 },
    {   43, 0x002c,  8 },
    {   44, 0x002d,  8 },
    {   45, 0x0004,  8 },
    {   46, 0x0005,  8 },
    {   47, 0x000a,  8 },
    {   48, 0x000b,  8 },
    {   49, 0x0052,  8 },
    {   50, 0x0053,  8 },
    {   51, 0x0054,  8 },
    {   52, 0x0055,  8 },
    {   53, 0x0024,  8 },
    {   54, 0x0025,  8 },
    {   55, 0x0058,  8 },
    {   56, 0x0059,  8 },
    {   57, 0x005a,  8 },
    {   58, 0x005b,  8 },
    {   59, 0x004a,  8 },
    {   60, 0x004b,  8 },
    {   61, 0x0032,  8 },
    {   62, 0x0033,  8 },
    {   63, 0x0034,  8 },
    {   64, 0x001b,  5 },
    {  128, 0x0012,  5 },
    {  192, 0x0017,  6 },
    {  256, 0x0037,  7 },
    {  320, 0x0036,  8 },
    {  384, 0x0037,  8 },
    {  448, 0x0064,  8 },
    {  512, 0x0065,  8 },
    {  576, 0x0068,  8 },
    {  640, 0x0067,  8 },
    {  704, 0x00cc,  9 },
    {  768, 0x00cd,  9 },
    {  832, 0x00d2,  9 },
    {  896, 0x00d3,  9 },
    {  960, 0x00d4,  9 },
    { 1024, 0x00d5,  9 },
    { 1088, 0x00d6,  9 },
    { 1152, 0x00d7,  9 },
    { 1216, 0x00d8,  9 },
    { 1280, 0x00d9,  9 },
    { 1344, 0x00da,  9 },
    { 1408, 0x00db,  9 },
    { 1472, 0x0098,  9 },
    { 1536, 0x0099,  9 },
    { 1600, 0x009a,  9 },
    { 1664, 0x0018,  6 },
    { 1728, 0x009b,  9 },
    { 1792, 0x0008, 11 },
    { 1856, 0x000c, 11 },
    { 1920, 0x000d, 11 },
    { 1984, 0x0012, 12 },
    { 2048, 0x0013, 12 },
    { 2112, 0x0014, 12 },
    { 2176, 0x0015, 12 },
    { 2240, 0x0016, 12 },
    { 2304, 0x0017, 12 },
    { 2368, 0x001c, 12 },
    { 2432, 0x001d, 12 },
    { 2496, 0x001e, 12 },
    { 2560, 0x001f, 12 },
    { 9999, 0x0001, 12 }    //  EOL
};

//---------------------------- Black-Run ------------------------------

#define CCIBlackTableSize 105

const CCIHuffmanTableEntry CCIBlackTable[CCIBlackTableSize]={
    {    0, 0x0037, 10 },
    {    1, 0x0002,  3 },
    {    2, 0x0003,  2 },
    {    3, 0x0002,  2 },
    {    4, 0x0003,  3 },
    {    5, 0x0003,  4 },
    {    6, 0x0002,  4 },
    {    7, 0x0003,  5 },
    {    8, 0x0005,  6 },
    {    9, 0x0004,  6 },
    {   10, 0x0004,  7 },
    {   11, 0x0005,  7 },
    {   12, 0x0007,  7 },
    {   13, 0x0004,  8 },
    {   14, 0x0007,  8 },
    {   15, 0x0018,  9 },
    {   16, 0x0017, 10 },
    {   17, 0x0018, 10 },
    {   18, 0x0008, 10 },
    {   19, 0x0067, 11 },
    {   20, 0x0068, 11 },
    {   21, 0x006c, 11 },
    {   22, 0x0037, 11 },
    {   23, 0x0028, 11 },
    {   24, 0x0017, 11 },
    {   25, 0x0018, 11 },
    {   26, 0x00ca, 12 },
    {   27, 0x00cb, 12 },
    {   28, 0x00cc, 12 },
    {   29, 0x00cd, 12 },
    {   30, 0x0068, 12 },
    {   31, 0x0069, 12 },
    {   32, 0x006a, 12 },
    {   33, 0x006b, 12 },
    {   34, 0x00d2, 12 },
    {   35, 0x00d3, 12 },
    {   36, 0x00d4, 12 },
    {   37, 0x00d5, 12 },
    {   38, 0x00d6, 12 },
    {   39, 0x00d7, 12 },
    {   40, 0x006c, 12 },
    {   41, 0x006d, 12 },
    {   42, 0x00da, 12 },
    {   43, 0x00db, 12 },
    {   44, 0x0054, 12 },
    {   45, 0x0055, 12 },
    {   46, 0x0056, 12 },
    {   47, 0x0057, 12 },
    {   48, 0x0064, 12 },
    {   49, 0x0065, 12 },
    {   50, 0x0052, 12 },
    {   51, 0x0053, 12 },
    {   52, 0x0024, 12 },
    {   53, 0x0037, 12 },
    {   54, 0x0038, 12 },
    {   55, 0x0027, 12 },
    {   56, 0x0028, 12 },
    {   57, 0x0058, 12 },
    {   58, 0x0059, 12 },
    {   59, 0x002b, 12 },
    {   60, 0x002c, 12 },
    {   61, 0x005a, 12 },
    {   62, 0x0066, 12 },
    {   63, 0x0067, 12 },
    {   64, 0x000f, 10 },
    {  128, 0x00c8, 12 },
    {  192, 0x00c9, 12 },
    {  256, 0x005b, 12 },
    {  320, 0x0033, 12 },
    {  384, 0x0034, 12 },
    {  448, 0x0035, 12 },
    {  512, 0x006c, 13 },
    {  576, 0x006d, 13 },
    {  640, 0x004a, 13 },
    {  704, 0x004b, 13 },
    {  768, 0x004c, 13 },
    {  832, 0x004d, 13 },
    {  896, 0x0072, 13 },
    {  960, 0x0073, 13 },
    { 1024, 0x0074, 13 },
    { 1088, 0x0075, 13 },
    { 1152, 0x0076, 13 },
    { 1216, 0x0077, 13 },
    { 1280, 0x0052, 13 },
    { 1344, 0x0053, 13 },
    { 1408, 0x0054, 13 },
    { 1472, 0x0055, 13 },
    { 1536, 0x005a, 13 },
    { 1600, 0x005b, 13 },
    { 1664, 0x0064, 13 },
    { 1728, 0x0065, 13 },
    { 1792, 0x0008, 11 },
    { 1856, 0x000c, 11 },
    { 1920, 0x000d, 11 },
    { 1984, 0x0012, 12 },
    { 2048, 0x0013, 12 },
    { 2112, 0x0014, 12 },
    { 2176, 0x0015, 12 },
    { 2240, 0x0016, 12 },
    { 2304, 0x0017, 12 },
    { 2368, 0x001c, 12 },
    { 2432, 0x001d, 12 },
    { 2496, 0x001e, 12 },
    { 2560, 0x001f, 12 },
    { 9999, 0x0001, 12 }    //  EOL
};


//---------------------------- 2D-Mode --------------------------------

#define CCI2DMODE_UNCOMP   0
#define CCI2DMODE_PASS     1
#define CCI2DMODE_HORZ     2
#define CCI2DMODE_VERT_L3  3
#define CCI2DMODE_VERT_L2  4
#define CCI2DMODE_VERT_L1  5
#define CCI2DMODE_VERT_0   6
#define CCI2DMODE_VERT_R1  7
#define CCI2DMODE_VERT_R2  8
#define CCI2DMODE_VERT_R3  9

#define CCI2DModeTableSize 10

const CCIHuffmanTableEntry CCI2DModeTable[CCI2DModeTableSize]={
    { CCI2DMODE_UNCOMP , 0x000f, 10 },
    { CCI2DMODE_PASS   , 0x0001,  4 },
    { CCI2DMODE_HORZ   , 0x0001,  3 },
    { CCI2DMODE_VERT_L3, 0x0002,  7 },
    { CCI2DMODE_VERT_L2, 0x0002,  6 },
    { CCI2DMODE_VERT_L1, 0x0002,  3 },
    { CCI2DMODE_VERT_0 , 0x0001,  1 },
    { CCI2DMODE_VERT_R1, 0x0003,  3 },
    { CCI2DMODE_VERT_R2, 0x0003,  6 },
    { CCI2DMODE_VERT_R3, 0x0003,  7 }
};


//-------------------------- 2D-Uncompressed-Mode ----------------------

#define CCIUNCOMP_0White_1Black  0
#define CCIUNCOMP_1White_1Black  1
#define CCIUNCOMP_2White_1Black  2
#define CCIUNCOMP_3White_1Black  3
#define CCIUNCOMP_4White_1Black  4
#define CCIUNCOMP_5White         5
#define CCIUNCOMP_0White_End     6
#define CCIUNCOMP_1White_End     7
#define CCIUNCOMP_2White_End     8
#define CCIUNCOMP_3White_End     9
#define CCIUNCOMP_4White_End    10

#define CCIUncompTableSize 11

const CCIHuffmanTableEntry CCIUncompTable[CCIUncompTableSize]={
    { CCIUNCOMP_0White_1Black, 0x0001,  1 },
    { CCIUNCOMP_1White_1Black, 0x0001,  2 },
    { CCIUNCOMP_2White_1Black, 0x0001,  3 },
    { CCIUNCOMP_3White_1Black, 0x0001,  4 },
    { CCIUNCOMP_4White_1Black, 0x0001,  5 },
    { CCIUNCOMP_5White       , 0x0001,  6 },
    { CCIUNCOMP_0White_End   , 0x0001,  7 },
    { CCIUNCOMP_1White_End   , 0x0001,  8 },
    { CCIUNCOMP_2White_End   , 0x0001,  9 },
    { CCIUNCOMP_3White_End   , 0x0001, 10 },
    { CCIUNCOMP_4White_End   , 0x0001, 11 }
};


//================== backup of the Huffman tables ============================
// To make sure that the Huffman tables do not contain errors they were entered
// from two different sources (Phew) and compared.
// Since an error could creep in to the source code while maintaining it
// (e.g. an accidentaly key press in the editor) the tables are listed twice
// and are compared during runtime. (If the comparison fails CCIDcompressor
// throws an error) The whole thing may appear insane, but an error within the
// tables would otherwise be really hard to discover and it's very unlikely that 
// one or more sample files run through all codes.

const CCIHuffmanTableEntry CCIWhiteTableSave[CCIWhiteTableSize]={
    {    0, 0x0035,  8 },
    {    1, 0x0007,  6 },
    {    2, 0x0007,  4 },
    {    3, 0x0008,  4 },
    {    4, 0x000b,  4 },
    {    5, 0x000c,  4 },
    {    6, 0x000e,  4 },
    {    7, 0x000f,  4 },
    {    8, 0x0013,  5 },
    {    9, 0x0014,  5 },
    {   10, 0x0007,  5 },
    {   11, 0x0008,  5 },
    {   12, 0x0008,  6 },
    {   13, 0x0003,  6 },
    {   14, 0x0034,  6 },
    {   15, 0x0035,  6 },
    {   16, 0x002a,  6 },
    {   17, 0x002b,  6 },
    {   18, 0x0027,  7 },
    {   19, 0x000c,  7 },
    {   20, 0x0008,  7 },
    {   21, 0x0017,  7 },
    {   22, 0x0003,  7 },
    {   23, 0x0004,  7 },
    {   24, 0x0028,  7 },
    {   25, 0x002b,  7 },
    {   26, 0x0013,  7 },
    {   27, 0x0024,  7 },
    {   28, 0x0018,  7 },
    {   29, 0x0002,  8 },
    {   30, 0x0003,  8 },
    {   31, 0x001a,  8 },
    {   32, 0x001b,  8 },
    {   33, 0x0012,  8 },
    {   34, 0x0013,  8 },
    {   35, 0x0014,  8 },
    {   36, 0x0015,  8 },
    {   37, 0x0016,  8 },
    {   38, 0x0017,  8 },
    {   39, 0x0028,  8 },
    {   40, 0x0029,  8 },
    {   41, 0x002a,  8 },
    {   42, 0x002b,  8 },
    {   43, 0x002c,  8 },
    {   44, 0x002d,  8 },
    {   45, 0x0004,  8 },
    {   46, 0x0005,  8 },
    {   47, 0x000a,  8 },
    {   48, 0x000b,  8 },
    {   49, 0x0052,  8 },
    {   50, 0x0053,  8 },
    {   51, 0x0054,  8 },
    {   52, 0x0055,  8 },
    {   53, 0x0024,  8 },
    {   54, 0x0025,  8 },
    {   55, 0x0058,  8 },
    {   56, 0x0059,  8 },
    {   57, 0x005a,  8 },
    {   58, 0x005b,  8 },
    {   59, 0x004a,  8 },
    {   60, 0x004b,  8 },
    {   61, 0x0032,  8 },
    {   62, 0x0033,  8 },
    {   63, 0x0034,  8 },
    {   64, 0x001b,  5 },
    {  128, 0x0012,  5 },
    {  192, 0x0017,  6 },
    {  256, 0x0037,  7 },
    {  320, 0x0036,  8 },
    {  384, 0x0037,  8 },
    {  448, 0x0064,  8 },
    {  512, 0x0065,  8 },
    {  576, 0x0068,  8 },
    {  640, 0x0067,  8 },
    {  704, 0x00cc,  9 },
    {  768, 0x00cd,  9 },
    {  832, 0x00d2,  9 },
    {  896, 0x00d3,  9 },
    {  960, 0x00d4,  9 },
    { 1024, 0x00d5,  9 },
    { 1088, 0x00d6,  9 },
    { 1152, 0x00d7,  9 },
    { 1216, 0x00d8,  9 },
    { 1280, 0x00d9,  9 },
    { 1344, 0x00da,  9 },
    { 1408, 0x00db,  9 },
    { 1472, 0x0098,  9 },
    { 1536, 0x0099,  9 },
    { 1600, 0x009a,  9 },
    { 1664, 0x0018,  6 },
    { 1728, 0x009b,  9 },
    { 1792, 0x0008, 11 },
    { 1856, 0x000c, 11 },
    { 1920, 0x000d, 11 },
    { 1984, 0x0012, 12 },
    { 2048, 0x0013, 12 },
    { 2112, 0x0014, 12 },
    { 2176, 0x0015, 12 },
    { 2240, 0x0016, 12 },
    { 2304, 0x0017, 12 },
    { 2368, 0x001c, 12 },
    { 2432, 0x001d, 12 },
    { 2496, 0x001e, 12 },
    { 2560, 0x001f, 12 },
    { 9999, 0x0001, 12 }    //  EOL
};

const CCIHuffmanTableEntry CCIBlackTableSave[CCIBlackTableSize]={
    {    0, 0x0037, 10 },
    {    1, 0x0002,  3 },
    {    2, 0x0003,  2 },
    {    3, 0x0002,  2 },
    {    4, 0x0003,  3 },
    {    5, 0x0003,  4 },
    {    6, 0x0002,  4 },
    {    7, 0x0003,  5 },
    {    8, 0x0005,  6 },
    {    9, 0x0004,  6 },
    {   10, 0x0004,  7 },
    {   11, 0x0005,  7 },
    {   12, 0x0007,  7 },
    {   13, 0x0004,  8 },
    {   14, 0x0007,  8 },
    {   15, 0x0018,  9 },
    {   16, 0x0017, 10 },
    {   17, 0x0018, 10 },
    {   18, 0x0008, 10 },
    {   19, 0x0067, 11 },
    {   20, 0x0068, 11 },
    {   21, 0x006c, 11 },
    {   22, 0x0037, 11 },
    {   23, 0x0028, 11 },
    {   24, 0x0017, 11 },
    {   25, 0x0018, 11 },
    {   26, 0x00ca, 12 },
    {   27, 0x00cb, 12 },
    {   28, 0x00cc, 12 },
    {   29, 0x00cd, 12 },
    {   30, 0x0068, 12 },
    {   31, 0x0069, 12 },
    {   32, 0x006a, 12 },
    {   33, 0x006b, 12 },
    {   34, 0x00d2, 12 },
    {   35, 0x00d3, 12 },
    {   36, 0x00d4, 12 },
    {   37, 0x00d5, 12 },
    {   38, 0x00d6, 12 },
    {   39, 0x00d7, 12 },
    {   40, 0x006c, 12 },
    {   41, 0x006d, 12 },
    {   42, 0x00da, 12 },
    {   43, 0x00db, 12 },
    {   44, 0x0054, 12 },
    {   45, 0x0055, 12 },
    {   46, 0x0056, 12 },
    {   47, 0x0057, 12 },
    {   48, 0x0064, 12 },
    {   49, 0x0065, 12 },
    {   50, 0x0052, 12 },
    {   51, 0x0053, 12 },
    {   52, 0x0024, 12 },
    {   53, 0x0037, 12 },
    {   54, 0x0038, 12 },
    {   55, 0x0027, 12 },
    {   56, 0x0028, 12 },
    {   57, 0x0058, 12 },
    {   58, 0x0059, 12 },
    {   59, 0x002b, 12 },
    {   60, 0x002c, 12 },
    {   61, 0x005a, 12 },
    {   62, 0x0066, 12 },
    {   63, 0x0067, 12 },
    {   64, 0x000f, 10 },
    {  128, 0x00c8, 12 },
    {  192, 0x00c9, 12 },
    {  256, 0x005b, 12 },
    {  320, 0x0033, 12 },
    {  384, 0x0034, 12 },
    {  448, 0x0035, 12 },
    {  512, 0x006c, 13 },
    {  576, 0x006d, 13 },
    {  640, 0x004a, 13 },
    {  704, 0x004b, 13 },
    {  768, 0x004c, 13 },
    {  832, 0x004d, 13 },
    {  896, 0x0072, 13 },
    {  960, 0x0073, 13 },
    { 1024, 0x0074, 13 },
    { 1088, 0x0075, 13 },
    { 1152, 0x0076, 13 },
    { 1216, 0x0077, 13 },
    { 1280, 0x0052, 13 },
    { 1344, 0x0053, 13 },
    { 1408, 0x0054, 13 },
    { 1472, 0x0055, 13 },
    { 1536, 0x005a, 13 },
    { 1600, 0x005b, 13 },
    { 1664, 0x0064, 13 },
    { 1728, 0x0065, 13 },
    { 1792, 0x0008, 11 },
    { 1856, 0x000c, 11 },
    { 1920, 0x000d, 11 },
    { 1984, 0x0012, 12 },
    { 2048, 0x0013, 12 },
    { 2112, 0x0014, 12 },
    { 2176, 0x0015, 12 },
    { 2240, 0x0016, 12 },
    { 2304, 0x0017, 12 },
    { 2368, 0x001c, 12 },
    { 2432, 0x001d, 12 },
    { 2496, 0x001e, 12 },
    { 2560, 0x001f, 12 },
    { 9999, 0x0001, 12 }    //  EOL
};


const CCIHuffmanTableEntry CCI2DModeTableSave[CCI2DModeTableSize]={
    { CCI2DMODE_UNCOMP , 0x000f, 10 },
    { CCI2DMODE_PASS   , 0x0001,  4 },
    { CCI2DMODE_HORZ   , 0x0001,  3 },
    { CCI2DMODE_VERT_L3, 0x0002,  7 },
    { CCI2DMODE_VERT_L2, 0x0002,  6 },
    { CCI2DMODE_VERT_L1, 0x0002,  3 },
    { CCI2DMODE_VERT_0 , 0x0001,  1 },
    { CCI2DMODE_VERT_R1, 0x0003,  3 },
    { CCI2DMODE_VERT_R2, 0x0003,  6 },
    { CCI2DMODE_VERT_R3, 0x0003,  7 }
};


const CCIHuffmanTableEntry CCIUncompTableSave[CCIUncompTableSize]={
    { CCIUNCOMP_0White_1Black, 0x0001,  1 },
    { CCIUNCOMP_1White_1Black, 0x0001,  2 },
    { CCIUNCOMP_2White_1Black, 0x0001,  3 },
    { CCIUNCOMP_3White_1Black, 0x0001,  4 },
    { CCIUNCOMP_4White_1Black, 0x0001,  5 },
    { CCIUNCOMP_5White       , 0x0001,  6 },
    { CCIUNCOMP_0White_End   , 0x0001,  7 },
    { CCIUNCOMP_1White_End   , 0x0001,  8 },
    { CCIUNCOMP_2White_End   , 0x0001,  9 },
    { CCIUNCOMP_3White_End   , 0x0001, 10 },
    { CCIUNCOMP_4White_End   , 0x0001, 11 }
};

//=========================================================================


CCIDecompressor::CCIDecompressor( sal_uLong nOpts, sal_uInt32 nImageWidth ) :
    bTableBad   ( sal_False ),
    bStatus     ( sal_False ),
    pByteSwap   ( NULL ),
    nWidth      ( nImageWidth ),
    nOptions    ( nOpts ),
    pLastLine   ( NULL )
{
    if ( nOpts & CCI_OPTION_INVERSEBITORDER )
    {
        pByteSwap = new sal_uInt8[ 256 ];
        for ( int i = 0; i < 256; i++ )
        {
            pByteSwap[ i ] = sal::static_int_cast< sal_uInt8 >(
                ( i << 7 ) | ( ( i & 2 ) << 5 ) | ( ( i & 4 ) << 3 ) | ( ( i & 8 ) << 1 ) |
                ( ( i & 16 ) >> 1 ) | ( ( i & 32 ) >> 3 ) | ( ( i & 64 ) >> 5 ) | ( ( i & 128 ) >> 7 ));
        }
    }

    pWhiteLookUp =new CCILookUpTableEntry[1<<13];
    pBlackLookUp =new CCILookUpTableEntry[1<<13];
    p2DModeLookUp=new CCILookUpTableEntry[1<<10];
    pUncompLookUp=new CCILookUpTableEntry[1<<11];

    MakeLookUp(CCIWhiteTable,CCIWhiteTableSave,pWhiteLookUp,CCIWhiteTableSize,13);
    MakeLookUp(CCIBlackTable,CCIBlackTableSave,pBlackLookUp,CCIBlackTableSize,13);
    MakeLookUp(CCI2DModeTable,CCI2DModeTableSave,p2DModeLookUp,CCI2DModeTableSize,10);
    MakeLookUp(CCIUncompTable,CCIUncompTableSave,pUncompLookUp,CCIUncompTableSize,11);
}


CCIDecompressor::~CCIDecompressor()
{
    delete[] pByteSwap;
    delete[] pLastLine;
    delete[] pWhiteLookUp;
    delete[] pBlackLookUp;
    delete[] p2DModeLookUp;
    delete[] pUncompLookUp;
}


void CCIDecompressor::StartDecompression( SvStream & rIStream )
{
    pIStream = &rIStream;
    nInputBitsBufSize = 0;
    bFirstEOL = sal_True;
    bStatus = sal_True;
    nEOLCount = 0;

    if ( bTableBad == sal_True )
        return;
}


sal_Bool CCIDecompressor::DecompressScanline( sal_uInt8 * pTarget, sal_uLong nTargetBits )
{
    sal_uInt16 i;
    sal_uInt8 * pSrc,* pDst;
    sal_Bool b2D;

    if ( nEOLCount >= 5 )   // RTC (Return To Controller)
        return sal_True;

    if ( bStatus == sal_False )
        return sal_False;

    // Wenn EOL-Codes vorhanden sind, steht der EOL-Code auch vor der ersten Zeile.
    // (und ich dachte EOL heisst 'End Of Line'...)
    // Daher lesen wir den EOL-Code immer vor jeder Zeile als erstes ein:
    if ( nOptions & CCI_OPTION_EOL )
    {
        if ( bFirstEOL )
        {
            sal_uInt32 nCurPos = pIStream->Tell();
            sal_uInt16 nOldInputBitsBufSize = nInputBitsBufSize;
            sal_uInt32 nOldInputBitsBuf = nInputBitsBuf;
            if ( ReadEOL( 32 ) == sal_False )
            {
                nInputBitsBufSize = nOldInputBitsBufSize;
                nInputBitsBuf = nOldInputBitsBuf;
                pIStream->Seek( nCurPos );
                nOptions &=~ CCI_OPTION_EOL;                // CCITT Group 3 - Compression Type 2
            }
            bFirstEOL = sal_False;
        }
        else
        {
            if ( ReadEOL( nTargetBits ) == sal_False )
            {
                return bStatus;
            }
        }
    }

    if ( nEOLCount >= 5 )   // RTC (Return To Controller)
        return sal_True;

    // should the situation arise, generate a white previous line for 2D:
    if ( nOptions & CCI_OPTION_2D )
    {
        if ( pLastLine == NULL || nLastLineSize != ( ( nTargetBits + 7 ) >> 3 ) )
        {
            if ( pLastLine == NULL )
                delete[] pLastLine;
            nLastLineSize = ( nTargetBits + 7 ) >> 3;
            pLastLine = new sal_uInt8[ nLastLineSize ];
            pDst = pLastLine;
            for ( i = 0; i < nLastLineSize; i++ ) *( pDst++ ) = 0x00;
        }
    }
    // ggf. Zeilen-Anfang auf naechste Byte-Grenze runden:
    if ( nOptions & CCI_OPTION_BYTEALIGNROW )
        nInputBitsBufSize &= 0xfff8;

    // is it a 2D row?
    if ( nOptions & CCI_OPTION_2D )
    {
        if ( nOptions & CCI_OPTION_EOL )
            b2D = Read2DTag();
        else
            b2D = sal_True;
    }
    else
        b2D = sal_False;

    // read scanline:
    if ( b2D )
        Read2DScanlineData( pTarget, (sal_uInt16)nTargetBits );
    else
        Read1DScanlineData( pTarget, (sal_uInt16)nTargetBits );

    // if we're in 2D mode we have to remember the line:
    if ( nOptions & CCI_OPTION_2D && bStatus == sal_True )
    {
        pSrc = pTarget;
        pDst = pLastLine;
        for ( i = 0; i < nLastLineSize; i++ ) *(pDst++)=*(pSrc++);
    }

    if ( pIStream->GetError() )
        bStatus = sal_False;

    return bStatus;
}


void CCIDecompressor::MakeLookUp(const CCIHuffmanTableEntry * pHufTab,
                                 const CCIHuffmanTableEntry * pHufTabSave,
                                 CCILookUpTableEntry * pLookUp,
                                 sal_uInt16 nHuffmanTableSize,
                                 sal_uInt16 nMaxCodeBits)
{
    sal_uInt16 nLookUpSize = 1 << nMaxCodeBits;
    memset(pLookUp, 0, nLookUpSize * sizeof(CCILookUpTableEntry));

    if (bTableBad==sal_True)
        return;

    sal_uInt16 nMask = 0xffff >> (16-nMaxCodeBits);

    for (sal_uInt16 i = 0; i < nHuffmanTableSize; ++i)
    {
        if ( pHufTab[i].nValue!=pHufTabSave[i].nValue ||
             pHufTab[i].nCode!=pHufTabSave[i].nCode ||
             pHufTab[i].nCodeBits!=pHufTabSave[i].nCodeBits ||
             pHufTab[i].nCodeBits==0 ||
             pHufTab[i].nCodeBits>nMaxCodeBits )
        {
            bTableBad=sal_True;
            return;
        }
        sal_uInt16 nMinCode = nMask & (pHufTab[i].nCode << (nMaxCodeBits-pHufTab[i].nCodeBits));
        sal_uInt16 nMaxCode = nMinCode | (nMask >> pHufTab[i].nCodeBits);
        for (sal_uInt16 j=nMinCode; j<=nMaxCode; ++j)
        {
            if (pLookUp[j].nCodeBits!=0)
            {
                bTableBad=sal_True;
                return;
            }
            pLookUp[j].nValue=pHufTab[i].nValue;
            pLookUp[j].nCodeBits=pHufTab[i].nCodeBits;
        }
    }
}


sal_Bool CCIDecompressor::ReadEOL( sal_uInt32 /*nMaxFillBits*/ )
{
    sal_uInt16  nCode;
    sal_uInt8   nByte;

    // if (nOptions&CCI_OPTION_BYTEALIGNEOL) nMaxFillBits=7; else nMaxFillBits=0;
    // Buuuh: Entweder wird die Option in itiff.cxx nicht richtig gesetzt (-> Fehler in Doku)
    // oder es gibt tatsaechlich gemeine Export-Filter, die immer ein Align machen.
    // Ausserdem wurden Dateien gefunden, in denen mehr als die maximal 7 noetigen
    // Fuellbits vor dem EOL-Code stehen. Daher akzeptieren wir nun grundsaetzlich
    // bis zu 32-nonsense-Bits vor dem EOL-Code:
    // und ich habe eine Datei gefunden in der bis zu ??? Bloedsinn Bits stehen, zudem ist dort die Bit Reihenfolge verdreht (SJ);

    sal_uInt32 nMaxPos = pIStream->Tell();
    nMaxPos += nWidth >> 3;

    for ( ;; )
    {
        while ( nInputBitsBufSize < 12 )
        {
            *pIStream >> nByte;
            if ( pIStream->IsEof() )
                return sal_False;
            if ( pIStream->Tell() > nMaxPos )
                return sal_False;

            if ( nOptions & CCI_OPTION_INVERSEBITORDER )
                nByte = pByteSwap[ nByte ];
            nInputBitsBuf=(nInputBitsBuf<<8) | (sal_uLong)nByte;
            nInputBitsBufSize += 8;
        }
        nCode = (sal_uInt16)( ( nInputBitsBuf >> ( nInputBitsBufSize - 12 ) ) & 0x0fff );
        if ( nCode == 0x0001 )
        {
            nEOLCount++;
            nInputBitsBufSize -= 12;
            break;
        }
        else
            nInputBitsBufSize--;
    }
    return sal_True;
}


sal_Bool CCIDecompressor::Read2DTag()
{
    sal_uInt8 nByte;

    // read abit and return sal_True if it's 0, otherwise return sal_False
    if (nInputBitsBufSize==0) {
        *pIStream >> nByte;
        if ( nOptions & CCI_OPTION_INVERSEBITORDER )
            nByte = pByteSwap[ nByte ];
        nInputBitsBuf=(sal_uLong)nByte;
        nInputBitsBufSize=8;
    }
    nInputBitsBufSize--;
    if ( ((nInputBitsBuf>>nInputBitsBufSize)&0x0001) ) return sal_False;
    else return sal_True;
}


sal_uInt8 CCIDecompressor::ReadBlackOrWhite()
{
    sal_uInt8 nByte;

    // read a bit and deliver 0x00 if it's 0, otherwise 0xff
    if (nInputBitsBufSize==0) {
        *pIStream >> nByte;
        if ( nOptions & CCI_OPTION_INVERSEBITORDER )
            nByte = pByteSwap[ nByte ];
        nInputBitsBuf=(sal_uLong)nByte;
        nInputBitsBufSize=8;
    }
    nInputBitsBufSize--;
    if ( ((nInputBitsBuf>>nInputBitsBufSize)&0x0001) ) return 0xff;
    else return 0x00;
}


sal_uInt16 CCIDecompressor::ReadCodeAndDecode(const CCILookUpTableEntry * pLookUp,
                                          sal_uInt16 nMaxCodeBits)
{
    // read a Huffman code and decode it:
    while (nInputBitsBufSize<nMaxCodeBits)
    {
        sal_uInt8 nByte(0);
        *pIStream >> nByte;
        if ( nOptions  & CCI_OPTION_INVERSEBITORDER )
            nByte = pByteSwap[ nByte ];
        nInputBitsBuf=(nInputBitsBuf<<8) | (sal_uLong)nByte;
        nInputBitsBufSize+=8;
    }
    sal_uInt16 nCode = (sal_uInt16)((nInputBitsBuf>>(nInputBitsBufSize-nMaxCodeBits))
                   &(0xffff>>(16-nMaxCodeBits)));
    sal_uInt16 nCodeBits = pLookUp[nCode].nCodeBits;
    if (nCodeBits==0) bStatus=sal_False;
    nInputBitsBufSize = nInputBitsBufSize - nCodeBits;
    return pLookUp[nCode].nValue;
}


void CCIDecompressor::FillBits(sal_uInt8 * pTarget, sal_uInt16 nTargetBits,
                               sal_uInt16 nBitPos, sal_uInt16 nNumBits,
                               sal_uInt8 nBlackOrWhite)
{
    if ( nBitPos >= nTargetBits )
        return;
    if ( nBitPos + nNumBits > nTargetBits )
        nNumBits = nTargetBits - nBitPos;

    pTarget+=nBitPos>>3;
    nBitPos&=7;

    if (nBlackOrWhite==0x00) *pTarget &= 0xff << (8-nBitPos);
    else                     *pTarget |= 0xff >> nBitPos;
    if (nNumBits>8-nBitPos) {
        nNumBits-=8-nBitPos;
        while (nNumBits>=8) {
            *(++pTarget)=nBlackOrWhite;
            nNumBits-=8;
        }
        if (nNumBits>0) *(++pTarget)=nBlackOrWhite;
    }
}


sal_uInt16 CCIDecompressor::CountBits(const sal_uInt8 * pData, sal_uInt16 nDataSizeBits,
                                  sal_uInt16 nBitPos, sal_uInt8 nBlackOrWhite)
{
    sal_uInt16 nPos,nLo;
    sal_uInt8 nData;

    // here the number of bits belonging together is being counted
    // which all have the color nBlackOrWhite (0xff oder 0x00)
    // from the position nBitPos on

    nPos=nBitPos;
    for (;;) {
        if (nPos>=nDataSizeBits) {
            nPos=nDataSizeBits;
            break;
        }
        nData=pData[nPos>>3];
        nLo=nPos & 7;
        if ( nLo==0 && nData==nBlackOrWhite) nPos+=8;
        else {
            if ( ((nData^nBlackOrWhite) & (0x80 >> nLo))!=0) break;
            nPos++;
        }
    }
    if (nPos<=nBitPos) return 0;
    else return nPos-nBitPos;
}


void CCIDecompressor::Read1DScanlineData(sal_uInt8 * pTarget, sal_uInt16 nTargetBits)
{
    sal_uInt16 nCode,nCodeBits,nDataBits,nTgtFreeByteBits;
    sal_uInt8 nByte;
    sal_uInt8 nBlackOrWhite; // is 0xff for black or 0x00 for white
    sal_Bool bTerminatingCode;

    // the first code is always a "white-code":
    nBlackOrWhite=0x00;

    // number of bits that aren't written in the byte *pTarget yet:
    nTgtFreeByteBits=8;

    // loop through codes from the input stream:
    do {

        // die naechsten 13 Bits nach nCode holen, aber noch nicht
        // aus dem Eingabe-Buffer loeschen:
        while (nInputBitsBufSize<13) {
            *pIStream >> nByte;
            if ( nOptions & CCI_OPTION_INVERSEBITORDER )
                nByte = pByteSwap[ nByte ];
            nInputBitsBuf=(nInputBitsBuf<<8) | (sal_uLong)nByte;
            nInputBitsBufSize+=8;
        }
        nCode=(sal_uInt16)((nInputBitsBuf>>(nInputBitsBufSize-13))&0x1fff);

        // determine the number of DataBits CodeBits:
        if (nBlackOrWhite) {
            nCodeBits=pBlackLookUp[nCode].nCodeBits;
            nDataBits=pBlackLookUp[nCode].nValue;
        }
        else {
            nCodeBits=pWhiteLookUp[nCode].nCodeBits;
            nDataBits=pWhiteLookUp[nCode].nValue;
        }
        // is that an invalid code?
        if ( nDataBits == 9999 )
        {
            return;
        }
        if ( nCodeBits == 0 )
        {
            return;             // das koennen sich jetzt um FuellBits handeln
        }
        nEOLCount = 0;
        // too much data?
        if (nDataBits>nTargetBits) {
            // Ja, koennte ein Folge-Fehler durch ungueltigen Code sein,
            // daher irdenwie weitermachen:
            nDataBits=nTargetBits;
        }

        // is that a 'Terminating-Code'?
        if (nDataBits<64) bTerminatingCode=sal_True; else bTerminatingCode=sal_False;

        // remove the read bits from the input buffer:
        nInputBitsBufSize = nInputBitsBufSize - nCodeBits;

        // write the number of data bits into the scanline:
        if (nDataBits>0) {
            nTargetBits = nTargetBits - nDataBits;
            if (nBlackOrWhite==0x00) *pTarget &= 0xff << nTgtFreeByteBits;
            else                     *pTarget |= 0xff >> (8-nTgtFreeByteBits);
            if (nDataBits<=nTgtFreeByteBits) {
                if (nDataBits==nTgtFreeByteBits) {
                    pTarget++;
                    nTgtFreeByteBits=8;
                }
                else nTgtFreeByteBits = nTgtFreeByteBits - nDataBits;
            }
            else {
                nDataBits = nDataBits - nTgtFreeByteBits;
                pTarget++;
                nTgtFreeByteBits=8;
                while (nDataBits>=8) {
                    *(pTarget++)=nBlackOrWhite;
                    nDataBits-=8;
                }
                if (nDataBits>0) {
                    *pTarget=nBlackOrWhite;
                    nTgtFreeByteBits = nTgtFreeByteBits - nDataBits;
                }
            }
        }

        // should the situation arise, switch Black <-> White:
        if (bTerminatingCode==sal_True) nBlackOrWhite=~nBlackOrWhite;

    } while (nTargetBits>0 || bTerminatingCode==sal_False);
}



void CCIDecompressor::Read2DScanlineData(sal_uInt8 * pTarget, sal_uInt16 nTargetBits)
{
    sal_uInt16 n2DMode,nBitPos,nUncomp,nRun,nRun2,nt;
    sal_uInt8 nBlackOrWhite;

    nBlackOrWhite=0x00;
    nBitPos=0;

    while (nBitPos<nTargetBits && bStatus==sal_True) {

        n2DMode=ReadCodeAndDecode(p2DModeLookUp,10);
        if (bStatus==sal_False) return;

        if (n2DMode==CCI2DMODE_UNCOMP) {
            for (;;) {
                nUncomp=ReadCodeAndDecode(pUncompLookUp,11);
                if ( nUncomp <= CCIUNCOMP_4White_1Black ) {
                    nRun=nUncomp-CCIUNCOMP_0White_1Black;
                    FillBits(pTarget,nTargetBits,nBitPos,nRun,0x00);
                    nBitPos = nBitPos + nRun;
                    FillBits(pTarget,nTargetBits,nBitPos,1,0xff);
                    nBitPos++;
                }
                else if ( nUncomp == CCIUNCOMP_5White ) {
                    FillBits(pTarget,nTargetBits,nBitPos,5,0x00);
                    nBitPos = nBitPos + 5;
                }
                else {
                    nRun=nUncomp-CCIUNCOMP_0White_End;
                    FillBits(pTarget,nTargetBits,nBitPos,nRun,0x00);
                    nBitPos = nBitPos + nRun;
                    nBlackOrWhite=ReadBlackOrWhite();
                    break;
                }
            }
        }

        else if (n2DMode==CCI2DMODE_PASS) {
            if (nBitPos==0 && nBlackOrWhite==0x00 && CountBits(pLastLine,nTargetBits,0,0xff)!=0) nRun=0;
            else {
                nRun=CountBits(pLastLine,nTargetBits,nBitPos,~nBlackOrWhite);
                nRun = nRun + CountBits(pLastLine,nTargetBits,nBitPos+nRun,nBlackOrWhite);
            }
            nRun = nRun + CountBits(pLastLine,nTargetBits,nBitPos+nRun,~nBlackOrWhite);
            FillBits(pTarget,nTargetBits,nBitPos,nRun,nBlackOrWhite);
            nBitPos = nBitPos + nRun;
        }

        else if (n2DMode==CCI2DMODE_HORZ) {
            if (nBlackOrWhite==0x00) {
                nRun=0;
                do {
                    nt=ReadCodeAndDecode(pWhiteLookUp,13);
                    nRun = nRun + nt;
                } while (nt>=64);
                nRun2=0;
                do {
                    nt=ReadCodeAndDecode(pBlackLookUp,13);
                    nRun2 = nRun2 + nt;
                } while (nt>=64);
            }
            else {
                nRun=0;
                do {
                    nt=ReadCodeAndDecode(pBlackLookUp,13);
                    nRun = nRun + nt;
                } while (nt>=64);
                nRun2=0;
                do {
                    nt=ReadCodeAndDecode(pWhiteLookUp,13);
                    nRun2 = nRun2 + nt;
                } while (nt>=64);
            }
            FillBits(pTarget,nTargetBits,nBitPos,nRun,nBlackOrWhite);
            nBitPos = nBitPos + nRun;
            FillBits(pTarget,nTargetBits,nBitPos,nRun2,~nBlackOrWhite);
            nBitPos = nBitPos + nRun2;
        }

        else { // it's one of the modes CCI2DMODE_VERT_...
            if (nBitPos==0 && nBlackOrWhite==0x00 && CountBits(pLastLine,nTargetBits,0,0xff)!=0) nRun=0;
            else {
                nRun=CountBits(pLastLine,nTargetBits,nBitPos,~nBlackOrWhite);
                nRun = nRun + CountBits(pLastLine,nTargetBits,nBitPos+nRun,nBlackOrWhite);
            }
            nRun+=n2DMode-CCI2DMODE_VERT_0;
            FillBits(pTarget,nTargetBits,nBitPos,nRun,nBlackOrWhite);
            nBitPos = nBitPos + nRun;
            nBlackOrWhite=~nBlackOrWhite;
        }
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
