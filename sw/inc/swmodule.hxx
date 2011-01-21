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
#ifndef _SWMODULE_HXX
#define _SWMODULE_HXX
#include <tools/link.hxx>
#include <tools/string.hxx>
#include <tools/fldunit.hxx>
#include <svl/lstner.hxx>
#include <unotools/options.hxx>
#include <sfx2/module.hxx>

#include <tools/shl.hxx>
#include <svl/lstner.hxx>
#include "swdllapi.h"
#include "shellid.hxx"
#include <fldupde.hxx>
#include <com/sun/star/linguistic2/XLinguServiceEventListener.hpp>
#include <com/sun/star/linguistic2/XLanguageGuessing.hpp>

class SvStringsDtor;
class Color;
class SfxItemSet;
class SfxRequest;
class SfxErrorHandler;
class SwDBConfig;
class SwModuleOptions;
class SwMasterUsrPref;
class SwViewOption;
class SwView;
class SwWrtShell;
class SwPrintOptions;
class SwChapterNumRules;
class SwStdFontConfig;
class SwNavigationConfig;
class SwTransferable;
class SwToolbarConfigItem;
class SwAttrPool;
namespace svtools{ class ColorConfig;}
class SvtAccessibilityOptions;
class SvtCTLOptions;
class SvtUserOptions;
class SvtUndoOptions;

struct SwDBData;
#define VIEWOPT_DEST_VIEW       0
#define VIEWOPT_DEST_TEXT       1
#define VIEWOPT_DEST_WEB        2
#define VIEWOPT_DEST_VIEW_ONLY  3 //ViewOptions werden nur an der ::com::sun::star::sdbcx::View, nicht an der Appl. gesetzt

namespace com{ namespace sun{ namespace star{ namespace scanner{
    class XScannerManager;
}}}}

class SW_DLLPUBLIC SwModule: public SfxModule, public SfxListener, public utl::ConfigurationListener
{
    String              sActAuthor;

    // ConfigItems
    SwModuleOptions*    pModuleConfig;
    SwMasterUsrPref*    pUsrPref;
    SwMasterUsrPref*    pWebUsrPref;
    SwPrintOptions*     pPrtOpt;
    SwPrintOptions*     pWebPrtOpt;
    SwChapterNumRules*  pChapterNumRules;
    SwStdFontConfig*    pStdFontConfig;
    SwNavigationConfig* pNavigationConfig;
    SwToolbarConfigItem*pToolbarConfig;     //fuer gestackte Toolbars, welche
    SwToolbarConfigItem*pWebToolbarConfig;  //war sichtbar?
    SwDBConfig*         pDBConfig;
    svtools::ColorConfig*   pColorConfig;
    SvtAccessibilityOptions* pAccessibilityOptions;
    SvtCTLOptions*      pCTLOptions;
    SvtUserOptions*     pUserOptions;
    SvtUndoOptions*     pUndoOptions;

    SfxErrorHandler*    pErrorHdl;

    SwAttrPool          *pAttrPool;

    // Die aktuelle View wird hier gehalten um nicht ueber
    // GetActiveView arbeiten zu muessen
    // Die View ist solange gueltig bis Sie im Activate
    // zerstoert oder ausgetauscht wird
    SwView*             pView;

    // Liste aller Redline-Autoren
    SvStringsDtor*      pAuthorNames;

    // DictionaryList listener to trigger spellchecking or hyphenation
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XLinguServiceEventListener > xLngSvcEvtListener;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::scanner::XScannerManager >    m_xScannerManager;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XLanguageGuessing >  m_xLanguageGuesser;

    sal_Bool                bAuthorInitialised : 1;
    sal_Bool                bEmbeddedLoadSave : 1;

    // Hint abfangen fuer DocInfo
    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    virtual void        ConfigurationChanged( utl::ConfigurationBroadcaster*, sal_uInt32 );

protected:
    // Briefumschlaege, Etiketten
    void                InsertEnv(SfxRequest&);
    void                InsertLab(SfxRequest&, sal_Bool bLabel);

public:
    // public Data - used for internal Clipboard / Drag & Drop / XSelection
    SwTransferable  *pDragDrop, *pXSelection;

    TYPEINFO();
    SFX_DECL_INTERFACE(SW_INTERFACE_MODULE)

    // dieser Ctor nur fuer SW-Dll
    SwModule( SfxObjectFactory* pFact,
                SfxObjectFactory* pWebFact,
                    SfxObjectFactory* pGlobalFact );

    ~SwModule();

    // View setzen nur fuer internen Gebrauch,
    // aus techn. Gruenden public
    //
    inline  void        SetView(SwView* pVw) { pView = pVw; }
    inline  SwView*     GetView() { return pView; }

