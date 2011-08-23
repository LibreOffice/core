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
/*************************************************************************
 * @file
 *  definitions for all object types VO_TYPE
 ************************************************************************/
/*************************************************************************
 * Change History
 Jan 2005			Created
 ************************************************************************/

#ifndef _LWPDEFS_HXX
#define _LWPDEFS_HXX

#define BAD_OFFSET 0xFFFFFFFFUL
#define BAD_ATOM ((sal_Int32)(-1))				// Invalid Atom ID

//Please remove the item from the commented defines after you add the tag to the following enum.
enum VO_TYPE
{
    VO_PARA = 0,
    VO_DOCUMENT = 5,
    VO_STORY = 13,
    VO_VERQTR = 40,
    VO_HEADCONTENT = 47,
    VO_HEADLAYOUT = 142,
    VO_PAGELAYOUT	= 143,
    VO_DOCSOCK = 155,
    VO_DIVISIONINFO = 7,
    VO_HEADHOLDER	 = 90,
    VO_DIVOPTS	= 176,
    VO_DOCDATA = 137,
    VO_HEADERLAYOUT = 139,
    VO_FOOTERLAYOUT =  140,
    VO_FRAMELAYOUT	= 141,
    VO_TOCSUPERTABLELAYOUT = 149,
    VO_TOCLEVELDATA =	171,
    VO_LAYOUTGEOMETRY	= 185,
    VO_LAYOUTSCALE	= 186,
    VO_LAYOUTMARGINS = 187,
    VO_LAYOUTCOLUMNS = 188,
    VO_LAYOUTBORDERSTUFF = 189,
    VO_LAYOUTGUTTERSTUFF = 190,
    VO_LAYOUTBACKGROUNDSTUFF = 191,
    VO_LAYOUTJOINSTUFF = 192,
    VO_LAYOUTSHADOW = 193,
    VO_LAYOUTNUMERICS = 194,
     VO_LAYOUTRELATIVITY	 = 195,
    VO_LAYOUTEXTERNALBORDERSTUFF = 219,
    VO_GRAPHIC = 41,
    VO_VERGTR =	205,

    //  01/12/2005
    VO_PARASTYLE = 23,
    VO_CHARACTERSTYLE = 106,
    VO_SILVERBULLET = 127,
    VO_ALIGNMENTPIECE = 196,
    VO_INDENTPIECE = 197,
    VO_SPACINGPIECE = 199,
    VO_CHARBORDERPIECE = 202,
    VO_AMIKAKEPIECE = 203,
    // end

    // start
    VO_TABRACK = 38,
    VO_PARABORDERPIECE = 198,
    VO_BREAKSPIECE = 200,
    VO_NUMBERINGPIECE = 201,
    VO_TABPIECE	= 204,
    VO_PARABACKGROUNDPIECE = 218,
    // end
    // 02/01/2005
    VO_SECTION = 136,
    VO_INDEXSECTION = 48,
    VO_HEADTAILHOLDER = 169,
    VO_PAGEHINT	= 10,
    VO_OBJECTHOLDER = 98,
    VO_POBJECTHOLDER = 133,
    //  added for OLE
    VO_OLEOBJECT = 54,
    // end OLE

    //  added for table
    VO_TABLE =				17,
    VO_CELLLAYOUT	=		18,
    VO_SUPERTABLELAYOUT	=	109,
    VO_TABLELAYOUT	=		31,
    VO_SUPERTABLE		=	115,
    VO_ROWLAYOUT		=	32,
    VO_ROWLIST			=	76,
    VO_CELLLIST		=	74,
    VO_NUMERICVALUE	=	123,
    VO_FORMULAINFO	=	82,
    VO_COLUMNLAYOUT	=	35,
    VO_TABLERANGE		=	66,
    VO_CELLRANGE		=	67,
    VO_FOLDER			=	75,
    VO_DEPENDENT		=	77,
    VO_CONNECTEDCELLLAYOUT =	28,
    VO_HIDDENCELLLAYOUT	=	29,
    VO_TABLEHEADINGLAYOUT =	117,
    VO_ROWHEADINGLAYOUT	=	118,
    VO_TABLEHEADING	=		209,
    VO_TABLEHINT		=	19,
    // end table

    //add by
    VO_DROPCAPLAYOUT = 217,
    VO_CHBLKMARKER	= 94,
    VO_PROPLIST = 165,
    VO_BOOKMARK = 64,
    VO_FIELDMARKER = 87,
    //add end

