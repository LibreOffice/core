/*************************************************************************
 *
 *  $RCSfile: config.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2000-11-14 13:42:33 $
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

#pragma hdrstop

#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif
#ifndef _SFXHINT_HXX //autogen
#include <svtools/hint.hxx>
#endif
#ifndef _SFXSMPLHINT_HXX //autogen
#include <svtools/smplhint.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _SFXSIDS_HRC //autogen
#include <sfx2/sfxsids.hrc>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif


#ifndef CONFIG_HXX
#include "config.hxx"
#endif
#ifndef FORMAT_HXX
#include "format.hxx"
#endif
#ifndef _SMMOD_HXX
#include "smmod.hxx"
#endif
#ifndef _STARMATH_HRC
#include "starmath.hrc"
#endif

#define DEFSYMFILE      "starmath$(lang).sms"
#define CONFIGVERSION   (INT16)0x0001;


SmConfig::SmConfig() :
    SfxConfigItem(SMCONFIGITEM)
{
    UseDefault();
    StartListening(aStandardFormat);
}


SmConfig::~SmConfig()
{
    EndListening(aStandardFormat);
}


void SmConfig::SFX_NOTIFY(SfxBroadcaster &, const TypeId &,
                          const SfxHint &rHint, const TypeId &)
{
    switch (((SfxSimpleHint &) rHint).GetId())
    {
        case HINT_FORMATCHANGED:
            SetModified(TRUE);
            SetDefault (FALSE);
            break;
    }
}


void SmConfig::ConfigChangedAction()
{
    SetModified(TRUE);
    SetDefault(FALSE);
    //Broadcast(SfxSimpleHint(HINT_CONFIGCHANGED));
}


void SmConfig::SetValueIfNE(BOOL &rItem, const BOOL bVal)
    // "Set 'Modified' and 'Value' if not equal"
{
    if (rItem != bVal)
    {   rItem = bVal;
        ConfigChangedAction();
    }
}


void SmConfig::SetSymbolFile(const String &rText)
{
    if (aSymbolFile != rText)
    {
        aSymbolFile = rText;
        ConfigChangedAction();
    }
}


int SmConfig::Load(SvStream &rStream)
{
    // Da die Fileformat Version in ConfigItems nur einen Defaultwert hat setzen
    // wir diesen hier auf die aktuelle Version, damit beim folgenden (impliziten)
    // einlesen des 'SmFormat' Objekts aus dem Stream dieses entsprechend
    // reagieren kann.
    rStream.SetVersion(SOFFICE_FILEFORMAT_NOW);

    rStream >> *this;
    SetDefault(FALSE);
    return SfxConfigItem::ERR_OK;
}


BOOL SmConfig::Store(SvStream &rStream)
{
    rStream << *this;
    return TRUE;
}

/**************************************************************************/

void SmConfig::UseDefault()
{
    SfxConfigItem::UseDefault();    // this implicitly calls 'SetDefault(TRUE)'

    bToolBoxVisible = bCmdBoxWindow = bAutoRedraw = bFormulaCursor =
    bPrintTitle = bPrintText = bPrintFrame = bWarnNoSymbols =
    bNoRightSpaces = TRUE;

    aSymbolFile = C2S(DEFSYMFILE);
    SvtPathOptions aOpt;
    aOpt.SearchFile( aSymbolFile, SvtPathOptions::PATH_USERCONFIG );

    ePrintSize  = PRINT_SIZE_NORMAL;
    nPrintZoom  = 100;
}

/**************************************************************************/

String SmConfig::GetName() const
{
    return C2S("StarMath");
}

/**************************************************************************/

void SmConfig::ItemSetToConfig(const SfxItemSet &rSet)
{
    const SfxPoolItem *pItem     = NULL;
    BOOL               bModified = FALSE;
    String             aSymbolFile;

    if (rSet.GetItemState(SID_SYMBOLFILE, TRUE, &pItem) == SFX_ITEM_SET)
    {   aSymbolFile = ((const SfxStringItem *) pItem)->GetValue();
        DBG_ASSERT(aSymbolFile.Len() > 0, "Symboldatei nicht angegeben !");

        if (aSymbolFile != aSymbolFile)
        {
            SfxModule *p = SM_MOD1();
            SmModule *pp = (SmModule *) p;

            pp->GetConfig()->SetWarnNoSymbols(TRUE);
            aSymbolFile = aSymbolFile;

            bModified = TRUE;
        }
    }

    UINT16 nU16;
    if (rSet.GetItemState(SID_PRINTSIZE, TRUE, &pItem) == SFX_ITEM_SET)
    {   nU16 = ((const SfxUInt16Item *) pItem)->GetValue();
        if (ePrintSize != nU16)
        {   ePrintSize = (SmPrintSize) nU16;
            bModified = TRUE;
        }
    }
    if (rSet.GetItemState(SID_PRINTZOOM, TRUE, &pItem) == SFX_ITEM_SET)
    {   nU16 = ((const SfxUInt16Item *) pItem)->GetValue();
        if (nPrintZoom != nU16)
        {   nPrintZoom = (USHORT) nU16;
            bModified = TRUE;
        }
    }

    BOOL bVal;
    if (rSet.GetItemState(SID_PRINTTITLE, TRUE, &pItem) == SFX_ITEM_SET)
    {   bVal = ((const SfxBoolItem *) pItem)->GetValue();
        if (bPrintTitle != bVal)
        {   bPrintTitle = bVal;
            bModified = TRUE;
        }
    }
    if (rSet.GetItemState(SID_PRINTTEXT, TRUE, &pItem) == SFX_ITEM_SET)
    {   bVal = ((const SfxBoolItem *) pItem)->GetValue();
        if (bPrintText != bVal)
        {   bPrintText = bVal;
            bModified = TRUE;
        }
    }
    if (rSet.GetItemState(SID_PRINTFRAME, TRUE, &pItem) == SFX_ITEM_SET)
    {   bVal = ((const SfxBoolItem *) pItem)->GetValue();
        if (bPrintFrame != bVal)
        {   bPrintFrame = bVal;
            bModified = TRUE;
        }
    }
    if (rSet.GetItemState(SID_AUTOREDRAW, TRUE, &pItem) == SFX_ITEM_SET)
    {   bVal = ((const SfxBoolItem *) pItem)->GetValue();
        if (bAutoRedraw != bVal)
        {   bAutoRedraw = bVal;
            bModified = TRUE;
        }
    }
    if (rSet.GetItemState(SID_NO_RIGHT_SPACES, TRUE, &pItem) == SFX_ITEM_SET)
    {   bVal = ((const SfxBoolItem *) pItem)->GetValue();
        if (bNoRightSpaces != bVal)
        {   bNoRightSpaces = bVal;
            bModified = TRUE;

            // (angezeigte) Formeln müssen entsprechen neu formatiert werden.
            // Das erreichen wir mit:
            Broadcast(SfxSimpleHint(HINT_FORMATCHANGED));
        }
    }

    if (bModified)
        ConfigChangedAction();
}


