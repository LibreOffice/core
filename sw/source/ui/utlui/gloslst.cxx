/*************************************************************************
 *
 *  $RCSfile: gloslst.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:50 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#define _SVSTDARR_STRINGSDTOR
#define _SVSTDARR_STRINGSISORTDTOR
#define _SVSTDARR_STRINGS
#include <svtools/svstdarr.hxx>

#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _SFX_INIMGR_HXX
#include <sfx2/inimgr.hxx>
#endif

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _INITUI_HXX
#include <initui.hxx>
#endif
#ifndef _GLOSDOC_HXX
#include <glosdoc.hxx>
#endif
#ifndef _GLOSLST_HXX
#include <gloslst.hxx>
#endif

#ifndef _UTLUI_HRC
#include <utlui.hrc>
#endif
#ifndef _GLOSLST_HRC
#include <gloslst.hrc>
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDENVIRONMENT_HPP_
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif

#ifdef DEBUG
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#endif

#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::ucb;
using namespace ::rtl;

#define STRING_DELIM (char)0x0A
#define GLOS_TIMEOUT 30000   // alle 30 s updaten
#define FIND_MAX_GLOS 20


struct TripleString
{
    String sGroup;
    String sBlock;
    String sShort;
};

typedef TripleString* TripleStringPtr;
SV_DECL_PTRARR_DEL( TripleStrings, TripleStringPtr, 0, 4 )
SV_IMPL_PTRARR( TripleStrings, TripleStringPtr )

class SwGlossDecideDlg : public ModalDialog
{
    OKButton        aOk;
    CancelButton    aCancel;
    HelpButton      aHelp;
    ListBox         aListLB;
    GroupBox        aGroup;

    DECL_LINK(DoubleClickHdl, ListBox*);
    DECL_LINK(SelectHdl, ListBox*);

    public:
        SwGlossDecideDlg(Window* pParent);
    ListBox&    GetListBox() {return aListLB;}
};

/*-----------------21.01.97 13.25-------------------

--------------------------------------------------*/

SwGlossDecideDlg::SwGlossDecideDlg(Window* pParent) :
    ModalDialog(pParent, SW_RES(DLG_GLOSSARY_DECIDE_DLG)),
    aOk(this,       ResId(PB_OK)),
    aCancel(this,   ResId(PB_CANCEL)),
    aHelp(this,     ResId(PB_HELP)),
    aListLB(this,   ResId(LB_LIST)),
    aGroup(this,    ResId(GB_GLOSS))
{
    FreeResource();
    aListLB.SetDoubleClickHdl(LINK(this, SwGlossDecideDlg, DoubleClickHdl));
    aListLB.SetSelectHdl(LINK(this, SwGlossDecideDlg, SelectHdl));
}

/*-----------------21.01.97 13.25-------------------

--------------------------------------------------*/

IMPL_LINK(SwGlossDecideDlg, DoubleClickHdl, ListBox*, EMPTYARG)
{
    EndDialog(RET_OK);
    return 0;
}
/*-----------------21.01.97 13.29-------------------

--------------------------------------------------*/

IMPL_LINK(SwGlossDecideDlg, SelectHdl, ListBox*, EMPTYARG)
{
    aOk.Enable(LISTBOX_ENTRY_NOTFOUND != aListLB.GetSelectEntryPos());
    return 0;
}

/********************************************************************

********************************************************************/


SwGlossaryList::SwGlossaryList() :
    bFilled(FALSE),
    sPath( SFX_INIMANAGER()->Get( SFX_KEY_GLOSSARY_PATH ) )
{
    SetTimeout(GLOS_TIMEOUT);
}

/********************************************************************

********************************************************************/


SwGlossaryList::~SwGlossaryList()
{
    ClearGroups();
}

/********************************************************************
 * Wenn der GroupName bereits bekannt ist, dann wird nur
 * rShortName gefuellt, sonst wird rGroupName ebenfals gesetzt und
 * bei Bedarf nach der richtigen Gruppe gefragt
********************************************************************/