    //for footnote
    VO_FOOTNOTE = 53,
    VO_FOOTNOTEOPTS = 177,
    VO_FOOTNOTELAYOUT = 65,
    VO_FNROWLAYOUT = 59,
    VO_FNCELLLAYOUT = 61,
    VO_ENDNOTELAYOUT = 138,
    VO_ENSUPERTABLELAYOUT = 113,
    VO_FNSUPERTABLELAYOUT = 111,
    VO_FOOTNOTETABLE = 58,
    VO_CONTONLAYOUT = 145,
    VO_CONTFROMLAYOUT = 146,

    //for frame group
    VO_GROUPLAYOUT = 108,
    VO_GROUPFRAME = 173,

    //For index manager
    VO_ROOTLEAFOBJINDEX = 0xFFFB,		// Root Index with no kids
    VO_ROOTOBJINDEX = 0xFFFC,			// Root Index with kids

    //Add new type of index
    VO_OBJINDEX = 0xFFFD,				// Obj Index with kids

    VO_LEAFOBJINDEX	 = 0xFFFE	,		// Leaf Index

    VO_LISTLIST = 100,

    //For comment notes
    VO_NOTELAYOUT = 159,
    VO_NOTEHEADERLAYOUT = 178,
    VO_NOTETEXTLAYOUT = 166,
    VO_VPLAYOUT = 132,

    // for parallel column
    VO_SUPERPARALLELCOLUMNLAYOUT =	174,
    VO_PCOLLAYOUT	=		206,
    VO_PCOLBLOCK	=		207,
    VO_PARALLELCOLUMNS	=	163,

    //for ruby mark
    VO_RUBYMARKER	= 179,
    VO_RUBYLAYOUT = 180,

    VO_GLOSSARY = 157,
    VO_SUPERGLOSSARYLAYOUT = 175,
    //for default tab distance
    VO_VERDOCUMENT = 161,


    //The last one
    VO_INVALID = -1
};

