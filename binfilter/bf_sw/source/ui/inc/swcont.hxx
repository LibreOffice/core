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

#ifndef _SWCONT_HXX
#define _SWCONT_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
namespace binfilter {

class SwContentType;

//Reihenfolge und Anzahl mit ResIds abgleichen!!
#define CONTENT_TYPE_OUTLINE		0
#define CONTENT_TYPE_TABLE 			1
#define CONTENT_TYPE_FRAME          2
#define CONTENT_TYPE_GRAPHIC        3
#define CONTENT_TYPE_OLE            4
#define CONTENT_TYPE_BOOKMARK       5
#define CONTENT_TYPE_REGION         6
#define CONTENT_TYPE_URLFIELD		7
#define CONTENT_TYPE_REFERENCE      8
#define CONTENT_TYPE_INDEX			9
#define CONTENT_TYPE_POSTIT			10
#define CONTENT_TYPE_DRAWOBJECT     11
#define CONTENT_TYPE_MAX 			CONTENT_TYPE_DRAWOBJECT +1


// Typen fuer das Globaldokument
#define GLOBAL_CONTENT_REGION		100
#define GLOBAL_CONTENT_INDEX        101
#define GLOBAL_CONTENT_TEXT        	102
#define GLOBAL_CONTENT_MAX			3

// Strings fuer Kontextmenue
#define CONTEXT_COUNT 	12
#define GLOBAL_CONTEXT_COUNT 14

// Modi fuer Drag 'n Drop
#define REGION_MODE_NONE    	0
#define REGION_MODE_LINK        1
#define REGION_MODE_EMBEDDED    2

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//mini rtti
 class SwTypeNumber
 {
     BYTE nTypeId;

     public:
         SwTypeNumber(BYTE nId) :nTypeId(nId){}
         virtual ~SwTypeNumber();

         virtual BYTE	GetTypeId();
 };
//----------------------------------------------------------------------------

class SwContent : public SwTypeNumber
{
    const SwContentType*  	pParent;
    String 					sContentName;
    long					nYPosition;
    BOOL					bInvisible;
public:
        SwContent(const SwContentType* pCnt, const String& rName, long nYPos );

    const SwContentType* 	GetParent() const {return pParent;}
    const String&			GetName() 	const {return sContentName;}
    int						operator==(const SwContent& rCont) const
                                {
                                    //gleich sind sie nie, sonst fallen sie aus dem Array
                                    return FALSE;
                                }
    int						operator<(const SwContent& rCont) const
                                {
                                    //zuerst nach Position dann nach Name sortieren
                                    return nYPosition != rCont.nYPosition ?
                                        nYPosition < rCont.nYPosition :
                                            sContentName < rCont.sContentName;;
                                }

    long		GetYPos() const {return nYPosition;}

    BOOL		IsInvisible() const {return bInvisible;}
    void		SetInvisible(){ bInvisible = TRUE;}
};

} //namespace binfilter
#endif