BOOL SwGlossaryList::GetShortName(const String& rLongName,
                                String& rShortName, String& rGroupName )
{
    if(!bFilled)
        Update();

    TripleStrings aTripleStrings;

    USHORT nCount = aGroupArr.Count();
    USHORT nFound = 0;
    for(USHORT i = 0; i < nCount; i++ )
    {
        AutoTextGroup* pGroup = aGroupArr.GetObject(i);
        if(!rGroupName.Len() || rGroupName == pGroup->sName)
            for(USHORT j = 0; j < pGroup->nCount; j++)
            {
                String sLong = pGroup->sLongNames.GetToken(j, STRING_DELIM);
                if((rLongName == sLong))
                {
                    TripleString* pTriple = new TripleString;
                    pTriple->sGroup = pGroup->sName;
                    pTriple->sBlock = sLong;
                    pTriple->sShort = pGroup->sShortNames.GetToken(j, STRING_DELIM);
                    aTripleStrings.Insert(pTriple, nFound++);
                }
            }
    }

    BOOL bRet = FALSE;
    nCount = aTripleStrings.Count();
    if(1 == nCount )
    {
        TripleString* pTriple = aTripleStrings[0];
        rShortName = pTriple->sShort;
        rGroupName = pTriple->sGroup;
        bRet = TRUE;
    }
    else if(1 < nCount)
    {
        SwGlossDecideDlg aDlg(0);
        String sTitle = aDlg.GetText();
        sTitle += aTripleStrings[0]->sBlock;
        aDlg.SetText(sTitle);

        ListBox& rLB = aDlg.GetListBox();
        for(USHORT i = 0; i < nCount; i++ )
            rLB.InsertEntry(aTripleStrings[i]->sGroup.GetToken(0, GLOS_DELIM));

        rLB.SelectEntryPos(0);
        if(RET_OK == aDlg.Execute() &&
            LISTBOX_ENTRY_NOTFOUND != rLB.GetSelectEntryPos())
        {
            TripleString* pTriple = aTripleStrings[rLB.GetSelectEntryPos()];
            rShortName = pTriple->sShort;
            rGroupName = pTriple->sGroup;
            bRet = TRUE;
        }
        else
            bRet = FALSE;
    }
    return bRet;
}

/********************************************************************

********************************************************************/


USHORT  SwGlossaryList::GetGroupCount()
{
    if(!bFilled)
        Update();
    return aGroupArr.Count();
}

/********************************************************************

********************************************************************/


String SwGlossaryList::GetGroupName(USHORT nPos, BOOL bNoPath, String* pTitle)
{
    DBG_ASSERT(aGroupArr.Count() > nPos, "Gruppe nicht vorhanden")
    String sRet(aEmptyStr);
    if(nPos < aGroupArr.Count())
    {
        AutoTextGroup* pGroup = aGroupArr.GetObject(nPos);
        sRet = pGroup->sName;
        if(bNoPath)
            sRet = sRet.GetToken(0, GLOS_DELIM);
        if(pTitle)
            *pTitle = pGroup->sTitle;
    }
    return sRet;

}

/********************************************************************

********************************************************************/


USHORT  SwGlossaryList::GetBlockCount(USHORT nGroup)
{
    DBG_ASSERT(aGroupArr.Count() > nGroup, "Gruppe nicht vorhanden")
    if(nGroup < aGroupArr.Count())
    {
        AutoTextGroup* pGroup = aGroupArr.GetObject(nGroup);
        return pGroup->nCount;
    }
    return 0;
}

/********************************************************************

********************************************************************/


String  SwGlossaryList::GetBlockName(USHORT nGroup, USHORT nBlock, String& rShortName)
{
    DBG_ASSERT(aGroupArr.Count() > nGroup, "Gruppe nicht vorhanden")
    if(nGroup < aGroupArr.Count())
    {
        AutoTextGroup* pGroup = aGroupArr.GetObject(nGroup);
        rShortName = pGroup->sShortNames.GetToken(nBlock, STRING_DELIM);
        return pGroup->sLongNames.GetToken(nBlock, STRING_DELIM);
    }
    return aEmptyStr;
}

/********************************************************************

********************************************************************/