    //Die Handler fuer die Slots
    void                StateOther(SfxItemSet &);   // andere

    void                ExecOther(SfxRequest &);    // Felder, Formel ..

    // Benutzereinstellungen modifizieren
    const SwMasterUsrPref *GetUsrPref(sal_Bool bWeb) const;
    const SwViewOption* GetViewOption(sal_Bool bWeb);
    void                ApplyUsrPref(const SwViewOption &, SwView*,
                                     sal_uInt16 nDest = VIEWOPT_DEST_VIEW );
    void ApplyUserMetric( FieldUnit eMetric, sal_Bool bWeb );
    void ApplyRulerMetric( FieldUnit eMetric, sal_Bool bHorizontal, sal_Bool bWeb );
    void ApplyFldUpdateFlags(SwFldUpdateFlags eFldFlags);
    void ApplyLinkMode(sal_Int32 nNewLinkMode);

    //default page mode for text grid
    void ApplyDefaultPageMode(sal_Bool bIsSquaredPageMode);

    // ConfigItems erzeugen
    SwModuleOptions*    GetModuleConfig()       { return pModuleConfig;}
    SwPrintOptions*     GetPrtOptions(sal_Bool bWeb);
    SwChapterNumRules*  GetChapterNumRules();
    SwStdFontConfig*    GetStdFontConfig()      { return pStdFontConfig; }
    SwNavigationConfig* GetNavigationConfig();
    SwToolbarConfigItem*GetToolbarConfig()      { return pToolbarConfig;    }
    SwToolbarConfigItem*GetWebToolbarConfig()   { return pWebToolbarConfig; }
    SwDBConfig*         GetDBConfig();
    svtools::ColorConfig&   GetColorConfig();
    SvtAccessibilityOptions&    GetAccessibilityOptions();
    SvtCTLOptions&      GetCTLOptions();
    SvtUserOptions&     GetUserOptions();
    SvtUndoOptions&     GetUndoOptions();

    // Ueber Sichten iterieren
    static SwView*      GetFirstView();
    static SwView*      GetNextView(SwView*);

    sal_Bool IsEmbeddedLoadSave() const         { return bEmbeddedLoadSave; }
    void SetEmbeddedLoadSave( sal_Bool bFlag )  { bEmbeddedLoadSave = bFlag; }

    void ShowDBObj( SwView& rView, const SwDBData& rData, sal_Bool bOnlyIfAvailable = sal_False);

    // Tabellenmodi
    sal_Bool            IsInsTblFormatNum(sal_Bool bHTML) const;
    sal_Bool            IsInsTblChangeNumFormat(sal_Bool bHTML) const;
    sal_Bool            IsInsTblAlignNum(sal_Bool bHTML) const;

    // Redlining
    sal_uInt16          GetRedlineAuthor();
    const String&       GetRedlineAuthor(sal_uInt16 nPos);
    sal_uInt16          InsertRedlineAuthor(const String& rAuthor);

    void                GetInsertAuthorAttr(sal_uInt16 nAuthor, SfxItemSet &rSet);
    void                GetDeletedAuthorAttr(sal_uInt16 nAuthor, SfxItemSet &rSet);
    void                GetFormatAuthorAttr(sal_uInt16 nAuthor, SfxItemSet &rSet);

    sal_uInt16              GetRedlineMarkPos();
    const Color&            GetRedlineMarkColor();

    // returne den definierten DocStat - WordDelimiter
    const String&       GetDocStatWordDelim() const;

    // Durchreichen der Metric von der ModuleConfig (fuer HTML-Export)
    sal_uInt16 GetMetric( sal_Bool bWeb ) const;

    // Update-Stati durchreichen
    sal_uInt16 GetLinkUpdMode( sal_Bool bWeb ) const;
    SwFldUpdateFlags GetFldUpdateFlags( sal_Bool bWeb ) const;

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
                    sal_Bool bIsSpellWrongAgain, sal_Bool bIsSpellAllAgain, sal_Bool bSmartTags );

    inline ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XLinguServiceEventListener >
            GetLngSvcEvtListener();
    inline void SetLngSvcEvtListener( ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XLinguServiceEventListener > & xLstnr);
    void    CreateLngSvcEvtListener();

    ::com::sun::star::uno::Reference<
        ::com::sun::star::scanner::XScannerManager >
            GetScannerManager();

    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XLanguageGuessing >
            GetLanguageGuesser();
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

SW_DLLPUBLIC SwView*    GetActiveView();
SW_DLLPUBLIC SwWrtShell* GetActiveWrtShell();

#endif
