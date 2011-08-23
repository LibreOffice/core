/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifdef _MSC_VER
#pragma hdrstop
#endif

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#include <hintids.hxx>

#include <bf_svtools/pathoptions.hxx>
#include <bf_svtools/fstathelper.hxx>
#include <bf_svtools/urihelper.hxx>
#include <bf_sfx2/docinf.hxx>
#include <bf_sfx2/app.hxx>
#include <bf_svx/boxitem.hxx>
#include <bf_svx/fontitem.hxx>
#include <bf_svx/lrspitem.hxx>
#include <bf_svx/ulspitem.hxx>
#include <bf_svx/brkitem.hxx>
#include <bf_svx/lspcitem.hxx>
#include <bf_svx/adjitem.hxx>
#include <bf_svx/spltitem.hxx>
#include <bf_svx/brshitem.hxx>
#include <bf_svx/tstpitem.hxx>
#include <bf_svx/fhgtitem.hxx>
#include <bf_svx/wghtitem.hxx>
#include <bf_svx/postitem.hxx>
#include <bf_svx/udlnitem.hxx>
#include <bf_svx/crsditem.hxx>
#include <bf_svx/escpitem.hxx>
#include <bf_svx/cntritem.hxx>
#include <bf_svx/shdditem.hxx>
#include <bf_svx/colritem.hxx>


#include <fmtpdsc.hxx>
#include <fmtftn.hxx>
#include <txtftn.hxx>
#include <fmtfld.hxx>
#include <fmtfsize.hxx>
#include <fmthdft.hxx>
#include <fmtcntnt.hxx>
#include <fmtcntnt.hxx>
#include <fmtfsize.hxx>
#include <fmtornt.hxx>
#include <fmthbsh.hxx>
#include <fchrfmt.hxx>
#include <frmatr.hxx>
#include <charatr.hxx>
#include <ftninfo.hxx>
#include <charfmt.hxx>
#include <pam.hxx>
#include <doc.hxx>
#include <ndtxt.hxx>
#include <fldbas.hxx>
#include <paratr.hxx>
#include <docufld.hxx>
#include <flddat.hxx>
#include <dbfld.hxx>
#include <fmtcol.hxx>
#include <linetype.hxx>
#include <pagedesc.hxx>
#include <fltini.hxx>
#include <poolfmt.hxx>

#ifdef __BORLANDC__
  #pragma warn -lvc
#endif

#include <sw6par.hxx>
#include <tox.hxx>
#include <SwStyleNameMapper.hxx>
#include "bf_so3/staticbaseurl.hxx"
#include "linetype.hxx"
namespace binfilter {

#ifdef __BORLANDC__
  #pragma warn +lvc
#endif



// Globals ******************************************************************

 static const SwLineType __FAR_DATA aLineTypeArr[] =
 {
 //  In          Out         Dist
  LIN_0,      LIN_1,      LIN_0, // Einfache Linien
  LIN_0,      LIN_2,      LIN_0,
  LIN_0,      LIN_3,      LIN_0,
  LIN_0,      LIN_5,      LIN_0,
  LIN_1,      LIN_1,      LIN_2, // Doppelte Linien
  LIN_2,      LIN_2,      LIN_2, // Doppelte Linien
  LIN_3,      LIN_3,      LIN_3,
  LIN_3,      LIN_2,      LIN_2,
  LIN_2,      LIN_3,      LIN_2,
  LIN_4,      LIN_3,      LIN_3,
  LIN_3,      LIN_4,      LIN_3
 };


// **************************************************************************