void SwGlossaryList::Update()
{
    if(!IsActive())
        Start();
    String sTemp( SFX_INIMANAGER()->Get( SFX_KEY_GLOSSARY_PATH ) );
    if(sTemp != sPath)
    {
        sPath = sTemp;
        bFilled = FALSE;
        ClearGroups();
    }
    SwGlossaries* pGlossaries = ::GetGlossaries();
    const SvStrings* pPathArr = pGlossaries->GetPathArray();
    if(!bFilled)
    {
        USHORT nGroupCount = pGlossaries->GetGroupCnt();
        for(USHORT i = 0; i < nGroupCount; i++)
        {
            String sGrpName = pGlossaries->GetGroupName(i);
            USHORT nPath = sGrpName.GetToken(1, GLOS_DELIM).ToInt32();
            if(nPath < pPathArr->Count())
            {
                AutoTextGroup* pGroup = new AutoTextGroup;
                pGroup->sName = sGrpName;

                FillGroup(pGroup, pGlossaries);
                String sName = *(*pPathArr)[nPath];
                sName += INET_PATH_TOKEN;
                sName += pGroup->sName.GetToken(0, GLOS_DELIM);
                sName += String::CreateFromAscii(pGlosExt);

                uno::Reference< XCommandEnvironment > xCmdEnv;
                ::ucb::Content aTestContent(
#if SUPD<591
                    SW_MOD()->GetContentBroker(),
#endif
                sName,
                xCmdEnv);

#ifdef DEBUG
                Reference< beans::XPropertySetInfo > xInfo = aTestContent.  getProperties();
                Sequence< beans::Property > aSeq = xInfo->getProperties(  );
                const beans::Property* pProps = aSeq.getConstArray();
                OUString sMsg;
                for(int nProp = 0; nProp < aSeq.getLength(); nProp++)
                {
                    sMsg += pProps[nProp].Name;
                    sMsg += OUString::createFromAscii("  ");
                }

#endif

                try
                {
                    uno::Any aAny = aTestContent.getPropertyValue( OUString::createFromAscii("DateModified") );
                    if(aAny.hasValue())
                    {
                         const util::DateTime* pDT = (util::DateTime*)aAny.getValue();
                        pGroup->aDateModified = ::DateTime(
                                    ::Date(pDT->Day, pDT->Month, pDT->Year),
                                    ::Time(pDT->Hours, pDT->Minutes, pDT->Seconds, pDT->HundredthSeconds));
                    }
                }
                catch(...)
                {
                }

                aGroupArr.Insert(pGroup, i);
            }
        }
        bFilled = TRUE;
    }
    else
    {
        for(USHORT nPath = 0; nPath < pPathArr->Count(); nPath++)
        {
              try
            {
                ::ucb::Content aCnt(
    #if SUPD<591
                SW_MOD()->GetContentBroker(),
    #endif
                        *(*pPathArr)[nPath], uno::Reference< XCommandEnvironment >());
                   Reference< sdbc::XResultSet > xResultSet;
                  Sequence< OUString > aProps(2);
                OUString* pProps = aProps.getArray();
                pProps[ 0 ] = OUString::createFromAscii( "Title" );
                pProps[ 1 ] = OUString::createFromAscii( "DateModified" );
                try
                {
                       xResultSet = aCnt.createCursor( aProps, ::ucb::INCLUDE_DOCUMENTS_ONLY );
                }
                catch ( Exception )
                {
                    DBG_ERRORFILE( "create cursor failed!" );
                }

                if ( xResultSet.is() )
                {
                    SvStringsDtor aFoundGroupNames;
                    Reference< sdbc::XRow > xRow( xResultSet, UNO_QUERY );
                    try
                    {
                        if ( xResultSet->first() )
                        {
                            do
                            {
                                String sTitle = xRow->getString( 1 );
                                xub_StrLen nFound = sTitle.SearchAscii( pGlosExt );
                                if( sTitle.Len() - 4 == nFound )
                                {
                                    util::DateTime aStamp = xRow->getTimestamp(2);
                                    ::DateTime aDateTime = ::DateTime(
                                        ::Date(aStamp.Day, aStamp.Month, aStamp.Year ),
                                        ::Time(aStamp.Hours, aStamp.Minutes, aStamp.Seconds, aStamp.HundredthSeconds )  );
                                    String sName = sTitle.Erase(sTitle.Len() - 4, 4);
                                    aFoundGroupNames.Insert(new String(sName), aFoundGroupNames.Count());
                                    sName += GLOS_DELIM;
                                    sName += String::CreateFromInt32( nPath );
                                    AutoTextGroup* pFound = FindGroup(sName);
                                    if(!pFound)
                                    {
                                        pFound = new AutoTextGroup;
                                        pFound->sName = sName;
                                        FillGroup(pFound, pGlossaries);
                                        pFound->aDateModified = aDateTime;

                                        aGroupArr.Insert(pFound, aGroupArr.Count());
                                    }
                                    else if(pFound->aDateModified < aDateTime)
                                    {
                                        FillGroup(pFound, pGlossaries);
                                        pFound->aDateModified = aDateTime;
                                    }
                                }
                            }
                            while ( xResultSet->next() );
                        }
                    }
                    catch ( ... )
                    {
                        DBG_ERRORFILE( "Exception caught!" );
                    }
                    USHORT nArrCount = aGroupArr.Count();
                    for( USHORT i = nArrCount; i; --i)
                    {
                        // evtl. geloeschte Gruppen entfernen
                        AutoTextGroup* pGroup = aGroupArr.GetObject(i - 1);
                        USHORT nGroupPath = pGroup->sName.GetToken(1, GLOS_DELIM).ToInt32();
                        // nur die Gruppen werden geprueft, die fuer den
                        // aktuellen Teilpfad registriert sind
                        if(nGroupPath == nPath)
                        {
                            BOOL bFound = FALSE;
                            String sCompareGroup = pGroup->sName.GetToken(0, GLOS_DELIM);
                            for( USHORT j = 0; j < aFoundGroupNames.Count() && !bFound; ++j)
                            {
                                bFound = sCompareGroup == *aFoundGroupNames[j];
                            }
                            if(!bFound)
                            {
                                aGroupArr.Remove(i - 1);
                                delete pGroup;
                            }
                        }
                    }
                }
            }
            catch(...){}
        }
    }
}

