/*************************************************************************
 *
 *  $RCSfile: sfxecode.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:53 $
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
#ifndef _SFXECODE_HXX
#define _SFXECODE_HXX

#include <tools/errcode.hxx>

#define ERRCODE_SFX_NOSTDTEMPLATE           (ERRCODE_AREA_SFX|ERRCODE_CLASS_PATH|1)
#define ERRCODE_SFX_NOTATEMPLATE            (ERRCODE_AREA_SFX|ERRCODE_CLASS_FORMAT|2)
#define ERRCODE_SFX_GENERAL                 (ERRCODE_AREA_SFX|ERRCODE_CLASS_GENERAL|3)
#define ERRCODE_SFX_DOLOADFAILED            (ERRCODE_AREA_SFX|ERRCODE_CLASS_READ|4)
#define ERRCODE_SFX_DOSAVECOMPLETEDFAILED   (ERRCODE_AREA_SFX|ERRCODE_CLASS_WRITE|5)
#define ERRCODE_SFX_COMMITFAILED            (ERRCODE_AREA_SFX|ERRCODE_CLASS_WRITE|6)
#define ERRCODE_SFX_HANDSOFFFAILED          (ERRCODE_AREA_SFX|ERRCODE_CLASS_GENERAL|7)
#define ERRCODE_SFX_DOINITNEWFAILED         (ERRCODE_AREA_SFX|ERRCODE_CLASS_CREATE|8)
#define ERRCODE_SFX_CANTREADDOCINFO         (ERRCODE_AREA_SFX|ERRCODE_CLASS_FORMAT|9)
#define ERRCODE_SFX_ALREADYOPEN             (ERRCODE_AREA_SFX|ERRCODE_CLASS_ALREADYEXISTS|10)
#define ERRCODE_SFX_WRONGPASSWORD           (ERRCODE_AREA_SFX|ERRCODE_CLASS_READ|11)
#define ERRCODE_SFX_DOCUMENTREADONLY        (ERRCODE_AREA_SFX|ERRCODE_CLASS_WRITE|12)
#define ERRCODE_SFX_OLEGENERAL              (ERRCODE_AREA_SFX|ERRCODE_CLASS_NONE|14)
#define ERRCODE_SFXMSG_STYLEREPLACE         (ERRCODE_WARNING_MASK|ERRCODE_AREA_SFX|ERRCODE_CLASS_NONE|13)
#define ERRCODE_SFX_TEMPLATENOTFOUND        (ERRCODE_AREA_SFX|ERRCODE_CLASS_NOTEXISTS|15)
#define ERRCODE_SFX_ISRELATIVE              (ERRCODE_WARNING_MASK|ERRCODE_AREA_SFX|ERRCODE_CLASS_NOTEXISTS|16)
#define ERRCODE_SFX_FORCEDOCLOAD            (ERRCODE_WARNING_MASK|ERRCODE_AREA_SFX|ERRCODE_CLASS_NONE|17)

#define ERRCODE_SFX_NOFILTER                (ERRCODE_AREA_SFX|ERRCODE_CLASS_NOTEXISTS|45)
#define ERRCODE_SFX_FORCEQUIET              (ERRCODE_WARNING_MASK|ERRCODE_AREA_SFX|ERRCODE_CLASS_NONE|47)
#define ERRCODE_SFX_CONSULTUSER             (ERRCODE_WARNING_MASK|ERRCODE_AREA_SFX|ERRCODE_CLASS_NONE|48)
#define ERRCODE_SFX_NEVERCHECKCONTENT       (ERRCODE_AREA_SFX|ERRCODE_CLASS_NONE|49)

#define ERRCODE_SFX_NODOCRELOAD             (ERRCODE_AREA_SFX|ERRCODE_CLASS_ACCESS|18)
#define ERRCODE_SFX_CANTFINDORIGINAL        (ERRCODE_AREA_SFX|ERRCODE_CLASS_GENERAL|19)
#define ERRCODE_SFX_RESTART                 (ERRCODE_AREA_SFX|ERRCODE_CLASS_GENERAL|20)
#define ERRCODE_SFX_CANTCREATECONTENT       (ERRCODE_AREA_SFX|ERRCODE_CLASS_CREATE|21)
#define ERRCODE_SFX_CANTCREATELINK          (ERRCODE_AREA_SFX|ERRCODE_CLASS_CREATE|22)
#define ERRCODE_SFX_WRONGBMKFORMAT          (ERRCODE_AREA_SFX|ERRCODE_CLASS_FORMAT|23)
#define ERRCODE_SFX_WRONGICONFILE           (ERRCODE_AREA_SFX|ERRCODE_CLASS_FORMAT|24)
#define ERRCODE_SFX_CANTDELICONFILE         (ERRCODE_AREA_SFX|ERRCODE_CLASS_ACCESS|25)
#define ERRCODE_SFX_CANTWRITEICONFILE       (ERRCODE_AREA_SFX|ERRCODE_CLASS_ACCESS|26)
#define ERRCODE_SFX_CANTRENAMECONTENT       (ERRCODE_AREA_SFX|ERRCODE_CLASS_ACCESS|27)
#define ERRCODE_SFX_INVALIDBMKPATH          (ERRCODE_AREA_SFX|ERRCODE_CLASS_PATH|28)
#define ERRCODE_SFX_CANTWRITEURLCFGFILE     (ERRCODE_AREA_SFX|ERRCODE_CLASS_ACCESS|29)
#define ERRCODE_SFX_WRONGURLCFGFORMAT       (ERRCODE_AREA_SFX|ERRCODE_CLASS_FORMAT|30)
#define ERRCODE_SFX_NODOCUMENT              (ERRCODE_AREA_SFX|ERRCODE_CLASS_NOTEXISTS|31)
#define ERRCODE_SFX_INVALIDLINK             (ERRCODE_AREA_SFX|ERRCODE_CLASS_NOTEXISTS|32)
#define ERRCODE_SFX_INVALIDTRASHPATH        (ERRCODE_AREA_SFX|ERRCODE_CLASS_PATH|33)
#define ERRCODE_SFX_NOTRESTORABLE           (ERRCODE_AREA_SFX|ERRCODE_CLASS_CREATE|34)
#define ERRCODE_SFX_NOTRASH                 (ERRCODE_AREA_SFX|ERRCODE_CLASS_NOTEXISTS|35)
#define ERRCODE_SFX_INVALIDSYNTAX           (ERRCODE_AREA_SFX|ERRCODE_CLASS_PATH|36)
#define ERRCODE_SFX_CANTCREATEFOLDER        (ERRCODE_AREA_SFX|ERRCODE_CLASS_CREATE|37)
#define ERRCODE_SFX_CANTRENAMEFOLDER        (ERRCODE_AREA_SFX|ERRCODE_CLASS_PATH|38)
#define ERRCODE_SFX_WRONG_CDF_FORMAT        (ERRCODE_AREA_SFX| ERRCODE_CLASS_READ | 39)
#define ERRCODE_SFX_EMPTY_SERVER            (ERRCODE_AREA_SFX|ERRCODE_CLASS_NONE|40)
#define ERRCODE_SFX_NO_ABOBOX               (ERRCODE_AREA_SFX| ERRCODE_CLASS_READ | 41)

//Dies und das
#define ERRCTX_ERROR                    21
#define ERRCTX_WARNING                  22

//Documentkontexte
#define ERRCTX_SFX_LOADTEMPLATE         1
#define ERRCTX_SFX_SAVEDOC              2
#define ERRCTX_SFX_SAVEASDOC            3
#define ERRCTX_SFX_DOCINFO              4
#define ERRCTX_SFX_DOCTEMPLATE          5
#define ERRCTX_SFX_MOVEORCOPYCONTENTS   6

//Appkontexte
#define ERRCTX_SFX_DOCMANAGER           50
#define ERRCTX_SFX_OPENDOC              51
#define ERRCTX_SFX_NEWDOCDIRECT         52
#define ERRCTX_SFX_NEWDOC               53

//Organizerkontexte
#define ERRCTX_SFX_CREATEOBJSH          70

//BASIC-Kontexte
#define ERRCTX_SFX_LOADBASIC            80

//Addressbook contexts
#define ERRCTX_SFX_SEARCHADDRESS        90

#endif // #ifndef _SFXECODE_HXX


