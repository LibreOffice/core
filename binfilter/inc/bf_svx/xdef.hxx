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

#ifndef _XDEF_HXX
#define _XDEF_HXX

/*************************************************************************
|*
|* Attribut einfuegen: Zeile kopieren, VOR welcher eingefuegt werden
|* soll, Bezeichnung aendern und diese auch am Ende der darauffolgenden
|* Zeile eintragen
|*
\************************************************************************/
namespace binfilter {

#define XATTR_START             1000

#define XATTR_LINE_FIRST        XATTR_START              		/* V3: 1000  V2: 1000 */
#define XATTR_LINESTYLE         XATTR_LINE_FIRST
#define XATTR_LINEDASH          (XATTR_LINE_FIRST + 1)			/* V3: 1001  V2: 1001 */
#define XATTR_LINEWIDTH         (XATTR_LINE_FIRST + 2)			/* V3: 1002  V2: 1002 */
#define XATTR_LINECOLOR         (XATTR_LINE_FIRST + 3)			/* V3: 1003  V2: 1003 */
#define XATTR_LINESTART         (XATTR_LINE_FIRST + 4)			/* V3: 1004  V2: 1004 */
#define XATTR_LINEEND           (XATTR_LINE_FIRST + 5)			/* V3: 1005  V2: 1005 */
#define XATTR_LINESTARTWIDTH    (XATTR_LINE_FIRST + 6)			/* V3: 1006  V2: 1006 */
#define XATTR_LINEENDWIDTH      (XATTR_LINE_FIRST + 7)			/* V3: 1007  V2: 1007 */
#define XATTR_LINESTARTCENTER   (XATTR_LINE_FIRST + 8)			/* V3: 1008  V2: 1008 */
#define XATTR_LINEENDCENTER     (XATTR_LINE_FIRST + 9)   		/* V3: 1009  V2: 1009 */

// Neu am 07.11.95 KA (7 neue Whiches)
#define XATTR_LINETRANSPARENCE  (XATTR_LINE_FIRST + 10)  		/* V3: 1010  V2: 1010 */
#define XATTR_LINEJOINT		    (XATTR_LINE_FIRST + 11)			/* V3: 1011  V2: 1011 */

// stille Reserven... (auch neu am 07.11.95)
#define XATTR_LINERESERVED2     (XATTR_LINE_FIRST + 12)			/* V3: 1012  V2: 1012 */
#define XATTR_LINERESERVED3     (XATTR_LINE_FIRST + 13)			/* V3: 1013  V2: 1013 */
#define XATTR_LINERESERVED4     (XATTR_LINE_FIRST + 14)			/* V3: 1014  V2: 1014 */
#define XATTR_LINERESERVED5     (XATTR_LINE_FIRST + 15)			/* V3: 1015  V2: 1015 */
#define XATTR_LINERESERVED_LAST (XATTR_LINE_FIRST + 16)			/* V3: 1016  V2: 1016 */
#define XATTR_LINE_LAST         XATTR_LINERESERVED_LAST

#define XATTRSET_LINE           (XATTR_LINE_LAST + 1)    		/* V3: 1017  V2: 1017 */

#define XATTR_FILL_FIRST        	(XATTRSET_LINE + 1)      	/* V3: 1018  V2: 1018 */
#define XATTR_FILLSTYLE         	XATTR_FILL_FIRST
#define XATTR_FILLCOLOR         	(XATTR_FILL_FIRST + 1)		/* V3: 1019  V2: 1019 */
#define XATTR_FILLGRADIENT      	(XATTR_FILL_FIRST + 2)		/* V3: 1020  V2: 1020 */
#define XATTR_FILLHATCH         	(XATTR_FILL_FIRST + 3)		/* V3: 1021  V2: 1021 */
#define XATTR_FILLBITMAP        	(XATTR_FILL_FIRST + 4)   	/* V3: 1022  V2: 1022 */

// Neu am 07.11.95 KA (7 neue Whiches)
#define XATTR_FILLTRANSPARENCE  	(XATTR_FILL_FIRST + 5)   	/* V3: 1023  V2: 1023 */
#define XATTR_GRADIENTSTEPCOUNT 	(XATTR_FILL_FIRST + 6)   	/* V3: 1024  V2: 1024 */
#define XATTR_FILLBMP_TILE 			(XATTR_FILL_FIRST + 7)	 	/* V3: 1025  V2: 1025 */
#define XATTR_FILLBMP_POS   		(XATTR_FILL_FIRST + 8)	 	/* V3: 1026  V2: 1026 */
#define XATTR_FILLBMP_SIZEX 		(XATTR_FILL_FIRST + 9)	 	/* V3: 1027  V2: 1027 */
#define XATTR_FILLBMP_SIZEY			(XATTR_FILL_FIRST + 10)	 	/* V3: 1028  V2: 1028 */

// Reserve
#define XATTR_FILLFLOATTRANSPARENCE	(XATTR_FILL_FIRST + 11)	 	/* V3: 1029  V2: 1029 */
#define XATTR_FILLRESERVED2			(XATTR_FILL_FIRST + 12)	 	/* V3: 1030 */

#define XATTR_FILLBMP_SIZELOG		(XATTR_FILL_FIRST + 13)	 	/* V3: 1031 */
#define XATTR_FILLBMP_TILEOFFSETX	(XATTR_FILL_FIRST + 14)	 	/* V3: 1032 */
#define XATTR_FILLBMP_TILEOFFSETY	(XATTR_FILL_FIRST + 15) 	/* V3: 1033 */
#define XATTR_FILLBMP_STRETCH		(XATTR_FILL_FIRST + 16) 	/* V3: 1034 */

// Reserve
#define XATTR_FILLRESERVED3			(XATTR_FILL_FIRST + 17)	 	/* V3: 1035 */
#define XATTR_FILLRESERVED4			(XATTR_FILL_FIRST + 18)	 	/* V3: 1036 */
#define XATTR_FILLRESERVED5			(XATTR_FILL_FIRST + 19) 	/* V3: 1037 */
#define XATTR_FILLRESERVED6			(XATTR_FILL_FIRST + 20)	 	/* V3: 1038 */
#define XATTR_FILLRESERVED7			(XATTR_FILL_FIRST + 21)	 	/* V3: 1039 */
#define XATTR_FILLRESERVED8			(XATTR_FILL_FIRST + 22)	 	/* V3: 1040 */

#define XATTR_FILLBMP_POSOFFSETX	(XATTR_FILL_FIRST + 23)	 	/* V3: 1041 */
#define XATTR_FILLBMP_POSOFFSETY	(XATTR_FILL_FIRST + 24)	 	/* V3: 1042 */

// Reserve
#define XATTR_FILLBACKGROUND		(XATTR_FILL_FIRST + 25)		/* V3: 1043 */
#define XATTR_FILLRESERVED10		(XATTR_FILL_FIRST + 26)		/* V3: 1044 */
#define XATTR_FILLRESERVED11		(XATTR_FILL_FIRST + 27)	 	/* V3: 1045 */
#define XATTR_FILLRESERVED_LAST 	(XATTR_FILL_FIRST + 28)		/* V3: 1046 */
#define XATTR_FILL_LAST         	XATTR_FILLRESERVED_LAST

#define XATTRSET_FILL           (XATTR_FILL_LAST + 1)    		/* V3: 1047  V2: 1030 */

#define XATTR_TEXT_FIRST        (XATTRSET_FILL + 1)
#define XATTR_FORMTXTSTYLE      XATTR_TEXT_FIRST				/* V3: 1048  V2: 1031 */
#define XATTR_FORMTXTADJUST     (XATTR_TEXT_FIRST + 1)			/* V3: 1049  V2: 1032 */
#define XATTR_FORMTXTDISTANCE   (XATTR_TEXT_FIRST + 2)			/* V3: 1050  V2: 1033 */
#define XATTR_FORMTXTSTART      (XATTR_TEXT_FIRST + 3)			/* V3: 1051  V2: 1034 */
#define XATTR_FORMTXTMIRROR     (XATTR_TEXT_FIRST + 4)			/* V3: 1052  V2: 1035 */
#define XATTR_FORMTXTOUTLINE    (XATTR_TEXT_FIRST + 5)			/* V3: 1053  V2: 1036 */
#define XATTR_FORMTXTSHADOW     (XATTR_TEXT_FIRST + 6)			/* V3: 1054  V2: 1037 */
#define XATTR_FORMTXTSHDWCOLOR  (XATTR_TEXT_FIRST + 7)			/* V3: 1055  V2: 1038 */
#define XATTR_FORMTXTSHDWXVAL   (XATTR_TEXT_FIRST + 8)			/* V3: 1056  V2: 1039 */
#define XATTR_FORMTXTSHDWYVAL   (XATTR_TEXT_FIRST + 9)			/* V3: 1057  V2: 1040 */
#define XATTR_FORMTXTSTDFORM    (XATTR_TEXT_FIRST + 10)			/* V3: 1058  V2: 1041 */
#define XATTR_FORMTXTHIDEFORM   (XATTR_TEXT_FIRST + 11)			/* V3: 1059  V2: 1042 */
#define XATTR_FORMTXTSHDWTRANSP (XATTR_TEXT_FIRST + 12)   		/* V3: 1060  V2: 1043 */
#define XATTR_FTRESERVED2       (XATTR_TEXT_FIRST + 13)   		/* V3: 1061  V2: 1044 */
#define XATTR_FTRESERVED3       (XATTR_TEXT_FIRST + 14)			/* V3: 1062  V2: 1045 */
#define XATTR_FTRESERVED4       (XATTR_TEXT_FIRST + 15)			/* V3: 1063  V2: 1046 */
#define XATTR_FTRESERVED5       (XATTR_TEXT_FIRST + 16)			/* V3: 1064  V2: 1047 */
#define XATTR_FTRESERVED_LAST   (XATTR_TEXT_FIRST + 17)			/* V3: 1065  V2: 1048 */
#define XATTR_TEXT_LAST         XATTR_FTRESERVED_LAST

#define XATTRSET_TEXT           (XATTR_TEXT_LAST + 1)    		/* V3: 1066  V2: 1049 */
#define XATTR_END               XATTRSET_TEXT

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