/********************************************************************

********************************************************************/


void __EXPORT SwGlossaryList::Timeout()
{
    // nur, wenn eine SwView den Fokus hat, wird automatisch upgedated
    if(::GetActiveView())
        Update();
}

/********************************************************************

********************************************************************/


AutoTextGroup*  SwGlossaryList::FindGroup(const String& rGroupName)
{
    for(USHORT i = 0; i < aGroupArr.Count(); i++)
    {
        AutoTextGroup* pRet = aGroupArr.GetObject(i);
        if(pRet->sName == rGroupName)
            return pRet;
    }
    return 0;
}

/********************************************************************

********************************************************************/


void SwGlossaryList::FillGroup(AutoTextGroup* pGroup, SwGlossaries* pGlossaries)
{
    SwTextBlocks*   pBlock = pGlossaries->GetGroupDoc(pGroup->sName);
    pGroup->nCount = pBlock ? pBlock->GetCount() : 0;
    pGroup->sLongNames = pGroup->sShortNames = aEmptyStr;
    if(pBlock)
        pGroup->sTitle = pBlock->GetName();

    for(USHORT j = 0; j < pGroup->nCount; j++)
    {
        pGroup->sLongNames  += pBlock->GetLongName(j);
        pGroup->sLongNames  += STRING_DELIM;
        pGroup->sShortNames += pBlock->GetShortName(j);
        pGroup->sShortNames += STRING_DELIM;
    }
    pGlossaries->PutGroupDoc(pBlock);
}

/********************************************************************
    Alle (nicht mehr als FIND_MAX_GLOS) gefunden Bausteine mit
    passendem Anfang zurueckgeben
********************************************************************/

BOOL SwGlossaryList::HasLongName(const String& rBegin, SvStringsISortDtor* pLongNames )
{
    if(!bFilled)
        Update();
    USHORT nFound = 0;
    USHORT nCount = aGroupArr.Count();
    USHORT nBeginLen = rBegin.Len();
    const International& rInt = Application::GetAppInternational();
    for(USHORT i = 0; i < nCount; i++ )
    {
        AutoTextGroup* pGroup = aGroupArr.GetObject(i);
        for(USHORT j = 0; j < pGroup->nCount; j++)
        {
            String sBlock = pGroup->sLongNames.GetToken(j, STRING_DELIM);
            if(rInt.CompareEqual(sBlock.Copy(0, nBeginLen), rBegin, INTN_COMPARE_IGNORECASE) &&
                nBeginLen + 1 < sBlock.Len())
            {
                String* pBlock = new String(sBlock);
                pLongNames->Insert(pBlock);
                nFound++;
                if(FIND_MAX_GLOS == nFound)
                    break;
            }
        }
    }
    return nFound > 0;
}