void SmConfig::ConfigToItemSet(SfxItemSet &rSet) const
{
    const SfxItemPool *pPool = rSet.GetPool();

    rSet.Put(SfxStringItem(pPool->GetWhich(SID_SYMBOLFILE),
                           aSymbolFile));

    rSet.Put(SfxUInt16Item(pPool->GetWhich(SID_PRINTSIZE),
                           (UINT16) ePrintSize));
    rSet.Put(SfxUInt16Item(pPool->GetWhich(SID_PRINTZOOM),
                           (UINT16) nPrintZoom));

    rSet.Put(SfxBoolItem(pPool->GetWhich(SID_PRINTTITLE), bPrintTitle));
    rSet.Put(SfxBoolItem(pPool->GetWhich(SID_PRINTTEXT),  bPrintText));
    rSet.Put(SfxBoolItem(pPool->GetWhich(SID_PRINTFRAME), bPrintFrame));
    rSet.Put(SfxBoolItem(pPool->GetWhich(SID_AUTOREDRAW), bAutoRedraw));
    rSet.Put(SfxBoolItem(pPool->GetWhich(SID_NO_RIGHT_SPACES), bNoRightSpaces));
}


/**************************************************************************/


SvStream & operator << (SvStream &rStream, const SmConfig &rConfig)
{
    rStream << CONFIGVERSION;

    UINT16  nFlags = rConfig.bPrintTitle
        | (rConfig.bPrintText      << 1)
        | (rConfig.bPrintFrame     << 2)
        | (rConfig.bWarnNoSymbols  << 3)
        | (rConfig.bToolBoxVisible << 4)
        | (rConfig.bCmdBoxWindow   << 5)
        | (rConfig.bAutoRedraw     << 6)
        | (rConfig.bFormulaCursor  << 7)
        | (rConfig.bNoRightSpaces  << 8);

    rStream << nFlags;

    rStream << (INT16) rConfig.ePrintSize;
    rStream << (INT16) rConfig.nPrintZoom;

    rStream.WriteByteString(rConfig.aSymbolFile, gsl_getSystemTextEncoding());
    rStream << rConfig.aStandardFormat;

    for (int i = 0;  i < 7;  i++)
        rStream << rConfig.vFontPickList[i];

    return rStream;
}


SvStream & operator >> (SvStream &rStream, SmConfig &rConfig)
{
    INT16  nVer;
    rStream >> nVer;

    UINT16 nFlags;
    rStream >> nFlags;

    rConfig.bPrintTitle     = nFlags & 0x01;
    rConfig.bPrintText      = (nFlags >> 1) & 0x01;
    rConfig.bPrintFrame     = (nFlags >> 2) & 0x01;
    rConfig.bWarnNoSymbols  = (nFlags >> 3) & 0x01;
    rConfig.bToolBoxVisible = (nFlags >> 4) & 0x01;
    rConfig.bCmdBoxWindow   = (nFlags >> 5) & 0x01;
    rConfig.bAutoRedraw     = (nFlags >> 6) & 0x01;
    rConfig.bFormulaCursor  = (nFlags >> 7) & 0x01;
    rConfig.bNoRightSpaces  = (nFlags >> 8) & 0x01;

    INT16  nI16;
    rStream >> nI16;
    rConfig.ePrintSize = (SmPrintSize) nI16;
    rStream >> nI16;
    rConfig.nPrintZoom = (USHORT) nI16;

    rStream.ReadByteString(rConfig.aSymbolFile, gsl_getSystemTextEncoding());
    rStream >> rConfig.aStandardFormat;

    for (int i = 0;  i < 7;  i++)
        rStream >> rConfig.vFontPickList[i];

    return rStream;
}


