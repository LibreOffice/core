/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _LWPOBJTAGS_HXX_
#define _LWPOBJTAGS_HXX_

// file header object
#define TAG_AMI             0x3750574CUL        // "LWP7"

// layout sub-objects
#define TAG_ENDSUBOBJ       0x646E4553UL        // "SEnd"
#define TAG_LAY_GEOMETRY    0x6F65474CUL        // "LGeo"
#define TAG_LAY_OFFSET      0x66664F4CUL        // "LOff"
#define TAG_LAY_SCALE       0x6C63534CUL        // "LScl"
#define TAG_LAY_MARGINS     0x72614D4CUL        // "LMar"
#define TAG_LAY_RELATIVITY  0x6C65524CUL        // "LRel"
#define TAG_LAY_COLUMNS     0x6C6F434CUL        // "LCol"
#define TAG_LAY_BORDER      0x726F424CUL        // "LBor"
#define TAG_LAY_GUTTER      0x7475474CUL        // "LGut"
#define TAG_LAY_JOIN        0x696F4A4CUL        // "LJoi"
#define TAG_LAY_BACKGROUND  0x6B63424CUL        // "LBck"
#define TAG_LAY_SHADOW      0x7764534CUL        // "LSdw"
#define TAG_LAY_LEADERDOTS  0x72644C4CUL        // "LLdr"
#define TAG_LAY_NUMERICS    0x6D754E4CUL        // "LNum"
#define TAG_LAY_HIERARCHY   0x6569484CUL        // "LHie"

// Paragraph properties
#define TAG_PARA_SHOW       0x65644850UL        // "PHde"
#define TAG_PARA_HIDE       0x77685350UL        // "PShw"
#define TAG_PARA_ALIGN      0x6e6c4150UL        // "PAln"
#define TAG_PARA_INDENT     0x74644950UL        // "PIdt"
#define TAG_PARA_SPACING    0x63705350UL        // "PSpc"
#define TAG_PARA_BORDER     0x64724250UL        // "PBrd"
#define TAG_PARA_BACKGROUND 0x6b614250UL        // "PBak"
#define TAG_PARA_BREAKS     0x6b724250UL        // "PBrk"
#define TAG_PARA_BULLET     0x746c4250UL        // "PBlt"
#define TAG_PARA_NUMBERING  0x6d754e50UL        // "PNum"
#define TAG_PARA_TAB        0x62615450UL        // "PTab"
#define TAG_PARA_KINSOKU    0x6e694b50UL        // "PKin"
#define TAG_PARA_PROPLIST   0x70725050UL        // "PPrp"

#define TAG_SLVLIST         0x734C4C53UL        // "SLLs"
#define TAG_DLFVLIST        0x4C464C44UL        // "DLFL"
#define TAG_DLVLISTHEAD     0x64484C44UL        // "DLHd"
#define TAG_DLNFVLIST       0x464E4C44UL        // "DLNF"
#define TAG_DLVLIST         0x734C4C44UL        // "DLLs"

