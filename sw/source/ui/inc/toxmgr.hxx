/*************************************************************************
 *
 *  $RCSfile: toxmgr.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:42 $
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

#ifndef _TOXMGR_HXX
#define _TOXMGR_HXX

#include "tox.hxx"
#ifndef _AUTHFLD_HXX
#include <authfld.hxx>
#endif

class SwWrtShell;
class SwForm;

/*--------------------------------------------------------------------
    Beschreibung: Manager fuer Verzeichnisfunktionalitaet
 --------------------------------------------------------------------*/
//one single method will be sufficient to insert AND upate indexes

class SwTOXDescription
{
    TOXTypes            eTOXType;
    String              aStyleNames[MAXLEVEL];
    String              sSequenceName;
    String              sMainEntryCharStyle;
    String              sAutoMarkURL;
    String*             pTitle;
    String*             pTOUName;
    SwForm*             pForm;
    USHORT              nContent;
    USHORT              nIndexOptions;
    USHORT              nOLEOptions;
    String              sAuthBrackets;
    SwCaptionDisplay    eCaptionDisplay;
    SwTOXSortKey        eSortKey1;
    SwTOXSortKey        eSortKey2;
    SwTOXSortKey        eSortKey3;
    BYTE                nLevel;
    BOOL                bFromObjectNames : 1;
    BOOL                bFromChapter : 1;
    BOOL                bReadonly: 1;
    BOOL                bLevelFromChapter : 1;
    BOOL                bIsAuthSequence :1;
    BOOL                bSortByDocument :1;

    //TODO: TemplateNames
    //const String* pTemplateName = 0, ???
    SwTOXDescription(SwTOXDescription&);
public:
    SwTOXDescription(TOXTypes eType) :
        eTOXType(eType),
        pTitle(0),
        pForm(0),
        nContent(TOX_MARK | TOX_OUTLINELEVEL),
        nLevel(MAXLEVEL),
        pTOUName(0),
        nIndexOptions(TOI_SAME_ENTRY|TOI_FF|TOI_CASE_SENSITIVE),
        nOLEOptions(0),
        eCaptionDisplay(CAPTION_COMPLETE),
        bFromObjectNames(FALSE),
        bFromChapter(FALSE),
        bReadonly(TRUE),
        bLevelFromChapter(FALSE),
        bIsAuthSequence(FALSE),
        bSortByDocument(TRUE)
        {}
    ~SwTOXDescription()
        {
            delete pTitle;
            delete pForm;
            delete pTOUName;
        }

    void            SetTOXType(TOXTypes eSet) { eTOXType = eSet;}
    TOXTypes        GetTOXType() const { return eTOXType;}

    const String&   GetStyleNames(USHORT nLevel) const
                                {return aStyleNames[nLevel];}
    void            SetStyleNames(const String& rSet, USHORT nLevel)
                                {aStyleNames[nLevel] = rSet; }

    const String&   GetAutoMarkURL() const { return sAutoMarkURL;}
    void            SetAutoMarkURL(const String& rSet) {sAutoMarkURL = rSet;}

    void            SetTitle(const String& pSet) {delete pTitle; pTitle = new String(pSet);}
    const String*   GetTitle() const {return pTitle; }

    void            SetTOUName(const String& pSet) {delete pTOUName; pTOUName = new String(pSet);}
    const String*   GetTOUName() const {return pTOUName; }

    void            SetForm(const SwForm& rSet) {delete pForm; pForm = new SwForm(rSet);}
    const SwForm*   GetForm() const {return pForm;}

    void            SetContentOptions(USHORT nSet) { nContent = nSet;}
    USHORT          GetContentOptions() const { return nContent;}

    void            SetIndexOptions(USHORT nSet) { nIndexOptions = nSet;}
    USHORT          GetIndexOptions() const { return nIndexOptions;}

    const String&   GetMainEntryCharStyle() const {return sMainEntryCharStyle;}
    void            SetMainEntryCharStyle(const String& rSet)  {sMainEntryCharStyle = rSet;}

    void            SetLevel(BYTE nSet) {nLevel = nSet;}
    BYTE            GetLevel()const  {return nLevel; }

    void            SetCreateFromObjectNames(BOOL bSet) { bFromObjectNames = bSet;}
    BOOL            IsCreateFromObjectNames() const {return bFromObjectNames;}

    const String&   GetSequenceName() const {return sSequenceName;}
    void            SetSequenceName(const String& rSet) {sSequenceName = rSet;}

    SwCaptionDisplay    GetCaptionDisplay() const { return eCaptionDisplay;}
    void                SetCaptionDisplay(SwCaptionDisplay eSet) {eCaptionDisplay = eSet;}

    void            SetFromChapter(BOOL bSet) { bFromChapter = bSet;}
    BOOL            IsFromChapter() const {return bFromChapter;}

    void            SetReadonly(BOOL bSet){bReadonly = bSet;}
    BOOL            IsReadonly() const {return bReadonly;}

    USHORT          GetOLEOptions() const {return nOLEOptions;}
    void            SetOLEOptions(USHORT nOpt) {nOLEOptions = nOpt;}

    BOOL            IsLevelFromChapter() const {return bLevelFromChapter;}
    void            SetLevelFromChapter(BOOL bSet) {bLevelFromChapter = bSet;}

    String          GetAuthBrackets() const {return sAuthBrackets;}
    void            SetAuthBrackets(const String& rSet) {sAuthBrackets = rSet;}

    BOOL            IsAuthSequence() const {return bIsAuthSequence;}
    void            SetAuthSequence(BOOL bSet){bIsAuthSequence = bSet;}