 const SwLineType& GetLineType(USHORT nIdx)
 {
  return aLineTypeArr[nIdx];
 }



#define SW6_MATCH_FONTNAMES_CNT 558
static const sal_Char* aFontNames[ SW6_MATCH_FONTNAMES_CNT ] = {
        /* 0 */ "Draft",
        /* 1 */ "Courier",
        /* 2 */ "Courier PS",
        /* 3 */ "LinePrtR8",
        /* 4 */ "CourierR8",
        /* 5 */ "Pica",
        /* 6 */ "OCR-B",
        /* 7 */ "OCR-A",
        /* 8 */ "Emphasis",
        /* 9 */ "Kana",
        /* 10 */ "Hebrew",
        /* 11 */ "Elite",
        /* 12 */ "Prestige",
        /* 13 */ "Script",
        /* 14 */ "Letter_Gothic",
        /* 15 */ "Narrator",
        /* 16 */ "Titan",
        /* 17 */ "Bold_PS",
        /* 18 */ "Cubic_PS",
        /* 19 */ "Trend_PS",
        /* 20 */ "Roman_PS",
        /* 21 */ "Script_PS",
        /* 22 */ "Gothic_PS",
        /* 23 */ "Gothic",
        /* 24 */ "Gothic PS",
        /* 25 */ "Prestige PS",
        /* 26 */ "Prestige_NLQ",
        /* 27 */ "Quadro",
        /* 28 */ "Gothik",
        /* 29 */ "Prestige_PS",
        /* 30 */ "Sans Serif",
        /* 31 */ "Symbol",
        /* 32 */ "Roman",
        /* 33 */ "Super Grande",
        /* 34 */ "Brougham",
        /* 35 */ "Sans Serif PS",
        /* 36 */ "Script PS",
        /* 37 */ "Quadro PS",
        /* 38 */ "Roman PS",
        /* 39 */ "Super Grande PS",
        /* 40 */ "Brougham PS",
        /* 41 */ "Quadro_NLQ",
        /* 42 */ "Gothik_NLQ",
        /* 43 */ "Anelia_PS",
        /* 44 */ "Quadro_PS",
        /* 45 */ "Brougham_PS",
        /* 46 */ "Letter Gothik",
        /* 47 */ "Bother Helvetica PS",
        /* 48 */ "PS Schrift",
        /* 49 */ "Brother Times Roman PS",
        /* 50 */ "Brougham12",
        /* 51 */ "PS LQ",
        /* 52 */ "OCR-B_LQ401",
        /* 53 */ "Gothik_LQ401",
        /* 54 */ "Gothik_LQ400",
        /* 55 */ "Brougham_LQ400",
        /* 56 */ "Anelia_PS_LQ400/401",
        /* 57 */ "OCR-B_LQ_401",
        /* 58 */ "Gothik_LQ_400",
        /* 59 */ "Gothik_LQ_401",
        /* 60 */ "Brougham_LQ_400",
        /* 61 */ "Letter Gothic",
        /* 62 */ "Sans Serif II",
        /* 63 */ "BR-02H",
        /* 64 */ "Roman II",
        /* 65 */ "BR-01T",
        /* 66 */ "BR-03B",
        /* 67 */ "Br-01T",
        /* 68 */ "Br-02H",
        /* 69 */ "Line Printer",
        /* 70 */ "Lori PS",
        /* 71 */ "Br-01T bold",
        /* 72 */ "Br-01T italic",
        /* 73 */ "Br-02H bold",
        /* 74 */ "Br-02H italic",
        /* 75 */ "BR-01H",
        /* 76 */ "Letter GothicIBM",
        /* 77 */ "BroughamIBM",
        /* 78 */ "Mars8",
        /* 79 */ "BR-11U (Univers)",
        /* 80 */ "BR01-T (CG Times)",
        /* 81 */ "LetterGothicIBM",
        /* 82 */ "LetterGothic",
        /* 83 */ "PrestigeElite",
        /* 84 */ "HELVETICA",
        /* 85 */ "Helvetica",
        /* 86 */ "BrotherHelv",
        /* 87 */ "TimesRoman",
        /* 88 */ "BrotherTimes",
        /* 89 */ "AneliaPSIBM",
        /* 90 */ "LoriPSIBM",
        /* 91 */ "LineDrawU",
        /* 92 */ "Anelia PS",
        /* 93 */ "Orator",
        /* 94 */ "OratorS",
        /* 95 */ "Typenrad",
        /* 96 */ "Pica_NLQ",
        /* 97 */ "Pica_PS",
        /* 98 */ "Pica_NLQ_PS",
        /* 99 */ "OCR",
        /* 100 */ "Italic",
        /* 101 */ "Courier_PS",
        /* 102 */ "Kassettenschrift",
        /* 103 */ "Letter Gothic PS",
        /* 104 */ "Courier_LQ",
        /* 105 */ "Prestige_Elite",
        /* 106 */ "LQ",
        /* 107 */ "NLQ_PS",
        /* 108 */ "LQ Display",
        /* 109 */ "Display PS",
        /* 110 */ "Times Roman",
        /* 111 */ "Times Roman PS",
        /* 112 */ "CourierIBM",
        /* 113 */ "LetterGothik",
        /* 114 */ "Quality",
        /* 115 */ "NLQ",
        /* 116 */ "Pica10",
        /* 117 */ "High-SpeedDraft",
        /* 118 */ "Scientific DQ    Cart",
        /* 119 */ "Scientific LQ    Cart",
        /* 120 */ "Scientific CQ    Cart",
        /* 121 */ "Compression",
        /* 122 */ "Courier10",
        /* 123 */ "PrestigeElite12",
        /* 124 */ "Correspondence",
        /* 125 */ "Letter Gothic LQ Cart",
        /* 126 */ "Draft PS",
        /* 127 */ "Pica10 PS",
        /* 128 */ "High-SpeedDraft PS",
        /* 129 */ "Compression PS",
        /* 130 */ "Boldface PS",
        /* 131 */ "Courier10 PS",
        /* 132 */ "PrestigeElite12 PS",
        /* 133 */ "Correspondence PS",
        /* 134 */ "Proportional",
        /* 135 */ "SansSerif",
        /* 136 */ "SansSerif PS",
        /* 137 */ "Roman NLQ PS",
        /* 138 */ "Draft_NLQ",
        /* 139 */ "Draft_PS",
        /* 140 */ "Script LQ",
        /* 141 */ "OCR B",
        /* 142 */ "Script C PS",
        /* 143 */ " OratorS",
        /* 144 */ "Sans Serif H",
        /* 145 */ "Roman T",
        /* 146 */ "High-Speed Draft",
        /* 147 */ "Scientific DQ",
        /* 148 */ "Scientific LQ",
        /* 149 */ "Scientific CQ",
        /* 150 */ "Prestige Elite",
        /* 151 */ "Old English",
        /* 152 */ "Humanist",
        /* 153 */ "Dutch",
        /* 154 */ "High-Speed Draft PS",
        /* 155 */ "Prestige Elite PS",
        /* 156 */ "Humanist PS",
        /* 157 */ "Dutch PS",
        /* 158 */ "Tile",
        /* 159 */ "Primus",
        /* 160 */ "Title PS",
        /* 161 */ "Fontkassette",
        /* 162 */ "EDP_Resident",
        /* 163 */ "Courier_Resident",
        /* 164 */ "Courier_Download",
        /* 165 */ "LegalCourier_Download",
        /* 166 */ "LetterGothic_Download",
        /* 167 */ "Prestige_Download",
        /* 168 */ "LegalPrestige_Download",
        /* 169 */ "HelvPS_Download",
        /* 170 */ "CourierPS_Resident",
        /* 171 */ "TMS_RomanPS_Download",
        /* 172 */ "Legal EDP 13",
        /* 173 */ "Courier_Outline",
        /* 174 */ "SansSerif_Outline",
        /* 175 */ "Roman_Outline",
        /* 176 */ "Symbol_Outline",
        /* 177 */ "Univers",
        /* 178 */ "Atlantic",
        /* 179 */ "Sans",
        /* 180 */ "Challenge",
        /* 181 */ "Sans Condensed",
        /* 182 */ "Classico",
        /* 183 */ "Milano",
        /* 184 */ "Office",
        /* 185 */ "Pentagon",
        /* 186 */ "Avanti",
        /* 187 */ "Classic",
        /* 188 */ "Finish",
        /* 189 */ "Hilton",
        /* 190 */ "Ovetti",
        /* 191 */ "Progress",
        /* 192 */ "Westcoast",
        /* 193 */ "Calculator",
        /* 194 */ "Cescendo",
        /* 195 */ "Console",
        /* 196 */ "Funky",
        /* 197 */ "NEWYORK",
        /* 198 */ "Nova",
        /* 199 */ "Paisley",
        /* 200 */ "Shanghai",
        /* 201 */ "Skyline",
        /* 202 */ "Speed",
        /* 203 */ "XPRESS",
        /* 204 */ "CG Times",
        /* 205 */ "PS Roman",
        /* 206 */ "Booklet",
        /* 207 */ "Centuri",
        /* 208 */ "Paltus",
        /* 209 */ "Tapir",
        /* 210 */ "Greek",
        /* 211 */ "Cyrillian",
        /* 212 */ "WingDings",
        /* 213 */ "LinePrtIBM",
        /* 214 */ "Univers Condensed",
        /* 215 */ "Arial",
        /* 216 */ "Antique Olive",
        /* 217 */ "Times New",
        /* 218 */ "Garamond",
        /* 219 */ "CG Omega",
        /* 220 */ "Coronet",
        /* 221 */ "Marigold",
        /* 222 */ "Clarendon Condensed",
        /* 223 */ "Albertus",
        /* 224 */ "Wingdings",
        /* 225 */ "SansCondensed",
        /* 226 */ "PS-Roman",
        /* 227 */ "Courier  6",
        /* 228 */ "Courier 12",
        /* 229 */ "Presentation 7      Cart_M",
        /* 230 */ "Presentation 8      Cart_M",
        /* 231 */ "Presentation 9      Cart_M",
        /* 232 */ "Presentation 14     Cart_M",
        /* 233 */ "Presentation 16     Cart_M",
        /* 234 */ "Presentation 18     Cart_M",
        /* 235 */ "CG Triumvirate      Cart_CB",
        /* 236 */ "CG Triumvirate Bold Cart_CB",
        /* 237 */ "Prestige 5          Cart_B",
        /* 238 */ "Prestige 7          Cart_B",
        /* 239 */ "Prestige 10         Cart_B",
        /* 240 */ "Letter Gothic 6",
        /* 241 */ "Letter Gothic 12",
        /* 242 */ "Letter Gothic 12    Cart_C",
        /* 243 */ "Courier 24          Cart_L",
        /* 244 */ "Letter Gothic 6     Cart_L",
        /* 245 */ "Letter Gothic 7     Cart_M",
        /* 246 */ "Letter Gothic 9.5   Cart_C",
        /* 247 */ "Letter Gothic 12    Cart_L",
        /* 248 */ "Letter Gothic 14    Cart_M",
        /* 249 */ "Letter Gothic 19    Cart_L",
        /* 250 */ "Letter Gothic 24    Cart_L",
        /* 251 */ "CG Century          Cart_CB",
        /* 252 */ "Helvetica           Cart_G",
        /* 253 */ "Helvetica           Cart_H",
        /* 254 */ "Helvetica           Cart_J",
        /* 255 */ "Helvetica           Cart_T",
        /* 256 */ "Helvetica           Cart_V",
        /* 257 */ "Brush               Cart_CE",
        /* 258 */ "Dom Casual          Cart_CD",
        /* 259 */ "Times Roman         Cart_R",
        /* 260 */ "Garamond Antiqua    Cart_CC",
        /* 261 */ "Garamond Halbfett   Cart_CC",
        /* 262 */ "Times Roman         Cart_U",
        /* 263 */ "Letter2 Gothic 14    Cart_M",
        /* 264 */ "Helvetica   Softfont",
        /* 265 */ "Times Roman Softfont",
        /* 266 */ "Helv_AD/AF",
        /* 267 */ "TmsRmn_AD/AF",
        /* 268 */ "TimesNewRoman",
        /* 269 */ "PS",
        /* 270 */ "Font",
        /* 271 */ "Pica_D",
        /* 272 */ "Proportional_LQ",
        /* 273 */ "HelveticaBold",
        /* 274 */ "Times",
        /* 275 */ "LinePrinter",
        /* 276 */ "Courier_Footnote",
        /* 277 */ "Courier Footnote",
        /* 278 */ "ARIAL",
        /* 279 */ "ARIAL Narrow",
        /* 280 */ "ITC Avant Garde",
        /* 281 */ "ITC Bookman",
        /* 282 */ "Century Schoolbook",
        /* 283 */ "Zapf Calligraphic",
        /* 284 */ "ITC Zapf Chancery",
        /* 285 */ "Sans Serif LQ",
        /* 286 */ "Orator gro\xE1",
        /* 287 */ "Orator klein",
        /* 288 */ "Courier LQ",
        /* 289 */ "Sans Serif NLQ PS",
        /* 290 */ "Courier NLQ PS",
        /* 291 */ "Sans_Serif_LQ",
        /* 292 */ "San_Serif_NLQ_PS",
        /* 293 */ "Courier_NLQ_PS",
        /* 294 */ "LQ San Serif",
        /* 295 */ "Optimo",
        /* 296 */ "LQ Helvetica",
        /* 297 */ "LQ Orator I",
        /* 298 */ "Code 39",
        /* 299 */ "LQ Orator II",
        /* 300 */ "LQ OCR-B",
        /* 301 */ "LQ OCR-A",
        /* 302 */ "LQ Script",
        /* 303 */ "LQ Courier",
        /* 304 */ "LQ Prestige",
        /* 305 */ "LQ Letter Gothic",
        /* 306 */ "LQ Times Roman",
        /* 307 */ "LQ TW Light",
        /* 308 */ "Cinema",
        /* 309 */ "LQ Blippo",
        /* 310 */ "PS San Serif",
        /* 311 */ "PS Helvetica",
        /* 312 */ "PS Orator I",
        /* 313 */ "PS Orator II",
        /* 314 */ "PS Courier",
        /* 315 */ "PS Prestige",
        /* 316 */ "PS Letter Gothic",
        /* 317 */ "PS Times Roman",
        /* 318 */ "PS TW Light",
        /* 319 */ "PS Blippo",
        /* 320 */ "S.Bern",
        /* 321 */ "S.Roman",
        /* 322 */ "S.Bern PS",
        /* 323 */ "Orator PS",
        /* 324 */ "S.Roman PS",
        /* 325 */ "HQ",
        /* 326 */ "Elite_D",
        /* 327 */ "Schrift-Art 1",
        /* 328 */ "Schrift-Art 2",
        /* 329 */ "Schrift-Art 3",
        /* 330 */ "Schrift-Art 4",
        /* 331 */ "Schrift-Art 5",
        /* 332 */ "Draft Gothik",
        /* 333 */ "Quick Gothik",
        /* 334 */ "Prestige Elite WP",
        /* 335 */ "Helvette PS",
        /* 336 */ "Bold PS",
        /* 337 */ "Times PS",
        /* 338 */ "Souvenier",
        /* 339 */ "Superfocus",
        /* 340 */ "LQ PS",
        /* 341 */ "Ext.Font",
        /* 342 */ "Pica LQ",
        /* 343 */ "Pica HD",
        /* 344 */ "Elite LQ",
        /* 345 */ "Elite HD",
        /* 346 */ "PS HD",
        /* 347 */ "Draft_Gothik",
        /* 348 */ "Fast Focus 10",
        /* 349 */ "Fast Focus 12",
        /* 350 */ "Pr._Elite",
        /* 351 */ "ITC_Souvenir",
        /* 352 */ "LQ Font Cartridges",
        /* 353 */ "Pica/Draft",
        /* 354 */ "Pica_HS",
        /* 355 */ "PS_LQ",
        /* 356 */ "PS_LQ(hoch/tief)",
        /* 357 */ "OCR B               702",
        /* 358 */ "OCR A               702",
        /* 359 */ "Bar Code 39         702",
        /* 360 */ "UPC / EAN           702",
        /* 361 */ "Super Focus         707",
        /* 362 */ "Helvette ASCII      701",
        /* 363 */ "Helvette Greek-Math 701",
        /* 364 */ "Prestige Pica       708",
        /* 365 */ "Letter Gothic       703",
        /* 366 */ "Letter Gothic       707",
        /* 367 */ "Letter Gothic 14 P. 703",
        /* 368 */ "ITC Souvenir",
        /* 369 */ "Times ASCII         701",
        /* 370 */ "Times Greek-Math    701",
        /* 371 */ "Helvette PS   18 P. 705",
        /* 372 */ "Helvette PS   24 P. 705",
        /* 373 */ "Bold Italic PS      708",
        /* 374 */ "Times PS      18 P. 704",
        /* 375 */ "Times PS      24 P. 704",
        /* 376 */ "ITC Bookman   10 P. 706",
        /* 377 */ "ITC Bookman   18 P. 706",
        /* 378 */ "ITC Bookman   24 P. 706",
        /* 379 */ "Helvetica PS",
        /* 380 */ "Helvetica PS Scale",
        /* 381 */ "Times PS Scale",
        /* 382 */ "Helvette_PS",
        /* 383 */ "Times_PS",
        /* 384 */ "NLQ##",
        /* 385 */ "HS_Draft",
        /* 386 */ "Helvette",
        /* 387 */ "Prestige      Cart",
        /* 388 */ "Letter Gothic Cart",
        /* 389 */ "Schrift       Cart",
        /* 390 */ "Hevette PS",
        /* 391 */ "Fontkassette LQ",
        /* 392 */ "Courier_LQ_PS",
        /* 393 */ "Utility",
        /* 394 */ "Gothik_LQ",
        /* 395 */ "Helvette_LQ",
        /* 396 */ "Bold_LQ",
        /* 397 */ "Prestige_LQ",
        /* 398 */ "Roman_LQ",
        /* 399 */ "Gothik_PS",
        /* 400 */ "Draft HS",
        /* 401 */ "Helvette LQ",
        /* 402 */ "OCR-B Cassette",
        /* 403 */ "OCR-A Cassette",
        /* 404 */ "Roman LQ",
        /* 405 */ "Prestige LQ",
        /* 406 */ "Letter Gothic LQ",
        /* 407 */ "Helvette LQ PS",
        /* 408 */ "Courier LQ PS",
        /* 409 */ "Roman LQ PS",
        /* 410 */ "Prestige LQ PS",
        /* 411 */ "Letter Gothic LQ PS",
        /* 412 */ "Cassettenschrift",
        /* 413 */ "Draft_HS",
        /* 414 */ "Fontkassette_LQ",
        /* 415 */ "HelveticaBold_S2",
        /* 416 */ "TimesRoman_S2",
        /* 417 */ "LinePrinterIBM",
        /* 418 */ "Helvetic",
        /* 419 */ "TimesRomanCompr.",
        /* 420 */ "LinePrinterR8",
        /* 421 */ "Pica NLQ",
        /* 422 */ "Proportional LQ",
        /* 423 */ "Letter Gothic  6",
        /* 424 */ "Letter Gothic  12",
        /* 425 */ "Letter Gothic  4\xAC",
        /* 426 */ "Letter Gothic  9\xAB",
        /* 427 */ "Times Nordic        Cart E",
        /* 428 */ "Courier        Cart 1",
        /* 429 */ "Prestige Elite Cart 1",
        /* 430 */ "Letter Gothic  Cart 6",
        /* 431 */ "Prestige Elite Cart 7",
        /* 432 */ "ARIAL R8       Cart 2",
        /* 433 */ "ARIAL R8       Cart 5",
        /* 434 */ "ARIAL R8       Cart 8",
        /* 435 */ "ARIAL R8",
        /* 436 */ "Dutch R8       Cart 3",
        /* 437 */ "Dutch R8       Cart 5",
        /* 438 */ "Dutch R8       Cart 8",
        /* 439 */ "Dutch R8",
        /* 440 */ "Century 702",
        /* 441 */ "NLQ PS",
        /* 442 */ "Elite NLQ",
        /* 443 */ "Pica PS",
        /* 444 */ "Pica PS NLQ",
        /* 445 */ "Datenschrift",
        /* 446 */ "Font Card",
        /* 447 */ "BOLD",
        /* 448 */ "BOLD PS",
        /* 449 */ "Outline",
        /* 450 */ "Century",
        /* 451 */ "Letter_Gothic_PS",
        /* 452 */ "Typenrad PS",
        /* 453 */ "IC-Font",
        /* 454 */ "Hoch-Tief-Schrift",
        /* 455 */ "Roman Script",
        /* 456 */ "Roman Script PS",
        /* 457 */ "Font 3",
        /* 458 */ "Font 1",
        /* 459 */ "Font 2",
        /* 460 */ "Font 3 PS",
        /* 461 */ "Font 1 PS",
        /* 462 */ "Font 2 PS",
        /* 463 */ "Helvetica-Narrow",
        /* 464 */ "AvantGarde-Book",
        /* 465 */ "Times-Roman",
        /* 466 */ "Bookman-Light",
        /* 467 */ "NewCentury-SchlbkRoman",
        /* 468 */ "Palatino-Roman",
        /* 469 */ "ZapfChancery-MediumItalic",
        /* 470 */ "P_LQ",
        /* 471 */ "P_NLQ",
        /* 472 */ "Elite_LQ",
        /* 473 */ "Elite_NLQ",
        /* 474 */ "Corpora_LQ",
        /* 475 */ "Corpora_NLQ",
        /* 476 */ "Cubic_PS_LQ",
        /* 477 */ "Cubic_PS_NLQ",
        /* 478 */ "Cubic_Mini_PS_LQ",
        /* 479 */ "Cubic_Mini_PS_NLQ",
        /* 480 */ "S.Helv",
        /* 481 */ "S.Helvetica",
        /* 482 */ "S.Helvetica PS",
        /* 483 */ "Gothic 12",
        /* 484 */ "TW-Light",
        /* 485 */ "San Serif PS",
        /* 486 */ "TW-Light PS",
        /* 487 */ "Cinema PS",
        /* 488 */ "Orator_gro\xE1",
        /* 489 */ "Orator_klein",
        /* 490 */ "Orator_LQ",
        /* 491 */ "Script_LQ",
        /* 492 */ "OPTIONAL_LQ *",
        /* 493 */ "Orator_PS",
        /* 494 */ "OPTIONAL_PS *",
        /* 495 */ "GothicR8",
        /* 496 */ "Helvet",
        /* 497 */ "TimesIBM",
        /* 498 */ "PS_NLQ",
        /* 499 */ "Draft Hoch",
        /* 500 */ "LQ Roman",
        /* 501 */ "LQ H-GOTHIC",
        /* 502 */ "LQ Roman PS",
        /* 503 */ "LQ Orator",
        /* 504 */ "LQ Optimo",
        /* 505 */ "LQ Courier Hoch",
        /* 506 */ "SLQ Times Roman",
        /* 507 */ "LQ TW-Light",
        /* 508 */ "SLQ TW-Light",
        /* 509 */ "Cassetten-Schrift",
        /* 510 */ "LQ Greek",
        /* 511 */ "LQ Cinema",
        /* 512 */ "LQ Helvetica PS",
        /* 513 */ "LQ Orator PS",
        /* 514 */ "LQ Optimo PS",
        /* 515 */ "LQ Courier PS",
        /* 516 */ "LQ Pestige PS",
        /* 517 */ "LQ Letter Gothic PS",
        /* 518 */ "LQ Times Roman PS",
        /* 519 */ "SLQ Times Roman PS",
        /* 520 */ "LQ TW-Light PS",
        /* 521 */ "SLQ TW-Light PS",
        /* 522 */ "LQ Cinema PS",
        /* 523 */ "LQ Blippo PS",
        /* 524 */ "LQ Bar Code 39",
        /* 525 */ "SLQ Script",
        /* 526 */ "LQ Orane (Optimo)",
        /* 527 */ "SLQ Roman",
        /* 528 */ "LQ H-Gothic (Helvet)",
        /* 529 */ "LQ Sanserif",
        /* 530 */ "LQ H-Gothic (Helvet) PS",
        /* 531 */ "LQ Orane (Optimo) PS",
        /* 532 */ "LQ Prestige PS",
        /* 533 */ "SLQ Roman PS",
        /* 534 */ "LQ Sanserif PS",
        /* 535 */ "LQ Times-Roman",
        /* 536 */ "SLQ Times-Roman",
        /* 537 */ "PS Sanserif",
        /* 538 */ "PS Script",
        /* 539 */ "PS Times-Roman",
        /* 540 */ "PS TW-Light",
        /* 541 */ "Optionaler Font",
        /* 542 */ "PrestigeElite PS",
        /* 543 */ "Kleinschrift",
        /* 544 */ "Schnellschrift",
        /* 545 */ "Font_4",
        /* 546 */ "Font_5",
        /* 547 */ "Font_6",
        /* 548 */ "Font_7",
        /* 549 */ "Deutsch mit Serifen",
        /* 550 */ "IBM mit Serifen",
        /* 551 */ "German Courier 10 (B8156-01)",
        /* 552 */ "Deutsch ohne Serifen",
        /* 553 */ "IBM ohne Serifen",
        /* 554 */ "Deutsch mit Serifen PS",
        /* 555 */ "IBM mit Serifen PS",
        /* 556 */ "Deutsch ohne Serifen PS",
        /* 557 */ "IBM ohne Serifen PS"
};

static const struct Sw6PrinterFont
{
    USHORT nFontNo, nFontNmNo;
    FontFamily eFamily;
    sal_Bool bPCFont, bFixPitch;
}
        a5000DI[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  35,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  98,   2, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aACERLP75[] = {
            {   2,   3, FAMILY_ROMAN, sal_True, sal_True },
            {  34,   4, FAMILY_ROMAN, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aAEG200I[] = {
            {   1,   5, FAMILY_ROMAN, sal_True, sal_True },
            {  15,   6, FAMILY_ROMAN, sal_True, sal_True },
            {  17,   7, FAMILY_ROMAN, sal_True, sal_True },
            {  19,   8, FAMILY_ROMAN, sal_True, sal_True },
            {  21,   9, FAMILY_ROMAN, sal_True, sal_True },
            {  23,  10, FAMILY_ROMAN, sal_True, sal_True },
            {  33,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  35,  11, FAMILY_ROMAN, sal_True, sal_True },
            {  37,  12, FAMILY_ROMAN, sal_True, sal_True },
            {  39,  13, FAMILY_ROMAN, sal_True, sal_True },
            {  41,  14, FAMILY_ROMAN, sal_True, sal_True },
            {  47,  15, FAMILY_ROMAN, sal_True, sal_True },
            {  49,  16, FAMILY_ROMAN, sal_True, sal_True },
            {  70,  17, FAMILY_SWISS, sal_True, sal_False },
            {  76,  18, FAMILY_SWISS, sal_True, sal_False },
            {  77,  19, FAMILY_SWISS, sal_True, sal_False },
            {  98,  20, FAMILY_ROMAN, sal_True, sal_False },
            { 100,  21, FAMILY_ROMAN, sal_True, sal_False },
            { 101,  22, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aBJ10E[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  34,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  97,   2, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aBJ130E[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  34,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  97,   2, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aBJ300[] = {
            {   2,   1, FAMILY_ROMAN, sal_True, sal_True },
            {   4,  23, FAMILY_ROMAN, sal_True, sal_True },
            {  35,  12, FAMILY_ROMAN, sal_True, sal_True },
            {  66,  24, FAMILY_SWISS, sal_True, sal_False },
            {  98,  25, FAMILY_ROMAN, sal_True, sal_False },
            { 102,   2, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aBR1109[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  38,  26, FAMILY_ROMAN, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aBR1209[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  31,  27, FAMILY_ROMAN, sal_True, sal_True },
            {  37,  12, FAMILY_ROMAN, sal_True, sal_True },
            {  41,  28, FAMILY_ROMAN, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aBR1224L[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  37,  12, FAMILY_ROMAN, sal_True, sal_True },
            {  99,  29, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aBR1324[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   4,  30, FAMILY_ROMAN, sal_True, sal_True },
            {   6,  31, FAMILY_ROMAN, sal_True, sal_True },
            {  15,   6, FAMILY_ROMAN, sal_True, sal_True },
            {  18,   7, FAMILY_ROMAN, sal_True, sal_True },
            {  26,  13, FAMILY_ROMAN, sal_True, sal_True },
            {  31,  27, FAMILY_ROMAN, sal_True, sal_True },
            {  36,  32, FAMILY_ROMAN, sal_True, sal_True },
            {  38,  12, FAMILY_ROMAN, sal_True, sal_True },
            {  41,  23, FAMILY_ROMAN, sal_True, sal_True },
            {  56,  33, FAMILY_ROMAN, sal_True, sal_True },
            {  57,  34, FAMILY_ROMAN, sal_True, sal_True },
            {  66,  35, FAMILY_SWISS, sal_True, sal_False },
            {  77,  36, FAMILY_SWISS, sal_True, sal_False },
            {  80,  37, FAMILY_SWISS, sal_True, sal_False },
            {  98,  38, FAMILY_ROMAN, sal_True, sal_True },
            {  99,  25, FAMILY_ROMAN, sal_True, sal_False },
            { 101,  24, FAMILY_ROMAN, sal_True, sal_False },
            { 108,  39, FAMILY_ROMAN, sal_True, sal_False },
            { 109,  40, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aBR1409[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  32,  41, FAMILY_ROMAN, sal_True, sal_True },
            {  38,  26, FAMILY_ROMAN, sal_True, sal_True },
            {  46,  42, FAMILY_ROMAN, sal_True, sal_True },
            { 110,  43, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aBR1509[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  32,  41, FAMILY_ROMAN, sal_True, sal_True },
            {  38,  26, FAMILY_ROMAN, sal_True, sal_True },
            {  42,  42, FAMILY_ROMAN, sal_True, sal_True },
            { 110,  43, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aBR1709[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  32,  41, FAMILY_ROMAN, sal_True, sal_True },
            {  38,  26, FAMILY_ROMAN, sal_True, sal_True },
            {  42,  42, FAMILY_ROMAN, sal_True, sal_True },
            { 110,  43, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aBR1724L[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  31,  27, FAMILY_ROMAN, sal_True, sal_True },
            {  37,  12, FAMILY_ROMAN, sal_True, sal_True },
            {  57,  34, FAMILY_ROMAN, sal_True, sal_True },
            {  80,  44, FAMILY_SWISS, sal_True, sal_False },
            {  99,  29, FAMILY_ROMAN, sal_True, sal_False },
            { 109,  45, FAMILY_ROMAN, sal_True, sal_False },
            { 110,  43, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aBR1824L[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   6,  31, FAMILY_ROMAN, sal_True, sal_True },
            {  15,   6, FAMILY_ROMAN, sal_True, sal_True },
            {  18,   7, FAMILY_ROMAN, sal_True, sal_True },
            {  31,  27, FAMILY_ROMAN, sal_True, sal_True },
            {  37,  12, FAMILY_ROMAN, sal_True, sal_True },
            {  41,  46, FAMILY_ROMAN, sal_True, sal_True },
            {  56,  33, FAMILY_ROMAN, sal_True, sal_True },
            {  57,  34, FAMILY_ROMAN, sal_True, sal_True },
            {  68,  47, FAMILY_SWISS, sal_True, sal_False },
            {  77,  36, FAMILY_SWISS, sal_True, sal_False },
            {  99,  48, FAMILY_ROMAN, sal_True, sal_False },
            { 102,  49, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aBR2024[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  58,  50, FAMILY_ROMAN, sal_True, sal_True },
            {  65,  51, FAMILY_SWISS, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aBR2518[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  16,  52, FAMILY_ROMAN, sal_True, sal_True },
            {  37,  12, FAMILY_ROMAN, sal_True, sal_True },
            {  41,  53, FAMILY_ROMAN, sal_True, sal_True },
            {  42,  54, FAMILY_ROMAN, sal_True, sal_True },
            {  58,  55, FAMILY_ROMAN, sal_True, sal_True },
            { 110,  56, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aBR3524L[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  31,  27, FAMILY_ROMAN, sal_True, sal_True },
            {  45,  28, FAMILY_ROMAN, sal_True, sal_True },
            {  57,  34, FAMILY_ROMAN, sal_True, sal_True },
            { 110,  43, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aBR4018[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  15,  57, FAMILY_ROMAN, sal_True, sal_True },
            {  31,  27, FAMILY_ROMAN, sal_True, sal_True },
            {  37,  12, FAMILY_ROMAN, sal_True, sal_True },
            {  41,  58, FAMILY_ROMAN, sal_True, sal_True },
            {  42,  59, FAMILY_ROMAN, sal_True, sal_True },
            {  57,  60, FAMILY_ROMAN, sal_True, sal_True },
            { 110,  43, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aBRHJ770[] = {
            {  16,   6, FAMILY_ROMAN, sal_True, sal_True },
            {  18,   7, FAMILY_ROMAN, sal_True, sal_True },
            {  26,  13, FAMILY_ROMAN, sal_True, sal_True },
            {  32,  27, FAMILY_ROMAN, sal_True, sal_True },
            {  38,  12, FAMILY_ROMAN, sal_True, sal_True },
            {  46,  61, FAMILY_ROMAN, sal_True, sal_True },
            {  56,  33, FAMILY_ROMAN, sal_True, sal_True },
            {  58,  34, FAMILY_ROMAN, sal_True, sal_True },
            {  65,  62, FAMILY_SWISS, sal_True, sal_False },
            {  66,  63, FAMILY_SWISS, sal_True, sal_False },
            {  97,  64, FAMILY_ROMAN, sal_True, sal_False },
            {  98,  65, FAMILY_ROMAN, sal_True, sal_False },
            { 109,  66, FAMILY_ROMAN, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aBRHL4[] = {
            {   1,  34, FAMILY_ROMAN, sal_True, sal_True },
            {   2,  61, FAMILY_ROMAN, sal_True, sal_True },
            {   3,  67, FAMILY_ROMAN, sal_True, sal_True },
            {   4,  68, FAMILY_ROMAN, sal_True, sal_True },
            {   5,  46, FAMILY_ROMAN, sal_True, sal_True },
            {   6,  12, FAMILY_ROMAN, sal_True, sal_True },
            {   7,  69, FAMILY_ROMAN, sal_True, sal_True },
            {   8,  70, FAMILY_ROMAN, sal_True, sal_True },
            {   9,  67, FAMILY_ROMAN, sal_True, sal_True },
            {  10,  71, FAMILY_ROMAN, sal_True, sal_True },
            {  11,  72, FAMILY_ROMAN, sal_True, sal_True },
            {  12,  68, FAMILY_ROMAN, sal_True, sal_True },
            {  13,  73, FAMILY_ROMAN, sal_True, sal_True },
            {  14,  69, FAMILY_ROMAN, sal_True, sal_True },
            {  15,  67, FAMILY_ROMAN, sal_True, sal_True },
            {  16,  71, FAMILY_ROMAN, sal_True, sal_True },
            {  17,  72, FAMILY_ROMAN, sal_True, sal_True },
            {  18,  68, FAMILY_ROMAN, sal_True, sal_True },
            {  19,  73, FAMILY_ROMAN, sal_True, sal_True },
            {  20,  74, FAMILY_ROMAN, sal_True, sal_True },
            {  21,  65, FAMILY_ROMAN, sal_True, sal_True },
            {  22,  75, FAMILY_ROMAN, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aBRHL48V[] = {
            {   1,  76, FAMILY_ROMAN, sal_True, sal_True },
            {  33,  77, FAMILY_ROMAN, sal_True, sal_True },
            {  34,  78, FAMILY_DONTKNOW, sal_False, sal_True },
            {  65,  79, FAMILY_SWISS, sal_True, sal_False },
            {  97,  80, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aBRHL8DB[] = {
            {   1,  77, FAMILY_ROMAN, sal_True, sal_True },
            {  43,  81, FAMILY_ROMAN, sal_True, sal_True },
            {  44,  82, FAMILY_ROMAN, sal_True, sal_True },
            {  53,  83, FAMILY_ROMAN, sal_True, sal_True },
            {  65,  84, FAMILY_SWISS, sal_True, sal_False },
            {  66,  85, FAMILY_SWISS, sal_True, sal_False },
            {  67,  86, FAMILY_SWISS, sal_True, sal_False },
            {  97,  87, FAMILY_ROMAN, sal_True, sal_False },
            {  98,  88, FAMILY_ROMAN, sal_True, sal_False },
            { 100,  89, FAMILY_ROMAN, sal_True, sal_False },
            { 101,  90, FAMILY_ROMAN, sal_True, sal_False },
            { 128,  91, FAMILY_ROMAN, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aBRHR[] = {
            {   1,   5, FAMILY_ROMAN, sal_True, sal_True },
            {  15,   6, FAMILY_ROMAN, sal_True, sal_True },
            {  17,   7, FAMILY_ROMAN, sal_True, sal_True },
            {  19,   8, FAMILY_ROMAN, sal_True, sal_True },
            {  21,   9, FAMILY_ROMAN, sal_True, sal_True },
            {  23,  10, FAMILY_ROMAN, sal_True, sal_True },
            {  33,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  35,  11, FAMILY_ROMAN, sal_True, sal_True },
            {  37,  12, FAMILY_ROMAN, sal_True, sal_True },
            {  39,  13, FAMILY_ROMAN, sal_True, sal_True },
            {  41,  14, FAMILY_ROMAN, sal_True, sal_True },
            {  47,  15, FAMILY_ROMAN, sal_True, sal_True },
            {  49,  16, FAMILY_ROMAN, sal_True, sal_True },
            {  70,  17, FAMILY_SWISS, sal_True, sal_False },
            {  76,  18, FAMILY_SWISS, sal_True, sal_False },
            {  77,  19, FAMILY_SWISS, sal_True, sal_False },
            {  98,  20, FAMILY_ROMAN, sal_True, sal_False },
            { 100,  21, FAMILY_ROMAN, sal_True, sal_False },
            { 101,  22, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aBROTHER[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  32,  27, FAMILY_ROMAN, sal_True, sal_True },
            {  38,  12, FAMILY_ROMAN, sal_True, sal_True },
            {  46,  23, FAMILY_ROMAN, sal_True, sal_True },
            {  58,  34, FAMILY_ROMAN, sal_True, sal_True },
            {  80,  37, FAMILY_SWISS, sal_True, sal_False },
            {  99,  25, FAMILY_ROMAN, sal_True, sal_False },
            { 103,  24, FAMILY_ROMAN, sal_True, sal_False },
            { 109,  40, FAMILY_ROMAN, sal_True, sal_False },
            { 110,  92, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aCANONBJX[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   2,   1, FAMILY_ROMAN, sal_True, sal_True },
            {   4,  30, FAMILY_ROMAN, sal_True, sal_True },
            {  13,  93, FAMILY_ROMAN, sal_True, sal_True },
            {  14,  94, FAMILY_ROMAN, sal_True, sal_True },
            {  35,  32, FAMILY_ROMAN, sal_True, sal_True },
            {  38,  12, FAMILY_ROMAN, sal_True, sal_True },
            {  40,  13, FAMILY_ROMAN, sal_True, sal_True },
            {  66,  35, FAMILY_SWISS, sal_True, sal_False },
            {  97,   2, FAMILY_ROMAN, sal_True, sal_False },
            {  98,  38, FAMILY_ROMAN, sal_True, sal_False },
            {  99,  25, FAMILY_ROMAN, sal_True, sal_False },
            { 100,  36, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aCE500[] = {
            {   1,  95, FAMILY_ROMAN, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aCE700[] = {
            {   1,   5, FAMILY_ROMAN, sal_True, sal_True },
            {  15,   6, FAMILY_ROMAN, sal_True, sal_True },
            {  17,   7, FAMILY_ROMAN, sal_True, sal_True },
            {  19,   8, FAMILY_ROMAN, sal_True, sal_True },
            {  21,   9, FAMILY_ROMAN, sal_True, sal_True },
            {  23,  10, FAMILY_ROMAN, sal_True, sal_True },
            {  33,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  35,  11, FAMILY_ROMAN, sal_True, sal_True },
            {  37,  12, FAMILY_ROMAN, sal_True, sal_True },
            {  39,  13, FAMILY_ROMAN, sal_True, sal_True },
            {  41,  14, FAMILY_ROMAN, sal_True, sal_True },
            {  47,  15, FAMILY_ROMAN, sal_True, sal_True },
            {  49,  16, FAMILY_ROMAN, sal_True, sal_True },
            {  70,  17, FAMILY_SWISS, sal_True, sal_False },
            {  76,  18, FAMILY_SWISS, sal_True, sal_False },
            {  77,  19, FAMILY_SWISS, sal_True, sal_False },
            {  98,  20, FAMILY_ROMAN, sal_True, sal_False },
            { 100,  21, FAMILY_ROMAN, sal_True, sal_False },
            { 101,  22, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aCIE_210E[] = {
            {   1,   5, FAMILY_ROMAN, sal_True, sal_True },
            {   2,  96, FAMILY_ROMAN, sal_True, sal_True },
            {  65,  97, FAMILY_SWISS, sal_True, sal_False },
            {  66,  98, FAMILY_SWISS, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aCIE_310E[] = {
            {   1,   5, FAMILY_ROMAN, sal_True, sal_True },
            {   2,  96, FAMILY_ROMAN, sal_True, sal_True },
            {  17,  99, FAMILY_ROMAN, sal_True, sal_True },
            {  19, 100, FAMILY_ROMAN, sal_True, sal_True },
            {  41,  14, FAMILY_ROMAN, sal_True, sal_True },
            {  65,  97, FAMILY_SWISS, sal_True, sal_False },
            {  66,  98, FAMILY_SWISS, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aCIE_610E[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  15,   6, FAMILY_ROMAN, sal_True, sal_True },
            {  17,   7, FAMILY_ROMAN, sal_True, sal_True },
            {  33,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  97, 101, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aCIE_610P[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   2,  30, FAMILY_ROMAN, sal_True, sal_True },
            {  13, 102, FAMILY_ROMAN, sal_True, sal_True },
            {  33,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  34,  32, FAMILY_ROMAN, sal_True, sal_True },
            {  50,  61, FAMILY_ROMAN, sal_True, sal_True },
            {  65,  35, FAMILY_SWISS, sal_True, sal_False },
            {  97,   2, FAMILY_ROMAN, sal_True, sal_False },
            {  98,  38, FAMILY_ROMAN, sal_True, sal_False },
            { 110, 103, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aCIE_715E[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   3,   5, FAMILY_ROMAN, sal_True, sal_True },
            {  13,  93, FAMILY_ROMAN, sal_True, sal_True },
            {  15,   7, FAMILY_ROMAN, sal_True, sal_True },
            {  17,   6, FAMILY_ROMAN, sal_True, sal_True },
            {  33,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 104, FAMILY_ROMAN, sal_True, sal_True },
            {  35,  11, FAMILY_ROMAN, sal_True, sal_True },
            {  37, 105, FAMILY_ROMAN, sal_True, sal_True },
            {  39,  13, FAMILY_ROMAN, sal_True, sal_True },
            {  41,  23, FAMILY_ROMAN, sal_True, sal_True },
            {  42,  14, FAMILY_ROMAN, sal_True, sal_True },
            {  97, 101, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aCIT120D[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  33, 106, FAMILY_ROMAN, sal_True, sal_True },
            {  97, 107, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aCIT120DP[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  33, 106, FAMILY_ROMAN, sal_True, sal_True },
            {  36, 108, FAMILY_ROMAN, sal_True, sal_True },
            {  97, 107, FAMILY_ROMAN, sal_True, sal_False },
            {  98, 109, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aCIT124D[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  33,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  35, 110, FAMILY_ROMAN, sal_True, sal_True },
            {  97,   2, FAMILY_ROMAN, sal_True, sal_False },
            {  98, 111, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aCITOH52[] = {
            {   1,   3, FAMILY_ROMAN, sal_True, sal_True },
            {  33, 112, FAMILY_ROMAN, sal_True, sal_True },
            {  43,  82, FAMILY_ROMAN, sal_True, sal_True },
            {  44, 113, FAMILY_ROMAN, sal_True, sal_True },
            {  45, 113, FAMILY_ROMAN, sal_True, sal_True },
            {  46, 113, FAMILY_ROMAN, sal_True, sal_True },
            {  53,  83, FAMILY_ROMAN, sal_True, sal_True },
            {  54,  83, FAMILY_ROMAN, sal_True, sal_True },
            {  55,  83, FAMILY_ROMAN, sal_True, sal_True },
            {  65,  85, FAMILY_SWISS, sal_True, sal_False },
            {  66,  85, FAMILY_SWISS, sal_True, sal_False },
            {  67,  85, FAMILY_SWISS, sal_True, sal_False },
            {  97,  87, FAMILY_ROMAN, sal_True, sal_False },
            {  98,  87, FAMILY_ROMAN, sal_True, sal_False },
            {  99,  87, FAMILY_ROMAN, sal_True, sal_False },
            { 100,  87, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aD_630[] = {
            {   1,   5, FAMILY_ROMAN, sal_True, sal_True },
            {  15,   6, FAMILY_ROMAN, sal_True, sal_True },
            {  17,   7, FAMILY_ROMAN, sal_True, sal_True },
            {  19,   8, FAMILY_ROMAN, sal_True, sal_True },
            {  21,   9, FAMILY_ROMAN, sal_True, sal_True },
            {  23,  10, FAMILY_ROMAN, sal_True, sal_True },
            {  33,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  35,  11, FAMILY_ROMAN, sal_True, sal_True },
            {  37,  12, FAMILY_ROMAN, sal_True, sal_True },
            {  39,  13, FAMILY_ROMAN, sal_True, sal_True },
            {  41,  14, FAMILY_ROMAN, sal_True, sal_True },
            {  47,  15, FAMILY_ROMAN, sal_True, sal_True },
            {  49,  16, FAMILY_ROMAN, sal_True, sal_True },
            {  70,  17, FAMILY_SWISS, sal_True, sal_False },
            {  76,  18, FAMILY_SWISS, sal_True, sal_False },
            {  77,  19, FAMILY_SWISS, sal_True, sal_False },
            {  98,  20, FAMILY_ROMAN, sal_True, sal_False },
            { 100,  21, FAMILY_ROMAN, sal_True, sal_False },
            { 101,  22, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aD_630E[] = {
            {   1,   5, FAMILY_ROMAN, sal_True, sal_True },
            {  15,   6, FAMILY_ROMAN, sal_True, sal_True },
            {  17,   7, FAMILY_ROMAN, sal_True, sal_True },
            {  19,   8, FAMILY_ROMAN, sal_True, sal_True },
            {  21,   9, FAMILY_ROMAN, sal_True, sal_True },
            {  23,  10, FAMILY_ROMAN, sal_True, sal_True },
            {  33,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  35,  11, FAMILY_ROMAN, sal_True, sal_True },
            {  37,  12, FAMILY_ROMAN, sal_True, sal_True },
            {  39,  13, FAMILY_ROMAN, sal_True, sal_True },
            {  41,  14, FAMILY_ROMAN, sal_True, sal_True },
            {  47,  15, FAMILY_ROMAN, sal_True, sal_True },
            {  49,  16, FAMILY_ROMAN, sal_True, sal_True },
            {  70,  17, FAMILY_SWISS, sal_True, sal_False },
            {  76,  18, FAMILY_SWISS, sal_True, sal_False },
            {  77,  19, FAMILY_SWISS, sal_True, sal_False },
            {  98,  20, FAMILY_ROMAN, sal_True, sal_False },
            { 100,  21, FAMILY_ROMAN, sal_True, sal_False },
            { 101,  22, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aD_630HZ[] = {
            {   1,   5, FAMILY_ROMAN, sal_True, sal_True },
            {  15,   6, FAMILY_ROMAN, sal_True, sal_True },
            {  17,   7, FAMILY_ROMAN, sal_True, sal_True },
            {  19,   8, FAMILY_ROMAN, sal_True, sal_True },
            {  21,   9, FAMILY_ROMAN, sal_True, sal_True },
            {  23,  10, FAMILY_ROMAN, sal_True, sal_True },
            {  33,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  35,  11, FAMILY_ROMAN, sal_True, sal_True },
            {  37,  12, FAMILY_ROMAN, sal_True, sal_True },
            {  39,  13, FAMILY_ROMAN, sal_True, sal_True },
            {  41,  14, FAMILY_ROMAN, sal_True, sal_True },
            {  47,  15, FAMILY_ROMAN, sal_True, sal_True },
            {  49,  16, FAMILY_ROMAN, sal_True, sal_True },
            {  70,  17, FAMILY_SWISS, sal_True, sal_False },
            {  76,  18, FAMILY_SWISS, sal_True, sal_False },
            {  77,  19, FAMILY_SWISS, sal_True, sal_False },
            {  98,  20, FAMILY_ROMAN, sal_True, sal_False },
            { 100,  21, FAMILY_ROMAN, sal_True, sal_False },
            { 101,  22, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aDICO150[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   3, 114, FAMILY_ROMAN, sal_True, sal_True },
            {  50,  13, FAMILY_ROMAN, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aDICO300I[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  25,  13, FAMILY_ROMAN, sal_True, sal_True },
            {  33, 115, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 114, FAMILY_ROMAN, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aDLP[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   2, 116, FAMILY_ROMAN, sal_True, sal_True },
            {   3, 117, FAMILY_ROMAN, sal_True, sal_True },
            {  21, 118, FAMILY_ROMAN, sal_True, sal_True },
            {  22, 119, FAMILY_ROMAN, sal_True, sal_True },
            {  24, 120, FAMILY_ROMAN, sal_True, sal_True },
            {  32, 121, FAMILY_ROMAN, sal_True, sal_True },
            {  33, 122, FAMILY_ROMAN, sal_True, sal_True },
            {  37, 123, FAMILY_ROMAN, sal_True, sal_True },
            {  39, 124, FAMILY_ROMAN, sal_True, sal_True },
            {  45, 125, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 126, FAMILY_SWISS, sal_True, sal_False },
            {  66, 127, FAMILY_SWISS, sal_True, sal_False },
            {  67, 128, FAMILY_SWISS, sal_True, sal_False },
            {  78, 129, FAMILY_SWISS, sal_True, sal_False },
            {  80, 130, FAMILY_SWISS, sal_True, sal_False },
            {  97, 131, FAMILY_ROMAN, sal_True, sal_False },
            {  99, 132, FAMILY_ROMAN, sal_True, sal_False },
            { 100, 133, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aDLQ[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   2, 106, FAMILY_ROMAN, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aDMP[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   2, 115, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 134, FAMILY_SWISS, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aEPLQ800[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  35,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  98,   2, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aEPLX800[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   3,  30, FAMILY_ROMAN, sal_True, sal_True },
            {  35,  32, FAMILY_ROMAN, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aEPSFX[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   3, 135, FAMILY_ROMAN, sal_True, sal_True },
            {  35,  32, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 126, FAMILY_SWISS, sal_True, sal_False },
            {  66, 136, FAMILY_SWISS, sal_True, sal_False },
            {  98, 137, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aEPSFX100[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   2, 138, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 139, FAMILY_SWISS, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aEPSFX80[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   2, 138, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 139, FAMILY_SWISS, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aEPSLQ[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   2,   1, FAMILY_ROMAN, sal_True, sal_True },
            {   4,  30, FAMILY_ROMAN, sal_True, sal_True },
            {  13,  93, FAMILY_ROMAN, sal_True, sal_True },
            {  14,  94, FAMILY_ROMAN, sal_True, sal_True },
            {  16,   6, FAMILY_ROMAN, sal_True, sal_True },
            {  18,   7, FAMILY_ROMAN, sal_True, sal_True },
            {  35,  32, FAMILY_ROMAN, sal_True, sal_True },
            {  38,  12, FAMILY_ROMAN, sal_True, sal_True },
            {  40, 140, FAMILY_ROMAN, sal_True, sal_True },
            {  66,  35, FAMILY_SWISS, sal_True, sal_False },
            {  97,   2, FAMILY_ROMAN, sal_True, sal_False },
            {  98,  38, FAMILY_ROMAN, sal_True, sal_False },
            {  99,  25, FAMILY_ROMAN, sal_True, sal_False },
            { 100,  36, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aEPSRX[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   2, 138, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 139, FAMILY_SWISS, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aESCP2[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   4,  30, FAMILY_ROMAN, sal_True, sal_True },
            {  16, 141, FAMILY_ROMAN, sal_True, sal_True },
            {  33,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  35,  32, FAMILY_ROMAN, sal_True, sal_True },
            {  38,  12, FAMILY_ROMAN, sal_True, sal_True },
            {  40,  13, FAMILY_ROMAN, sal_True, sal_True },
            {  41, 142, FAMILY_ROMAN, sal_True, sal_True },
            {  43,  93, FAMILY_ROMAN, sal_True, sal_True },
            {  44, 143, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 144, FAMILY_SWISS, sal_True, sal_False },
            {  66,  35, FAMILY_SWISS, sal_True, sal_False },
            {  97,   2, FAMILY_ROMAN, sal_True, sal_False },
            {  98,  38, FAMILY_ROMAN, sal_True, sal_False },
            {  99,  25, FAMILY_ROMAN, sal_True, sal_False },
            { 100, 145, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aFUJIDL24[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   3, 146, FAMILY_ROMAN, sal_True, sal_True },
            {  21, 147, FAMILY_ROMAN, sal_True, sal_True },
            {  22, 148, FAMILY_ROMAN, sal_True, sal_True },
            {  24, 149, FAMILY_ROMAN, sal_True, sal_True },
            {  26,  13, FAMILY_ROMAN, sal_True, sal_True },
            {  32, 121, FAMILY_ROMAN, sal_True, sal_True },
            {  33,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  37, 150, FAMILY_ROMAN, sal_True, sal_True },
            {  39, 124, FAMILY_ROMAN, sal_True, sal_True },
            {  42, 151, FAMILY_ROMAN, sal_True, sal_True },
            {  45,  61, FAMILY_ROMAN, sal_True, sal_True },
            {  56, 152, FAMILY_ROMAN, sal_True, sal_True },
            {  60, 153, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 126, FAMILY_SWISS, sal_True, sal_False },
            {  67, 154, FAMILY_SWISS, sal_True, sal_False },
            {  78, 129, FAMILY_SWISS, sal_True, sal_False },
            {  80, 130, FAMILY_SWISS, sal_True, sal_False },
            {  97,   2, FAMILY_ROMAN, sal_True, sal_False },
            {  99, 155, FAMILY_ROMAN, sal_True, sal_False },
            { 100, 133, FAMILY_ROMAN, sal_True, sal_False },
            { 103, 103, FAMILY_ROMAN, sal_True, sal_False },
            { 108, 156, FAMILY_ROMAN, sal_True, sal_False },
            { 110, 157, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aGABRIELE[] = {
            {   1, 158, FAMILY_ROMAN, sal_True, sal_True },
            {   4, 159, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 160, FAMILY_SWISS, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aGENICOM[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  34,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  40, 161, FAMILY_ROMAN, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aGQ3500SF[] = {
            {   1, 162, FAMILY_ROMAN, sal_True, sal_True },
            {  33, 163, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 164, FAMILY_ROMAN, sal_True, sal_True },
            {  35, 165, FAMILY_ROMAN, sal_True, sal_True },
            {  43, 166, FAMILY_ROMAN, sal_True, sal_True },
            {  55, 167, FAMILY_ROMAN, sal_True, sal_True },
            {  56, 168, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 169, FAMILY_SWISS, sal_True, sal_False },
            {  70, 170, FAMILY_SWISS, sal_True, sal_False },
            {  97, 171, FAMILY_ROMAN, sal_True, sal_False },
            { 164, 172, FAMILY_DECORATIVE, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aGQ5000[] = {
            {  33, 173, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 174, FAMILY_SWISS, sal_True, sal_False },
            {  97, 175, FAMILY_ROMAN, sal_True, sal_False },
            { 128, 176, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aGRAPHICS[] = {
            {  34,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 177, FAMILY_SWISS, sal_True, sal_True },
            {  66, 178, FAMILY_SWISS, sal_True, sal_True },
            {  67, 179, FAMILY_SWISS, sal_True, sal_True },
            {  68, 180, FAMILY_SWISS, sal_True, sal_True },
            {  69, 181, FAMILY_SWISS, sal_True, sal_True },
            {  70, 182, FAMILY_SWISS, sal_True, sal_True },
            {  71, 183, FAMILY_SWISS, sal_True, sal_True },
            {  72, 184, FAMILY_SWISS, sal_True, sal_True },
            {  73, 185, FAMILY_SWISS, sal_True, sal_True },
            {  74, 186, FAMILY_SWISS, sal_True, sal_True },
            {  75, 187, FAMILY_SWISS, sal_True, sal_True },
            {  76, 188, FAMILY_SWISS, sal_True, sal_True },
            {  77, 189, FAMILY_SWISS, sal_True, sal_True },
            {  78, 190, FAMILY_SWISS, sal_True, sal_True },
            {  79, 150, FAMILY_SWISS, sal_True, sal_True },
            {  80, 191, FAMILY_SWISS, sal_True, sal_True },
            {  81, 192, FAMILY_SWISS, sal_True, sal_True },
            {  82, 193, FAMILY_SWISS, sal_True, sal_True },
            {  83, 194, FAMILY_SWISS, sal_True, sal_True },
            {  84, 195, FAMILY_SWISS, sal_True, sal_True },
            {  85, 196, FAMILY_SWISS, sal_True, sal_True },
            {  86, 197, FAMILY_SWISS, sal_True, sal_True },
            {  87, 198, FAMILY_SWISS, sal_True, sal_True },
            {  88, 199, FAMILY_SWISS, sal_True, sal_True },
            {  89, 200, FAMILY_SWISS, sal_True, sal_True },
            {  90, 201, FAMILY_SWISS, sal_True, sal_True },
            {  91, 202, FAMILY_SWISS, sal_True, sal_True },
            {  92, 203, FAMILY_SWISS, sal_True, sal_True },
            {  97, 204, FAMILY_ROMAN, sal_True, sal_True },
            {  98, 205, FAMILY_ROMAN, sal_True, sal_True },
            { 103, 206, FAMILY_ROMAN, sal_True, sal_True },
            { 105, 207, FAMILY_ROMAN, sal_True, sal_True },
            { 109, 208, FAMILY_ROMAN, sal_True, sal_True },
            { 130, 209, FAMILY_DECORATIVE, sal_True, sal_True },
            { 135,  78, FAMILY_DONTKNOW, sal_False, sal_True },
            { 136,  78, FAMILY_DONTKNOW, sal_False, sal_True },
            { 137, 210, FAMILY_DONTKNOW, sal_False, sal_True },
            { 138, 211, FAMILY_DONTKNOW, sal_False, sal_True },
            { 140, 212, FAMILY_DONTKNOW, sal_False, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aHP4[] = {
            {   1, 213, FAMILY_ROMAN, sal_True, sal_True },
            {  33,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 177, FAMILY_SWISS, sal_True, sal_False },
            {  66, 214, FAMILY_SWISS, sal_True, sal_False },
            {  67, 215, FAMILY_SWISS, sal_True, sal_False },
            {  68, 216, FAMILY_SWISS, sal_True, sal_False },
            {  97, 204, FAMILY_ROMAN, sal_True, sal_False },
            {  98, 217, FAMILY_ROMAN, sal_True, sal_False },
            {  99, 218, FAMILY_ROMAN, sal_True, sal_False },
            { 102, 219, FAMILY_ROMAN, sal_True, sal_False },
            { 103,  61, FAMILY_ROMAN, sal_True, sal_True },
            { 131, 220, FAMILY_DECORATIVE, sal_True, sal_False },
            { 132, 221, FAMILY_DECORATIVE, sal_True, sal_False },
            { 133, 222, FAMILY_DECORATIVE, sal_True, sal_False },
            { 134, 223, FAMILY_DECORATIVE, sal_True, sal_False },
            { 135,  78, FAMILY_DONTKNOW, sal_False, sal_False },
            { 136,  78, FAMILY_DONTKNOW, sal_False, sal_False },
            { 137, 210, FAMILY_DONTKNOW, sal_False, sal_False },
            { 138, 211, FAMILY_DONTKNOW, sal_False, sal_False },
            { 140, 212, FAMILY_DONTKNOW, sal_False, sal_False },
            { 141, 224, FAMILY_DECORATIVE, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aHP4SFS[] = {
            {   1, 213, FAMILY_ROMAN, sal_True, sal_True },
            {  33,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 177, FAMILY_SWISS, sal_True, sal_False },
            {  66, 214, FAMILY_SWISS, sal_True, sal_False },
            {  67, 215, FAMILY_SWISS, sal_True, sal_False },
            {  68, 216, FAMILY_SWISS, sal_True, sal_False },
            {  69, 225, FAMILY_SWISS, sal_True, sal_False },
            {  74, 186, FAMILY_SWISS, sal_True, sal_False },
            {  97, 204, FAMILY_ROMAN, sal_True, sal_False },
            {  98, 226, FAMILY_ROMAN, sal_True, sal_False },
            {  99, 218, FAMILY_ROMAN, sal_True, sal_False },
            { 102, 219, FAMILY_ROMAN, sal_True, sal_False },
            { 103, 206, FAMILY_ROMAN, sal_True, sal_True },
            { 105, 207, FAMILY_ROMAN, sal_True, sal_False },
            { 109, 208, FAMILY_ROMAN, sal_True, sal_False },
            { 130, 209, FAMILY_DECORATIVE, sal_True, sal_False },
            { 131, 220, FAMILY_DECORATIVE, sal_True, sal_False },
            { 132, 221, FAMILY_DECORATIVE, sal_True, sal_False },
            { 133, 222, FAMILY_DECORATIVE, sal_True, sal_False },
            { 134, 223, FAMILY_DECORATIVE, sal_True, sal_False },
            { 135,  78, FAMILY_DONTKNOW, sal_False, sal_False },
            { 137, 210, FAMILY_DONTKNOW, sal_False, sal_False },
            { 138, 211, FAMILY_DONTKNOW, sal_False, sal_False },
            { 140, 212, FAMILY_DONTKNOW, sal_False, sal_False },
            { 141, 224, FAMILY_DECORATIVE, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aHPDJ[] = {
            {  33, 227, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 228, FAMILY_ROMAN, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aHPDJ500C[] = {
            {  17, 229, FAMILY_ROMAN, sal_True, sal_True },
            {  18, 230, FAMILY_ROMAN, sal_True, sal_True },
            {  19, 231, FAMILY_ROMAN, sal_True, sal_True },
            {  20, 232, FAMILY_ROMAN, sal_True, sal_True },
            {  21, 233, FAMILY_ROMAN, sal_True, sal_True },
            {  22, 234, FAMILY_ROMAN, sal_True, sal_True },
            {  28, 235, FAMILY_ROMAN, sal_True, sal_True },
            {  32, 236, FAMILY_ROMAN, sal_True, sal_True },
            {  33, 227, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 228, FAMILY_ROMAN, sal_True, sal_True },
            {  37, 237, FAMILY_ROMAN, sal_True, sal_True },
            {  38, 238, FAMILY_ROMAN, sal_True, sal_True },
            {  39, 239, FAMILY_ROMAN, sal_True, sal_True },
            {  43, 240, FAMILY_ROMAN, sal_True, sal_True },
            {  44, 241, FAMILY_ROMAN, sal_True, sal_True },
            {  46, 242, FAMILY_ROMAN, sal_True, sal_True },
            {  49, 243, FAMILY_ROMAN, sal_True, sal_True },
            {  51, 244, FAMILY_ROMAN, sal_True, sal_True },
            {  52, 245, FAMILY_ROMAN, sal_True, sal_True },
            {  53, 246, FAMILY_ROMAN, sal_True, sal_True },
            {  54, 247, FAMILY_ROMAN, sal_True, sal_True },
            {  55, 248, FAMILY_ROMAN, sal_True, sal_True },
            {  56, 249, FAMILY_ROMAN, sal_True, sal_True },
            {  57, 250, FAMILY_ROMAN, sal_True, sal_True },
            {  64, 251, FAMILY_ROMAN, sal_True, sal_True },
            {  68, 252, FAMILY_SWISS, sal_True, sal_False },
            {  69, 253, FAMILY_SWISS, sal_True, sal_False },
            {  70, 254, FAMILY_SWISS, sal_True, sal_False },
            {  71, 255, FAMILY_SWISS, sal_True, sal_False },
            {  72, 256, FAMILY_SWISS, sal_True, sal_False },
            {  77, 257, FAMILY_SWISS, sal_True, sal_False },
            {  78, 258, FAMILY_SWISS, sal_True, sal_False },
            { 101, 218, FAMILY_ROMAN, sal_True, sal_False },
            { 102, 259, FAMILY_ROMAN, sal_True, sal_False },
            { 103, 260, FAMILY_ROMAN, sal_True, sal_False },
            { 104, 261, FAMILY_ROMAN, sal_True, sal_False },
            { 105, 204, FAMILY_ROMAN, sal_True, sal_False },
            { 107, 262, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aHPDJ550[] = {
            {  33, 227, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 228, FAMILY_ROMAN, sal_True, sal_True },
            {  43, 240, FAMILY_ROMAN, sal_True, sal_True },
            {  44, 241, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 177, FAMILY_SWISS, sal_True, sal_False },
            {  97, 204, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aHPDJCAR[] = {
            {  17, 229, FAMILY_ROMAN, sal_True, sal_True },
            {  18, 230, FAMILY_ROMAN, sal_True, sal_True },
            {  19, 231, FAMILY_ROMAN, sal_True, sal_True },
            {  20, 232, FAMILY_ROMAN, sal_True, sal_True },
            {  21, 233, FAMILY_ROMAN, sal_True, sal_True },
            {  22, 234, FAMILY_ROMAN, sal_True, sal_True },
            {  33, 227, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 228, FAMILY_ROMAN, sal_True, sal_True },
            {  37, 237, FAMILY_ROMAN, sal_True, sal_True },
            {  38, 238, FAMILY_ROMAN, sal_True, sal_True },
            {  39, 239, FAMILY_ROMAN, sal_True, sal_True },
            {  46, 242, FAMILY_ROMAN, sal_True, sal_True },
            {  49, 243, FAMILY_ROMAN, sal_True, sal_True },
            {  51, 244, FAMILY_ROMAN, sal_True, sal_True },
            {  52, 245, FAMILY_ROMAN, sal_True, sal_True },
            {  53, 246, FAMILY_ROMAN, sal_True, sal_True },
            {  54, 247, FAMILY_ROMAN, sal_True, sal_True },
            {  55, 263, FAMILY_ROMAN, sal_True, sal_True },
            {  56, 249, FAMILY_ROMAN, sal_True, sal_True },
            {  57, 250, FAMILY_ROMAN, sal_True, sal_True },
            {  68, 252, FAMILY_SWISS, sal_True, sal_False },
            {  69, 253, FAMILY_SWISS, sal_True, sal_False },
            {  70, 254, FAMILY_SWISS, sal_True, sal_False },
            {  71, 255, FAMILY_SWISS, sal_True, sal_False },
            {  72, 256, FAMILY_SWISS, sal_True, sal_False },
            { 102, 259, FAMILY_ROMAN, sal_True, sal_False },
            { 107, 262, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aHPDJSF[] = {
            {  33, 227, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 228, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 264, FAMILY_SWISS, sal_True, sal_False },
            {  97, 265, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aHPIII[] = {
            {   1, 213, FAMILY_ROMAN, sal_True, sal_True },
            {  33, 112, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 177, FAMILY_SWISS, sal_True, sal_False },
            {  66, 214, FAMILY_SWISS, sal_True, sal_False },
            {  97, 204, FAMILY_ROMAN, sal_True, sal_False },
            { 135,  78, FAMILY_DONTKNOW, sal_False, sal_False },
            { 140, 212, FAMILY_DONTKNOW, sal_False, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aHPIIISFS[] = {
            {   1, 213, FAMILY_ROMAN, sal_True, sal_True },
            {  33, 112, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 177, FAMILY_SWISS, sal_True, sal_False },
            {  66, 178, FAMILY_SWISS, sal_True, sal_False },
            {  67, 179, FAMILY_SWISS, sal_True, sal_False },
            {  68, 180, FAMILY_SWISS, sal_True, sal_False },
            {  69, 225, FAMILY_SWISS, sal_True, sal_False },
            {  70, 182, FAMILY_SWISS, sal_True, sal_False },
            {  71, 183, FAMILY_SWISS, sal_True, sal_False },
            {  72, 184, FAMILY_SWISS, sal_True, sal_False },
            {  73, 185, FAMILY_SWISS, sal_True, sal_False },
            {  74, 186, FAMILY_SWISS, sal_True, sal_False },
            {  75, 187, FAMILY_SWISS, sal_True, sal_False },
            {  76, 188, FAMILY_SWISS, sal_True, sal_False },
            {  77, 189, FAMILY_SWISS, sal_True, sal_False },
            {  78, 190, FAMILY_SWISS, sal_True, sal_False },
            {  79, 150, FAMILY_SWISS, sal_True, sal_False },
            {  80, 191, FAMILY_SWISS, sal_True, sal_False },
            {  81, 192, FAMILY_SWISS, sal_True, sal_False },
            {  82, 193, FAMILY_SWISS, sal_True, sal_False },
            {  83, 194, FAMILY_SWISS, sal_True, sal_False },
            {  85, 196, FAMILY_SWISS, sal_True, sal_False },
            {  86, 197, FAMILY_SWISS, sal_True, sal_False },
            {  87, 198, FAMILY_SWISS, sal_True, sal_False },
            {  88, 199, FAMILY_SWISS, sal_True, sal_False },
            {  89, 200, FAMILY_SWISS, sal_True, sal_False },
            {  90, 201, FAMILY_SWISS, sal_True, sal_False },
            {  91, 202, FAMILY_SWISS, sal_True, sal_False },
            {  92, 203, FAMILY_SWISS, sal_True, sal_False },
            {  97, 204, FAMILY_ROMAN, sal_True, sal_False },
            {  98, 226, FAMILY_ROMAN, sal_True, sal_False },
            { 103, 206, FAMILY_ROMAN, sal_True, sal_False },
            { 105, 207, FAMILY_ROMAN, sal_True, sal_False },
            { 109, 208, FAMILY_ROMAN, sal_True, sal_False },
            { 130, 209, FAMILY_DECORATIVE, sal_True, sal_False },
            { 135,  78, FAMILY_DONTKNOW, sal_False, sal_False },
            { 136,  78, FAMILY_DONTKNOW, sal_False, sal_False },
            { 137, 210, FAMILY_DONTKNOW, sal_False, sal_False },
            { 138, 211, FAMILY_DONTKNOW, sal_False, sal_False },
            { 140, 212, FAMILY_DONTKNOW, sal_False, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aHPIIP[] = {
            {   1, 213, FAMILY_ROMAN, sal_True, sal_True },
            {   2,   3, FAMILY_ROMAN, sal_True, sal_True },
            {  33, 112, FAMILY_ROMAN, sal_True, sal_True },
            {  34,   4, FAMILY_ROMAN, sal_True, sal_True },
            {  43,  82, FAMILY_ROMAN, sal_True, sal_True },
            {  44, 113, FAMILY_ROMAN, sal_True, sal_True },
            {  45, 113, FAMILY_ROMAN, sal_True, sal_True },
            {  46, 113, FAMILY_ROMAN, sal_True, sal_True },
            {  53,  83, FAMILY_ROMAN, sal_True, sal_True },
            {  54,  83, FAMILY_ROMAN, sal_True, sal_True },
            {  55,  83, FAMILY_ROMAN, sal_True, sal_True },
            {  65,  85, FAMILY_SWISS, sal_True, sal_False },
            {  66,  85, FAMILY_SWISS, sal_True, sal_False },
            {  67,  85, FAMILY_SWISS, sal_True, sal_False },
            {  97,  87, FAMILY_ROMAN, sal_True, sal_False },
            {  98,  87, FAMILY_ROMAN, sal_True, sal_False },
            {  99,  87, FAMILY_ROMAN, sal_True, sal_False },
            { 100,  87, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aHPQJ[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  33, 115, FAMILY_ROMAN, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aHPXL300[] = {
            {   1, 213, FAMILY_ROMAN, sal_True, sal_True },
            {  33, 112, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 177, FAMILY_SWISS, sal_True, sal_False },
            {  66, 214, FAMILY_SWISS, sal_True, sal_False },
            {  97, 204, FAMILY_ROMAN, sal_True, sal_False },
            { 135,  78, FAMILY_DONTKNOW, sal_False, sal_False },
            { 140, 212, FAMILY_DONTKNOW, sal_False, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aHP_ADAF[] = {
            {   1, 213, FAMILY_ROMAN, sal_True, sal_True },
            {   2,   3, FAMILY_ROMAN, sal_True, sal_True },
            {  33, 112, FAMILY_ROMAN, sal_True, sal_True },
            {  34,   4, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 266, FAMILY_SWISS, sal_True, sal_False },
            {  97, 267, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aIBM_PPDS[] = {
            {  33,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  65,  85, FAMILY_SWISS, sal_True, sal_False },
            {  97, 268, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aIBM4019I[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 269, FAMILY_SWISS, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aIBM4072[] = {
            {  33,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  34,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  38,  12, FAMILY_ROMAN, sal_True, sal_True },
            {  46,  23, FAMILY_ROMAN, sal_True, sal_True },
            {  97,   2, FAMILY_ROMAN, sal_True, sal_False },
            {  98, 126, FAMILY_ROMAN, sal_True, sal_False },
            {  99,  25, FAMILY_ROMAN, sal_True, sal_False },
            { 107,  24, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aIBM5202[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 115, FAMILY_ROMAN, sal_True, sal_True },
            {  70, 130, FAMILY_SWISS, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aIBM6750[] = {
            {   1, 270, FAMILY_ROMAN, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aIBMGR[] = {
            {   1,   5, FAMILY_ROMAN, sal_True, sal_True },
            {   3, 271, FAMILY_ROMAN, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aIBMPRO[] = {
            {   1,   5, FAMILY_ROMAN, sal_True, sal_True },
            {   2,  96, FAMILY_ROMAN, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aIBMPRO24[] = {
            {   1,   5, FAMILY_ROMAN, sal_True, sal_True },
            {   2,  96, FAMILY_ROMAN, sal_True, sal_True },
            {  33,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  37, 105, FAMILY_ROMAN, sal_True, sal_True },
            {  97, 272, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aJUKI6500[] = {
            {   1,   5, FAMILY_ROMAN, sal_True, sal_True },
            {  15,   6, FAMILY_ROMAN, sal_True, sal_True },
            {  17,   7, FAMILY_ROMAN, sal_True, sal_True },
            {  19,   8, FAMILY_ROMAN, sal_True, sal_True },
            {  21,   9, FAMILY_ROMAN, sal_True, sal_True },
            {  23,  10, FAMILY_ROMAN, sal_True, sal_True },
            {  33,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  35,  11, FAMILY_ROMAN, sal_True, sal_True },
            {  37,  12, FAMILY_ROMAN, sal_True, sal_True },
            {  39,  13, FAMILY_ROMAN, sal_True, sal_True },
            {  41,  14, FAMILY_ROMAN, sal_True, sal_True },
            {  47,  15, FAMILY_ROMAN, sal_True, sal_True },
            {  49,  16, FAMILY_ROMAN, sal_True, sal_True },
            {  70,  17, FAMILY_SWISS, sal_True, sal_False },
            {  76,  18, FAMILY_SWISS, sal_True, sal_False },
            {  77,  19, FAMILY_SWISS, sal_True, sal_False },
            {  98,  20, FAMILY_ROMAN, sal_True, sal_False },
            { 100,  21, FAMILY_ROMAN, sal_True, sal_False },
            { 101,  22, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aKONICA[] = {
            {   1, 213, FAMILY_ROMAN, sal_True, sal_True },
            {   2,   3, FAMILY_ROMAN, sal_True, sal_True },
            {  33, 112, FAMILY_ROMAN, sal_True, sal_True },
            {  34,   4, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 273, FAMILY_SWISS, sal_True, sal_False },
            {  97, 274, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aKYOCERA[] = {
            {   1, 275, FAMILY_ROMAN, sal_True, sal_True },
            {  33,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  43,  82, FAMILY_ROMAN, sal_True, sal_True },
            {  53,  83, FAMILY_ROMAN, sal_True, sal_True },
            {  65,  85, FAMILY_SWISS, sal_True, sal_False },
            {  97,  87, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aKYOF1010[] = {
            {   1, 275, FAMILY_ROMAN, sal_True, sal_True },
            {  33,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  34,  82, FAMILY_ROMAN, sal_True, sal_True },
            {  35,  83, FAMILY_ROMAN, sal_True, sal_True },
            {  65,  85, FAMILY_SWISS, sal_True, sal_False },
            {  97,  87, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aLBP8II[] = {
            {  33, 276, FAMILY_ROMAN, sal_True, sal_True },
            {  34,   1, FAMILY_ROMAN, sal_True, sal_True },
            { 110, 153, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aLBP8III[] = {
            {  33, 277, FAMILY_ROMAN, sal_True, sal_True },
            {  34,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 278, FAMILY_SWISS, sal_True, sal_False },
            {  69, 279, FAMILY_SWISS, sal_True, sal_False },
            {  74, 280, FAMILY_SWISS, sal_True, sal_False },
            {  97, 153, FAMILY_ROMAN, sal_True, sal_False },
            { 103, 281, FAMILY_ROMAN, sal_True, sal_False },
            { 105, 282, FAMILY_ROMAN, sal_True, sal_False },
            { 109, 283, FAMILY_ROMAN, sal_True, sal_False },
            { 130, 284, FAMILY_DECORATIVE, sal_True, sal_False },
            { 140, 212, FAMILY_DONTKNOW, sal_False, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aLC_20[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   3, 285, FAMILY_ROMAN, sal_True, sal_True },
            {  13, 286, FAMILY_ROMAN, sal_True, sal_True },
            {  14, 287, FAMILY_ROMAN, sal_True, sal_True },
            {  35, 288, FAMILY_ROMAN, sal_True, sal_True },
            {  66, 289, FAMILY_SWISS, sal_True, sal_False },
            {  97, 290, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aLC_200[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   3, 291, FAMILY_ROMAN, sal_True, sal_True },
            {  13,  93, FAMILY_ROMAN, sal_True, sal_True },
            {  14,  13, FAMILY_ROMAN, sal_True, sal_True },
            {  35, 104, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 139, FAMILY_SWISS, sal_True, sal_False },
            {  66, 292, FAMILY_SWISS, sal_True, sal_False },
            {  97, 293, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aLC24_200[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   4, 294, FAMILY_ROMAN, sal_True, sal_True },
            {   6, 295, FAMILY_ROMAN, sal_True, sal_True },
            {   8, 296, FAMILY_ROMAN, sal_True, sal_True },
            {  10, 297, FAMILY_ROMAN, sal_True, sal_True },
            {  12, 298, FAMILY_ROMAN, sal_True, sal_True },
            {  14, 299, FAMILY_ROMAN, sal_True, sal_True },
            {  16, 300, FAMILY_ROMAN, sal_True, sal_True },
            {  18, 301, FAMILY_ROMAN, sal_True, sal_True },
            {  26, 302, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 303, FAMILY_ROMAN, sal_True, sal_True },
            {  38, 304, FAMILY_ROMAN, sal_True, sal_True },
            {  40, 305, FAMILY_ROMAN, sal_True, sal_True },
            {  44, 306, FAMILY_ROMAN, sal_True, sal_True },
            {  48, 307, FAMILY_ROMAN, sal_True, sal_True },
            {  62, 308, FAMILY_ROMAN, sal_True, sal_True },
            {  64, 309, FAMILY_ROMAN, sal_True, sal_True },
            {  66, 310, FAMILY_SWISS, sal_True, sal_False },
            {  68, 311, FAMILY_SWISS, sal_True, sal_False },
            {  69, 312, FAMILY_SWISS, sal_True, sal_False },
            {  71, 313, FAMILY_SWISS, sal_True, sal_False },
            {  97, 314, FAMILY_ROMAN, sal_True, sal_False },
            {  99, 315, FAMILY_ROMAN, sal_True, sal_False },
            { 100, 316, FAMILY_ROMAN, sal_True, sal_False },
            { 102, 317, FAMILY_ROMAN, sal_True, sal_False },
            { 104, 318, FAMILY_ROMAN, sal_True, sal_False },
            { 112, 319, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aLC866[] = {
            {   1, 213, FAMILY_ROMAN, sal_True, sal_True },
            {   2,   3, FAMILY_ROMAN, sal_True, sal_True },
            {  33, 112, FAMILY_ROMAN, sal_True, sal_True },
            {  34,   4, FAMILY_ROMAN, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aLC866SF[] = {
            {   1, 213, FAMILY_ROMAN, sal_True, sal_True },
            {   2,   3, FAMILY_ROMAN, sal_True, sal_True },
            {  33, 112, FAMILY_ROMAN, sal_True, sal_True },
            {  34,   4, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 266, FAMILY_SWISS, sal_True, sal_False },
            {  97, 267, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aLT20[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   2, 320, FAMILY_ROMAN, sal_True, sal_True },
            {  10,  93, FAMILY_ROMAN, sal_True, sal_True },
            {  16,   6, FAMILY_ROMAN, sal_True, sal_True },
            {  18,   7, FAMILY_ROMAN, sal_True, sal_True },
            {  26,  13, FAMILY_ROMAN, sal_True, sal_True },
            {  33,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  36, 321, FAMILY_ROMAN, sal_True, sal_True },
            {  38, 150, FAMILY_ROMAN, sal_True, sal_True },
            {  40,  61, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 322, FAMILY_SWISS, sal_True, sal_False },
            {  69, 323, FAMILY_SWISS, sal_True, sal_False },
            {  77,  36, FAMILY_SWISS, sal_True, sal_False },
            {  97,   2, FAMILY_ROMAN, sal_True, sal_False },
            {  98, 324, FAMILY_ROMAN, sal_True, sal_False },
            {  99, 155, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aMA_MT81[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   2, 115, FAMILY_ROMAN, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aMA_MT91[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   2, 325, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 269, FAMILY_SWISS, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aMP1300[] = {
            {   1,   5, FAMILY_ROMAN, sal_True, sal_True },
            {   3, 271, FAMILY_ROMAN, sal_True, sal_True },
            {  33,  11, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 115, FAMILY_ROMAN, sal_True, sal_True },
            {  35, 326, FAMILY_ROMAN, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aMT222[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  34,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  36, 327, FAMILY_ROMAN, sal_True, sal_True },
            {  38, 328, FAMILY_ROMAN, sal_True, sal_True },
            {  40, 329, FAMILY_ROMAN, sal_True, sal_True },
            {  44, 330, FAMILY_ROMAN, sal_True, sal_True },
            {  46, 331, FAMILY_ROMAN, sal_True, sal_True },
            {  97,   2, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aMT910[] = {
            {   1,   3, FAMILY_ROMAN, sal_True, sal_True },
            {  33, 112, FAMILY_ROMAN, sal_True, sal_True },
            {  43,  82, FAMILY_ROMAN, sal_True, sal_True },
            {  44, 113, FAMILY_ROMAN, sal_True, sal_True },
            {  45, 113, FAMILY_ROMAN, sal_True, sal_True },
            {  46, 113, FAMILY_ROMAN, sal_True, sal_True },
            {  53,  83, FAMILY_ROMAN, sal_True, sal_True },
            {  54,  83, FAMILY_ROMAN, sal_True, sal_True },
            {  55,  83, FAMILY_ROMAN, sal_True, sal_True },
            {  65,  85, FAMILY_SWISS, sal_True, sal_False },
            {  66,  85, FAMILY_SWISS, sal_True, sal_False },
            {  67,  85, FAMILY_SWISS, sal_True, sal_False },
            {  97,  87, FAMILY_ROMAN, sal_True, sal_False },
            {  98,  87, FAMILY_ROMAN, sal_True, sal_False },
            {  99,  87, FAMILY_ROMAN, sal_True, sal_False },
            { 100,  87, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aNECP2030[] = {
            {   1, 332, FAMILY_ROMAN, sal_True, sal_True },
            {   2, 333, FAMILY_ROMAN, sal_True, sal_True },
            {  33,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  35, 334, FAMILY_ROMAN, sal_True, sal_True },
            {  68, 335, FAMILY_SWISS, sal_True, sal_False },
            {  70, 336, FAMILY_SWISS, sal_True, sal_False },
            { 103, 337, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aNECP2200[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  16,   6, FAMILY_ROMAN, sal_True, sal_True },
            {  34,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  52, 338, FAMILY_ROMAN, sal_True, sal_True },
            {  53, 339, FAMILY_ROMAN, sal_True, sal_True },
            {  97, 340, FAMILY_ROMAN, sal_True, sal_False },
            { 128, 341, FAMILY_ROMAN, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aNECP23[] = {
            {   1,   5, FAMILY_ROMAN, sal_True, sal_True },
            {   2, 342, FAMILY_ROMAN, sal_True, sal_True },
            {   7, 343, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 344, FAMILY_ROMAN, sal_True, sal_True },
            {  43, 345, FAMILY_ROMAN, sal_True, sal_True },
            {  65,  51, FAMILY_SWISS, sal_True, sal_False },
            {  66, 346, FAMILY_SWISS, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aNECP2P[] = {
            {   1, 347, FAMILY_ROMAN, sal_True, sal_True },
            {   3, 348, FAMILY_ROMAN, sal_True, sal_True },
            {   5, 349, FAMILY_ROMAN, sal_True, sal_True },
            {  33,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  35, 350, FAMILY_ROMAN, sal_True, sal_True },
            {  51, 351, FAMILY_ROMAN, sal_True, sal_True },
            {  68, 335, FAMILY_SWISS, sal_True, sal_False },
            {  70, 336, FAMILY_SWISS, sal_True, sal_False },
            { 103, 337, FAMILY_ROMAN, sal_True, sal_False },
            { 133, 352, FAMILY_DECORATIVE, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aNECP567[] = {
            {   1, 353, FAMILY_ROMAN, sal_True, sal_True },
            {   7, 354, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 104, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 355, FAMILY_SWISS, sal_True, sal_False },
            { 128, 356, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aNECP6070[] = {
            {   1, 332, FAMILY_ROMAN, sal_True, sal_True },
            {   2, 333, FAMILY_ROMAN, sal_True, sal_True },
            {  16, 357, FAMILY_ROMAN, sal_True, sal_True },
            {  18, 358, FAMILY_ROMAN, sal_True, sal_True },
            {  20, 359, FAMILY_ROMAN, sal_True, sal_True },
            {  22, 360, FAMILY_ROMAN, sal_True, sal_True },
            {  24, 361, FAMILY_ROMAN, sal_True, sal_True },
            {  28, 362, FAMILY_ROMAN, sal_True, sal_True },
            {  32, 363, FAMILY_ROMAN, sal_True, sal_True },
            {  33,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  35, 334, FAMILY_ROMAN, sal_True, sal_True },
            {  37, 364, FAMILY_ROMAN, sal_True, sal_True },
            {  39, 365, FAMILY_ROMAN, sal_True, sal_True },
            {  40, 366, FAMILY_ROMAN, sal_True, sal_True },
            {  43, 367, FAMILY_ROMAN, sal_True, sal_True },
            {  51, 368, FAMILY_ROMAN, sal_True, sal_True },
            {  62, 369, FAMILY_ROMAN, sal_True, sal_True },
            {  64, 370, FAMILY_ROMAN, sal_True, sal_True },
            {  68, 335, FAMILY_SWISS, sal_True, sal_False },
            {  69, 371, FAMILY_SWISS, sal_True, sal_False },
            {  70, 372, FAMILY_SWISS, sal_True, sal_False },
            {  71, 336, FAMILY_SWISS, sal_True, sal_False },
            {  72, 373, FAMILY_SWISS, sal_True, sal_False },
            { 103, 337, FAMILY_ROMAN, sal_True, sal_False },
            { 104, 374, FAMILY_ROMAN, sal_True, sal_False },
            { 105, 375, FAMILY_ROMAN, sal_True, sal_False },
            { 108, 376, FAMILY_ROMAN, sal_True, sal_False },
            { 109, 377, FAMILY_ROMAN, sal_True, sal_False },
            { 110, 378, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aNECP6272[] = {
            {   1, 332, FAMILY_ROMAN, sal_True, sal_True },
            {   2, 333, FAMILY_ROMAN, sal_True, sal_True },
            {  16, 357, FAMILY_ROMAN, sal_True, sal_True },
            {  18, 358, FAMILY_ROMAN, sal_True, sal_True },
            {  20, 359, FAMILY_ROMAN, sal_True, sal_True },
            {  22, 360, FAMILY_ROMAN, sal_True, sal_True },
            {  24, 361, FAMILY_ROMAN, sal_True, sal_True },
            {  28, 362, FAMILY_ROMAN, sal_True, sal_True },
            {  32, 363, FAMILY_ROMAN, sal_True, sal_True },
            {  33,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  35, 334, FAMILY_ROMAN, sal_True, sal_True },
            {  37, 364, FAMILY_ROMAN, sal_True, sal_True },
            {  39, 365, FAMILY_ROMAN, sal_True, sal_True },
            {  40, 366, FAMILY_ROMAN, sal_True, sal_True },
            {  43, 367, FAMILY_ROMAN, sal_True, sal_True },
            {  51, 368, FAMILY_ROMAN, sal_True, sal_True },
            {  62, 369, FAMILY_ROMAN, sal_True, sal_True },
            {  64, 370, FAMILY_ROMAN, sal_True, sal_True },
            {  68, 379, FAMILY_SWISS, sal_True, sal_False },
            {  69, 371, FAMILY_SWISS, sal_True, sal_False },
            {  70, 372, FAMILY_SWISS, sal_True, sal_False },
            {  71, 336, FAMILY_SWISS, sal_True, sal_False },
            {  72, 373, FAMILY_SWISS, sal_True, sal_False },
            {  96, 380, FAMILY_SWISS, sal_True, sal_False },
            { 103, 337, FAMILY_ROMAN, sal_True, sal_False },
            { 104, 374, FAMILY_ROMAN, sal_True, sal_False },
            { 105, 375, FAMILY_ROMAN, sal_True, sal_False },
            { 108, 376, FAMILY_ROMAN, sal_True, sal_False },
            { 109, 377, FAMILY_ROMAN, sal_True, sal_False },
            { 110, 378, FAMILY_ROMAN, sal_True, sal_False },
            { 111, 381, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aNECPLUS[] = {
            {   1, 347, FAMILY_ROMAN, sal_True, sal_True },
            {  16, 357, FAMILY_ROMAN, sal_True, sal_True },
            {  18, 358, FAMILY_ROMAN, sal_True, sal_True },
            {  20, 359, FAMILY_ROMAN, sal_True, sal_True },
            {  22, 360, FAMILY_ROMAN, sal_True, sal_True },
            {  28, 362, FAMILY_ROMAN, sal_True, sal_True },
            {  32, 363, FAMILY_ROMAN, sal_True, sal_True },
            {  33,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  35, 350, FAMILY_ROMAN, sal_True, sal_True },
            {  37, 364, FAMILY_ROMAN, sal_True, sal_True },
            {  39, 365, FAMILY_ROMAN, sal_True, sal_True },
            {  40, 366, FAMILY_ROMAN, sal_True, sal_True },
            {  43, 367, FAMILY_ROMAN, sal_True, sal_True },
            {  51, 351, FAMILY_ROMAN, sal_True, sal_True },
            {  62, 369, FAMILY_ROMAN, sal_True, sal_True },
            {  64, 370, FAMILY_ROMAN, sal_True, sal_True },
            {  68, 382, FAMILY_SWISS, sal_True, sal_False },
            {  69, 371, FAMILY_SWISS, sal_True, sal_False },
            {  70, 372, FAMILY_SWISS, sal_True, sal_False },
            {  72, 373, FAMILY_SWISS, sal_True, sal_False },
            { 103, 383, FAMILY_ROMAN, sal_True, sal_False },
            { 104, 374, FAMILY_ROMAN, sal_True, sal_False },
            { 105, 375, FAMILY_ROMAN, sal_True, sal_False },
            { 108, 376, FAMILY_ROMAN, sal_True, sal_False },
            { 109, 377, FAMILY_ROMAN, sal_True, sal_False },
            { 110, 378, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aNECS60[] = {
            {   2, 275, FAMILY_ROMAN, sal_True, sal_True },
            {  34,   1, FAMILY_ROMAN, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aOKI192[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 115, FAMILY_ROMAN, sal_True, sal_True },
            {  97, 107, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aOKI192OR[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   2, 384, FAMILY_ROMAN, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aOKI193[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 115, FAMILY_ROMAN, sal_True, sal_True },
            {  97, 107, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aOKI292F[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 115, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 139, FAMILY_SWISS, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aOKI293F[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 115, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 139, FAMILY_SWISS, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aOKI320[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   2, 385, FAMILY_ROMAN, sal_True, sal_True },
            {   3, 115, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 139, FAMILY_SWISS, sal_True, sal_False },
            {  66, 107, FAMILY_SWISS, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aOKI380E[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   8, 386, FAMILY_ROMAN, sal_True, sal_True },
            {  14,  93, FAMILY_ROMAN, sal_True, sal_True },
            {  33,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  38, 387, FAMILY_ROMAN, sal_True, sal_True },
            {  40, 388, FAMILY_ROMAN, sal_True, sal_True },
            {  44, 389, FAMILY_ROMAN, sal_True, sal_True },
            {  68, 390, FAMILY_SWISS, sal_True, sal_False },
            {  71, 323, FAMILY_SWISS, sal_True, sal_False },
            {  97,   2, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aOKI390E[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 104, FAMILY_ROMAN, sal_True, sal_True },
            {  55, 391, FAMILY_ROMAN, sal_True, sal_True },
            {  97, 392, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aOKI390EL[] = {
            {   1, 393, FAMILY_ROMAN, sal_True, sal_True },
            {   2, 394, FAMILY_ROMAN, sal_True, sal_True },
            {   3, 395, FAMILY_ROMAN, sal_True, sal_True },
            {   4, 396, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 104, FAMILY_ROMAN, sal_True, sal_True },
            {  35, 397, FAMILY_ROMAN, sal_True, sal_True },
            {  36, 398, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 382, FAMILY_SWISS, sal_True, sal_False },
            {  66,  17, FAMILY_SWISS, sal_True, sal_False },
            {  67, 399, FAMILY_SWISS, sal_True, sal_False },
            {  97, 101, FAMILY_ROMAN, sal_True, sal_False },
            {  98,  20, FAMILY_ROMAN, sal_True, sal_False },
            {  99,  29, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aOKI393EL[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   5, 400, FAMILY_ROMAN, sal_True, sal_True },
            {   8, 401, FAMILY_ROMAN, sal_True, sal_True },
            {  16, 402, FAMILY_ROMAN, sal_True, sal_True },
            {  18, 403, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 288, FAMILY_ROMAN, sal_True, sal_True },
            {  36, 404, FAMILY_ROMAN, sal_True, sal_True },
            {  38, 405, FAMILY_ROMAN, sal_True, sal_True },
            {  40, 406, FAMILY_ROMAN, sal_True, sal_True },
            {  68, 407, FAMILY_SWISS, sal_True, sal_False },
            {  97, 408, FAMILY_ROMAN, sal_True, sal_False },
            {  98, 409, FAMILY_ROMAN, sal_True, sal_False },
            {  99, 410, FAMILY_ROMAN, sal_True, sal_False },
            { 100, 411, FAMILY_ROMAN, sal_True, sal_False },
            { 128, 412, FAMILY_ROMAN, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aOKI393F[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   5, 413, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 104, FAMILY_ROMAN, sal_True, sal_True },
            {  35, 414, FAMILY_ROMAN, sal_True, sal_True },
            {  97, 392, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aOKI400[] = {
            {   1, 213, FAMILY_ROMAN, sal_True, sal_True },
            {  33, 112, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 273, FAMILY_SWISS, sal_True, sal_False },
            {  66, 415, FAMILY_SWISS, sal_True, sal_False },
            {  97,  87, FAMILY_ROMAN, sal_True, sal_False },
            {  98, 416, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aOKI800[] = {
            {   1, 213, FAMILY_ROMAN, sal_True, sal_True },
            {  33, 112, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 273, FAMILY_SWISS, sal_True, sal_False },
            {  97,  87, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aOKILL6[] = {
            {   1, 417, FAMILY_ROMAN, sal_True, sal_True },
            {  33, 112, FAMILY_ROMAN, sal_True, sal_True },
            {  43,  23, FAMILY_ROMAN, sal_True, sal_True },
            {  53,  12, FAMILY_ROMAN, sal_True, sal_True },
            {  54,  12, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 418, FAMILY_SWISS, sal_True, sal_False },
            {  97,  87, FAMILY_ROMAN, sal_True, sal_False },
            {  98, 419, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aOKILL6R8[] = {
            {   1, 420, FAMILY_ROMAN, sal_True, sal_True },
            {  33,   4, FAMILY_ROMAN, sal_True, sal_True },
            {  43,  23, FAMILY_ROMAN, sal_True, sal_True },
            {  53,  12, FAMILY_ROMAN, sal_True, sal_True },
            {  54,  12, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 418, FAMILY_SWISS, sal_True, sal_False },
            {  97,  87, FAMILY_ROMAN, sal_True, sal_False },
            {  98, 419, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aOLI324L[] = {
            {   1,   5, FAMILY_ROMAN, sal_True, sal_True },
            {   2, 421, FAMILY_ROMAN, sal_True, sal_True },
            {  33,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  37, 150, FAMILY_ROMAN, sal_True, sal_True },
            {  97, 422, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aOLIDM109[] = {
            {   1,   5, FAMILY_ROMAN, sal_True, sal_True },
            {   2,  96, FAMILY_ROMAN, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aOLIJP350[] = {
            {  33, 227, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 228, FAMILY_ROMAN, sal_True, sal_True },
            {  45, 423, FAMILY_ROMAN, sal_True, sal_True },
            {  46, 424, FAMILY_ROMAN, sal_True, sal_True },
            {  47, 425, FAMILY_ROMAN, sal_True, sal_True },
            {  48, 426, FAMILY_ROMAN, sal_True, sal_True },
            { 102, 427, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aOLIPG306[] = {
            {   2, 275, FAMILY_ROMAN, sal_True, sal_True },
            {  34,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  36, 428, FAMILY_ROMAN, sal_True, sal_True },
            {  38, 429, FAMILY_ROMAN, sal_True, sal_True },
            {  40, 430, FAMILY_ROMAN, sal_True, sal_True },
            {  44, 431, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 432, FAMILY_SWISS, sal_True, sal_False },
            {  66, 433, FAMILY_SWISS, sal_True, sal_False },
            {  67, 434, FAMILY_SWISS, sal_True, sal_False },
            {  68, 435, FAMILY_DONTKNOW, sal_False, sal_False },
            { 102, 436, FAMILY_ROMAN, sal_True, sal_False },
            { 103, 437, FAMILY_ROMAN, sal_True, sal_False },
            { 104, 438, FAMILY_ROMAN, sal_True, sal_False },
            { 105, 439, FAMILY_DONTKNOW, sal_False, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aPAINT[] = {
            {  33,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  40,  61, FAMILY_ROMAN, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aPAN4450I[] = {
            {   1, 213, FAMILY_ROMAN, sal_True, sal_True },
            {  33, 112, FAMILY_ROMAN, sal_True, sal_True },
            {  97, 440, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aPANA1080[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   2, 106, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 126, FAMILY_SWISS, sal_True, sal_False },
            {  67, 340, FAMILY_SWISS, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aPANA1081[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  33, 115, FAMILY_ROMAN, sal_True, sal_True },
            {  97, 441, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aPANA1090[] = {
            {   1,   5, FAMILY_ROMAN, sal_True, sal_True },
            {  33,  11, FAMILY_ROMAN, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aPANA1091[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  33,  11, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 115, FAMILY_ROMAN, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aPANA1092[] = {
            {   1,   5, FAMILY_ROMAN, sal_True, sal_True },
            {  33,  11, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 442, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 443, FAMILY_SWISS, sal_True, sal_False },
            {  67, 444, FAMILY_SWISS, sal_True, sal_False },
            { 132, 444, FAMILY_DECORATIVE, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aPANA1124[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   4,  30, FAMILY_ROMAN, sal_True, sal_True },
            {  14,  93, FAMILY_ROMAN, sal_True, sal_True },
            {  34,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  36,  32, FAMILY_ROMAN, sal_True, sal_True },
            {  38,  12, FAMILY_ROMAN, sal_True, sal_True },
            {  40,  13, FAMILY_ROMAN, sal_True, sal_True },
            {  66,  35, FAMILY_SWISS, sal_True, sal_False },
            {  70, 336, FAMILY_SWISS, sal_True, sal_False },
            {  71, 323, FAMILY_SWISS, sal_True, sal_False },
            {  77,  36, FAMILY_SWISS, sal_True, sal_False },
            {  98,  38, FAMILY_ROMAN, sal_True, sal_False },
            {  99,  25, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aPANA1180[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   4,  30, FAMILY_ROMAN, sal_True, sal_True },
            {  33,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  37,  12, FAMILY_ROMAN, sal_True, sal_True },
            {  66,  35, FAMILY_SWISS, sal_True, sal_False },
            {  70, 336, FAMILY_SWISS, sal_True, sal_False },
            {  99,  25, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aPANA1540[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   2, 445, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 288, FAMILY_ROMAN, sal_True, sal_True },
            {  97,   2, FAMILY_ROMAN, sal_True, sal_False },
            { 129, 446, FAMILY_DECORATIVE, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aPANA1592[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 115, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 126, FAMILY_SWISS, sal_True, sal_False },
            {  97, 441, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aPANA1595[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  11, 447, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 115, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 126, FAMILY_SWISS, sal_True, sal_False },
            {  70, 448, FAMILY_SWISS, sal_True, sal_False },
            {  97, 441, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aPR241[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  15,   6, FAMILY_ROMAN, sal_True, sal_True },
            {  17, 449, FAMILY_ROMAN, sal_True, sal_True },
            {  33,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  37, 105, FAMILY_ROMAN, sal_True, sal_True },
            {  41,  14, FAMILY_ROMAN, sal_True, sal_True },
            {  57, 450, FAMILY_ROMAN, sal_True, sal_True },
            {  97, 101, FAMILY_ROMAN, sal_True, sal_False },
            { 101, 451, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aPR5200[] = {
            {   1,  95, FAMILY_ROMAN, sal_True, sal_True },
            {  70, 452, FAMILY_SWISS, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aPRODOT24[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   2, 453, FAMILY_ROMAN, sal_True, sal_True },
            {   5, 454, FAMILY_ROMAN, sal_True, sal_True },
            {   7,  85, FAMILY_ROMAN, sal_True, sal_True },
            {  33,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  35, 455, FAMILY_ROMAN, sal_True, sal_True },
            {  43, 110, FAMILY_ROMAN, sal_True, sal_True },
            {  68, 379, FAMILY_SWISS, sal_True, sal_False },
            {  97,   2, FAMILY_ROMAN, sal_True, sal_False },
            {  98, 456, FAMILY_ROMAN, sal_True, sal_False },
            { 102, 111, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aPRODOT9[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   2, 453, FAMILY_ROMAN, sal_True, sal_True },
            {   7, 457, FAMILY_ROMAN, sal_True, sal_True },
            {  33, 458, FAMILY_ROMAN, sal_True, sal_True },
            {  43, 459, FAMILY_ROMAN, sal_True, sal_True },
            {  68, 460, FAMILY_SWISS, sal_True, sal_False },
            {  97, 461, FAMILY_ROMAN, sal_True, sal_False },
            { 102, 462, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aPS[] = {
            {  33,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  65,  85, FAMILY_SWISS, sal_True, sal_True },
            {  69, 463, FAMILY_SWISS, sal_True, sal_True },
            {  74, 464, FAMILY_SWISS, sal_True, sal_True },
            {  97, 465, FAMILY_ROMAN, sal_True, sal_True },
            { 103, 466, FAMILY_ROMAN, sal_True, sal_True },
            { 105, 467, FAMILY_ROMAN, sal_True, sal_True },
            { 109, 468, FAMILY_ROMAN, sal_True, sal_True },
            { 130, 469, FAMILY_DECORATIVE, sal_True, sal_True },
            { 135,  78, FAMILY_DONTKNOW, sal_False, sal_True },
            { 140, 212, FAMILY_DONTKNOW, sal_False, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aPT90[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   2, 115, FAMILY_ROMAN, sal_True, sal_True },
            {   3, 470, FAMILY_ROMAN, sal_True, sal_True },
            {   4, 471, FAMILY_ROMAN, sal_True, sal_True },
            {  33, 472, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 473, FAMILY_ROMAN, sal_True, sal_True },
            {  61, 474, FAMILY_ROMAN, sal_True, sal_True },
            {  62, 475, FAMILY_ROMAN, sal_True, sal_True },
            {  76, 476, FAMILY_SWISS, sal_True, sal_False },
            {  77, 477, FAMILY_SWISS, sal_True, sal_False },
            {  78, 478, FAMILY_SWISS, sal_True, sal_False },
            {  79, 479, FAMILY_SWISS, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aRASTER[] = {
            {  34,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 177, FAMILY_SWISS, sal_True, sal_True },
            {  66, 178, FAMILY_SWISS, sal_True, sal_True },
            {  67, 179, FAMILY_SWISS, sal_True, sal_True },
            {  68, 180, FAMILY_SWISS, sal_True, sal_True },
            {  69, 181, FAMILY_SWISS, sal_True, sal_True },
            {  70, 182, FAMILY_SWISS, sal_True, sal_True },
            {  71, 183, FAMILY_SWISS, sal_True, sal_True },
            {  72, 184, FAMILY_SWISS, sal_True, sal_True },
            {  73, 185, FAMILY_SWISS, sal_True, sal_True },
            {  74, 186, FAMILY_SWISS, sal_True, sal_True },
            {  75, 187, FAMILY_SWISS, sal_True, sal_True },
            {  76, 188, FAMILY_SWISS, sal_True, sal_True },
            {  77, 189, FAMILY_SWISS, sal_True, sal_True },
            {  78, 190, FAMILY_SWISS, sal_True, sal_True },
            {  79, 150, FAMILY_SWISS, sal_True, sal_True },
            {  80, 191, FAMILY_SWISS, sal_True, sal_True },
            {  81, 192, FAMILY_SWISS, sal_True, sal_True },
            {  82, 193, FAMILY_SWISS, sal_True, sal_True },
            {  83, 194, FAMILY_SWISS, sal_True, sal_True },
            {  84, 195, FAMILY_SWISS, sal_True, sal_True },
            {  85, 196, FAMILY_SWISS, sal_True, sal_True },
            {  86, 197, FAMILY_SWISS, sal_True, sal_True },
            {  87, 198, FAMILY_SWISS, sal_True, sal_True },
            {  88, 199, FAMILY_SWISS, sal_True, sal_True },
            {  89, 200, FAMILY_SWISS, sal_True, sal_True },
            {  90, 201, FAMILY_SWISS, sal_True, sal_True },
            {  91, 202, FAMILY_SWISS, sal_True, sal_True },
            {  92, 203, FAMILY_SWISS, sal_True, sal_True },
            {  97, 204, FAMILY_ROMAN, sal_True, sal_True },
            {  98, 205, FAMILY_ROMAN, sal_True, sal_True },
            { 103, 206, FAMILY_ROMAN, sal_True, sal_True },
            { 105, 207, FAMILY_ROMAN, sal_True, sal_True },
            { 109, 208, FAMILY_ROMAN, sal_True, sal_True },
            { 130, 209, FAMILY_DECORATIVE, sal_True, sal_True },
            { 135,  78, FAMILY_DONTKNOW, sal_False, sal_True },
            { 136,  78, FAMILY_DONTKNOW, sal_False, sal_True },
            { 137, 210, FAMILY_DONTKNOW, sal_False, sal_True },
            { 138, 211, FAMILY_DONTKNOW, sal_False, sal_True },
            { 140, 212, FAMILY_DONTKNOW, sal_False, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aSD15[] = {
            {   1,  95, FAMILY_ROMAN, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aSD24[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 115, FAMILY_ROMAN, sal_True, sal_True },
            {  97, 107, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aSEIKO[] = {
            {   1, 213, FAMILY_ROMAN, sal_True, sal_True },
            {  33, 112, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 480, FAMILY_SWISS, sal_True, sal_False },
            {  97, 321, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aSIHP4100[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 104, FAMILY_ROMAN, sal_True, sal_True },
            {  55, 391, FAMILY_ROMAN, sal_True, sal_True },
            {  97, 392, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aSIHP4200[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 104, FAMILY_ROMAN, sal_True, sal_True },
            {  55, 391, FAMILY_ROMAN, sal_True, sal_True },
            {  97, 392, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aSIHP4400[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   5, 413, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 104, FAMILY_ROMAN, sal_True, sal_True },
            {  35, 414, FAMILY_ROMAN, sal_True, sal_True },
            {  97, 392, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aSIHP440F[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   5, 413, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 104, FAMILY_ROMAN, sal_True, sal_True },
            {  35, 414, FAMILY_ROMAN, sal_True, sal_True },
            {  97, 392, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aSIPT18[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 115, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 139, FAMILY_SWISS, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aSIPT18F[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 115, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 139, FAMILY_SWISS, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aSIPT19[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 115, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 139, FAMILY_SWISS, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aSL230AI[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   2,   1, FAMILY_ROMAN, sal_True, sal_True },
            {   4, 481, FAMILY_ROMAN, sal_True, sal_True },
            {  14,  93, FAMILY_ROMAN, sal_True, sal_True },
            {  16,   6, FAMILY_ROMAN, sal_True, sal_True },
            {  18,   7, FAMILY_ROMAN, sal_True, sal_True },
            {  35, 321, FAMILY_ROMAN, sal_True, sal_True },
            {  38,  12, FAMILY_ROMAN, sal_True, sal_True },
            {  40,  13, FAMILY_ROMAN, sal_True, sal_True },
            {  46,  23, FAMILY_ROMAN, sal_True, sal_True },
            {  66, 482, FAMILY_SWISS, sal_True, sal_False },
            {  71, 323, FAMILY_SWISS, sal_True, sal_False },
            {  97,   2, FAMILY_ROMAN, sal_True, sal_False },
            {  98,  38, FAMILY_ROMAN, sal_True, sal_False },
            {  99,  25, FAMILY_ROMAN, sal_True, sal_False },
            { 100,  36, FAMILY_ROMAN, sal_True, sal_False },
            { 103,  24, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aSL80AI[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  35, 398, FAMILY_ROMAN, sal_True, sal_True },
            {  98,  20, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aSL80IP[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  35, 398, FAMILY_ROMAN, sal_True, sal_True },
            {  98,  20, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aSL92[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   8, 481, FAMILY_ROMAN, sal_True, sal_True },
            {  14,  93, FAMILY_ROMAN, sal_True, sal_True },
            {  16,   6, FAMILY_ROMAN, sal_True, sal_True },
            {  18,   7, FAMILY_ROMAN, sal_True, sal_True },
            {  26,  13, FAMILY_ROMAN, sal_True, sal_True },
            {  33,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  36, 321, FAMILY_ROMAN, sal_True, sal_True },
            {  38,  12, FAMILY_ROMAN, sal_True, sal_True },
            {  46, 483, FAMILY_ROMAN, sal_True, sal_True },
            {  68, 482, FAMILY_SWISS, sal_True, sal_False },
            {  71, 323, FAMILY_SWISS, sal_True, sal_False },
            {  77,  36, FAMILY_SWISS, sal_True, sal_False },
            {  97,   2, FAMILY_ROMAN, sal_True, sal_False },
            {  98, 324, FAMILY_ROMAN, sal_True, sal_False },
            {  99,  25, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aSP1600AI[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  33, 115, FAMILY_ROMAN, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aSP264[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  34,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  36, 327, FAMILY_ROMAN, sal_True, sal_True },
            {  38, 328, FAMILY_ROMAN, sal_True, sal_True },
            {  40, 329, FAMILY_ROMAN, sal_True, sal_True },
            {  44, 330, FAMILY_ROMAN, sal_True, sal_True },
            {  46, 331, FAMILY_ROMAN, sal_True, sal_True },
            {  97,   2, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aSTARFR10[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   3,  30, FAMILY_ROMAN, sal_True, sal_True },
            {  14,  93, FAMILY_ROMAN, sal_True, sal_True },
            {  16,   6, FAMILY_ROMAN, sal_True, sal_True },
            {  26,  13, FAMILY_ROMAN, sal_True, sal_True },
            {  35,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  40,  61, FAMILY_ROMAN, sal_True, sal_True },
            {  48, 484, FAMILY_ROMAN, sal_True, sal_True },
            {  62, 308, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 126, FAMILY_SWISS, sal_True, sal_False },
            {  66, 485, FAMILY_SWISS, sal_True, sal_False },
            {  71, 323, FAMILY_SWISS, sal_True, sal_False },
            {  97,   2, FAMILY_ROMAN, sal_True, sal_False },
            { 100, 103, FAMILY_ROMAN, sal_True, sal_False },
            { 104, 486, FAMILY_ROMAN, sal_True, sal_False },
            { 111, 487, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aSTARLC[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   3, 291, FAMILY_ROMAN, sal_True, sal_True },
            {  13, 488, FAMILY_ROMAN, sal_True, sal_True },
            {  14, 489, FAMILY_ROMAN, sal_True, sal_True },
            {  35, 104, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 139, FAMILY_SWISS, sal_True, sal_False },
            {  66, 292, FAMILY_SWISS, sal_True, sal_False },
            {  97, 293, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aSTARLC24[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {  14, 490, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 104, FAMILY_ROMAN, sal_True, sal_True },
            {  38, 397, FAMILY_ROMAN, sal_True, sal_True },
            {  40, 491, FAMILY_ROMAN, sal_True, sal_True },
            {  64, 492, FAMILY_ROMAN, sal_True, sal_True },
            {  71, 493, FAMILY_SWISS, sal_True, sal_False },
            {  97, 101, FAMILY_ROMAN, sal_True, sal_False },
            {  99,  29, FAMILY_ROMAN, sal_True, sal_False },
            { 100,  21, FAMILY_ROMAN, sal_True, sal_False },
            { 112, 494, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aSTARLP8[] = {
            {   1, 213, FAMILY_ROMAN, sal_True, sal_True },
            {   2,   3, FAMILY_ROMAN, sal_True, sal_True },
            {  33, 112, FAMILY_ROMAN, sal_True, sal_True },
            {  34,   4, FAMILY_ROMAN, sal_True, sal_True },
            {  35, 495, FAMILY_ROMAN, sal_True, sal_True },
            {  36,  82, FAMILY_ROMAN, sal_True, sal_True },
            {  37,  12, FAMILY_ROMAN, sal_True, sal_True },
            {  39,   4, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 496, FAMILY_SWISS, sal_True, sal_False },
            {  97, 274, FAMILY_ROMAN, sal_True, sal_False },
            {  98, 497, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aSTARNB[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   2, 115, FAMILY_ROMAN, sal_True, sal_True },
            {  37,  12, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 441, FAMILY_SWISS, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aSTARNL_I[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   2, 115, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 498, FAMILY_SWISS, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aSTARSJ48[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   2, 499, FAMILY_ROMAN, sal_True, sal_True },
            {  44, 500, FAMILY_ROMAN, sal_True, sal_True },
            {  50, 501, FAMILY_ROMAN, sal_True, sal_True },
            { 102, 502, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aSTARXB24[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   2, 499, FAMILY_ROMAN, sal_True, sal_True },
            {   8, 296, FAMILY_ROMAN, sal_True, sal_True },
            {  14, 503, FAMILY_ROMAN, sal_True, sal_True },
            {  16, 300, FAMILY_ROMAN, sal_True, sal_True },
            {  18, 301, FAMILY_ROMAN, sal_True, sal_True },
            {  26, 302, FAMILY_ROMAN, sal_True, sal_True },
            {  32, 504, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 303, FAMILY_ROMAN, sal_True, sal_True },
            {  35, 505, FAMILY_ROMAN, sal_True, sal_True },
            {  38, 304, FAMILY_ROMAN, sal_True, sal_True },
            {  40, 305, FAMILY_ROMAN, sal_True, sal_True },
            {  44, 306, FAMILY_ROMAN, sal_True, sal_True },
            {  46, 506, FAMILY_ROMAN, sal_True, sal_True },
            {  48, 507, FAMILY_ROMAN, sal_True, sal_True },
            {  50, 508, FAMILY_ROMAN, sal_True, sal_True },
            {  58, 509, FAMILY_ROMAN, sal_True, sal_True },
            {  60, 510, FAMILY_ROMAN, sal_True, sal_True },
            {  62, 511, FAMILY_ROMAN, sal_True, sal_True },
            {  64, 309, FAMILY_ROMAN, sal_True, sal_True },
            {  68, 512, FAMILY_SWISS, sal_True, sal_False },
            {  71, 513, FAMILY_SWISS, sal_True, sal_False },
            {  80, 514, FAMILY_SWISS, sal_True, sal_False },
            {  97, 515, FAMILY_ROMAN, sal_True, sal_False },
            {  99, 516, FAMILY_ROMAN, sal_True, sal_False },
            { 100, 517, FAMILY_ROMAN, sal_True, sal_False },
            { 102, 518, FAMILY_ROMAN, sal_True, sal_False },
            { 103, 519, FAMILY_ROMAN, sal_True, sal_False },
            { 104, 520, FAMILY_ROMAN, sal_True, sal_False },
            { 105, 521, FAMILY_ROMAN, sal_True, sal_False },
            { 111, 522, FAMILY_ROMAN, sal_True, sal_False },
            { 112, 523, FAMILY_ROMAN, sal_True, sal_False },
            { 129, 524, FAMILY_DECORATIVE, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aSTLC2420[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   2, 499, FAMILY_ROMAN, sal_True, sal_True },
            {  14, 503, FAMILY_ROMAN, sal_True, sal_True },
            {  16, 300, FAMILY_ROMAN, sal_True, sal_True },
            {  18, 301, FAMILY_ROMAN, sal_True, sal_True },
            {  26, 302, FAMILY_ROMAN, sal_True, sal_True },
            {  27, 525, FAMILY_ROMAN, sal_True, sal_True },
            {  32, 526, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 303, FAMILY_ROMAN, sal_True, sal_True },
            {  38, 304, FAMILY_ROMAN, sal_True, sal_True },
            {  40, 305, FAMILY_ROMAN, sal_True, sal_True },
            {  44, 500, FAMILY_ROMAN, sal_True, sal_True },
            {  46, 527, FAMILY_ROMAN, sal_True, sal_True },
            {  48, 507, FAMILY_ROMAN, sal_True, sal_True },
            {  49, 508, FAMILY_ROMAN, sal_True, sal_True },
            {  50, 528, FAMILY_ROMAN, sal_True, sal_True },
            {  60, 529, FAMILY_ROMAN, sal_True, sal_True },
            {  62, 511, FAMILY_ROMAN, sal_True, sal_True },
            {  64, 309, FAMILY_ROMAN, sal_True, sal_True },
            {  68, 530, FAMILY_SWISS, sal_True, sal_False },
            {  71, 513, FAMILY_SWISS, sal_True, sal_False },
            {  80, 531, FAMILY_SWISS, sal_True, sal_False },
            {  97, 515, FAMILY_ROMAN, sal_True, sal_False },
            {  99, 532, FAMILY_ROMAN, sal_True, sal_False },
            { 100, 517, FAMILY_ROMAN, sal_True, sal_False },
            { 102, 502, FAMILY_ROMAN, sal_True, sal_False },
            { 103, 533, FAMILY_ROMAN, sal_True, sal_False },
            { 104, 520, FAMILY_ROMAN, sal_True, sal_False },
            { 105, 521, FAMILY_ROMAN, sal_True, sal_False },
            { 106, 534, FAMILY_ROMAN, sal_True, sal_False },
            { 111, 522, FAMILY_ROMAN, sal_True, sal_False },
            { 112, 523, FAMILY_ROMAN, sal_True, sal_False },
            { 129, 524, FAMILY_DECORATIVE, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aSTXB2420[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   4, 529, FAMILY_ROMAN, sal_True, sal_True },
            {   6, 504, FAMILY_ROMAN, sal_True, sal_True },
            {   8, 296, FAMILY_ROMAN, sal_True, sal_True },
            {  10, 297, FAMILY_ROMAN, sal_True, sal_True },
            {  12, 298, FAMILY_ROMAN, sal_True, sal_True },
            {  14, 299, FAMILY_ROMAN, sal_True, sal_True },
            {  16, 300, FAMILY_ROMAN, sal_True, sal_True },
            {  18, 301, FAMILY_ROMAN, sal_True, sal_True },
            {  26, 302, FAMILY_ROMAN, sal_True, sal_True },
            {  28, 525, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 303, FAMILY_ROMAN, sal_True, sal_True },
            {  38, 304, FAMILY_ROMAN, sal_True, sal_True },
            {  40, 305, FAMILY_ROMAN, sal_True, sal_True },
            {  44, 535, FAMILY_ROMAN, sal_True, sal_True },
            {  46, 536, FAMILY_ROMAN, sal_True, sal_True },
            {  48, 507, FAMILY_ROMAN, sal_True, sal_True },
            {  50, 508, FAMILY_ROMAN, sal_True, sal_True },
            {  62, 511, FAMILY_ROMAN, sal_True, sal_True },
            {  64, 309, FAMILY_ROMAN, sal_True, sal_True },
            {  66, 537, FAMILY_SWISS, sal_True, sal_False },
            {  68, 311, FAMILY_SWISS, sal_True, sal_False },
            {  69, 312, FAMILY_SWISS, sal_True, sal_False },
            {  71, 313, FAMILY_SWISS, sal_True, sal_False },
            {  77, 538, FAMILY_SWISS, sal_True, sal_False },
            {  97, 314, FAMILY_ROMAN, sal_True, sal_False },
            {  99, 315, FAMILY_ROMAN, sal_True, sal_False },
            { 100, 316, FAMILY_ROMAN, sal_True, sal_False },
            { 102, 539, FAMILY_ROMAN, sal_True, sal_False },
            { 104, 540, FAMILY_ROMAN, sal_True, sal_False },
            { 112, 319, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aSWIFT24E[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   2, 541, FAMILY_ROMAN, sal_True, sal_True },
            {   4,  85, FAMILY_ROMAN, sal_True, sal_True },
            {  33,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  35, 110, FAMILY_ROMAN, sal_True, sal_True },
            {  37,  12, FAMILY_ROMAN, sal_True, sal_True },
            {  66, 379, FAMILY_SWISS, sal_True, sal_False },
            {  98, 111, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aTOSH301[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   2,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  35,  83, FAMILY_ROMAN, sal_True, sal_True },
            {  97,   2, FAMILY_ROMAN, sal_True, sal_False },
            {  98, 542, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aTOSH321[] = {
            {   1,   0, FAMILY_ROMAN, sal_True, sal_True },
            {   3, 543, FAMILY_ROMAN, sal_True, sal_True },
            {  33, 105, FAMILY_ROMAN, sal_True, sal_True },
            {  43,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  97, 134, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aTOSH351[] = {
            {   1, 544, FAMILY_ROMAN, sal_True, sal_True },
            {  33,  11, FAMILY_ROMAN, sal_True, sal_True },
            {  35,   1, FAMILY_ROMAN, sal_True, sal_True },
            {  37, 105, FAMILY_ROMAN, sal_True, sal_True },
            {  39, 543, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 134, FAMILY_SWISS, sal_True, sal_False },
            { 129, 545, FAMILY_DECORATIVE, sal_True, sal_False },
            { 130, 546, FAMILY_DECORATIVE, sal_True, sal_False },
            { 131, 547, FAMILY_DECORATIVE, sal_True, sal_False },
            { 132, 548, FAMILY_DECORATIVE, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aTTY[] = {
            {   1, 270, FAMILY_ROMAN, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aTTYDEU[] = {
            {   1, 270, FAMILY_ROMAN, sal_True, sal_True },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        },
        aTYPENRAD[] = {
            {   1, 549, FAMILY_ROMAN, sal_True, sal_True },
            {   2, 550, FAMILY_ROMAN, sal_True, sal_True },
            {   3, 551, FAMILY_ROMAN, sal_True, sal_True },
            {  33, 552, FAMILY_ROMAN, sal_True, sal_True },
            {  34, 553, FAMILY_ROMAN, sal_True, sal_True },
            {  65, 554, FAMILY_SWISS, sal_True, sal_False },
            {  66, 555, FAMILY_SWISS, sal_True, sal_False },
            {  97, 556, FAMILY_ROMAN, sal_True, sal_False },
            {  98, 557, FAMILY_ROMAN, sal_True, sal_False },
            {   0,   0, FAMILY_ROMAN, sal_False, sal_False }
        };


static const struct Sw6FontMatch
{
    const sal_Char* pPrtName;
    const Sw6PrinterFont* pFonts;
} aFntMatch[] = {
        { "5000DI", a5000DI },
        { "ACERLP75", aACERLP75 },
        { "AEG200I", aAEG200I },
        { "BJ10E", aBJ10E },
        { "BJ130E", aBJ130E },
        { "BJ300", aBJ300 },
        { "BR1109", aBR1109 },
        { "BR1209", aBR1209 },
        { "BR1224L", aBR1224L },
        { "BR1324", aBR1324 },
        { "BR1409", aBR1409 },
        { "BR1509", aBR1509 },
        { "BR1709", aBR1709 },
        { "BR1724L", aBR1724L },
        { "BR1824L", aBR1824L },
        { "BR2024", aBR2024 },
        { "BR2518", aBR2518 },
        { "BR3524L", aBR3524L },
        { "BR4018", aBR4018 },
        { "BRHJ770", aBRHJ770 },
        { "BRHL4", aBRHL4 },
        { "BRHL48V", aBRHL48V },
        { "BRHL8DB", aBRHL8DB },
        { "BRHR", aBRHR },
        { "BROTHER", aBROTHER },
        { "CANONBJX", aCANONBJX },
        { "CE500", aCE500 },
        { "CE700", aCE700 },
        { "CIE_210E", aCIE_210E },
        { "CIE_310E", aCIE_310E },
        { "CIE_610E", aCIE_610E },
        { "CIE_610P", aCIE_610P },
        { "CIE_715E", aCIE_715E },
        { "CIT120D", aCIT120D },
        { "CIT120DP", aCIT120DP },
        { "CIT124D", aCIT124D },
        { "CITOH52", aCITOH52 },
        { "D-630", aD_630 },
        { "D-630E", aD_630E },
        { "D-630HZ", aD_630HZ },
        { "DICO150", aDICO150 },
        { "DICO300I", aDICO300I },
        { "DLP", aDLP },
        { "DLQ", aDLQ },
        { "DMP", aDMP },
        { "EPLQ800", aEPLQ800 },
        { "EPLX800", aEPLX800 },
        { "EPSFX", aEPSFX },
        { "EPSFX100", aEPSFX100 },
        { "EPSFX80", aEPSFX80 },
        { "EPSLQ", aEPSLQ },
        { "EPSRX", aEPSRX },
        { "ESCP2", aESCP2 },
        { "FUJIDL24", aFUJIDL24 },
        { "GABRIELE", aGABRIELE },
        { "GENICOM", aGENICOM },
        { "GQ3500SF", aGQ3500SF },
        { "GQ5000", aGQ5000 },
        { "GRAPHICS", aGRAPHICS },
        { "HP4", aHP4 },
        { "HP4SFS", aHP4SFS },
        { "HPDJ", aHPDJ },
        { "HPDJ500C", aHPDJ500C },
        { "HPDJ550", aHPDJ550 },
        { "HPDJCAR", aHPDJCAR },
        { "HPDJSF", aHPDJSF },
        { "HPIII", aHPIII },
        { "HPIIISFS", aHPIIISFS },
        { "HPIIP", aHPIIP },
        { "HPQJ", aHPQJ },
        { "HPXL300", aHPXL300 },
        { "HP_ADAF", aHP_ADAF },
        { "IBM-PPDS", aIBM_PPDS },
        { "IBM4019I", aIBM4019I },
        { "IBM4072", aIBM4072 },
        { "IBM5202", aIBM5202 },
        { "IBM6750", aIBM6750 },
        { "IBMGR", aIBMGR },
        { "IBMPRO", aIBMPRO },
        { "IBMPRO24", aIBMPRO24 },
        { "JUKI6500", aJUKI6500 },
        { "KONICA", aKONICA },
        { "KYOCERA", aKYOCERA },
        { "KYOF1010", aKYOF1010 },
        { "LBP8II", aLBP8II },
        { "LBP8III", aLBP8III },
        { "LC-20", aLC_20 },
        { "LC-200", aLC_200 },
        { "LC24-200", aLC24_200 },
        { "LC866", aLC866 },
        { "LC866SF", aLC866SF },
        { "LT20", aLT20 },
        { "MA-MT81", aMA_MT81 },
        { "MA-MT91", aMA_MT91 },
        { "MP1300", aMP1300 },
        { "MT222", aMT222 },
        { "MT910", aMT910 },
        { "NECP2030", aNECP2030 },
        { "NECP2200", aNECP2200 },
        { "NECP23", aNECP23 },
        { "NECP2P", aNECP2P },
        { "NECP567", aNECP567 },
        { "NECP6070", aNECP6070 },
        { "NECP6272", aNECP6272 },
        { "NECPLUS", aNECPLUS },
        { "NECS60", aNECS60 },
        { "OKI192", aOKI192 },
        { "OKI192OR", aOKI192OR },
        { "OKI193", aOKI193 },
        { "OKI292F", aOKI292F },
        { "OKI293F", aOKI293F },
        { "OKI320", aOKI320 },
        { "OKI380E", aOKI380E },
        { "OKI390E", aOKI390E },
        { "OKI390EL", aOKI390EL },
        { "OKI393EL", aOKI393EL },
        { "OKI393F", aOKI393F },
        { "OKI400", aOKI400 },
        { "OKI800", aOKI800 },
        { "OKILL6", aOKILL6 },
        { "OKILL6R8", aOKILL6R8 },
        { "OLI324L", aOLI324L },
        { "OLIDM109", aOLIDM109 },
        { "OLIJP350", aOLIJP350 },
        { "OLIPG306", aOLIPG306 },
        { "PAINT", aPAINT },
        { "PAN4450I", aPAN4450I },
        { "PANA1080", aPANA1080 },
        { "PANA1081", aPANA1081 },
        { "PANA1090", aPANA1090 },
        { "PANA1091", aPANA1091 },
        { "PANA1092", aPANA1092 },
        { "PANA1124", aPANA1124 },
        { "PANA1180", aPANA1180 },
        { "PANA1540", aPANA1540 },
        { "PANA1592", aPANA1592 },
        { "PANA1595", aPANA1595 },
        { "PR241", aPR241 },
        { "PR5200", aPR5200 },
        { "PRODOT24", aPRODOT24 },
        { "PRODOT9", aPRODOT9 },
        { "PS", aPS },
        { "PT90", aPT90 },
        { "RASTER", aRASTER },
        { "SD15", aSD15 },
        { "SD24", aSD24 },
        { "SEIKO", aSEIKO },
        { "SIHP4100", aSIHP4100 },
        { "SIHP4200", aSIHP4200 },
        { "SIHP4400", aSIHP4400 },
        { "SIHP440F", aSIHP440F },
        { "SIPT18", aSIPT18 },
        { "SIPT18F", aSIPT18F },
        { "SIPT19", aSIPT19 },
        { "SL230AI", aSL230AI },
        { "SL80AI", aSL80AI },
        { "SL80IP", aSL80IP },
        { "SL92", aSL92 },
        { "SP1600AI", aSP1600AI },
        { "SP264", aSP264 },
        { "STARFR10", aSTARFR10 },
        { "STARLC", aSTARLC },
        { "STARLC24", aSTARLC24 },
        { "STARLP8", aSTARLP8 },
        { "STARNB", aSTARNB },
        { "STARNL-I", aSTARNL_I },
        { "STARSJ48", aSTARSJ48 },
        { "STARXB24", aSTARXB24 },
        { "STLC2420", aSTLC2420 },
        { "STXB2420", aSTXB2420 },
        { "SWIFT24E", aSWIFT24E },
        { "TOSH301", aTOSH301 },
        { "TOSH321", aTOSH321 },
        { "TOSH351", aTOSH351 },
        { "TTY", aTTY },
        { "TTYDEU", aTTYDEU },
        { "TYPENRAD", aTYPENRAD },
        { "", 0 }
};



//using namespace ::com::sun::star;
//using namespace ::com::sun::star::ucb;
//using namespace ::com::sun::star::uno;
//using namespace ::ucb;
//using namespace ::rtl;

// --- Sw6Layout ---

short Sw6Layout::ZchnZahl(sal_Char Ch1,sal_Char Ch2) const
// Ermittelt die tatsaechliche Zeichenzahl-1 eines kompletten SW6-Zchns
// Oftmals ist Ch1 ein Steuerzeichen und Ch2 ist dann ein Laengenbyte,
// so dass die Zeichenzahl erst mit Ch2 zu ermitteln ist. Wenn man wissen
// will, ob Ch1 ein nachfolgezeichen Ch2 hat, dann mit Ch2==1 aufrufen
// und auf == 0 abfragen.
{
    short nRet;

    switch (Ch1)
    {
        case PrSoftSp:
        case PrPrFix:
        case PrAttr:
        case PrSize:
        case PrUser:
        case PrFont:
            nRet=1;             // 2 Char/Zchn
            break;
        case PrFnote:
        case PrEnote:
            nRet=2;             // 3 Char/Zchn
            break;
        case PrGraf:
        case PrMerker:
        case PrKomm:
        case PrGlied:
        case PrHidd:            // Ch2 ist Laengenbyte
            nRet=((short)(BYTE)Ch2)+1;
            break;
        case PrSForm:           // Harte Seitenformatierung nie implementiert
            switch (Ch2)
            {
                case SF_Guel:   // Wie Layout gueltig
                case SF_SpAn:   // Anzahl Spalten
                case SF_KoTx:   // Nummer Kopftext
                case SF_FuTx:   // Nummer Fusstext
                    nRet=2;
                    break;
                case SF_KoAb:   // Kopfabstand in Twips
                case SF_FuAb:   // Fussabstand
                case SF_Lks:    // Linker Rand
                case SF_Rts:    // Rechter Rand
                case SF_Obn:    // Oberer Rand
                case SF_Unt:    // Unterer Rand
                case SF_SpAb:   // Spaltenabstand
                    nRet=3;
                    break;
                default:
                    nRet=1;     // Default aus dem Writer
                    break;
            } // switch
            break;
        case PrAForm:
            switch (Ch2)
            {
                case AF_Form:   // Formatierart, Aufzaehltyp
                case AF_SpAn:   // Spaltenanzahl im Absatz
                case AF_SAbs:   // Bool, ist dies ein Spaltenabsatz
                case AF_Auto:   // Autom. Zeilenabstand (Durchschuss)
                case AF_Glied:  // War mal fuer Gliederung vorgesehen
                case AF_Zus:    // Absatz zusammenhalten
                    nRet=2;
                    break;
                case AF_Ein:    // Einzug in Twips
                case AF_Lks:    // Linker Rand
                case AF_Rts:    // Rechter Rand
                case AF_AbAb:   // Absatzabstand
                case AF_SpAb:   // Spaltenabstand
                case AF_ZlAb:   // Zeilenabstand
                    nRet=3;
                    break;
                case AF_Tab:    // Harter Tab mit Tabart, Position (alt, SW5)
                    nRet=4;
                    break;
                case AF_TabN:   // Harter Tab plus Fuellzeichen, (neu, SW6)
                    nRet=5;
                    break;
                default:
                    nRet=1;     // Default aus dem Writer
                    break;
            } // switch
            break;
        default:
            nRet=0;             // Default 1 Char/Zchn
            break;
    } // switch
    return nRet;
}

sal_Char Sw6Layout::UpCaseOEM(sal_Char c) const
// Upcase fuer OEM-Zeichensatz
{
    if (c<'a' || c>'z')
    {
        switch (c)
        {
            case '\x84':
                c='\x8E';
                break;
            case '\x94':
                c='\x99';
                break;
            case '\x81':
                c='\x9A';
                break;
        } // switch
    }
    else c-='\040';
    return c;
}

// Sucht eine Buchstabenkennung raus...
// Uebergabe abgeleiteter Strcts ist leider
// nicht als Pointer Pointer moeglich.
// Templates darf ich auch nicht also!

#define SCANLAYOUT() \
{ \
  short nIdx=0; \
  while (nIdx<nAnz) \
  { \
    if (UpCaseOEM(rKey[(USHORT)nIdx]->Key.GetChar(0))==UpCaseOEM(c1) && \
        UpCaseOEM(rKey[(USHORT)nIdx]->Key.GetChar(1))==UpCaseOEM(c2)) \
    { \
      rnLay=nIdx; \
      break; \
    } \
    nIdx++; \
  } \
}

void Sw6Layout::ScanLayoutB(short &rnLay, BLAY *rKey[],
    const sal_Char c1,const sal_Char c2,const short nAnz) const
{
    SCANLAYOUT();
}

void Sw6Layout::ScanLayoutA(short &rnLay, ALAY *rKey[],
    const sal_Char c1,const sal_Char c2,const short nAnz) const
{
    SCANLAYOUT();
}

void Sw6Layout::ScanLayoutS(short &rnLay, SLAY *rKey[],
    const sal_Char c1,const sal_Char c2,const short nAnz) const
{
    SCANLAYOUT();
}

void Sw6Layout::ScanLayoutKF(short &rnLay, KOFU *rKey[],
    const sal_Char c1,const sal_Char c2,const short nAnz) const
{
    SCANLAYOUT();
}

void Sw6Layout::MerkeNoten(Bereich eNot)
// Merkt sich die jeweilige Notenposition
{
    if (pLay)
    {
        switch (eNot)
        {
            case FNot:
                pLay->FNotenLink=Tell();
                break;
            case ENot:
                pLay->ENotenLink=Tell();
                break;
            case ANot:
                pLay->ANotenLink=Tell();
                break;
        }
    }
}

void Sw6Layout::SetGpmFile( const String &rGpm )
// Meldet GPM-Datei fuer Fontuebersetzung an
{
    // is the last printer the wanted printer?
    String sGpm( rGpm );
    sGpm.SearchAndReplaceAscii( ".GPM", aEmptyStr );
    if( USHRT_MAX == nLastFontMatch ||
        !sGpm.EqualsAscii( aFntMatch[ nLastFontMatch ].pPrtName ))
    {
        // search the wanted printer
        for( nLastFontMatch = 0; aFntMatch[ nLastFontMatch ].pFonts;
                ++nLastFontMatch )
            if( sGpm.EqualsAscii( aFntMatch[ nLastFontMatch ].pPrtName ))
                break;

        if( !aFntMatch[ nLastFontMatch ].pFonts )
            nLastFontMatch = USHRT_MAX;
    }
}

void Sw6Layout::SetAlayTrans(void)
// Uebersetzen OEM->SYSSET je nach Alay ein/aus
{
    if (pLay!=NULL && aSta.nAlay<pLay->NumAlay)
    {
        SetTrans(pLay->FileAlay[aSta.nAlay]->ABlay==0
            ?pLay->FileAlay[aSta.nAlay]->bTran   // Setze das Uebersetzflag
            :pLay->FileBlay[pLay->FileAlay[      // OEM->eDfltSet entweder
                aSta.nAlay]->ABlay-1]->bTran);   // aus ALAY oder aus BLAY
    }
}

void Sw6Layout::SetFollow(short nVon,short nNach,SwDoc &rDoc)
// Setzt die Folge-Seitenlayouts
{
    if (pLay->FileSlay[nVon]->Follow==nVon)
    {
        SwPageDesc &rPg=(SwPageDesc &)rDoc.GetPageDesc(nVon);
        rPg.SetFollow(&rDoc.GetPageDesc(nNach));
        rDoc.ChgPageDesc(nVon,rPg);
        pLay->FileSlay[nVon]->Follow=nNach;
    }
}

void Sw6Layout::ResetLay(SwDoc &rDoc)
// Wird aufgerufen wenn Haupttextbereich gelesen werden soll
{
    aSta.nAlay=0;  aSta.nBlay=-1;      // Status der Layouts bei ReadLn
    aSta.nKopf=-1; aSta.nFuss=-1;      // Dann suche nach Sonder-Alays
    aSta.nSFst=-1; aSta.nSLft=-1;      // der verschiedenen Bereiche
    aSta.nSRgt=-1; aSta.bSlay=TRUE;
    aSta.nPage=1;

    short n=-1;

    while (++n<pLay->NumSlay)          // Suche nun das erste guel-
    {                                  // tige Seitenlayout heraus.
        switch (pLay->FileSlay[n]->Gueltig)
        {
            case 1:
                if (aSta.nSRgt<0) aSta.nSRgt=n;
                break;
            case 2:
                if (aSta.nSLft<0) aSta.nSLft=n;
                break;
            case 3:
                if (aSta.nSFst<0) aSta.nSFst=n;
                break;
            default:
                if (aSta.nSRgt<0) aSta.nSRgt=n;
                if (aSta.nSLft<0) aSta.nSLft=n;
                break;
        } // switch
    } // while
    if (aSta.nSRgt<0) aSta.nSRgt=0;
    if (aSta.nSLft<0) aSta.nSLft=0;
    if (aSta.nSFst>=0) SetFollow(aSta.nSFst,aSta.nSRgt,rDoc);
}

void Sw6Layout::PushStat(long nFPos)
{
    Sw6File::PushStat(nFPos);          // Merke Zustand Sw6File & Seek(Pos)
    aStk.Push(&aSta,sizeof(aSta));     // Dann merke aSta mit harte Formatinfo
    aSta.pFrmt=NULL;                   // Einen neuen Absatz mit pFrm=NULL
    aSta.nAddo=0;
}

void Sw6Layout::PopStat(void)
{
    DelHForm();                        // Harte Formatierinfo weg, dann
    aStk.Pop(&aSta,sizeof(aSta));      // hole auch den alten Status
    Sw6File::PopStat();                // Ein Seek(Zurueck) in Sw6File
}

void Sw6Layout::SetSlay(SwDoc &rDoc,SwPaM &rPaM)
// Setze am Pam eine neues Seitenlayout
{
    if (aSta.bSlay)
    {
        if (aSta.nSFst<0)
        {
            aSta.nSFst=(aSta.nPage&1?aSta.nSRgt:aSta.nSLft);
            if (aSta.nSLft!=aSta.nSRgt)
            {
                SetFollow(aSta.nSRgt,aSta.nSLft,rDoc);
                SetFollow(aSta.nSLft,aSta.nSRgt,rDoc);
            }
        }
        rDoc.Insert(rPaM,SwFmtPageDesc(
            &rDoc.GetPageDesc(aSta.nSFst)));
        aSta.bSlay=FALSE;
        aSta.nSFst=-1;
    }
}

BOOL Sw6Layout::ScanKreuz(const sal_Char *pPatt,const sal_Char *pOrig,size_t &rIgnr,sal_Char *pPara)
// Sucht aus rOrig Kreuzbefehl pPatt raus
// und gibt TRUE zurueck wenn was gefunden,
// Ignr gibt an, wieviele Zeichen #-Befehl
{
    size_t n;
    for (n =0;*pPatt && *pOrig;pPatt++,pOrig++,n++)
    {
        switch (*pPatt)
        {
            case '?':
                if (UpCaseOEM(*pOrig)>='A' &&
                    UpCaseOEM(*pOrig)>='Z')
                {
                    if (pPara) *pPara=UpCaseOEM(*pOrig);
                }
                else return FALSE;
                break;
            case '\xA8':
                if (*pOrig>='0' && *pOrig<='9')
                {
                    if (pPara) *pPara=*pOrig;
                }
                else return FALSE;
                break;
            default:
                if (UpCaseOEM(*pOrig)!=*pPatt)
                {
                    return FALSE;
                }
                break;
        } // switch
    } // for
    if (!*pPatt)
    {
        while (*pOrig>'#' && (
            *pOrig=='*' || *pOrig=='+' ||
            *pOrig=='-' || *pOrig=='^'))
        {
            pOrig++; n++;
        }
        if (*pOrig=='#')
        {
            rIgnr=n+2;
            return TRUE;
        }
    }
    return FALSE;
}

BOOL Sw6Layout::TextBefehl(const sal_Char *pPatt,const sal_Char *pOrig)
// Ermittelt, ob es einen Textbefehl pPatt gibt
// Achtung: pOrig zeigt auf Pascal-String!
{
    sal_Char cLen=*pOrig++;
    while (cLen && *pPatt && UpCaseOEM(*pOrig)==*pPatt)
    {
        pPatt++;
        pOrig++;
        cLen--;
    }
    return !*pPatt && (!cLen || *pOrig<'A' || *pOrig>'Z');
}

void Sw6Layout::AddXForm(sal_Char cLead,ByteString &rTmp,String &rStr)
// Kurzform, wird oefter benoetigt
{
    rTmp.EraseLeadingChars();
    rTmp.EraseTrailingChars();
    rTmp.Insert((sal_Char)rTmp.Len(),0);
    rTmp.Insert(cLead,0);
    AddHForm(rTmp.GetBuffer(),rStr.Len(),rTmp.Len(),1);
}

size_t Sw6Layout::PutRest(String &rStr,sal_Char *pCtrl)
// Holt aus ReadLn die Info. Behandelt versteckte Zchn
// Wird immer nach ReadLn() aufgerufen (virtuell)
{
    size_t nRet=1;

    aSta.cFrst|='\x80';                // Merker fuer Zeile nicht leer
    switch (pCtrl[0])
    {
        case SoftSp:
            break;
        case PrSoftSp:
            nRet++;
            break;
        case NewPage:
            aSta.nPage++;
        case NewCol:
            if ((aSta.cFrst & '\x7F')==0 && rStr.Len()==0)
            {
                aSta.cFrst=(BYTE)pCtrl[0];
            }
            break;
        case PrKomm:
            if ((nRet=pCtrl[1]+2)==5)
            {
                switch (UpCaseOEM(pCtrl[2]))
                {
                    case 'A':
                        ScanLayoutA(aSta.nAlay,pLay->FileAlay,
                            pCtrl[3],pCtrl[4],pLay->NumAlay);
                        SetAlayTrans();
                        break;
                    case 'B':
                        {
                            short nOldL=aSta.nBlay;
                            aSta.nBlay=-1;
                            ScanLayoutB(aSta.nBlay,pLay->FileBlay,
                                pCtrl[3],pCtrl[4],pLay->NumBlay);
                            if (!(++aSta.nBlay))
                            {
                                aSta.nBlay=pLay->FileAlay[
                                    aSta.nAlay]->ABlay;
                            }
                            if (aSta.nBlay)
                            {
                                SetTrans(pLay->FileBlay[
                                    aSta.nBlay-1]->bTran);
                            }
                            else
                            {
                                SetTrans(pLay->FileAlay[
                                    aSta.nAlay]->bTran);
                            }
                            if (nOldL!=aSta.nBlay)
                            {                    // Blay evt. schon gueltig?
                                sal_Char cTmp[2];

                                cTmp[0]='B';
                                cTmp[1]=(sal_Char)aSta.nBlay;
                                AddHForm(cTmp,rStr.Len(),2);
                            }
                        }
                        break;
                    case 'S':
                        {
                            short nNewL=-1;
                            ScanLayoutS(nNewL,pLay->FileSlay,
                                pCtrl[3],pCtrl[4],pLay->NumSlay);
                            if (nNewL>=0) {
                                switch (pLay->FileSlay[nNewL]->Gueltig)
                                {
                                    case 1:
                                        aSta.nSRgt=nNewL;
                                        break;
                                    case 2:
                                        aSta.nSLft=nNewL;
                                        break;
                                    case 3:
                                        break;
                                    default:
                                        aSta.nSRgt=nNewL;
                                        aSta.nSLft=nNewL;
                                        break;
                                } // switch
                                aSta.bSlay=TRUE;
                                aSta.nSFst=-1;
                            }
                        }
                        break;
                    case 'K':
                        ScanLayoutKF(aSta.nKopf,pLay->HartKopf,
                            pCtrl[3],pCtrl[4],pLay->NumKopf);
                        break;
                    case 'F':
                        ScanLayoutKF(aSta.nFuss,pLay->HartFuss,
                            pCtrl[3],pCtrl[4],pLay->NumFuss);
                        break;
                } // switch
            }
            else
            {
                if (TextBefehl("PA",&pCtrl[1]) ||
                    TextBefehl("CP",&pCtrl[1]))
                {
                    static sal_Char __READONLY_DATA p[2]={NewPage,'\0'};
                    AddHForm(p,rStr.Len(),1);
                }
                else
                if (TextBefehl("INDEX",&pCtrl[1]))
                {
                    ByteString aString(&pCtrl[8],(BYTE)pCtrl[1]-6);
                    AddXForm('X',aString,rStr);
                }
                else
                if (TextBefehl("INHALT",&pCtrl[1]))
                {
                    ByteString aString(&pCtrl[9],(BYTE)pCtrl[1]-7);
                    AddXForm('V',aString,rStr);
                }
                else
                {
                    ByteString aTmp( &pCtrl[2], pCtrl[1] );
                    aTmp.Convert( RTL_TEXTENCODING_IBM_850,
                                    gsl_getSystemTextEncoding() );
                    aTmp.EraseLeadingChars();
                    aTmp.EraseTrailingChars();
                    aTmp.Insert('}');            // Mache versteckten Text
                    aTmp.Insert('{',0);
                    AddXForm('-',aTmp,rStr);
                }
            }
            break;
        case HardSp:                             // HardSpace
            AddHForm(pCtrl,rStr.Len(),1,1);
            break;
        case HTrenn:                             // HTrenn nach Minus
            rStr+='-';
            break;
        case SoftK:                              // Softk nach 'C'
            rStr+='c';
            break;
        case PrGlied:                            // PrGlied: Gliedernummer
            {                                    // einfach rauswerfen...
                nRet=pCtrl[1]+2;
                if (rStr.Len() && rStr.GetChar( rStr.Len()-1 )=='#')
                {
                    rStr.Erase(rStr.Len()-1);
                }
            }
            break;
        case '#':
            {
                sal_Char *p=++pCtrl;

                while (p-pCtrl<32 && (BYTE)*p>'#') p++;
                if (p!=pCtrl && *p=='#')
                {
                         if (ScanKreuz("_S",pCtrl,nRet))
                             AddHForm("S",rStr.Len(),1,1);
                    else if (ScanKreuz("_UHR",pCtrl,nRet))
                             AddHForm("U",rStr.Len(),1,1);
                    else if (ScanKreuz("_PFAD",pCtrl,nRet))
                             AddHForm("P",rStr.Len(),1,1);
                    else if (ScanKreuz("_AUTOR",pCtrl,nRet))
                             AddHForm("A",rStr.Len(),1,1);
                    else if (ScanKreuz("_WORTE",pCtrl,nRet))
                             AddHForm("sW",rStr.Len(),2,1);
                    else if (ScanKreuz("_ZEILEN",pCtrl,nRet))
                             AddHForm("sZ",rStr.Len(),2,1);
                    else if (ScanKreuz("_SEITEN",pCtrl,nRet))
                             AddHForm("sS",rStr.Len(),2,1);
                    else if (ScanKreuz("_DATUM",pCtrl,nRet))
                             AddHForm("dT",rStr.Len(),2,1);
                    else if (ScanKreuz("_DATUMZ",pCtrl,nRet))
                             AddHForm("dZ",rStr.Len(),2,1);
                    else if (ScanKreuz("_DATUMBU",pCtrl,nRet))
                             AddHForm("dB",rStr.Len(),2,1);
                    else if (ScanKreuz("_DATUMMON",pCtrl,nRet))
                             AddHForm("dM",rStr.Len(),2,1);
                    else if (ScanKreuz("_TEXTNAME",pCtrl,nRet))
                             AddHForm("N",rStr.Len(),1,1);
                    else if (ScanKreuz("_KATEGORIE",pCtrl,nRet))
                             AddHForm("iK",rStr.Len(),2,1);
                    else if (ScanKreuz("_THEMA\xA8",pCtrl,nRet))
                             AddHForm("iT",rStr.Len(),2,1);
                    else if (ScanKreuz("_SCHL\x9A" "SSELWORT\xA8",pCtrl,nRet))
                             AddHForm("iS",rStr.Len(),2,1);
                    else if (ScanKreuz("_N",pCtrl,nRet)){}
                    else if (ScanKreuz("_FN",pCtrl,nRet)){}
                    else if (ScanKreuz("_EN",pCtrl,nRet)){}
                    else if (*pCtrl>='A' && *pCtrl<='Z' ||
                             *pCtrl>='a' && *pCtrl<='z')
                    {
                        ByteString aFld(pCtrl,p-pCtrl);
                        aFld.Convert( RTL_TEXTENCODING_IBM_850,
                                        gsl_getSystemTextEncoding() );
                        nRet=(size_t)(p-pCtrl)+2;
                        aFld.Insert((sal_Char)(p-pCtrl),0);
                        aFld.Insert('+',0);
                        AddHForm(aFld.GetBuffer(),rStr.Len(),aFld.Len(),1);
                    }
                }
                if (nRet<=1) rStr+='#';
            }
            break;
        case ParaGraf:                           // Achtung: mit IBM_PC geht
            rStr+=ByteString::ConvertToUnicode( '\xA7',  RTL_TEXTENCODING_MS_1252 );// nix: Zchn>=128 dann Conv
        break;
        case TabZch:                             // Tab erst spaeter einfg.
        case InhKenn:                            // Dies Steuerzeichen werden
        case IdxKenn:                            // erst ausgewertet, wenn
        case PrGraf:                             // der komplette String ein-
        case PrFnote:                            // gelesen worden ist.
        case PrEnote:
        case PrAttr:
        case PrFont:
        case PrSize:
        case PrAForm:
            AddHForm(pCtrl,rStr.Len(),           // Attributumschaltung merken
                ZchnZahl(pCtrl[0],pCtrl[1])+1);
        default:
            nRet=ZchnZahl(pCtrl[0],pCtrl[1])+1;
    } // switch
    return nRet;
}

BOOL Sw6Layout::ReadLn(String &rStr)
// Liesst eine Writerzeile mit Steuerzeichen in rStr ein.
// Gibt FALSE zurueck, wenn ein Fehler auftrat...
// Einlesen eines Strings, Incrementiert lLin
{
    aSta.nAddo=0;                       // Kein Adder fuer HFORM->nOfs
    aSta.cFrst='\0';                    // cFrst nicht gesetzt
    DelHForm();                         // Falls in pFrm noch was ist
    lLin++;
    return Sw6File::ReadLn(rStr);       // Kein Fehler aufgetreten
}

BOOL Sw6Layout::IgnExtra(BOOL bNote,short nAnzl,SwTwips *plHoch)
// Liesst und ignoriert Texte bis eine Sonderzeile
// entdeckt wird. Bei Noten==FALSE bis Zeile==^G
// und bei Noten==TRUE bis Zeile==^L, dann Ende...
{
    String aStg;

    if (plHoch) *plHoch=0;
    do {
        if (Eof())                               // Unexpc'd Eof?
        {
            *pErr=ERR_SW6_UNEXPECTED_EOF;
            return FALSE;
        }
        SetAlayTrans();                          // Setze Translate je nach
        if (!ReadLn(aStg)) return FALSE;         // Alay und lesen Absatz
        if (aStg.Len()) aSta.cFrst='\0';         // Ignore ^L/^G if aStg!=""
        if ((bNote || aSta.cFrst!=NewCol) &&
           (!bNote || aSta.cFrst!=NewPage) &&
            (plHoch)) *plHoch+=TWIPINCH/6;       // Ja, Text gefunden...
        if (nAnzl && aSta.cFrst==NewCol)
        {
            if (!--nAnzl) return TRUE;           // Nte Note gefunden...
        }
    } while ((bNote || aSta.cFrst!=NewCol) &&    // Lese bis zum Ende des
            (!bNote || aSta.cFrst!=NewPage));    // Textes (Zeile mit ^L/^G)
    return TRUE;                                 // Keine Fehler aufgetreten
}

BOOL Sw6Layout::ReadBool(void)
{
    String aStg;

    ReadLn(aStg);
    return aStg.EqualsAscii(MLDBOOJAJA);
}

void Sw6Layout::DelHForm(void)
{
    HFORM *pTmp;                        // Hangelzeiger fuer delete

    aSta.nAddo=0;                       // Adder fuer HFORM->nOfs auch 0
    while (aSta.pFrmt)
    {                                   // Falls noch harte Formatinfo
        pTmp=aSta.pFrmt;                // gespeichert ist wech damit
        aSta.pFrmt=aSta.pFrmt->pNxt;
        delete (sal_Char *)pTmp;            // HFORM ist flexibler struct...
    }
}

void Sw6Layout::AddHForm(const sal_Char *pCtrl,short nPos,short nAnz,short nAdd)
{
    HFORM *pTmp=(HFORM *)&aSta.pFrmt;

    while (pTmp->pNxt)
    {                                   // Suche letztes Element
        pTmp=pTmp->pNxt;                // Aus der vorhandenen Liste
    }
    if (NULL!=(pTmp->pNxt=(HFORM *)new sal_Char[
        sizeof(*pTmp)-sizeof(pTmp->cDat)+nAnz]))
    {
        pTmp=pTmp->pNxt;                // Fuelle Stackelement aus
        pTmp->pNxt=NULL;                // Next ist natuerlich NULL
        pTmp->nOfs=nPos+aSta.nAddo;     // Merke Pos im String
        pTmp->nXtd=nAdd;                // Merke fuers Attribut-Kramen
        for (short n=0; n<nAnz; n++)
        {
            pTmp->cDat[n]=*pCtrl++;     // Kopiere die Formatdaten
        }
        aSta.nAddo+=nAdd;               // Evt. mehr addieren!
    }
}

BOOL Sw6Layout::LeseLayInfo(BOOL Konv)
{
    BOOL bIsTrans = IsTrans();

    String aStg;        // Einlese-String

    short Idx;          // Indexe
    short Idy;
    short NumB;         // Anzahl Blays
    short NumA;         // Anzahl Alays
    short NumS;         // Anzahl Slays

    ALAY *LocA;         // WITH Ersatz...
    BLAY *LocB;
    SLAY *LocS;
    TABU *LocT;

    pLay->DelLayout(TRUE,FALSE,FALSE);           // Layout-Info loeschen
    pLay->LastLayTm=0;                           // Keine Verbindung zum Layout
    if (!Konv)
    {                                            // Zeitangabe Verbundenes Lay
        ReadLn(aStg);                            // Steht hier damit die Text-
        pLay->LastLayTm=aStg.ToInt32();                    // blockangaben ab Zeile 4...
    }
    ReadLn(aStg);
    pLay->PapierLen=aStg.ToInt32();                        // Laenge des Papiers
    ReadLn(aStg);
    pLay->PapierBrt=aStg.ToInt32();                        // Breite des Papiers
    ReadLn(aStg);

    pLay->FNotenFlg=(0!=aStg.ToInt32());            // Fussnotennummerierung
    ReadLn(aStg);
    pLay->FNotenForm=(NUMMERIERNG)aStg.ToInt32();   // Fussnotenformat
    pLay->FNotenLin=ReadBool();                  // Fussnotenstrich
    if (Konv)
    {
        pLay->FNotSForm=ZentrSatz;               // Writer 5.x kennt das nicht
        pLay->FNotSPerc=75;
        pLay->FNotURand=0;
        pLay->SpTrennWd=20;
    }
    else
    {
        ReadLn(aStg);
        pLay->FNotSForm=(SATZTYP)aStg.ToInt32();
        ReadLn(aStg);
        pLay->FNotSPerc=aStg.ToInt32();
        ReadLn(aStg);
        pLay->FNotURand=aStg.ToInt32();
        ReadLn(aStg);
        pLay->SpTrennWd=aStg.ToInt32();                    // Twips Spaltentrennstrich
    }
    ReadLn(aStg);
    pLay->ENotenForm=(NUMMERIERNG)aStg.ToInt32();   // Enotenformat
    ReadLn(aStg);
    pLay->SeitenForm=(NUMMERIERNG)aStg.ToInt32();   // Seitennummernformat
    if (Konv)
    {
        ReadLn(aStg);                            // Erweitere Formatierung
        pLay->GliedForm[0]=(NUMMERIERNG)aStg.ToInt32();
        for (Idx=1; Idx<MAXGLIED; Idx++)
        {
            pLay->GliedForm[Idx]=
                pLay->GliedForm[0];
        }                                        // fuer Gleiderungsebenen
    }
    else
    {
        for (Idx=0; Idx<MAXGLIED; Idx++)
        {
            ReadLn(aStg);
            pLay->GliedForm[Idx]=(NUMMERIERNG)aStg.ToInt32();
        }
    }
    pLay->ANotenFlg=ReadBool();
    ReadLn(aStg);                                // 3 Zeilen Reserve
    ReadLn(aStg);
    ReadLn(aStg);
    ReadLn(aStg);
    NumB=aStg.ToInt32();                                   // Anzahl Fontuebersetz
    ReadLn(aStg);
    NumA=aStg.ToInt32();                                   // Anzahl Alays
    ReadLn(aStg);
    NumS=aStg.ToInt32();                                   // Anzahl Slays
    if (!FileOk()) return FALSE;
    if ((NumA==0) || (NumS==0))
    {
        *pErr = ERR_SWG_FILE_FORMAT_ERROR;
        return FALSE;                            // Mind. 1 Alay und 1 Slay
    }
    for (Idx=0; Idx<NumB; Idx++)                 // Fontuebersetzungen lesen }
    {
        if ((LocB=pLay->FileBlay[Idx]=
            new(BLAY))==NULL) return FALSE;
        pLay->NumBlay++;
        SetTrans( TRUE );
        ReadLn(LocB->Remark);                    // Anmerkung
        ReadLn(LocB->Key);                       // Tastenschluessel
        SetTrans( bIsTrans );
        ReadLn(aStg);
        LocB->BFNum=aStg.ToInt32();                        // Fontnummer
        LocB->bTran=TRUE;                        // Font OEM->ANSI
        ReadLn(aStg);
        LocB->BFSiz=aStg.ToInt32();                        // FontSize
        ReadLn(aStg);
        LocB->BAttr=aStg.ToInt32();                  // Fontattribute
        if (!Konv) ReadLn(aStg);                 // 1 * Reserve
    }
    for (Idx=0; Idx<NumA; Idx++)                 // Alays lesen
    {
        if ((LocA=pLay->FileAlay[Idx]=
            new(ALAY))==NULL) return FALSE;
        pLay->NumAlay++;
        SetTrans( TRUE );
        ReadLn(LocA->Remark);                    // Anmerkung
        ReadLn(LocA->Key);                       // Tastenschluessel
        SetTrans( bIsTrans );
        ReadLn(aStg);
        LocA->Einzug=aStg.ToInt32();                       // Linke Einrueckung 1. Zeile
        ReadLn(aStg);
        LocA->LRand=aStg.ToInt32();                        // Linke Einrueckung
        ReadLn(aStg);
        LocA->RRand=aStg.ToInt32();                        // Rechte Einrueckung
        ReadLn(aStg);
        LocA->ORand=aStg.ToInt32();                        // Absatzabstand
        ReadLn(aStg);
        LocA->ABlay=aStg.ToInt32();                        // Fontuebersetz
        if (LocA->ABlay>NumB)                    // Falls Fontuebersetzung
        {                                        // eigentlich gar nicht vor
            LocA->ABlay=0;                       // handen sein sollte...
        }
        ReadLn(aStg);
        LocA->AFNum=aStg.ToInt32();                        // Fontnummer
        LocA->bTran=TRUE;                        // Font OEM->ANSI
        ReadLn(aStg);
        LocA->AFSiz=aStg.ToInt32();                        // FontSize
        ReadLn(aStg);
        LocA->AAttr=aStg.ToInt32();                        // Wegen Runtime-Lib-Err
        ReadLn(aStg);
        LocA->FormatFlg=(SATZTYP)aStg.ToInt32();    // Formatierart
        ReadLn(aStg);
        LocA->SpaltAbst=aStg.ToInt32();                    // Abstand der Spalten
        ReadLn(aStg);
        LocA->SpAnzahl=aStg.ToInt32();                     // Spaltenanzahl-Flag
        if (LocA->SpAnzahl==0 ||
            LocA->SpAnzahl>5)
        {
            LocA->SpAnzahl=1;
        }
        if (Konv)
        {
            ReadLn(aStg);                        // SpaltenTrennzeichen
            LocA->SpTrenn=(aStg.ToInt32()>0);
        }
        else LocA->SpTrenn=ReadBool();
        LocA->SpAbsatz=ReadBool();               // Spaltenabsatz-Flag
        LocA->AutoZlAb=ReadBool();               // Durchschussflag
        ReadLn(aStg);
        LocA->ZlAbstand=aStg.ToInt32();                    // Zeilenabstand
        ReadLn(aStg);
        LocA->GliedLay=aStg.ToInt32();                     // Gliederungsebene
        if (LocA->GliedLay!=0 && LocA->
            GliedLay<=sizeof(Glieder) &&
            !Glieder[LocA->GliedLay-1])          // Gliederebene schon
        {                                        // mal genutzt worden?
          Glieder[LocA->GliedLay-1]=TRUE;
        }
        else LocA->GliedLay=0;
        if (Konv)
        {
            LocA->FrmAbst=0;
            LocA->FrmWdth=16;
            LocA->FrmArt=0;
            LocA->LinLTyp=0;
            LocA->LinRTyp=0;
            LocA->Zusammen=FALSE;
        }
        else
        {
            ReadLn(aStg);
            LocA->FrmAbst=aStg.ToInt32();                  // Rahmenabstand
            ReadLn(aStg);
            LocA->FrmWdth=aStg.ToInt32();                  // Liniendicke
            LocA->FrmArt=0;                      // Vorinitialisierung
            for (Idy=0; Idy<MaxFRMARTEN; Idy++)
            {                                    // 5 * Rahmenflag: O,U,L,R,Box
                ReadLn(aStg);
                LocA->FrmArt+=aStg.ToInt32()<<Idy;
            }
            ReadLn(aStg);
            LocA->LinLTyp=aStg.ToInt32();                  // Gpm-LinienArt
            ReadLn(aStg);
            LocA->LinRTyp=aStg.ToInt32();                  // Gpm-RasterArt
            LocA->Zusammen=ReadBool();           // Absatz zusammenhalten?
            ReadLn(aStg);                        // 1 * Reserve
        }
        ReadLn(aStg);
        LocA->MaxTabs=aStg.ToInt32();                      // Anzahl Tabulatoren
        for (Idy=0; Idy<LocA->MaxTabs; Idy++)
        {
            LocT=&LocA->Tabs[Idy];
            ReadLn(aStg);
            LocT->TabPos=aStg.ToInt32();                   // Tabulator-Position
            ReadLn(aStg);                        // Tabulator-Art
            LocT->TabArt=(TABARTEN)aStg.ToInt32();
            if (!Konv)
            {
                ReadLn(aStg);
                LocT->TabZch=(sal_Char)aStg.ToInt32();  // Tabulator-Zeichen
            }
            else LocT->TabZch='\0';
            if (LocT->TabArt>Tab_D)
            {
                LocT->TabZch='.';                // Frueher auffuellende Tabs
                LocT->TabArt=(TABARTEN)          // Begenze Tabart auf Normal
                    ((short)LocT->TabArt & 3);
            }
        }  // for Idy

    }  // for Idx
    for (Idx=0; Idx<NumS; Idx++)                 // Alle Seitenlayouts lesen
    {
        if ((LocS=pLay->FileSlay[Idx]=
            new(SLAY))==NULL) return FALSE;
        pLay->NumSlay++;
        LocS->Follow=Idx;                        // Merke Folgelayout
        LocS->KopfText=0;                        // Kein K/F-Text gelesen
        LocS->KopfHoch=0;
        LocS->FussText=0;
        LocS->FussHoch=0;
        SetTrans( TRUE );
        ReadLn(LocS->Remark);                    // Anmerkung
        ReadLn(LocS->Key);                       // Tastenschluessel
        SetTrans( bIsTrans );
        ReadLn(aStg);
        LocS->KopfAbst=aStg.ToInt32();                     // Kopfabstand
        ReadLn(aStg);
        LocS->FussAbst=aStg.ToInt32();                     // Fussabstand
        if (Konv)
        {
            if (LocS->FussAbst>(TWIPINCH/6))
            {                                    // Frueher war das anders
                LocS->FussAbst-=(TWIPINCH/6);
            }
            else LocS->FussAbst=0;
        }
        ReadLn(aStg);
        LocS->LRand=aStg.ToInt32();                        // Linker Rand
        ReadLn(aStg);
        LocS->RRand=aStg.ToInt32();                        // Rechter Rand
        ReadLn(aStg);
        LocS->ORand=aStg.ToInt32();                        // Oberer Rand
        ReadLn(aStg);
        LocS->URand=aStg.ToInt32();                        // Unterer Rand
        ReadLn(aStg);
        LocS->SpaltAbst=aStg.ToInt32();                    // Spaltenabstand
        ReadLn(aStg);
        LocS->Gueltig=aStg.ToInt32();                      // Gueltigkeit
        ReadLn(aStg);
        LocS->SpAnzahl=aStg.ToInt32();                     // Anzahl der Spalten
        if (LocS->SpAnzahl==0 ||
            LocS->SpAnzahl>5)
        {
            LocS->SpAnzahl=1;
        }
        if (Konv)
        {
            ReadLn(aStg);                        // SpaltenTrennzeichen
            LocS->SpTrenn=(aStg.ToInt32()>0);
        }
        else LocS->SpTrenn=ReadBool();
        ReadLn(aStg);
        LocS->KopfTNum=aStg.ToInt32();                     // Nummer Kopftext
        ReadLn(aStg);
        LocS->FussTNum=aStg.ToInt32();                     // Nummer Fusstext
        if (!Konv) ReadLn(aStg);                 // 1 * Reserve

        LocS->KopfText=Tell();
        if (!IgnExtra(FALSE,0,&LocS->            // Kopftext ueberlesen
            KopfHoch)) return FALSE;
        LocS->FussText=Tell();
        if (!IgnExtra(FALSE,0,&LocS->            // Fusstext ueberlesen
            FussHoch)) return FALSE;
    }
    return TRUE;
}

BOOL Sw6Layout::LeseKoFu(BOOL Kopf)
// Liesst harte Kopf- oder Fusstexte ein
{
    short  Idx;
    short  Anz;
    String Stg;
    KOFU  *LocF;

    pLay->DelLayout(FALSE,Kopf,!Kopf);           // Kopf oder Fusslayout loeschen
    ReadLn(Stg);
    Anz=Stg.ToInt32();
    if (!FileOk()) return FALSE;
    for (Idx=0; Idx<Anz; Idx++)
    {
        if (Kopf)
        {
            if ((LocF=pLay->HartKopf[Idx]=
                new(KOFU))==NULL) return FALSE;
            pLay->NumKopf++;
        }
        else
        {
            if ((LocF=pLay->HartFuss[Idx]=
                new(KOFU))==NULL) return FALSE;
            pLay->NumFuss++;
        }
        ReadLn(LocF->Remark);
        ReadLn(LocF->Key);
        LocF->KFText=Tell();
        if (!FileOk() || !IgnExtra(FALSE))
        {
            return FALSE;
        }
    }
    return IgnExtra(TRUE);
}

SwTxtFmtColl *Sw6Layout::GetAlay(void)
// Holt die gerade gueltige FmtColl
{
    ASSERT(aSta.nAlay>=0 && aSta.nAlay<pLay->NumAlay,"Ungueltiges Alay");
    return pLay->FileAlay[aSta.nAlay]->pFmtColl;
}

SvxAdjust Sw6Layout::TransAdj(SATZTYP eForm)
// Uebersetzt Formatierart
{
    SvxAdjust eAdj=SVX_ADJUST_LEFT;             // Default immer links
    switch (eForm)                     // Satztyp uebergeben
    {
        case AustrSatz:                // Unbekannt in SWG!
        case SperrSatz:                // Gibt es auch nicht
        case BlockSatz:
            eAdj=SVX_ADJUST_BLOCK;
            break;
        case RechtsSatz:
            eAdj=SVX_ADJUST_RIGHT;
            break;
        case ZentrSatz:
            eAdj=SVX_ADJUST_CENTER;
            break;
        case UnformSatz:               // Behandle wie linksbuendig
            break;
    } // switch
    return eAdj;
}

SfxPoolItem *Sw6Layout::TransAtt(ATTRBITS eAttr,BOOL bEin,SwTwips nSiz)
// Uebersetzt Fontattribut SW5->SWG
{
    switch (eAttr)
    {
        case Gross2 :return new SvxFontHeightItem( (ULONG)nSiz*(bEin?2:1));
        case Gross4 :return new SvxFontHeightItem( (ULONG)nSiz*(bEin?4:1));
        case Fett   :return new SvxWeightItem(bEin?WEIGHT_BOLD:WEIGHT_NORMAL);
        case Kursiv :return new SvxPostureItem(bEin?ITALIC_NORMAL:ITALIC_NONE);
        case Unter  :return new SvxUnderlineItem(bEin?UNDERLINE_SINGLE:UNDERLINE_NONE);
        case DUnter :return new SvxUnderlineItem(bEin?UNDERLINE_DOUBLE:UNDERLINE_NONE);
        case Durch  :return new SvxCrossedOutItem(bEin?STRIKEOUT_DOUBLE:STRIKEOUT_NONE);
        case DDurch :return new SvxCrossedOutItem(bEin?STRIKEOUT_SINGLE:STRIKEOUT_NONE);
        case Hoch   :return new SvxEscapementItem(bEin?SVX_ESCAPEMENT_SUPERSCRIPT:SVX_ESCAPEMENT_OFF);
        case Tief   :return new SvxEscapementItem(bEin?SVX_ESCAPEMENT_SUBSCRIPT:SVX_ESCAPEMENT_OFF);
        case Outl   :return new SvxContourItem(bEin);
        case OutShad:return new SvxContourItem(bEin);
        case Shad   :return new SvxShadowedItem(bEin);
        case FRot   :return new SvxColorItem( Color( bEin?COL_RED:COL_BLACK));
        case FBlau  :return new SvxColorItem( Color( bEin?COL_BLUE:COL_BLACK));
        case FMag   :return new SvxColorItem( Color( bEin?COL_MAGENTA:COL_BLACK));
        case FGelb  :return new SvxColorItem( Color( bEin?COL_YELLOW:COL_BLACK));
        case FOrange:return new SvxColorItem( Color( bEin?COL_LIGHTRED:COL_BLACK));
        case FGruen :return new SvxColorItem( Color( bEin?COL_GREEN:COL_BLACK));
        case FCyan  :return new SvxColorItem( Color( bEin?COL_CYAN:COL_BLACK));
        default: return NULL;
    } // switch
}

void Sw6Layout::TransFnt( short nFnt, FontFamily &rFam,
                          FontPitch &rPit, CharSet &rSet, String &rNam )
// Uebersetzt Fontnummer nFnt nach Family und Pitch
{
    Sw6PrinterFont aFontDef;
    aFontDef.nFontNo = 0;
    if( USHRT_MAX != nLastFontMatch )
    {
        const Sw6PrinterFont* pFonts = aFntMatch[ nLastFontMatch ].pFonts;
        for( ; pFonts->nFontNo; ++pFonts )
            if( pFonts->nFontNo == nFnt )
            {
                aFontDef = *pFonts;
                break;
            }
    }

    if( !aFontDef.nFontNo )
    {
        // Je nach Fontnummer einen Default zusammenstellen, wie er im Config
        // stehen koennte ...
        aFontDef.bPCFont = sal_True;
        aFontDef.bFixPitch = sal_True;
        if( nFnt < 33 )
        {
            aFontDef.nFontNo = 33;
            aFontDef.nFontNmNo = 69;
            aFontDef.eFamily = FAMILY_SYSTEM;
        }
        else if( nFnt < 65 )
        {
            aFontDef.nFontNo = 65;
            aFontDef.nFontNmNo = 1;
            aFontDef.eFamily = FAMILY_MODERN;
        }
        else if( nFnt < 97 )
        {
            aFontDef.nFontNo = 97;
            aFontDef.nFontNmNo = 215;
            aFontDef.eFamily = FAMILY_SWISS;
            aFontDef.bFixPitch = sal_False;
        }
        else if( nFnt < 129 )
        {
            aFontDef.nFontNo = 129;
            aFontDef.nFontNmNo = 274;
            aFontDef.eFamily = FAMILY_ROMAN;
            aFontDef.bFixPitch = sal_False;
        }
        else
        {
            rFam = FAMILY_DONTKNOW;
            rSet = RTL_TEXTENCODING_IBM_850;
            rPit = PITCH_VARIABLE;
        }
    }

    if( aFontDef.nFontNo )
    {
        rNam.AssignAscii( aFontNames[ aFontDef.nFontNmNo ] );
        rFam = aFontDef.eFamily;
        if( aFontDef.bPCFont )
            rSet = RTL_TEXTENCODING_IBM_850;
        rPit = aFontDef.bFixPitch ? PITCH_FIXED : PITCH_VARIABLE;
    }
}

BOOL Sw6Layout::TransFSA(SwFmt &rFmt, short nFnt, short nSiz, ATTRS nAtt)
// Uebersetzt Font/SSize/Attribut aus den DOS-SW in ein SwCharFmt
// Dabei wird TRUE zurueckgegeben, wenn so ausgezeichnete Zeichen
// von IBM-PC Zeichensatz nach OEM umgesetzt werden sollten.
{
    const SvxFontItem& rFont = rFmt.GetFont();
    FontFamily eFam=rFont.GetFamily();
    FontPitch  ePit=rFont.GetPitch();
    CharSet    eSet=rFont.GetCharSet();
    String     aNam=rFont.GetFamilyName();
    String     aSty=rFont.GetStyleName();

    SfxItemSet aSet( *rFmt.GetAttrSet().GetPool(),
                    rFmt.GetAttrSet().GetRanges() );
    TransFnt(nFnt,eFam,ePit,eSet,aNam);          // Matsche SW6->SWG
    aSet.Put( SvxFontItem(eFam,aNam,aSty,ePit,
                RTL_TEXTENCODING_DONTKNOW ) );	         // Neuen Font anmelden
    aSet.Put( SvxFontHeightItem(nSiz*10) );			 // Neue SSize anmelden

    for (short nIdx=0; nIdx<ATTRBITS_END; nIdx++)
    {
        if (nAtt & ((ATTRS)1<<nIdx))
        {
            SfxPoolItem *pTmp=TransAtt(
                (ATTRBITS)nIdx,TRUE,nSiz*10);
            if (pTmp)
            {
                aSet.Put( *pTmp );
                delete pTmp;
            }
        }
    }
    rFmt.SetAttr( aSet );
    return eSet==RTL_TEXTENCODING_IBM_850;                  // Merke ob Uebersetzen
}

void Sw6Layout::InsertTab(USHORT nTab,TABU &rTab,SvxTabStopItem &rTabs,SwTwips nKor)
// Packt einen Writer-TABU in ein SvxTabStop
// nTab ist die Nummer des Tabs aus FmtTabStops
// nKor ist ein Korrekturwert wegen linker Rand++
{
    sal_Unicode eFill=cDfltFillChar;
    if ((BYTE)rTab.TabZch>' ')
    {
        eFill= ByteString::ConvertToUnicode( rTab.TabZch, RTL_TEXTENCODING_IBM_850 );
    }

    SvxTabAdjust eAdjust=SVX_TAB_ADJUST_LEFT;
    switch (rTab.TabArt)
    {
        case Tab_R:eAdjust=SVX_TAB_ADJUST_RIGHT;
            break;
        case Tab_Z:eAdjust=SVX_TAB_ADJUST_CENTER;
            break;
        case Tab_D:eAdjust=SVX_TAB_ADJUST_DECIMAL;
            break;
    } // switch

    SvxTabStop aTab(rTab.TabPos-nKor,eAdjust,cDfltDecimalChar,eFill);
    if(nTab<rTabs.Count())
    {
        rTabs.Remove(nTab);
    }
    rTabs.Insert(aTab);
}

void Sw6Layout::InsertExtra(SwDoc &rDoc,SwPaM &rPaM,
     const SwNodeIndex &rWohin,long lFPos,sal_Char *pcAKey)
// Fuege einen Sondertext an die Stelle rDoc/rPaM ein.
// Dazu suche mittels Seek den Text, lese die Absaetze
// ein und seeke wieder zurueck an die Ausgangsposition
{
    String     aStg;
    short      nOldA=aSta.nAlay;                 // Gemerkte Alaynummer
    BOOL       bErrs=FALSE;                      // Fehler aufgetreten?
    SwPosition aMerk(*rPaM.GetPoint());          // Merke alte Einfgposition

    aSta.nAlay=0;
    if (pcAKey)                                  // Vorgabe eines Layout-
    {                                            // kennzeichners vorhanden?
        ScanLayoutA(aSta.nAlay,pLay->FileAlay,   // z.B. Standard KO fuer
            pcAKey[0],pcAKey[1],pLay->NumAlay);  // Kopftexte hier suchen
    }
    PushStat(lFPos);                             // Springe zum Extratext
    rPaM.GetPoint()->nNode=rWohin;               // Setze den PaM auf den in
    rPaM.GetPoint()->nContent.                   // rWohin angegebenen Node
        Assign(rPaM.GetCntntNode(),0);
    aSta.nBlay=0;                                // Kein BLay gueltig

    do {
        SetAlayTrans();                          // Setze Translate je nach
        if (ReadLn(aStg))                        // Alay und lesen Absatz
        {
            if (aStg.Len()) aSta.cFrst='\0';     // Ignore ^L/^G if Stg!=""
            if ((aSta.cFrst!=NewCol) &&
                (aSta.cFrst!=NewPage))
            {
                rDoc.SetTxtFmtColl(rPaM,GetAlay());
                if (!InsertLine(rDoc,rPaM,aStg,TestCtrl(
                    NewPage) || TestCtrl(NewCol)))
                {
                    bErrs=TRUE;
                }
            }
        }
        else bErrs=TRUE;
    } while (!bErrs &&                           // Lese bis zum Ende des
             (aSta.cFrst!=NewCol) &&             // Textes (Zeile mit ^L/^G)
             (aSta.cFrst!=NewPage));

    *rPaM.GetPoint()=aMerk;                      // PaM wieder zuruecksetzen
    aSta.nAlay=nOldA;                            // Altes Layout zurueck
    PopStat();                                   // Wieder zur Ausgangspos
}

BOOL Sw6Layout::InsertNote(SwDoc &rDoc,SwPaM &rPaM,USHORT nNote, BOOL bFuss)
// Fuege eine Fuss/Endnote in den Text ein.
// Gibt TRUE zurueck wenn tatsaechlich eine
// Fuss- oder Endnote eingefuegt wurde.
// Wenn nNote<0 dann Anmerkung->PostIt
{
    BOOL bRet=FALSE;

    if (bFuss)
    {
        BOOL bErrs=FALSE;

        PushStat(nNote&0x8000?                   // Suchen entweder Anmer-
            pLay->ANotenLink:                    // kung oder FussNote raus
            pLay->FNotenLink);
        if (nNote&0x7FFF)
        {                                        // Suche Anfang dieser Note
            bErrs=!IgnExtra(TRUE,nNote&0x7FFF);  // Wenn das noetig ist.
        }
        if (!bErrs)
        {
            if (!(nNote&0x8000))
            {
                SwFmtFtn aFtn;                   // Ein Fussnotennode in
                rDoc.Insert(rPaM,aFtn);          // den Text einfuegen und
                SwPaM aPaM(rPaM);                // dann den Text dazu holen
                aPaM.Move(fnMoveBackward,fnGoCntnt);
                                                 // Auf's Fussnotenzeichen
                SwNodeIndex aSttIdx(*(((SwTxtFtn*)aPaM.GetNode()->
                    GetTxtNode()->GetTxtAttr(aPaM.GetPoint()->
                    nContent,RES_TXTATR_FTN))->GetStartNode()),1);
                InsertExtra(rDoc,rPaM,aSttIdx,-1,"FN");
                bRet=TRUE;                       // Hier alles Fehlerfrei...
            }
            else
            {
                String aStg,aNot;                // Liess Anmerkung nach aNot
                do                               // und verknuepfe mehrere
                {                                // Absaetze zu einem String
                    if (ReadLn(aStg))
                    {
                        if (aStg.Len()) aSta.cFrst='\0';
                        if ((aSta.cFrst!=NewCol) &&
                            (aSta.cFrst!=NewPage))
                        {
#ifdef WIN
                            if (aNot.Len()) aNot+="\r\n";
#else
                            if (aNot.Len()) aNot+='\n';
#endif
                            aNot+=aStg;
                        }
                    }
                    else bErrs=TRUE;
                } while (!bErrs &&               // Lese bis zum Ende des
                    (aSta.cFrst!=NewCol) &&      // Textes (Zeile mit ^L/^G)
                    (aSta.cFrst!=NewPage));
                if (!bErrs && aNot.Len())
                {
                    Date aDate;
                    String aLeer;

                    SwPostItField aFld((SwPostItFieldType*)
                        rDoc.GetSysFldType(RES_POSTITFLD),
                        pDocInfo?pDocInfo->Autor:aLeer,aNot,aDate);
                        rDoc.Insert(rPaM,SwFmtFld(aFld));
                    bRet=TRUE;
                }
            }
        }
        PopStat();                               // Wieder zur Ausgangspos
    }
    return bRet;
}

static SwTxtFmtColl* _GetTxtColl( SwDoc* pDoc, const String& rName,
                                    const String& rKey )
{
    USHORT n;

    // 1. Runde: ist der Name ein Poolformatname?
    if( rName.Len() &&
        USHRT_MAX != (n = SwStyleNameMapper::GetPoolIdFromUIName( rName, GET_POOLID_TXTCOLL )) )
        return pDoc->GetTxtCollFromPoolSimple( n, FALSE );

    // 2. Runde: ueber den Namen suchen; ueber den vollstaendigen !
    String sNm( rKey );
    if( rName.Len() )
    {
        sNm += ' ';
        sNm += rName;
    }

    SwTxtFmtColl* pColl = pDoc->FindTxtFmtCollByName( sNm );
    if( !pColl )
        pColl = pDoc->MakeTxtFmtColl( sNm, (SwTxtFmtColl*)pDoc->GetDfltTxtFmtColl() );
    return pColl;
}

static SwCharFmt* _GetCharFmt( SwDoc* pDoc, const String& rName,
                                    const String& rKey )
{
    USHORT n;

    // 1. Runde: ist der Name ein Poolformatname?
    if( rName.Len() &&
        USHRT_MAX != (n = SwStyleNameMapper::GetPoolIdFromUIName( rName, GET_POOLID_CHRFMT )) )
        return pDoc->GetCharFmtFromPool( n );

    // 2. Runde: ueber den Namen suchen
    String sNm( rKey );
    if( rName.Len() )
    {
        sNm += ' ';
        sNm += rName;
    }

    SwCharFmt* pFmt = pDoc->FindCharFmtByName( sNm );
    if( !pFmt )
        pFmt = pDoc->MakeCharFmt( sNm, pDoc->GetDfltCharFmt() );
    return pFmt;
}

void Sw6Layout::InsertLayouts(SwDoc &rDoc, SwPaM *pPaM)
// Schiesst alle Layouts in das Dokument
{
    short  Idx;         // Index fuer Layouts

    SwFtnInfo aInfo;
    aInfo=rDoc.GetFtnInfo();                     // Wow: was 'ne Aktion...!
    aInfo.ePos=FTNPOS_PAGE;                      // GetFntInfo ist const &
    rDoc.SetFtnInfo(aInfo);

    for (Idx=pLay->NumBlay-1; Idx>=0; Idx--)
    {
        BLAY  *pBly=pLay->FileBlay[Idx];         // Zeiger auf aktuelles Blay
        // Erzeuge neues SwCharFmt
        pBly->pCharFmt = _GetCharFmt( &rDoc, pBly->Remark, pBly->Key );

        pBly->bTran=TransFSA(*pBly->             // Uebertrage die Fontnummer
            pCharFmt,pBly->BFNum,                // und die Attribute aus SW
            pBly->BFSiz,pBly->BAttr);            // in die SWG-Umgebung
    } // for pLay->NumBlay

    for (Idx=0; Idx<pLay->NumAlay; Idx++)
    {
        ALAY  *pAly=pLay->FileAlay[Idx];         // Zeiger auf aktuelles Alay
        // Suche/Erzeuge SwTxtFmtColl
        pAly->pFmtColl = _GetTxtColl( &rDoc, pAly->Remark, pAly->Key );

        SfxItemSet aSet( *pAly->pFmtColl->GetAttrSet().GetPool(),
                        pAly->pFmtColl->GetAttrSet().GetRanges() );
        // Gliederebene setzen
        pAly->pFmtColl->SetOutlineLevel( pAly->GliedLay>0
                                            ? pAly->GliedLay-1
                                            : NO_NUMBERING );

        if (pAly->ABlay!=0)                      // BLay-Nummer ist gueltig?
        {
            BLAY *pBly=pLay->FileBlay[pAly->ABlay-1];

            pAly->bTran=pBly->bTran;             // Merke auch Uebersetzung
#ifdef JP_NEWCORE
JP 29.09.94: zur Zeit keine Zeichen-Vorlage an der Format-Vorlage
            pAly->pFmtColl->SetCharFmt(          // Trage das Blay am Alay ein
                pBly->pCharFmt);
#endif
        }
        else
        {                                        // Harter Font/SSize/Attr
            pAly->bTran=TransFSA(                // Merke ob Trans OEM->Dflt
                *pAly->pFmtColl,   				 // Aendere SwCharFmt Alay
                pAly->AFNum,pAly->AFSiz,pAly->AAttr);
        }
        SvxLRSpaceItem aLR;                        // Linken und rechten
        aLR.SetLeft(USHORT(pAly->LRand));          // Absatzeinzug bestimmen
        aLR.SetRight(USHORT(pAly->RRand));
        aLR.SetTxtFirstLineOfst(                 // Einzug erste Zeile
        USHORT(pAly->Einzug-pAly->LRand));
        aSet.Put( aLR );

        SvxULSpaceItem aUL;                        // Absatzabstand setzen
        aUL.SetUpper(USHORT(pAly->ORand));
        aSet.Put( aUL );

        SvxLineSpacingItem aLSpc;                  // Height=0, Methode MIN, INTER_OFF
        aLSpc.SetLineHeight(pAly->ZlAbstand);

        if (pAly->AutoZlAb)                      // ZlAbstand Durchschuss?
        {
            aLSpc.SetLineHeight(0);
            aLSpc.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;
            aLSpc.SetInterLineSpace(pAly->ZlAbstand);
        }
        aSet.Put( aLSpc );		                  // Abstand uebergeben

        aSet.Put( SvxAdjustItem(TransAdj(pAly->FormatFlg) ));
        aSet.Put( SvxFmtSplitItem(!pAly->Zusammen) );	// Absatz zusammenhalten?

        if (pAly->FrmArt&0x1F && pAly->LinLTyp>1)
        {
            if (pAly->FrmArt&0x10)               // Falls Rahmen drumherum
                pAly->FrmArt|=0x0F;
            ColorData eColor=COL_BLACK;          // Zu verwendende Farbe
            switch (pAly->LinLTyp)
            {
                case 2:
                case 3:
                    eColor=COL_WHITE;
                    break;
                case 4:
                case 5:
                case 6:
                    eColor=COL_LIGHTGRAY;
                    break;
                case 7:
                case 8:
                    eColor=COL_GRAY;
            } // switch
            SvxBoxItem   aFBox;
            Color        aColor(eColor);
            SwLineType   aLTyp=GetLineType((USHORT)(pAly->
                FrmWdth>10*20?10:pAly->FrmWdth/20));
            SvxBorderLine aBord(&aColor,aLTyp.
                nOut,aLTyp.nIn,aLTyp.nDist);

            aFBox.SetDistance((USHORT)pAly->FrmAbst);
            if (pAly->FrmArt&0x2)
                aFBox.SetLine (&aBord, BOX_LINE_RIGHT);
            if (pAly->FrmArt&0x8)
                aFBox.SetLine (&aBord, BOX_LINE_BOTTOM);
            if (pAly->FrmArt&0x1)
                aFBox.SetLine (&aBord, BOX_LINE_LEFT);
            if (pAly->FrmArt&0x4)
                aFBox.SetLine (&aBord, BOX_LINE_TOP);
            aSet.Put( aFBox );
        }
        if (pAly->LinRTyp>1)
        {
            ColorData eColor=COL_BLACK;
//			BrushStyle eBrush=BRUSH_SOLID;
            switch (pAly->LinRTyp)
            {
                case 2:eColor=COL_WHITE; break;
//				case 3:eBrush=BRUSH_UPDIAG; break;
//				case 4:eBrush=BRUSH_DOWNDIAG; break;
//				case 5:eBrush=BRUSH_CROSS; break;
//				case 6:eBrush=BRUSH_DIAGCROSS; break;
//				case 7:eBrush=BRUSH_25; break;
//				case 8:eBrush=BRUSH_50; break;
//				case 9:eBrush=BRUSH_75; break;
            } // switch
//			Brush aBrush=Brush(Color(eColor),eBrush);
//			aBrush.SetTransparent(FALSE);
            Color aTmpColor(eColor);
            SvxBrushItem aBack( aTmpColor, RES_BACKGROUND);
            aSet.Put( aBack );
        }

        SvxTabStopItem aTabs(0,0);                  // Erzeuge Tab-Tabelle
        for (short n=0; n<pAly->MaxTabs; n++)    // muss sein, falls LRand &
        {                                        // und Einzug ungleich sind
            InsertTab(n,pAly->Tabs[n],
                aTabs,pAly->LRand);
        }
        aSet.Put( aTabs );						 // Alle Tabs ins Format
        pAly->pFmtColl->SetAttr( aSet );
    } // for pLay->NumAlay

    for (Idx=0; Idx<pLay->NumSlay; Idx++)
    {
        USHORT nPageDesc=0;
        SLAY  *pSly=pLay->FileSlay[Idx];         // Zeiger auf aktuelles Slay
        String aNam=pSly->Key;
        if (pSly->Remark.Len())
            aNam+=' ';
        aNam += pSly->Remark;

        if (Idx!=0)                              // Default Pagedesc ein-
        {                                        // fach ueberschreiben oder
          nPageDesc=rDoc.MakePageDesc(aNam, 0, FALSE);
          // sonst einen neuen machen
        }

        SwPageDesc &rPg=(SwPageDesc &)rDoc.
            GetPageDesc(nPageDesc);

        if (Idx==0) rPg.SetName(aNam);           // setze den Follow-PageDesc
        rPg.SetFollow(&rPg);                     // erstmal auf sich selbst
        rPg.SetUseOn((UseOnPage)(PD_ALL|PD_HEADERSHARE|PD_FOOTERSHARE));

        if (Idx==0)
        {                                        // 1.Slay auch gueltig machen
            rDoc.Insert(*pPaM,SwFmtPageDesc(
                &rDoc.GetPageDesc(nPageDesc)));
        }
        // Seitenhoehe einstellen
        SwFrmFmt &rFmt=rPg.GetMaster();
        SwFmtFrmSize aSiz(rFmt.GetFrmSize());
        aSiz.SetWidth(pLay->PapierBrt);          // Setze Papierbreite
        aSiz.SetHeight(pLay->PapierLen);         // Setze Papierlaenge
        rFmt.SetAttr(aSiz);                      // Melde Dokumentformat an

        {   // Seitenformat setzen
            SvxLRSpaceItem aLR(rFmt.GetLRSpace());
            aLR.SetLeft(USHORT(pSly->LRand));
            aLR.SetRight(USHORT(pSly->RRand));
            rFmt.SetAttr(aLR);

            SvxULSpaceItem aUL(rFmt.GetULSpace());

            if (pSly->KopfHoch>pSly->ORand)      // Begrenze Kopftexthoehe
                pSly->KopfHoch=pSly->ORand;      // und den Kopftextabstand
            if (pSly->KopfAbst>pSly->ORand-pSly->KopfHoch)
                pSly->KopfAbst=pSly->ORand-pSly->KopfHoch;
            if (pSly->KopfHoch)                  // Kopftext ueberhaupt da?
            {
                aUL.SetUpper(USHORT(pSly->KopfAbst));
            }
            else aUL.SetUpper(USHORT(pSly->ORand));

            if (pSly->FussAbst>pSly->URand)      // Begrenze Fusstextabstand
                pSly->FussAbst=pSly->URand;      // und die Fusstexthoehe
            if (pSly->FussHoch>pSly->FussAbst)
                pSly->FussHoch=pSly->FussAbst;
            if (pSly->FussHoch)                  // Fusstext ueberhaupt da?
            {
                aUL.SetLower((USHORT(pSly->FussAbst-    // UL.Lower auf den
                    pSly->FussHoch)));            // Fusstextabstand
            }
            else aUL.SetLower(USHORT(pSly->URand));

            rFmt.SetAttr(aUL);
        }
        {   // Abst and Kopftext setzen

            rFmt.SetAttr(SwFmtHeader(            // Evt. kein Kopftext...
                pSly->KopfHoch!=0));
            if (pSly->KopfHoch!=0)
            {
                SwFrmFmt *pFmt=(SwFrmFmt*)rFmt.
                    GetHeader().GetHeaderFmt();
                SvxULSpaceItem aUL(pFmt->GetULSpace());
                aUL.SetUpper(0);
                aUL.SetLower(USHORT(pSly->ORand-pSly->
                    KopfAbst-pSly->KopfHoch));
                pFmt->SetAttr(aUL);              // Attribute Kopftext setzen
                SwNodeIndex aIdx( *pFmt->GetCntnt().GetCntntIdx(), 1 );
                InsertExtra(rDoc,*pPaM,aIdx,   // Eigentlichen Text einlesen
                    pSly->KopfText,"KO");
            }
        }
        {   // Abstand Fusstext setzen
            rFmt.SetAttr(SwFmtFooter(            // Evt. kein Fusstext...
                pSly->FussHoch!=0));
            if (pSly->FussHoch!=0)
            {
                SwFrmFmt *pFmt=(SwFrmFmt*)rFmt.
                    GetFooter().GetFooterFmt();
                SvxULSpaceItem aUL(pFmt->GetULSpace());
                aUL.SetUpper(USHORT(pSly->
                    URand-pSly->FussAbst));
                aUL.SetLower(0);
                pFmt->SetAttr(aUL);              // Attribute Fusstext setzen
                SwNodeIndex aIdx( *pFmt->GetCntnt().GetCntntIdx(), 1 );
                InsertExtra(rDoc,*pPaM,aIdx,   // Eigentlichen Text einlesen
                    pSly->FussText,"FU");
            }
        }
        {   // Fussnoteninfo setzen
            SwPageFtnInfo aInfo(rPg.GetFtnInfo());

//			Pen aPen(aInfo.GetPen());
//			aPen.SetStyle(pLay->FNotenLin?PEN_SOLID:PEN_NULL);
            if(!pLay->FNotenLin)
                aInfo.SetLineColor(Color(COL_TRANSPARENT));

            switch (pLay->FNotSForm)
            {
                case RechtsSatz:
                    aInfo.SetAdj(FTNADJ_RIGHT);
                    break;
                case ZentrSatz:
                    aInfo.SetAdj(FTNADJ_CENTER);
                    break;
                default:
                    aInfo.SetAdj(FTNADJ_LEFT);
                    break;
            } // switch
            aInfo.SetWidth(Fraction(pLay->FNotSPerc,100));
            if (pLay->FNotURand>aInfo.GetTopDist())
            {
                aInfo.SetTopDist(pLay->FNotURand);
            }
            rPg.SetFtnInfo(aInfo);
        }
        rDoc.ChgPageDesc(nPageDesc,rPg);
    } // for pLay->NumSlay
}

void Sw6Layout::InsertAForm(SwDoc &rDoc,SwPaM &rPaM,BYTE *pCtrl)
// Gibt einem Absatz die spezielle Form, die der wohl
// soll; mittels harter Formatierung ist dies moeglich
{
    SvxLineSpacingItem aLSpc;     // Zum Zeilenabstand setzen
    SvxLRSpaceItem aLR;         // Zum link/rechten Rand setzen
    SvxULSpaceItem aUL;         // Zum Absatztabstand setzen
    SwTxtNode *pTxt;            // Aktueller Absatz
    SwAttrSet* pSet;			// aktuelle Format-Attribute
    short nWert;                // Wert Zeilenabstand/Raender

    if (NULL!=(pTxt=rDoc.GetNodes()[rPaM.
        GetPoint()->nNode]->GetTxtNode()) &&
        NULL!=(pSet=&pTxt->GetSwAttrSet()) )
    {
        // Hole Wert in Twips und evt. drehen
        nWert=SVBT16ToShort(&pCtrl[2]);
        switch (pCtrl[1])              // Je nach Absatzformataenderung
        {
            case AF_Ein:               // Einrueckung Twips
            case AF_Lks:               // Linker Rand Twips
                {
                SwTwips nLRnd;         // Linker Rand Absatz (absolut)
                SwTwips nEinz;         // Einzug Absatz (absolut)
                SwTwips nTemp;         // Temporaerwert

                const SvxLRSpaceItem& rLR = pSet->GetLRSpace();
                nLRnd=rLR.GetLeft();
                if ((nTemp=nEinz=rLR.GetTxtFirstLineOfst())<0)
                {
                    nEinz=nLRnd;       // Der einfachheit halber Einzug
                    nLRnd-=nTemp;      // und LRand als Absolutwerte
                }
                else nEinz+=nLRnd;
                                       // Raender neu setzen
                if (pCtrl[1]==AF_Ein)
                    nEinz=nWert;
                else
                    nLRnd=nWert;

                aLR.SetLeft((USHORT)(nEinz<nLRnd?nEinz:nLRnd));
                aLR.SetRight(rLR.GetRight());
                aLR.SetTxtFirstLineOfst((USHORT)(nEinz-nLRnd));

                rDoc.Insert(rPaM,aLR); // Und dann Raender hart rein...
                }
                break;
            case AF_Rts:               // Rechter Rand Twips
                aLR.SetLeft(pSet->GetLRSpace().GetLeft());
                aLR.SetRight(nWert);
                rDoc.Insert(rPaM,aLR);
                break;
            case AF_AbAb:              // Absatzabstand Twips
                aUL.SetUpper(nWert);
                aUL.SetLower(pSet->GetULSpace().GetLower());
                rDoc.Insert(rPaM,aUL);
                break;
            case AF_Form:              // Absatzformat Enum
                rDoc.Insert(rPaM,SvxAdjustItem(
                    TransAdj((SATZTYP)pCtrl[2])));
                break;
            case AF_SpAb:              // Spaltenabstand Twips
                break;
            case AF_SpAn:              // Spaltenanzahl Byte
                break;
            case AF_SAbs:              // Spaltenabsatz Bool
                break;
            case AF_Auto:              // Durchschuss Bool
                nWert=(pSet->GetLineSpacing().GetInterLineSpaceRule()
                    ==SVX_INTER_LINE_SPACE_OFF) ?
                    pSet->GetLineSpacing().GetInterLineSpace():
                    pSet->GetLineSpacing().GetLineHeight();

                aLSpc.SetLineHeight(nWert);
                if (pCtrl[2]!='\0')
                {
                    aLSpc.SetLineHeight(0);
                    aLSpc.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;
                    aLSpc.SetInterLineSpace(nWert);
                }
                rDoc.Insert(rPaM,aLSpc);
                break;
            case AF_ZlAb:              // Zeilenabstand Twips
                aLSpc.SetLineHeight(nWert);
                if (pSet->GetLineSpacing().GetInterLineSpaceRule()
                    !=SVX_INTER_LINE_SPACE_OFF)
                {
                    aLSpc.SetLineHeight(0);
                    aLSpc.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;
                    aLSpc.SetInterLineSpace(nWert);
                }
                rDoc.Insert(rPaM,aLSpc);
                break;
            case AF_Glied:             // Gliederungslay Enum
                break;
            case AF_Tab:               // Tabulator alt Struct
            case AF_TabN:              // Tabulator neu Struct
                {
                    TABU aTab;         // Ein SW6-Tab
                    SvxTabStopItem aTabs(0,0); // Eine neue Tab-Tabelle

                    aTab.TabArt=(TABARTEN)(pCtrl[2]);
                    aTab.TabPos=SVBT16ToShort(&pCtrl[3]);
                    aTab.TabZch=(pCtrl[1]==AF_TabN)?pCtrl[5]:'\0';
                    if (aTab.TabArt>Tab_D)
                    {
                        aTab.TabZch='.';
                        aTab.TabArt=(TABARTEN)(aTab.TabArt & 3);
                    }
                    for (USHORT n=0; n<pSet->GetTabStops().Count(); n++)
                    {
                        aTabs.Insert(pSet->GetTabStops()[n]);
                    }
                    InsertTab(pSet->GetTabStops().Count(),aTab,
                        aTabs,pSet->GetLRSpace().GetTxtLeft());
                    rDoc.Insert(rPaM,aTabs);
                    break;
                }
            case AF_Zus:               // Zusammenhalten
                rDoc.Insert(rPaM,SvxFmtSplitItem(pCtrl[2]));
                break;
        } // switch
    }
}

void Sw6Layout::InsertTOX(SwDoc &rDoc,SwPaM &rPaM,
           USHORT nLen,HFORM *pTmp,TOXTypes eTyp)
// Kramt vom HFORM Stack den Rest eines
// Inhalts bzw. Indexeintrages zusammen
{
    const USHORT nToxID=0;                       // Leider nur ToxID==0

    if (!rDoc.GetTOXTypeCount(eTyp))             // Noch keine TOX
    {                                            // da: mache neue
        rDoc.InsertTOXType(SwTOXType(            // Die Namen sind leider
            eTyp,
            String::CreateFromAscii(
                eTyp==TOX_INDEX?         // hart Kodiert, weil der
                "Stichwortverzeichnis":              // ctor zu bloed ist...
                "Inhaltsverzeichnis")));
    }

    String aEntry;                               // TOX Entrystring
    USHORT nMzAnz=0;                             // Anzahl markiert Zeichen

    if (pTmp->cDat[0]<' ')                       // Kein Textbefehl?
    {                                            // Suche das Ende
        HFORM *pScd=pTmp->pNxt;                  // der Markierung
        USHORT nKor=0;                           // Korrektur fuer Felder

        while (pScd && !nMzAnz)                  // oder evt. auch
        {                                        // {INDEX-Befehle}
            nKor+=pScd->nXtd;
            if (pScd->cDat[0]<' ')
            {
                if (pTmp->cDat[0]==pScd->cDat[0])
                {
                    nMzAnz=pScd->nOfs-pTmp->nOfs-nKor;
                    pScd->cDat[0]='@';           // HForm ungueltig
                }
            }
            else
            {   if ((pScd->cDat[0]=='V' ||
                     pScd->cDat[0]=='W') &&
                    ((pTmp->cDat[0]==InhKenn)    // Wenn Addon mittels Text-
                    ==(pScd->cDat[0]=='V')))     // befehl addiere aEntry+Bef
                {
                    aEntry+=String((sal_Char *)&pScd->
                        cDat[2],pScd->cDat[1]);
                    pScd->cDat[0]='@';           // HForm ungueltig
                }
            }
            pScd=pScd->pNxt;                     // Naechsten HFORM
        } // while
        if (!nMzAnz)
        {
            nMzAnz+=nLen-pTmp->nOfs;             // Default bis EndOfLine
        }
    }
    else
    {
        aEntry=String((sal_Char *)&pTmp->            // Einfach nur einen
            cDat[2],pTmp->cDat[1]);              // neuen Eintrag schreiben
    }
    SwTOXMark aMark(rDoc.                        // Mache eine neue TOX-Marke
        GetTOXType(eTyp,nToxID));                // und lasse sie von der
    if (aEntry.Len())                            // derzeitigen Position aus
    {                                            // nMzAnz Zeichen markieren
        aMark.SetAlternativeText(aEntry);
    }
    if (eTyp==TOX_CONTENT) aMark.SetLevel(1);    // Level muss man setzen!

    if( !aEntry.Len() )
    {
        rPaM.SetMark();
        rPaM.GetMark()->nContent += nMzAnz;
    }
    rDoc.Insert( rPaM, aMark );
    if( !aEntry.Len() )
        rPaM.DeleteMark();
}

extern void sw3io_ConvertFromOldField( SwDoc& rDoc, USHORT& rWhich,
                                        USHORT& rSubType, UINT32 &rFmt,
                                        USHORT nVersion );

BOOL Sw6Layout::InsertLine(SwDoc &rDoc,SwPaM &rPaM,String &rStg,BOOL bLast)
// Fuege eine neuen Zeile bei rPam ein
{
    if ((rStg.Len()==0 ||
            rDoc.Insert(rPaM, rStg)) &&
                (bLast || rDoc.SplitNode(*rPaM.GetPoint())))
    {
        if (aSta.pFrmt)                          // Gibt es harte Forminfo?
        {
            HFORM *pTmp=aSta.pFrmt;              // Klapper HForm-Stack durch

            SwPaM aPaM(rPaM);                    // NeuPaM, zeigt auf AbsAnf
            aPaM.DeleteMark();                   // Falls Mark noch gesetzt
            if (!bLast) aPaM.Move(               // Absatz zurueck falls ein
                fnMoveBackward,fnGoNode);        // Splitnode war

            while (pTmp)
            {
                USHORT nEnd=rDoc.GetNodes()[         // Merke jetzige Absatzlaenge
                    aPaM.GetPoint()->nNode]->
                    GetTxtNode()->GetTxt().Len();
                aPaM.GetPoint()->nContent=pTmp->nOfs;
                switch (pTmp->cDat[0])
                {
                    case TabZch:
                        {
                            ALAY *pTmpLay=pLay->FileAlay[aSta.nAlay];
                            if (pTmp->nOfs!=0 || // Autotab hier unnoetig
                                pTmpLay->GliedLay==0 ||
                                pTmpLay->LRand==pTmpLay->Einzug)
                            {
                                rDoc.Insert(aPaM,'\t');
                                HFORM *pScd=pTmp->pNxt;

                                while (pScd)     // Alle Zeichen verschieben
                                {                // sich weil ContentNode
                                    if (pScd->nOfs>pTmp->nOfs)
                                    {
                                        pScd->nOfs++;
                                    }
                                    pScd=pScd->pNxt;
                                }
                            }
                        }
                        break;
                    case PrGraf:
                        {
                            String    aTrb;      // Treibername
                            String    aNam; 	// Gemerkte Name falls Fehler
                            // Attribute fuer FlyFrames
                            SfxItemSet aFAtt(rDoc.GetAttrPool(),
                                        RES_FRMATR_BEGIN,
                                        RES_FRMATR_END-1 );
                            GRAFDESC  aGrf;

                            aGrf.Hoch =SVBT16ToShort(&pTmp->cDat[2]);
                            aGrf.Breit=SVBT16ToShort(&pTmp->cDat[4]);
                            aGrf.Rand =SVBT16ToShort(&pTmp->cDat[6]);
                            aGrf.Rahm =SVBT8ToByte(&pTmp->cDat[8]);
                            aGrf.Bund =(GBUNDTYP)SVBT8ToByte(&pTmp->cDat[9]);
                            aGrf.Form =(GRAFSTAT)SVBT8ToByte(&pTmp->cDat[12]);
                            aGrf.Flag =SVBT8ToByte(&pTmp->cDat[14]);
                            for (USHORT n=0;n<pTmp->cDat[15];n++)
                            {
                                aNam+=(sal_Char)pTmp->cDat[n+16];
                            }

                            // search the correct path of the graphic
                            aNam = ::binfilter::StaticBaseUrl::SmartRelToAbs( aNam );
                            if( !::binfilter::IsDocument( aNam ) )
                            {
                                String sFNm( INetURLObject( aNam ).getName() );
                                INetURLObject aURL( *pMyName );
                                aURL.setName( sFNm );
                                String sSrchGrf( aURL.GetMainURL(
                                                    INetURLObject::NO_DECODE ));
                                if( ::binfilter::IsDocument( sSrchGrf ) )
                                    aNam = sSrchGrf;
                                else
                                {
                                    SvtPathOptions aPathOpt;
                                    if( aPathOpt.SearchFile( sFNm,
                                                SvtPathOptions::PATH_MODULE ))
                                        aNam = sFNm;
                                }
                            }

#define MAX_BORDER_SIZE  210    // So Breit ist max. der Border
#define MAX_EMPTY_BORDER 1      // So Breit wenn kein Rahmen drum

                            SwTwips nMaxi=MAX_EMPTY_BORDER;
                            if ((aGrf.Rahm&0xf)>1)
                            {
                                SvxBoxItem     aFBox;
                                SwLineType   aLTyp=GetLineType(
                                    (aGrf.Rahm&0xf)==3?5:1);
                                Color aColor(COL_BLACK);
                                SvxBorderLine aBord(&aColor,aLTyp.
                                    nOut,aLTyp.nIn,aLTyp.nDist);

                                aFBox.SetLine (&aBord, BOX_LINE_RIGHT);
                                aFBox.SetLine (&aBord, BOX_LINE_BOTTOM);
                                aFBox.SetLine (&aBord, BOX_LINE_LEFT);
                                aFBox.SetLine (&aBord, BOX_LINE_TOP);
                                aFAtt.Put(aFBox);
                                nMaxi=MAX_BORDER_SIZE;
                            }
                            SvxULSpaceItem aUL;
                            SvxLRSpaceItem aLR;

                            aUL.SetUpper(USHORT(aGrf.Rand));
                            aUL.SetLower(USHORT(aGrf.Rand));
                            aLR.SetLeft (USHORT(aGrf.Rand));
                            aLR.SetRight(USHORT(aGrf.Rand));
                            aFAtt.Put(aUL);
                            aFAtt.Put(aLR);

                            if (aGrf.Breit<MINFLY+2*aGrf.Rand+nMaxi)
                                aGrf.Breit=MINFLY+2*aGrf.Rand+nMaxi;
                            if (aGrf.Hoch<MINFLY+2*aGrf.Rand+nMaxi)
                                aGrf.Hoch=MINFLY+2*aGrf.Rand+nMaxi;

                            aFAtt.Put(SwFmtFrmSize(ATT_FIX_SIZE,
                                aGrf.Breit+2*aGrf.Rand,
                                aGrf.Hoch+2*aGrf.Rand));

                            switch (aGrf.Bund)
                            {
                                case G_Nicht:
                                    aFAtt.Put(SwFmtHoriOrient(
                                        0,HORI_NONE));
                                    break;
                                case G_Rechts:
                                    aFAtt.Put(SwFmtHoriOrient(
                                        0,HORI_RIGHT));
                                    break;
                                case G_Zent:
                                    aFAtt.Put(SwFmtHoriOrient(
                                        0,HORI_CENTER));
                                    break;
                                default:
                                    aFAtt.Put(SwFmtHoriOrient(
                                        0,HORI_LEFT));
                                    break;
                            }

                            rDoc.Insert( aPaM, aNam, aTrb,NULL, &aFAtt, 0 );
                        }
                        break;
                    case PrFnote:
                    case PrEnote:
                        {
                            InsertNote(rDoc,aPaM,
                                SVBT16ToShort(&pTmp->cDat[1]),
                                pTmp->cDat[0]==PrFnote);

                            HFORM *pScd=pTmp->pNxt;        // Fnote ist Ctent
                            while (pScd)                   // Daher verschiebt
                            {                              // sich alle HForm
                                if (pScd->nOfs>pTmp->nOfs)
                                {
                                    pScd->nOfs++;
                                }
                                pScd=pScd->pNxt;
                            }
                        }
                        break;
                    case PrAttr:
                        {

                            HFORM *pScd=pTmp->pNxt;     // Suche dann das Pedant
                            USHORT nKor=0;              // Korrigiere nAdd zurueck

                            while (pScd && (pScd->cDat[0]!=PrAttr ||
                                pScd->cDat[1]!=pTmp->cDat[1]))
                            {
                                nKor+=pScd->nXtd;
                                pScd=pScd->pNxt;
                            }
                            if (pScd)
                            {
                                pScd->cDat[0]='@';      // Pedant ungueltig machen
                                nEnd=pScd->nOfs-nKor;   // Merken neuen Ende-Offs im Absatz
// Falls ueber AddHForm eine Korrektur (nAdd) fuer druckbare Zeichen an-
// gegeben wurde, muss diese Korrektur hier rueckgaengig gemacht werden,
// weil dieses "druckbare" Zeichen nun erst spaeter eingefuegt wird.
                            }
                            if (pTmp->nOfs<nEnd && pTmp->cDat[1]>0 &&
                                pTmp->cDat[1]<ATTRBITS_END)
                            {                           // Lohnt sonst nicht
                                SwPaM aTmpPaM(aPaM);    // Noch'n PaM
                                aTmpPaM.SetMark();
                                aTmpPaM.GetPoint()->    // Zum Ende Attribut
                                    nContent+=(nEnd-pTmp->nOfs);
                                SfxPoolItem *pFmt=TransAtt(
                                    (ATTRBITS)pTmp->cDat[1],
                                    (pLay->FileAlay[aSta.nAlay]->
                                    AAttr & (1<<pTmp->cDat[1]))==0,
                                    pLay->FileAlay[aSta.nAlay]->AFSiz*10);
                                if (pFmt)
                                {
                                    rDoc.Insert(aTmpPaM,*pFmt);
                                    delete pFmt;
                                }
                            }
                            break;
                        }
                    case PrFont:
                        {
                            SvxFontItem &rTmpFnt=(SvxFontItem &)pLay->
                                FileAlay[aSta.nAlay]->pFmtColl->GetFont();

                            FontFamily eFam=rTmpFnt.GetFamily();
                            FontPitch  ePit=rTmpFnt.GetPitch();
                            CharSet    eSet=rTmpFnt.GetCharSet();
                            String     aNam=rTmpFnt.GetFamilyName();
                            String     aSty=rTmpFnt.GetStyleName();

                            short nFnt=pTmp->cDat[1];

                            if (nFnt==0)
                            {
                                nFnt=pLay->FileAlay[aSta.nAlay]->AFNum;
                            }

                            SwPaM aTmpPaM(aPaM);           // Noch'n PaM
                            aTmpPaM.SetMark();             // Setze Markanf
                            aTmpPaM.GetPoint()->           // Zum Ende des
                                nContent+=(nEnd-           // Attributs
                                pTmp->nOfs);               // setzen.
                            TransFnt(nFnt,eFam,            // Font uebersetzen
                                ePit,eSet,aNam);
                            SetTrans(eSet==RTL_TEXTENCODING_IBM_850); // Evt. nicht trans
                            rDoc.Insert(aTmpPaM,SvxFontItem( // Nun Font ins Doc
                              eFam,aNam,aSty,ePit,RTL_TEXTENCODING_DONTKNOW));
                        }
                        break;
                    case PrSize:
                        {
                            short nSiz=pTmp->cDat[1];
                            if (nSiz==0)
                            {
                                nSiz=pLay->FileAlay[
                                    aSta.nAlay]->AFSiz;
                            }

                            SwPaM aTmpPaM(aPaM);           // Noch'n PaM
                            aTmpPaM.SetMark();             // Setze Markanf
                            aTmpPaM.GetPoint()->           // Zum Ende des
                                nContent+=(nEnd-           // Attributs
                                pTmp->nOfs);               // setzen.
                            rDoc.Insert(aTmpPaM,           // Dann Fontsize
                                SvxFontHeightItem(nSiz*10));     // am Doc einfgen
                        }
                        break;
                    case PrAForm:
                        InsertAForm(rDoc,aPaM,pTmp->cDat);
                        break;
                    case HardSp:                           // HardSpace
                        rDoc.Insert(aPaM, CHAR_HARDBLANK);
                        break;
                    case STrenn:                           // Weicher Trenner
                        rDoc.Insert(aPaM,CHAR_SOFTHYPHEN);
                        break;
                    case 'B':
                        if (pTmp->cDat[1]!='\0')           // Nur wenn Blay
                        {                                  // neu gueltig ist
                            HFORM *pScd=pTmp->pNxt;        // Suche immer das
                            USHORT nKor=0;                 // Gueltigkeitsende

                            while (pScd && pScd->cDat[0]!='B')
                            {
                                nKor+=pScd->nXtd;
                                pScd=pScd->pNxt;
                            }
                            if (pScd)                      // Sonst bis Absend
                            {
                                nEnd=pScd->nOfs-nKor;
                            }

                            SwPaM aTmpPaM(aPaM);           // Noch'n PaM
                            aTmpPaM.SetMark();             // Setze Markanf
                            aTmpPaM.GetPoint()->           // Zum Ende Blay
                                nContent+=(nEnd-pTmp->nOfs);
                            BYTE nTmp=pTmp->cDat[1];
                            rDoc.Insert(aTmpPaM,
                                SwFmtCharFmt((pLay->FileBlay[
                                  nTmp-1]->pCharFmt)));
                        }
                        break;
                    case NewPage:
                        rDoc.Insert(aPaM,SvxFmtBreakItem(SVX_BREAK_PAGE_BEFORE));
                        break;
                    case 'V':
                    case InhKenn:
                        InsertTOX(rDoc,aPaM,nEnd,pTmp,TOX_CONTENT);
                        break;
                    case 'X':
                    case IdxKenn:
                        InsertTOX(rDoc,aPaM,nEnd,pTmp,TOX_INDEX);
                        break;

#define InsField(TypeName,ResName,FieldSet) \
{ \
    TypeName aFld((TypeName##Type*) \
        rDoc.GetSysFldType(ResName)); \
    aFld.ChangeFormat(FieldSet); \
    rDoc.Insert(aPaM,SwFmtFld(aFld)); \
}
                    case 'A':
                        InsField(SwAuthorField,RES_AUTHORFLD,AF_NAME);
                        break;
                    case 'P':
                        InsField(SwFileNameField,RES_FILENAMEFLD,FF_PATH);
                        break;
                    case 'S':
                    {
                        SwPageNumberField aFld((SwPageNumberFieldType*)
                            rDoc.GetSysFldType(RES_PAGENUMBERFLD),PG_RANDOM,SVX_NUM_ARABIC);
                        rDoc.Insert(aPaM,SwFmtFld(aFld));
                        break;
                    }
                    case 'U':
                    {
                        USHORT nSubType = TIMEFLD, nWhich = RES_DATETIMEFLD;
                        UINT32 nFormat = (UINT32)TF_SSMM_24;
                        sw3io_ConvertFromOldField( rDoc, nWhich, nSubType,
                                                    nFormat, 0x0110 );

                        SwDateTimeField aFld((SwDateTimeFieldType*)rDoc.GetSysFldType(RES_DATETIMEFLD), TIMEFLD, nFormat);
                        rDoc.Insert(aPaM,SwFmtFld(aFld));
                        //InsField(SwTimeField,RES_TIMEFLD,TF_SSMM_24);
                        break;
                    }
                    case 'N':
                        InsField(SwFileNameField,RES_FILENAMEFLD,FF_NAME);
                        break;
                    case 'd':
                        {
                            SwDateFormat eDat=DF_SSYS;
                            switch (pTmp->cDat[1])
                            {
                                case 'Z':eDat=DF_LSYS; break;
                                case 'B':eDat=DF_LMON; break;
                                case 'M':eDat=DF_LMONTH; break;
                            }
                            USHORT nSubType = DATEFLD, nWhich = RES_DATETIMEFLD;
                            UINT32 nFormat = (UINT32)eDat;

                            sw3io_ConvertFromOldField( rDoc, nWhich, nSubType,
                                                    nFormat, 0x0110 );

                            SwDateTimeField aFld((SwDateTimeFieldType*)rDoc.GetSysFldType(RES_DATETIMEFLD), DATEFLD, nFormat);
                            rDoc.Insert(aPaM,SwFmtFld(aFld));
                            //InsField(SwDateField,RES_DATEFLD,eDat);
                        }
                        break;
                    case 'i':
                        {
                            SwDocInfoSubType eTyp=DI_TITEL;
                            switch (pTmp->cDat[1])
                            {
                                case 'T':eTyp=DI_THEMA; break;
                                case 'S':eTyp=DI_KEYS; break;
                            }
                            SwDocInfoField aFld((SwDocInfoFieldType*)
                                rDoc.GetSysFldType(RES_DOCINFOFLD),eTyp);
                            rDoc.Insert(aPaM,SwFmtFld(aFld));
                        }
                        break;
                    case 's':
                        {
                            SwDocStatSubType eSta=DS_PARA;
                            switch (pTmp->cDat[1])
                            {
                                case 'W':eSta=DS_WORD; break;
                                case 'S':eSta=DS_PAGE; break;
                            }
                            InsField(SwDocStatField,RES_DOCSTATFLD,eSta);
                        }
                        break;
                    case '-':
                    case '+':
                        if (pTmp->cDat[0]=='-' || pTmp->cDat[2]=='_')
                        {
                            SwHiddenTxtField aFld((SwHiddenTxtFieldType*)
                                rDoc.GetSysFldType(RES_HIDDENTXTFLD),
                                FALSE,aEmptyStr,String((sal_Char *)(
                                pTmp->cDat+2),pTmp->cDat[1]));
                            rDoc.Insert(aPaM,SwFmtFld(aFld));
                        }
                        else
                        {
                            SwDBField aFld((SwDBFieldType*)rDoc.
                                InsertFldType(SwDBFieldType(&rDoc, String(
                                (sal_Char *)(pTmp->cDat+2),pTmp->cDat[1]), SwDBData())));
                            aFld.ChangeFormat(UF_STRING);
                            rDoc.Insert(aPaM,SwFmtFld(aFld));
                        }
                        break;
                } // switch
                pTmp=pTmp->pNxt;
            } // while(pTmp)
        } // if(pFrm)
    }
    else return FALSE;                  // Insert oder SplitNode ging nicht
    return TRUE;                        // Ok.
}

Sw6Layout::Sw6Layout(SvStream &rInp,ULONG *pErrno,const String *pFileName): //$ istream
    Sw6File(rInp,2048,258,pErrno),pMyName(pFileName),
    pDocInfo(NULL), nLastFontMatch( USHRT_MAX ),
    lLin(0)
// Konstruktor der Klasse Sw6Layout
{
    pLay=new LAYDESC;                  // Neuer LAYDESC noetig
    Glieder[0]=FALSE;                  // Noch keine Gliederungen
    Glieder[1]=FALSE;
    Glieder[2]=FALSE;
    Glieder[3]=FALSE;
    Glieder[4]=FALSE;
}

Sw6Layout::~Sw6Layout(void)
// Destruktor Sw6Layout
{
    if (pLay) DELETEZ(pLay);            // LAYDESC wech
    DelHForm();                         // Loesche noch pFrm
}

// --- SwSw6Parser ---

BOOL SwSw6Parser::Ueberlese(const long Blk)
// Ueberlesen eines Writer-Dateiblocks
{
    long Nxt;
    String Stg;

    Nxt=lLin+Blk;
    while (lLin<Nxt)
    {
        if (!ReadLn(Stg)) return FALSE;
        if (Eof())
        {
            *pErr=ERR_SW6_UNEXPECTED_EOF;
            return FALSE;
        }
    }
    return TRUE;
}

BOOL SwSw6Parser::ReadDocInfo(void)
// Einlesen des DocInfo-Blocks
// Wird auch gleich ins Doc gegeben
{
    BOOL bRet = FALSE, bIsTrans = IsTrans();
    SetTrans( TRUE );

    short Idx;
    if (ReadLn(pDat->DocInfo.Autor) &&
        ReadLn(pDat->DocInfo.Kateg))
    {
        bRet = TRUE;
        pDat->DocInfo.Autor=pDat->DocInfo.Autor;
        pDat->DocInfo.Kateg=pDat->DocInfo.Kateg;

        for (Idx=0;Idx<4;Idx++)
        {
            String &rTmp=pDat->DocInfo.Thema[Idx];
            if (ReadLn(rTmp))
            {
                rTmp.EraseLeadingChars();
                rTmp.EraseTrailingChars();
            }
            else
            {
                bRet = FALSE;
                break;
            }
        }
        for (Idx=0;Idx<4;Idx++)
        {
            String &rTmp=pDat->DocInfo.Keys[Idx];
            if (ReadLn(rTmp))
            {
                rTmp.EraseLeadingChars();
                rTmp.EraseTrailingChars();
            }
            else
            {
                bRet = FALSE;
                break;
            }
        }
    }
    SetTrans( bIsTrans );
    return TRUE;
}

BOOL SwSw6Parser::CallParser()
// Wird von ReadDoc aufgerufen. Gibt FALSE zurueck wenn es
// einen Fehler gab. Ungeklaert ob Stream noch geschlossen
// werden muss, und wer wie die Fehlermeldungen ausgibt...
{
    short  Idx;         // Index
    short  Anz;         // Anzahl der Bloecke
    long   Blk[20];     // Zeilenzahl je Block
    sal_Char   Ver;         // Versionskennung Datei
    String Stg;         // Zum Einlesen


    if (Sw6LayoutOk() && pDat!=NULL)    // Sonst gibt Aerger!!!
    {
        lLin=0;                         // Wieder von vorne
        Seek(0);
        if (!ReadLn(Stg))
            return FALSE; // Lese Dateikennung
        Ver = Stg.GetChar( ByteString(DOKUKENN).Search('#') );
        Stg.SetChar((ByteString(DOKUKENN).Search('#')), '#');
        if( Stg.EqualsAscii(DOKUKENN) )
        {
            if (!ReadLn(pDat->LayName)) // Name Layoutdatei
                return FALSE;
            if (!ReadLn(Stg))           // Druckerkennung
                return FALSE;
            while (Stg.Search('\\')!=STRING_NOTFOUND)
            {
                Stg.Erase(0,Stg.Search('\\')+1);
            }                           // Nur den GPM-Dateinamen bitte
            SetGpmFile(Stg);            // Melde richtige Gpmdatei an

            if (!ReadLn(Stg))           // Anzahl Bloecke
                return FALSE;
            if (0==(Anz=Stg.ToInt32()))
            {
                *pErr = ERR_SWG_FILE_FORMAT_ERROR;
                return FALSE;           // Es muessen mehrere Bloecke da sein
            }
            for (Idx=0; Idx<Anz; Idx++)
            {                           // Lese Zeilen ein
                if (!ReadLn(Stg))
                    return FALSE;
                Blk[Idx]=Stg.ToInt32();
            }
            if (Anz>11 && Blk[11]!=0)
            {
                *pErr=ERR_SW6_PASSWD;
                return FALSE;           // Passwort-Dateien kann ich nicht
            }
            if (Anz>0)                  // LeseHistInfo
            {
                if (!Ueberlese(Blk[0])) return FALSE;
            }
            if (Anz>1)                  // LeseDocInfo
            {
                if (ReadDocInfo())
                {
                    SfxDocumentInfo aInfo(*pDoc->GetInfo());
                    SfxStamp aTmp;
                    if (pDat->DocInfo.Autor.Len())
                    {
                        aTmp=aInfo.GetCreated();
                        aTmp.SetName(pDat->DocInfo.Autor);
                        aInfo.SetCreated(aTmp);
                    }
                    if (pDat->DocInfo.Kateg.Len())
                    {
                        aInfo.SetTitle(pDat->DocInfo.Kateg);
                    }
                    short n;
                    for (n=0; n<4; n++)
                    {
                        if (pDat->DocInfo.Thema[n].Len())
                        {
                            String s(aInfo.GetTheme());
                            if (s.Len() &&
                                s.GetChar( s.Len() ) !=' ' &&
                                s.GetChar( s.Len() ) !='-')
                            {
                                s.Insert(' ');
                            }
                            s+=pDat->DocInfo.Thema[n];
                            aInfo.SetTheme(s);
                        }
                    } // for n
                    for (n=0; n<4; n++)
                    {
                        if (pDat->DocInfo.Keys[n].Len())
                        {
                            String s(aInfo.GetKeywords());
                            if (s.Len() &&
                                s.GetChar( s.Len() )!=' ' &&
                                s.GetChar( s.Len() )!='-')
                            {
                                s.Insert(' ');
                            }
                            s+=pDat->DocInfo.Keys[n];
                            aInfo.SetKeywords(s);
                        }
                    } // for n
                    pDoc->SetInfo(aInfo);
                }
                else return FALSE;
            }
            if (Anz>2)                  // LeseLayInfo
            {
                if (!LeseLayInfo(Ver==AVERSION)) return FALSE;
            }
            if (Anz>3)                  // ReadExtra(FNotenlink)
            {
                MerkeNoten(FNot);
                if (!IgnExtra(TRUE)) return FALSE;
            }
            if (Anz>4)                  // ReadExtra(ENotenLink)
            {
                MerkeNoten(ENot);
                if (!IgnExtra(TRUE)) return FALSE;
            }
            if (Anz>5)                  // ReadExtra(ANotenLink)
            {
                MerkeNoten(ANot);
                if (!IgnExtra(TRUE)) return FALSE;
            }
            if (Anz>6 && Blk[6])        // ReadHartKopf
            {
                if (!LeseKoFu(TRUE)) return FALSE;
            }
            if (Anz>7 && Blk[7])        // ReadHartFuss
            {
                if (!LeseKoFu(FALSE)) return FALSE;
            }
            for (Idx=8; Idx<Anz; Idx++)
            {                           // Sonstige Bloecke ueberlesen
                if (!Ueberlese(Blk[Idx])) return FALSE;
            }
            InsertLayouts(*pDoc,pPaM);  // Layouts in das Dokument schieaen
            ResetLay(*pDoc);            // Ab hier wird Haupttext gelesen
            if (Anz>11 && Blk[11]!=0)
            {
                if (!ActivatePass() ||
                    !ReadLn(Stg) || !Stg.EqualsAscii("PASS"))
                {
                    *pErr=ERR_SW6_PASSWD;
                    return FALSE;
                }
            }
            do {                        // Zeile Haupttext lesen
                SetAlayTrans();
                if (!ReadLn(Stg)) return FALSE;
                pDoc->SetTxtFmtColl(*pPaM,GetAlay());
                if (TstSlay()) SetSlay(*pDoc,*pPaM);
                if (!InsertLine(*pDoc,*pPaM,Stg,Eof()))
                {
                    return FALSE;
                }
            } while (!Eof());           // Rest der Datei bis eof()
            *pErr = 0;
            return TRUE;
        }
        else
        {
            *pErr=ERR_SW6_NOWRITER_FILE;
        }
    }
    return FALSE;
}

SwSw6Parser::SwSw6Parser(SwDoc *pSwDoc,const SwPaM &rSwPaM,
                         SvStream *pIstream,BOOL bNewDoc,
                         ULONG *pErrno,const String *pFileName):
        pDoc(pSwDoc),bNew(bNewDoc),
        Sw6Layout(*pIstream,pErrno,pFileName)

// Konstruktor wie ihn der SWG benoetigt
{
    pDat=new DATEIDESC;
    pPaM=new SwPaM( *(rSwPaM.GetPoint() ) );
    SetDocInfo(&pDat->DocInfo);

    SwTxtNode *pTxtNode=0;

    // Ist das ein SwPaM auf einen TextNode?
    if (0==(pTxtNode=pDoc->GetNodes()[pPaM->GetPoint()->nNode]->GetTxtNode()))
    {
        // Ist keiner, dann machen wir uns einen
        pDoc->GetNodes().MakeTxtNode(
                pPaM->GetPoint()->nNode,
            (SwTxtFmtColl*)pDoc->GetDfltTxtFmtColl(), 0 );
        // Da zeigt jetzt auch der StartIdx hin
        pPaM->GetPoint()->nNode-=1;
    }
}

SwSw6Parser::~SwSw6Parser(void)
// Destructor von SwSw6Parser
{
    if (pDat) delete pDat;
    if (pPaM) delete pPaM;
}

ULONG Sw6Reader::Read( SwDoc &rDoc,SwPaM &rPam,const String &rFileName)
{
    ULONG nRet = ERR_SWG_READ_ERROR;
#ifndef COMPACT

    if( pStrm )
    {
        //JP 18.01.96: Alle Ueberschriften sind normalerweise ohne
        //              Kapitelnummer. Darum hier explizit abschalten
        //              weil das Default jetzt wieder auf AN ist.
        if( !bInsertMode )
            SetNoOutlineNum( rDoc );

        SwSw6Parser *pSw6Parser = new SwSw6Parser( &rDoc, rPam, pStrm,
                                        !bInsertMode, &nRet, &rFileName );
        pSw6Parser->CallParser();
        delete pSw6Parser;
    }
    else
        ASSERT( !this, "SW6-Read ohne Stream" );
#endif
    return nRet;
}



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