#define TAG_DOCUMENT        0x20636F44UL        // "Doc "
#define TAG_VERDOCUMENT     0x636F4456UL        // "VDoc"
#define TAG_DOCDATA         0x44636F44UL        // "DocD"
#define TAG_OBJINDEX        0x78644E4FUL        // "ONdx"
#define TAG_DIVISIONINFO    0x49766944UL        // "DivI"
#define TAG_CONTENT         0x746E6F43UL        // "Cont"
#define TAG_HEADCONTENT     0x746E4348UL        // "HCnt"
#define TAG_STORY           0x79727453UL        // "Stry"
#define TAG_TABLE           0x6C626154UL        // "Tabl"
#define TAG_SUPERTABLE      0x62547553UL        // "SuTb"
#define TAG_GLOSSARY        0x736F6C47UL        // "Glos"
#define TAG_PARALLELCOLUMNS 0x6C6F4350UL        // "PCol"
#define TAG_PARALLELCOLUMNSLAYOUT   0x794C4350UL    // "PCLy"
#define TAG_PARALLELCOLUMNSBLOCK    0x6C424350UL    // "PCBl"
#define TAG_PARALLELCOLUMNSCONNECTEDBLOCK   0x42434350UL    // "PCCB"
#define TAG_FOOTNOTETABLE   0x62546E46UL        // "FnTb"
#define TAG_PARAGRAPH       0x61726150UL        // "Para"
#define TAG_BOOKMARK        0x6B4D6B42UL        // "BkMk"
#define TAG_DDELINK         0x4C656444UL        // "DdeL"
#define TAG_VIRTUALLAYOUT   0x79614C56UL        // "VLay"
#define TAG_LITELAYOUT      0x79614C4CUL        // "LLay"
#define TAG_LAYOUT          0x2079614CUL        // "Lay "
#define TAG_PERMISSIVELAYOUT    0x79614C50UL    // "PLay"
#define TAG_HEADLAYOUT      0x79614C48UL        // "HLay"
#define TAG_FRAMELAYOUT     0x6D617246UL        // "Fram"
#define TAG_DROPCAPLAYOUT   0x70614344UL        // "DCap"
#define TAG_NOTELAYOUT      0x794C744EUL        // "NtLy"
#define TAG_NOTETEXTLAYOUT  0x6C54744EUL        // "NtTl"
#define TAG_NOTEHEADERLAYOUT 0x6C48744EUL       // "NtHl"
#define TAG_PAGELAYOUT      0x65676150UL        // "Page"
#define TAG_HEADERLAYOUT    0x4C726448UL        // "HdrL"
#define TAG_FOOTERLAYOUT    0x4C727446UL        // "FtrL"
#define TAG_ROWLAYOUT       0x4C776F52UL        // "RowL"
#define TAG_COLUMNLAYOUT    0x4C6C6F43UL        // "ColL"
#define TAG_FNROWLAYOUT     0x4C526E46UL        // "FnRL"
#define TAG_CELLLAYOUT      0x4C6C6543UL        // "CelL"
#define TAG_VIEWPORTLAYOUT  0x794C7056UL        // "VpLy"
#define TAG_CONNECTEDCELLLAYOUT 0x4C436E43UL    // "CnCL"
#define TAG_HIDDENCELLLAYOUT    0x4C436E48UL    // "HnCL"
#define TAG_TABLEHEADINGLAYOUT  0x4C486254UL    // "TbHL"
#define TAG_ROWHEADINGLAYOUT    0x4C486F52UL    // "RoHL"
#define TAG_FNCELLLAYOUT    0x4C436E46UL        // "FnCL"
#define TAG_FOOTNOTELAYOUT  0x794C6E46UL        // "FnLy"
#define TAG_TABLELAYOUT     0x794C6254UL        // "TbLy"
#define TAG_SUPERTABLELAYOUT    0x4C625453UL    // "STbL"
#define TAG_FNSUPERTABLELAYOUT  0x4C545346UL    // "FSTL"
#define TAG_ENSUPERTABLELAYOUT  0x4C545345UL    // "ESTL"
#define TAG_SUPERPARALLELCOLUMNLAYOUT   0x4C435053UL    // "SPCL"
#define TAG_SUPERGLOSSARYLAYOUT 0x534C4753UL    // "SGLS"
#define TAG_CONTONLAYOUT    0x4C6E4F43UL        // "COnL"
#define TAG_CONTFROMLAYOUT  0x4C724643UL        // "CFrL"
#define TAG_CHARSTYLE       0x79745343UL        // "CSty"
#define TAG_PARASTYLE       0x79745350UL        // "PSty"
#define TAG_TABRACK         0x20626154UL        // "Tab "
#define TAG_GROUPLAYOUT     0x4c707247UL        // "GrpL"
#define TAG_GROUPFRAME      0x46707247UL        // "GrpF"
#define TAG_SILVERBULLET    0x6c754253UL        // "SBul"
#define TAG_TABLEHEADING    0x486c6254UL        // "TblH"
#define TAG_LINEHINT        0x746e484cUL        // "LHnt"
#define TAG_TABLEHINT       0x746e4854UL        // "THnt"
#define TAG_PAGEHINT        0x746e4850UL        // "PHnt"
#define TAG_NUMBERHINT      0x746e484EUL        // "NHnt"
#define TAG_FOOTNOTESEEN    0x65655346UL        // "FSee"
#define TAG_OBJECTHOLDER    0x646c6F48UL        // "Hold"
#define TAG_LISTLIST        0x744c744cUL        // "LtLt"
#define TAG_FOOTNOTELISTLIST 0x744c7446UL       // "FtLt"
#define TAG_ENDNOTELAYOUT   0x794C6E45UL        // "EnLy"
#define TAG_FOOTNOTE        0x746F4E46UL        // "FNot"
#define TAG_CELLNUMBERHINT  0x74486e43UL        // "CnHt"
#define TAG_MARKER          0x6b72614DUL        // "Mark"
#define TAG_STORYMARKER     0x6b724d53UL        // "SMrk"
#define TAG_TABLEMARKER     0x6b724d54UL        // "TMrk"
#define TAG_FIELDMARKER     0x4c724d54UL        // "FMrk"
#define TAG_NUMERICVALUE    0x6c61564eUL        // "NVal"
#define TAG_TEXTVALUE       0x6c615654UL        // "TVal"
#define TAG_REFERENCEVALUE  0x6c615652UL        // "RVal"
#define TAG_CELLLIST        0x64734c43UL        // "CLst"
#define TAG_DEPENDENT       0x74647044UL        // "Dpdt"
#define TAG_FORMULAINFO     0x616c7246UL        // "Frla"
#define TAG_TABLERANGE      0x676e5254UL        // "TRng"
#define TAG_CELLRANGE       0x676e5243UL        // "CRng"
#define TAG_FOLDER          0x726c6446UL        // "Fldr"
#define TAG_ROWLIST         0x74734c52UL        // "RLst"
#define TAG_SECTION         0x74636553UL        // "Sect"
#define TAG_INDEXSECTION    0x63657349UL        // "Isec"
#define TAG_GRAPHIC         0x68707247UL        // "Grph"
#define TAG_DIRTBAG         0x74726944UL        // "Dirt"
#define TAG_NAMEDOUTLINESEQ 0x71534f4eUL        // "NOSq"
#define TAG_OUTLINESEQITEM  0x7449534fUL        // "OSIt"
#define TAG_TOCSUPERTABLELAYOUT 0x6C747354UL    // "Tstl"
#define TAG_DOCSOCKET       0x786F5344UL        // "DSox"
#define TAG_USER_VERSION    0x72655655UL        // "UVer"
#define TAG_NOTE            0x65746F4EUL        // "Note"
#define TAG_MARKERLIST      0x74734c4dUL        // "MLst"
#define TAG_BOOKMARKMARKERLIST  0x744c4d42UL    // "BMLt"
#define TAG_DDEMARKMARKERLIST   0x744c4d44UL    // "DMLt"
#define TAG_FIELDMARKERLIST 0x744c6c46UL        // "FlLt"
#define TAG_CHBLKMARKER     0x6b4d4843UL        // "CHMk"
#define TAG_CHBMARKERLIST   0x424d4c43UL        // "CMLt"
#define TAG_PPTRHOLDER      0x72745070UL        // "pPtr"
#define TAG_QPTRHOLDER      0x72745071UL        // "qPtr"
#define TAG_GPTRHOLDER      0x72745067UL        // "gPtr"
#define TAG_PROPLIST        0x706f7250UL        // "Prop"
#define TAG_HEADHOLDER      0x646C4848UL        // "HHld"
#define TAG_HEADTAILHOLDER  0x72485448UL        // "HTHr"
#define TAG_OLEOBJECT       0x4F656C4FUL        // "OleO"
#define TAG_ENUMOBJHD       0x64484F45UL        // "EOHd"
#define TAG_TOCLEVELDATA    0x4C6f6354UL        // "TocL"
#define TAG_VERPTR          0x72745056UL        // "VPtr"
#define TAG_VERQTR          0x72745156UL        // "VQtr"
#define TAG_VERDOCINFO      0x6E494456UL        // "VDIn"
#define TAG_TOMBSTONE       0x20504952UL        // "RIP "
#define TAG_NOTIFY          0x7966744EUL        // "Ntfy"
#define TAG_LONGHOLDER      0x676E6F4CUL        // "Long"

