/*************************************************************************
 *
 *  $RCSfile: typemap.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:18 $
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


#ifndef _SFXMSG_HXX //autogen
#include <sfx2/msg.hxx>
#endif

#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif

#ifndef _FM_FMITEMS_HXX //autogen
#include "fmitems.hxx"
#endif

#ifndef _SFX_OBJITEM_HXX //autogen
#include <sfx2/objitem.hxx>
#endif

#ifndef _SVX_SVXIDS_HRC
#include "svxids.hrc"
#endif


/************************************************************/
extern SfxType0 aSfxUInt32Item_Impl;
SfxType0 aSfxUInt32Item_Impl =
{
    TYPE(SfxUInt32Item), 0
};

/************************************************************/
extern SfxType3 aFmFormInfoItem_Impl;
SfxType3 aFmFormInfoItem_Impl =
{
    TYPE(FmFormInfoItem), 3, { {MID_POS,"Pos"}, {MID_COUNT,"Count"}, {MID_READONLY,"ReadOnly"} }
};

/************************************************************/
extern SfxType0 aSfxObjectItem_Impl;
SfxType0 aSfxObjectItem_Impl =
{
    TYPE(SfxObjectItem), 0
};

// diese werden nur bei einer Aufteilung in 2 Dll's gebraucht
#if 1
#if defined(WNT) || defined(OS2)
/************************************************************/
extern SfxType0 aSfxUInt16Item_Impl;
SfxType0 aSfxUInt16Item_Impl =
{
    TYPE(SfxUInt16Item), 0
};



/************************************************************/
extern SfxType0 aSfxVoidItem_Impl;
SfxType0 aSfxVoidItem_Impl =
{
    TYPE(SfxVoidItem), 0
};


/************************************************************/
extern SfxType0 aSfxStringItem_Impl;
SfxType0 aSfxStringItem_Impl =
{
    TYPE(SfxStringItem), 0
};


/************************************************************/
extern SfxType0 aSfxBoolItem_Impl;
SfxType0 aSfxBoolItem_Impl =
{
    TYPE(SfxBoolItem), 0
};

#endif
#endif