/*
    static const VO_OBJINDEX	= 0xFFFD	;			// Intermediate Level Index
*/
/*
#define	VO_LAYOUT				1
#define	VO_NOTIFY				2
#define VO_LINE					3
#define VO_PRESENT				4
#define VO_PAGECONTAINER 		6
#define VO_COLUMN				8
#define VO_CELLGADGET			9
#define VO_PAGEHINT				10
#define VO_PANE					11
#define VO_SUBPAGECONTAINER		12
#define VO_TABSEG				14
#define VO_FRAMECONTAINER 		15
#define VO_LINEHINT				16
#define	VO_TABLE				17
#define VO_CELLLAYOUT			18
#define VO_TABLEHINT			19
#define VO_CELLCONTAINER		20
#define VO_TABLECONTAINER		21
#define VO_ROWCONTAINER			22

#define VO_COVERCELLCONTAINER	24
#define VO_HIDDENCELLCONTAINER	25
#define VO_SPACESHAPE			26
#define VO_TABLEHEADINGCONTAINER	27
#define VO_CONNECTEDCELLLAYOUT	28
#define VO_HIDDENCELLLAYOUT		29
#define VO_SUPERLINE			30
#define VO_TABLELAYOUT			31
#define VO_ROWLAYOUT			32
#define VO_TABLEROWCACHE		33
#define VO_TABLECOLUMNCACHE		34
#define VO_COLUMNLAYOUT			35
#define VO_HANDLENODE			36
#define VO_FORMATLISTNODE		37

#define VO_TEMPPAGEHINT			39

#define VO_COLLAPSEGADGET       42
#define VO_PERMISSIVELAYOUT     43
#define VO_ELEVATORGADGET		44
#define VO_STORYMARKER			45
#define VO_HEADCONTAINER		46


#define	VO_NAMEDOUTLINESEQ		49
#define VO_VIEWPORTCONTAINER	50
#define VO_PRESENTATIONINFO		51
#define VO_OBJECTINDEX			52

#define VO_OLEOBJECT            54
#define VO_FOOTNOTECONTAINER    55
#define VO_LITTLEPAGEGADGET		56
#define VO_OUTLINEBUTTONGADGET	57

#define VO_FNROWLAYOUT			59
#define VO_FNROWCONTAINER		60
#define VO_FNCELLLAYOUT			61
#define VO_FNCELLCONTAINER		62
#define VO_3DBUTTONGADGET		63
#define VO_BOOKMARK				64

#define VO_TABLERANGE			66
#define VO_CELLRANGE			67
#define VO_REVLINEGADGET		68
#define VO_REVCHARGADGET		69
#define VO_SCROLLBUTTONGADGET	70
#define VO_VSCROLLBARGADGET		71
#define VO_HSCROLLBARGADGET		72
#define VO_SCROLLBARGADGET		73
#define VO_CELLLIST				74
#define VO_FOLDER				75
#define VO_ROWLIST				76
#define VO_DEPENDENT			77
#define VO_NUMBERHINT			78
#define VO_MARKER				79
#define VO_NOTEGADGET			80
#define VO_DDELINK				81
#define VO_FORMULAINFO			82
#define VO_THUMBGADGET			83
#define VO_GHOSTTHUMBGADGET		84
#define VO_BOOKMARKGADGET		85
#define VO_LINKGADGET			86

#define VO_CHECKPOINT			88
#define VO_STORYCHECKPOINT		89

#define VO_TABLEMARKER			91
#define VO_TABLECHECKPOINT		92
#define VO_FOOTNOTELISTLIST		93
#define VO_CHBLKMARKER			94
#define VO_ROWGADGET			95
#define VO_COLUMNGADGET			96
#define VO_CORNERGADGET			97

#define VO_GRAPHICOLECHECKPOINT	99

#define VO_DEMANDLOADER			101
#define VO_COLUMNSIZEGADGET		102
#define VO_ROWSIZEGADGET		103
#define VO_CELLMOVEGADGET		104
#define VO_CELLSELECTIONGADGET	105
#define VO_CHARACTERSTYLE		106
#define VO_GROUPCONTAINER		107

#define VO_SUPERTABLELAYOUT		109
#define VO_SUPERTABLECONTAINER	110

#define VO_FNSUPERTABLECONTAINER	112

#define VO_ENSUPERTABLECONTAINER	114
#define VO_SUPERTABLE			115
#define VO_ROWCOLUMNSIZEGADGET	116
#define VO_TABLEHEADINGLAYOUT	117
#define VO_ROWHEADINGLAYOUT		118
#define VO_TOMBSTONE			119
#define VO_CAPTION				120
#define VO_VERCONTROL			121
#define VO_ANCHORGADGET			122
#define VO_NUMERICVALUE			123
#define VO_TEXTVALUE			124
#define VO_REFERENCEVALUE		125
#define VO_DLTREENODE			126

#define VO_PARADATAHOLDER		128
#define VO_NOTIFYPERSISTENT		129
#define VO_TYPVERCONTROL		130
#define VO_USRVERCONTROL		131
#define VO_POBJECTHOLDER		133
#define VO_CELLNUMBERHINT		134
#define VO_STXINFO				135		// SmarText object







#define VO_MARKERLIST			144

#define VO_NAMEDVERCONTROL		147
#define VO_DIRTBAG				148

#define VO_OBJPTRHOLDER			150
#define VO_COLUMNWIDTHCACHE		151
#define VO_NOTIFYLISTHOLDER		152
#define VO_OBJECTHOLDERNV		153
#define VO_BASELINEGADGET		154

#define VO_DLVOLISTHEADHOLDER	156
#define VO_DELETEDTEXTGADGET	158

#define VO_FOOTNOTESEEN			160

#define VO_OUTLINESEQITEM		162

#define VO_CHBGADGET			164
#define VO_PROPLIST				165
#define VO_STXMANAGER			167		// SmarText object
#define VO_STXLINK				168		// SmarText object

#define VO_VERPTR				170

#define VO_VERDOCINFO			172


#ifdef RUBY

#define VO_RUBYGADGET			181
#endif
#define VO_OLEGADGET			182
#define VO_AMIASWPSEUDOOBJHOLDER 183
#define VO_LONGHOLDER			184


#define VO_REVA_MAXTYPES		186



#define VO_CONNECTEDBLOCK		208
#define VO_TABLEHEADING			209
#define VO_BOOKMARKMARKERLIST	210
#define VO_FIELDMARKERLIST		211
#define VO_CHBMARKERLIST		212
#define VO_DDEMARKERLIST		213
#define VO_RUBYMARKERLIST		214
#ifdef RUBY
#define VO_RUBYMARKERGADGET		215
#endif
#define VO_DROPCAPCONTAINER		216



#define VO_LAYOUTEXTERNALJOINSTUFF		220


#define VO_OBJINDEX				0xFFFD			// Intermediate Level Index
*/

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