/********************************************************************

********************************************************************/
void    SwGlossaryList::ClearGroups()
{
    USHORT nCount = aGroupArr.Count();
    for( USHORT i = 0; i < nCount; ++i )
        delete aGroupArr.GetObject( i );

    aGroupArr.Remove( 0, nCount );
    bFilled = FALSE;
}

/*************************************************************************

    Source Code Control System - Header

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/ui/utlui/gloslst.cxx,v 1.1.1.1 2000-09-18 17:14:50 hr Exp $

    Source Code Control System - Update

    $Log: not supported by cvs2svn $
    Revision 1.29  2000/09/18 16:06:17  willem.vandorp
    OpenOffice header added.

    Revision 1.28  2000/06/13 09:57:49  os
    using UCB

    Revision 1.27  2000/06/08 09:45:45  os
    ContentBroker not in SwModule

    Revision 1.26  2000/06/07 13:29:51  os
    using UCB

    Revision 1.25  2000/05/23 19:54:24  jp
    Bugfixes for Unicode

    Revision 1.24  2000/04/18 15:14:08  os
    UNICODE

    Revision 1.23  2000/02/14 14:24:35  os
    #70473# Unicode

    Revision 1.22  2000/02/10 10:35:23  os
    #70359# titles added to AutoText groups

    Revision 1.21  1999/10/21 17:45:07  jp
    have to change - SearchFile with SfxIniManager, dont use SwFinder for this

    Revision 1.20  1999/09/20 09:59:08  os
    local resources separated

    Revision 1.19  1999/02/09 09:48:02  OS
    #61205# AutoText-Gruppen koennen beliebige Namen erhalten


      Rev 1.18   09 Feb 1999 10:48:02   OS
   #61205# AutoText-Gruppen koennen beliebige Namen erhalten

      Rev 1.17   18 Aug 1998 17:28:04   OS
   GetShortName sollte auf bekannte Gruppe richtig reagieren #55219#

      Rev 1.16   18 Mar 1998 17:47:40   OS
   kein GPF, wenn alle Glossary-Pfade ungueltig sind

      Rev 1.15   21 Nov 1997 12:10:14   MA
   includes

      Rev 1.14   03 Nov 1997 13:59:24   MA
   precomp entfernt

      Rev 1.13   01 Sep 1997 13:11:44   OS
   DLL-Umstellung

      Rev 1.12   30 Jul 1997 14:49:30   OM
   #41989# Autotextbereiche korrekt anzeigen

      Rev 1.11   09 Jul 1997 17:45:30   HJS
   includes

      Rev 1.10   17 Jun 1997 10:15:22   OS
   Update fuer mehrere AutoText-Pfade angepasst

      Rev 1.9   10 Jun 1997 14:26:12   OS
   AutoText aus mehreren Verzeichnissen

      Rev 1.8   30 Jan 1997 09:15:08   MA
   unbenutztes entfernt

      Rev 1.7   21 Jan 1997 15:00:30   OS
   Autotext-Auswahl-Dialog verfeinert

      Rev 1.6   21 Jan 1997 09:39:26   OS
   Gruppenname zurueckgeben

      Rev 1.5   20 Jan 1997 16:58:50   OS
   Entscheidungsdialog fuer AutoComplete

      Rev 1.4   01 Nov 1996 04:44:36   MH
   add: includes

      Rev 1.3   01 Oct 1996 09:00:16   JP
   DTOR korrigiert

      Rev 1.2   27 Sep 1996 16:33:22   HJS
   include

      Rev 1.1   27 Sep 1996 12:55:20   OS
   nach Kuerzeln suchen

      Rev 1.0   26 Sep 1996 16:55:54   OS
   Initial revision.

*************************************************************************/


