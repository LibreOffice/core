/*************************************************************************
 *
 *  $RCSfile: gloslst.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 15:55:29 $
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

#define _SVSTDARR_STRINGSDTOR
#define _SVSTDARR_STRINGSISORTDTOR
#define _SVSTDARR_STRINGS
#include <svtools/svstdarr.hxx>

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef SVTOOLS_FSTATHELPER_HXX
#include <svtools/fstathelper.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _UNOTOOLS_TRANSLITERATIONWRAPPER_HXX
#include <unotools/transliterationwrapper.hxx>
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
#ifndef _SWUNOHELPER_HXX
#include <swunohelper.hxx>
#endif

#ifndef _UTLUI_HRC
#include <utlui.hrc>
#endif
#ifndef _GLOSLST_HRC
#include <gloslst.hrc>
#endif


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
    FixedLine       aFL;

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
    aFL(this,    ResId(FL_GLOSS))
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
    bFilled(FALSE)
{
    SvtPathOptions aPathOpt;
    sPath = aPathOpt.GetAutoTextPath();
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

    SvtPathOptions aPathOpt;
    String sTemp( aPathOpt.GetAutoTextPath() );
    if(sTemp != sPath)
    {
        sPath = sTemp;
        bFilled = FALSE;
        ClearGroups();
    }
    SwGlossaries* pGlossaries = ::GetGlossaries();
    const SvStrings* pPathArr = pGlossaries->GetPathArray();
    String sExt( SwGlossaries::GetExtension() );
    if(!bFilled)
    {
        USHORT nGroupCount = pGlossaries->GetGroupCnt();
        for(USHORT i = 0; i < nGroupCount; i++)
        {
            String sGrpName = pGlossaries->GetGroupName(i);
            USHORT nPath = (USHORT)sGrpName.GetToken(1, GLOS_DELIM).ToInt32();
            if(nPath < pPathArr->Count())
            {
                AutoTextGroup* pGroup = new AutoTextGroup;
                pGroup->sName = sGrpName;

                FillGroup(pGroup, pGlossaries);
                String sName = *(*pPathArr)[nPath];
                sName += INET_PATH_TOKEN;
                sName += pGroup->sName.GetToken(0, GLOS_DELIM);
                sName += sExt;

                FStatHelper::GetModifiedDateTimeOfFile( sName,
                                                &pGroup->aDateModified,
                                                &pGroup->aDateModified );

                aGroupArr.Insert( pGroup, i );
            }
        }
        bFilled = TRUE;
    }
    else
    {
        for(USHORT nPath = 0; nPath < pPathArr->Count(); nPath++)
        {
            SvStringsDtor aFoundGroupNames;
            SvStrings aFiles( 16, 16 );
            SvPtrarr aDateTimeArr( 16, 16 );

            SWUnoHelper::UCB_GetFileListOfFolder( *(*pPathArr)[nPath], aFiles,
                                                    &sExt, &aDateTimeArr );
            for( USHORT nFiles = 0, nFEnd = aFiles.Count();
                    nFiles < nFEnd; ++nFiles )
            {
                String* pTitle = aFiles[ nFiles ];
                ::DateTime* pDT = (::DateTime*) aDateTimeArr[ nFiles ];

                String sName( pTitle->Copy( 0, pTitle->Len() - sExt.Len() ));

                aFoundGroupNames.Insert( new String(sName),
                                            aFoundGroupNames.Count());
                sName += GLOS_DELIM;
                sName += String::CreateFromInt32( nPath );
                AutoTextGroup* pFound = FindGroup( sName );
                if( !pFound )
                {
                    pFound = new AutoTextGroup;
                    pFound->sName = sName;
                    FillGroup( pFound, pGlossaries );
                    pFound->aDateModified = *pDT;

                    aGroupArr.Insert(pFound, aGroupArr.Count());
                }
                else if( pFound->aDateModified < *pDT )
                {
                    FillGroup(pFound, pGlossaries);
                    pFound->aDateModified = *pDT;
                }

                // don't need any more these pointers
                delete pTitle;
                delete pDT;
            }

            USHORT nArrCount = aGroupArr.Count();
            for( USHORT i = nArrCount; i; --i)
            {
                // evtl. geloeschte Gruppen entfernen
                AutoTextGroup* pGroup = aGroupArr.GetObject(i - 1);
                USHORT nGroupPath = (USHORT)pGroup->sName.GetToken( 1,
                                                        GLOS_DELIM).ToInt32();
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
}

/********************************************************************

********************************************************************/


void SwGlossaryList::Timeout()
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
    const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();

    for(USHORT i = 0; i < nCount; i++ )
    {
        AutoTextGroup* pGroup = aGroupArr.GetObject(i);
        for(USHORT j = 0; j < pGroup->nCount; j++)
        {
            String sBlock = pGroup->sLongNames.GetToken(j, STRING_DELIM);
            if( rSCmp.isEqual( sBlock.Copy(0, nBeginLen), rBegin ) &&
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


