/*************************************************************************
 *
 *  $RCSfile: swmodule.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: tl $ $Date: 2001-02-27 14:47:40 $
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
#ifndef _SWMODULE_HXX
#define _SWMODULE_HXX


#ifndef _LINK_HXX //autogen
#include <tools/link.hxx>
#endif
#ifndef _SFXMODULE_HXX //autogen
#include <sfx2/module.hxx>
#endif

#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif
#ifndef SW_SWDLL_HXX
#include <swdll.hxx>
#endif
#include "shellid.hxx"
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XLINGUSERVICEEVENTLISTENER_HPP_
#include <com/sun/star/linguistic2/XLinguServiceEventListener.hpp>
#endif
#ifndef _VCL_FLDUNIT_HXX
#include <vcl/fldunit.hxx>
#endif

class SvStringsDtor;
class Color;
class AuthorCharAttr;
class SfxItemSet;
class SfxRequest;
class SfxErrorHandler;
class SwSrcViewConfig;
class SwDBConfig;
class SwModuleOptions;
class SwMasterUsrPref;
class SwViewOption;
class SwView;
class SwWrtShell;
class SwPrintOptions;
class SwAutoFmtOpt;
class SwChapterNumRules;
class SwStdFontConfig;
class SwNavigationConfig;
class SwDataExchange;
class SwToolbarConfigItem;
class SwAttrPool;
struct SwDBData;
#define VIEWOPT_DEST_VIEW       0
#define VIEWOPT_DEST_TEXT       1
#define VIEWOPT_DEST_WEB        2
#define VIEWOPT_DEST_VIEW_ONLY  3 //ViewOptions werden nur an der ::com::sun::star::sdbcx::View, nicht an der Appl. gesetzt

namespace com{ namespace sun{ namespace star{ namespace scanner{
    class XScannerManager;
}}}}

class SwModule: public SwModuleDummy , public SfxListener
{
    String              sActAuthor;

    // ConfigItems
    SwSrcViewConfig*    pSrcViewConfig;
    SwModuleOptions*    pModuleConfig;
    SwMasterUsrPref*    pUsrPref;
    SwMasterUsrPref*    pWebUsrPref;
    SwPrintOptions*     pPrtOpt;
    SwPrintOptions*     pWebPrtOpt;
    SwChapterNumRules*  pChapterNumRules;
    SwStdFontConfig*    pStdFontConfig;
    SwNavigationConfig* pNavigationConfig;
    SwToolbarConfigItem*pToolbarConfig;     //fÅr gestackte Toolbars, welche
    SwToolbarConfigItem*pWebToolbarConfig;  //war sichtbar?
    SwDBConfig*         pDBConfig;

    SfxErrorHandler*    pErrorHdl;

    SwAttrPool          *pAttrPool;

    // Die aktuelle ::com::sun::star::sdbcx::View wird hier gehalten um nicht ueber
    // GetActiveView arbeiten zu muessen
    // Die ::com::sun::star::sdbcx::View ist solange gueltig bis Sie im Activate
    // zerstoert oder ausgetauscht wird
    SwView*             pView;

    // Liste aller Redline-Autoren
    SvStringsDtor*      pAuthorNames;

    // DictionaryList listener to trigger spellchecking or hyphenation
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XLinguServiceEventListener >     xLngSvcEvtListener;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::scanner::XScannerManager >                    m_xScannerManager;

    sal_Bool                bAuthorInitialised : 1;
    sal_Bool                bEmbeddedLoadSave : 1;

    virtual void    FillStatusBar( StatusBar& );

    // Hint abfangen fuer DocInfo
    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

protected:
    // Briefumschlaege, Etiketten
    void                InsertEnv();
    void                InsertLab(SfxRequest&, sal_Bool bLabel);

public:
    TYPEINFO();
    SFX_DECL_INTERFACE(SW_INTERFACE_MODULE);

    // dieser Ctor nur fuer SW-Dll
    SwModule( SvFactory* pFact,
                SvFactory* pWebFact,
                    SvFactory* pGlobalFact );
    // dieser Ctor nur fuer Web-Dll
    SwModule( SvFactory* pWebFact );

    ~SwModule();

    virtual SfxModule* Load();

    // ::com::sun::star::sdbcx::View setzen nur fuer internen Gebrauch,
    // aus techn. Gruenden public
    //
    inline  void        SetView(SwView* pVw) { pView = pVw; }
    inline  SwView*     GetView() { return pView; }

    //Die Handler fuer die Slots
    void                StateOther(SfxItemSet &);   // andere
    void                StateViewOptions(SfxItemSet &);
    void                StateIsView(SfxItemSet &);

    void                ExecOther(SfxRequest &);    // Felder, Formel ..
    void                ExecViewOptions(SfxRequest &);
    void                ExecWizzard(SfxRequest &);
    void                ExecDB(SfxRequest &);       // DBManager

    // Benutzereinstellungen modifizieren
    const SwMasterUsrPref *GetUsrPref(sal_Bool bWeb) const;
    const SwViewOption* GetViewOption(sal_Bool bWeb);
    void                MakeUsrPref( SwViewOption &rToFill, sal_Bool bWeb ) const;
    void                ApplyUsrPref(const SwViewOption &, SwView*,
                                     sal_uInt16 nDest = VIEWOPT_DEST_VIEW );
    void ApplyUserMetric( FieldUnit eMetric, BOOL bWeb );
    void ApplyFldUpdateFlags(sal_Int32 nFldFlags);
    void ApplyLinkMode(sal_Int32 nNewLinkMode);
    void ApplyLinkFieldToDocMode(BOOL bFldDocOnly, BOOL bLinkDocOnly);


    // ConfigItems erzeugen
    SwSrcViewConfig*    GetSourceViewConfig();
    SwModuleOptions*    GetModuleConfig()       { return pModuleConfig;}
    SwPrintOptions*     GetPrtOptions(sal_Bool bWeb);
    SwChapterNumRules*  GetChapterNumRules();
    SwStdFontConfig*    GetStdFontConfig()      { return pStdFontConfig; }
    SwNavigationConfig* GetNavigationConfig();
    SwToolbarConfigItem*GetToolbarConfig()      { return pToolbarConfig;    }
    SwToolbarConfigItem*GetWebToolbarConfig()   { return pWebToolbarConfig; }
    SwDBConfig*         GetDBConfig();

    // UNO
//  virtual Reflection* GetReflection( UsrUik aUIK );

    // Ueber Sichten iterieren
    static SwView*      GetFirstView();
    static SwView*      GetNextView(SwView*);

    sal_Bool IsEmbeddedLoadSave() const         { return bEmbeddedLoadSave; }
    void SetEmbeddedLoadSave( sal_Bool bFlag )  { bEmbeddedLoadSave = bFlag; }


    // Public Data
    //
    SwDataExchange*     pClipboard;
    SwDataExchange*     pDragDrop;

    void ShowDBObj(SwWrtShell& rShell, const SwDBData& rData, sal_Bool bShowError = sal_False);

    // Tabellenmodi
    sal_Bool                IsInsTblFormatNum(sal_Bool bHTML) const;
    sal_Bool                IsInsTblChangeNumFormat(sal_Bool bHTML) const;
    sal_Bool                IsInsTblAlignNum(sal_Bool bHTML) const;

    // Redlining
    sal_uInt16              GetRedlineAuthor();
    sal_uInt16              GetRedlineAuthorCount();
    const String&       GetRedlineAuthor(sal_uInt16 nPos);
    sal_uInt16              InsertRedlineAuthor(const String& rAuthor);

    void                GetInsertAuthorAttr(sal_uInt16 nAuthor, SfxItemSet &rSet);
    void                GetDeletedAuthorAttr(sal_uInt16 nAuthor, SfxItemSet &rSet);
    void                GetFormatAuthorAttr(sal_uInt16 nAuthor, SfxItemSet &rSet);

    const AuthorCharAttr&   GetInsertAuthorAttr() const;
    const AuthorCharAttr&   GetDeletedAuthorAttr() const;
    const AuthorCharAttr&   GetFormatAuthorAttr() const;

    sal_uInt16              GetRedlineMarkPos();
    const Color&        GetRedlineMarkColor();

    // returne den definierten DocStat - WordDelimiter
    const String&       GetDocStatWordDelim() const;

    // Durchreichen der Metric von der ModuleConfig (fuer HTML-Export)
    sal_uInt16 GetMetric( sal_Bool bWeb ) const;

    // Update-Stati durchreichen
    sal_uInt16 GetLinkUpdMode( sal_Bool bWeb ) const;
    sal_uInt16 GetFldUpdateFlags( sal_Bool bWeb ) const;

    //virtuelle Methoden fuer den Optionendialog
    virtual SfxItemSet*  CreateItemSet( sal_uInt16 nId );
    virtual void         ApplyItemSet( sal_uInt16 nId, const SfxItemSet& rSet );
    virtual SfxTabPage*  CreateTabPage( sal_uInt16 nId, Window* pParent, const SfxItemSet& rSet );

    //hier wird der Pool angelegt und an der SfxShell gesetzt
    void    InitAttrPool();
    //Pool loeschen bevor es zu spaet ist
    void    RemoveAttrPool();

    // Invalidiert ggf. OnlineSpell-WrongListen
    void    CheckSpellChanges( sal_Bool bOnlineSpelling,
                    sal_Bool bIsSpellWrongAgain, sal_Bool bIsSpellAllAgain );

    inline ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XLinguServiceEventListener >
            GetLngSvcEvtListener();
    inline void SetLngSvcEvtListener(
            ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic2::XLinguServiceEventListener > & xLstnr);

    ::com::sun::star::uno::Reference<
        ::com::sun::star::scanner::XScannerManager >
            GetScannerManager() const {return m_xScannerManager;}

};


inline ::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic2::XLinguServiceEventListener >
        SwModule::GetLngSvcEvtListener()
{
    return xLngSvcEvtListener;
}

inline void SwModule::SetLngSvcEvtListener(
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XLinguServiceEventListener > & xLstnr)
{
    xLngSvcEvtListener = xLstnr;
}


/*-----------------08.07.97 10.33-------------------
    Zugriff auf das SwModule, die ::com::sun::star::sdbcx::View und die Shell
--------------------------------------------------*/

#define SW_MOD() ( *(SwModule**) GetAppData(SHL_WRITER))
SwView*     GetActiveView();
SwWrtShell* GetActiveWrtShell();



#endif
