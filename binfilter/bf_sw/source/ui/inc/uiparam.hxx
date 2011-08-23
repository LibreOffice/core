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
#ifndef _UIPARAM_HXX
#define _UIPARAM_HXX

#ifndef _SVX_DIALOGS_HRC
#include <bf_svx/dialogs.hrc>
#endif
namespace binfilter {

/*--------------------------------------------------------------------
    Beschreibung:	Mapping fuer UI-Paramter
 --------------------------------------------------------------------*/

#define ITEMID_FONTLIST			SID_ATTR_CHAR_FONTLIST
#define ITEMID_TABPOS       	SID_ATTR_TABSTOP_POS
#define ITEMID_DEFTABS      	SID_ATTR_TABSTOP_DEFAULTS
#define ITEMID_TABOFFSET    	SID_ATTR_TABSTOP_OFFSET
#define ITEMID_SEARCH       	SID_SEARCH_ITEM
#define ITEMID_PARADLGLIMITS	SID_ATTR_PARA_DLGLIMITS
#define ITEMID_INFRAME          SID_ATTR_PARA_INFRAME

#define ITEMID_PAGEMODEL	    SID_ATTR_PARA_MODEL
#define	ITEMID_PAGE				SID_ATTR_PAGE
#define ITEMID_SIZE     		SID_ATTR_PAGE_SIZE
#define ITEMID_SETITEM     		SID_ATTR_PAGE_HEADERSET
#define ITEMID_PTR     			SID_ATTR_PAGE_BSP
#define ITEMID_SPELLCHECK 		SID_ATTR_SPELL
#define ITEMID_HYPHENREGION 	SID_ATTR_HYPHENREGION
#define ITEMID_HYPHENZONE       RES_PARATR_HYPHENZONE
#define ITEMID_LINE             SID_ATTR_LINE_STYLE
#define ITEMID_BOXINFO 			SID_ATTR_BORDER_INNER

#define ITEMID_AUTHOR			SID_ATTR_POSTIT_AUTHOR
#define ITEMID_DATE 			SID_ATTR_POSTIT_DATE
#define ITEMID_TEXT 			SID_ATTR_POSTIT_TEXT

#define ITEMID_NUMBERINFO 		SID_ATTR_NUMBERFORMAT_INFO

} //namespace binfilter
#endif // _UIPARAM_HXX