#define TAG_DIVOPTS         0x74704f44UL        // "DOpt"
#define TAG_FOOTNOTEOPTS    0x704f4e46UL        // "FNOp"

#ifdef RUBY
#define TAG_RUBYLAYOUT      0x794C6252UL        // "RbLy"
#define TAG_RUBYMARKER      0x6b724d52UL        // "RMrk"
#define TAG_RUBYMARKERLIST  0x744c6252UL        // "RbLt"
#endif

#define TAG_ALIGNMENTPIECE          0x63506c41UL    // "AlPc"
#define TAG_INDENTPIECE             0x63506e49UL    // "InPc"
#define TAG_PARABORDERPIECE         0x63504250UL    // "PBPc"
#define TAG_PARABACKGROUNDPIECE     0x63505250UL    // "PRPc"
#define TAG_SPACINGPIECE            0x63507053UL    // "SpPc"
#define TAG_BREAKSPIECE             0x63507242UL    // "BrPc"
#define TAG_NUMBERINGPIECE          0x6350724eUL    // "NrPc"
#define TAG_CHARBORDERPIECE         0x63504243UL    // "CBPc"
#define TAG_AMIKAKEPIECE            0x63504b41UL    // "AKPc"
#define TAG_TABPIECE                0x63506254UL    // "TbPc"

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