    BOOL            IsSortByDocument()const {return bSortByDocument ;}
    void            SetSortByDocument(BOOL bSet) {bSortByDocument = bSet;}

    void SetSortKeys(SwTOXSortKey eKey1,
                        SwTOXSortKey eKey2,
                            SwTOXSortKey eKey3);

    SwTOXSortKey GetSortKey1() const {return eSortKey1;}
    SwTOXSortKey GetSortKey2() const {return eSortKey2;}
    SwTOXSortKey GetSortKey3() const {return eSortKey3;}


    void            ApplyTo(SwTOXBase& rTOXBase);
};
// --------------------------------------------------------------------------------
class SwTOXMarkDescription
{
    TOXTypes    eTOXType;
    int         nLevel;
    BOOL        bMainEntry;

    String*     pPrimKey;
    String*     pSecKey;
    String*     pAltStr;
    String*     pTOUName;

public:
    SwTOXMarkDescription(TOXTypes eType) :
        eTOXType(eType),
        pPrimKey(0),
        pSecKey(0),
        pAltStr(0),
        pTOUName(0),
        nLevel(0),
        bMainEntry(FALSE)
        {
        }
    ~SwTOXMarkDescription()
    {
        delete pPrimKey;
        delete pSecKey;
        delete pAltStr;
        delete pTOUName;
    }

    TOXTypes        GetTOXType()const {return eTOXType;}

    void            SetLevel(int nSet) {nLevel = nSet;}
    int             GetLevel() const {return nLevel;}

    void            SetMainEntry(BOOL bSet) {bMainEntry = bSet;}
    BOOL            IsMainEntry() const {return bMainEntry;}

    void            SetPrimKey(const String& rSet)
                                {delete pPrimKey; pPrimKey = new String(rSet);}
    const String*   GetPrimKey() const {return pPrimKey;}

    void            SetSecKey(const String& rSet)
                                {delete pSecKey;  pSecKey  = new String(rSet);}
    const String*   GetSecKey() const { return pSecKey; }

    void            SetAltStr(const String& rSet)
                                {delete pAltStr;  pAltStr  = new String(rSet);}
    const String*   GetAltStr() const { return pAltStr; }

    void            SetTOUName(const String& rSet)
                                {delete pTOUName; pTOUName = new String(rSet);}
    const String*   GetTOUName() const {return pTOUName;}
};

class SwTOXMgr
{
    SwWrtShell*         pSh;
    SwTOXMark*          pCurTOXMark;
    SwTOXMarks          aCurMarks;

    USHORT              GetUserTypeID(const String& rStr);

public:
    SwTOXMgr(SwWrtShell* pShell);
    //
    // Methoden fuer Verzeichnismarkierungen
    //
    void    InsertTOXMark(const SwTOXMarkDescription& rDesc);

    void    UpdateTOXMark(const SwTOXMarkDescription& rDesc);


    void                DeleteTOXMark();
    void                NextTOXMark(BOOL bSame=FALSE);
    void                PrevTOXMark(BOOL bSame=FALSE);

    // Aktuelle TOXmarks holen
    USHORT              GetTOXMarks();
    USHORT              GetTOXMarkCount();
    SwTOXMark*          GetTOXMark(USHORT nId);
    SwTOXMark*          GetCurTOXMark();
    void                SetCurTOXMark(USHORT nId);

    //
    // Methoden fuer Verzeichnisse
    //
    BOOL    UpdateOrInsertTOX(const SwTOXDescription& rDesc, SwTOXBase** ppBase = 0, const SfxItemSet* pSet = 0);

#ifdef USE_OLD_INDEXES
    void    InsertTOI(const USHORT nOptions = TOI_SAME_ENTRY|TOI_FF|TOI_CASE_SENSITIVE,
                        const String* pTitel = 0,
                        const SwForm* pForm  = 0,
                        SwTOXBase** ppBase = 0 );

    BOOL    UpdateTOI(const USHORT nOptions,
                      const String* pTitel = 0,
                      const SwForm* pForm = 0);


    void    InsertTOC(const USHORT nContent = TOX_MARK | TOX_OUTLINELEVEL,
                        int nLevel = MAXLEVEL,
                        const String* pTitel = 0,
                        const SwForm* pForm = 0,
                        SwTOXBase** ppBase = 0);

    BOOL    UpdateTOC(const USHORT nContent = TOX_MARK | TOX_OUTLINELEVEL,
                      int nLevel = MAXLEVEL,
                      const String* pTitel = 0,
                      const SwForm* pForm = 0);

    void    InsertTOU(const USHORT nContent = TOX_MARK,
                        const String* pTOUName = 0,
                        const String* pTemplateName = 0,
                        int nLevel = MAXLEVEL,
                        const String* pTitel = 0,
                        const SwForm* pForm = 0,
                        SwTOXBase** ppBase = 0);

    BOOL    UpdateTOU(const USHORT nContent,
                        const String* pTOUName = 0,
                        const String* pTemplateName = 0,
                        int nLevel = MAXLEVEL,
                        const String* pTitel = 0,
                        const SwForm* pForm = 0);

#endif

    const SwTOXType*    GetTOXType(TOXTypes eTyp, USHORT nId) const;
    const SwTOXBase*    GetCurTOX();

};


/*--------------------------------------------------------------------
    Beschreibung: Inlines
 --------------------------------------------------------------------*/

inline USHORT SwTOXMgr::GetTOXMarkCount()
    {   return aCurMarks.Count();   }


inline SwTOXMark* SwTOXMgr::GetCurTOXMark()
    {   return pCurTOXMark; }

#endif
