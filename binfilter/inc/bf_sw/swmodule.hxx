/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include <bf_sfx2/module.hxx>

#include <bf_svtools/lstner.hxx>
#include <swdll.hxx>
#include "shellid.hxx"
#include <tools/string.hxx>
#include <com/sun/star/linguistic2/XLinguServiceEventListener.hpp>
#include <vcl/fldunit.hxx>
namespace com{ namespace sun{ namespace star{ namespace scanner{
    class XScannerManager;
}}}}

class Color;

namespace binfilter {
class SvStringsDtor;
class SvtAccessibilityOptions;
class SvtCTLOptions;
class SfxItemSet;
class SfxErrorHandler;
class ColorConfig;
class AuthorCharAttr;

class SfxRequest;

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
class SwAttrPool;

struct SwDBData;
#define VIEWOPT_DEST_VIEW 		0
#define VIEWOPT_DEST_TEXT		1
#define VIEWOPT_DEST_WEB    	2
#define VIEWOPT_DEST_VIEW_ONLY 	3 //ViewOptions werden nur an der ::com::sun::star::sdbcx::View, nicht an der Appl. gesetzt

class SwModule: public SwModuleDummy , public SfxListener
{
    String				sActAuthor;

    // ConfigItems
    SwModuleOptions*    pModuleConfig;
    SwMasterUsrPref*	pUsrPref;
    SwMasterUsrPref*	pWebUsrPref;
    SwPrintOptions* 	pPrtOpt;
    SwPrintOptions* 	pWebPrtOpt;
    SwChapterNumRules*	pChapterNumRules;
    SwStdFontConfig*	pStdFontConfig;
    SwDBConfig*			pDBConfig;
    ColorConfig*		pColorConfig;
    SvtCTLOptions*      pCTLOptions;

    SfxErrorHandler* 	pErrorHdl;

    SwAttrPool			*pAttrPool;

    // Die aktuelle View wird hier gehalten um nicht ueber
    // GetActiveView arbeiten zu muessen
    // Die View ist solange gueltig bis Sie im Activate
    // zerstoert oder ausgetauscht wird
    SwView* 			pView;

    // Liste aller Redline-Autoren
    SvStringsDtor*		pAuthorNames;

    // DictionaryList listener to trigger spellchecking or hyphenation
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XLinguServiceEventListener > xLngSvcEvtListener;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::scanner::XScannerManager >	m_xScannerManager;

    sal_Bool				bAuthorInitialised : 1;
    sal_Bool				bEmbeddedLoadSave : 1;

    // Hint abfangen fuer DocInfo
    virtual void		Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

public:
    TYPEINFO();

    // dieser Ctor nur fuer SW-Dll
    SwModule( SvFactory* pFact,
                SvFactory* pWebFact,
                    SvFactory* pGlobalFact );
    // dieser Ctor nur fuer Web-Dll
    SwModule( SvFactory* pWebFact );

    ~SwModule();

    virtual	SfxModule* Load();

    // View setzen nur fuer internen Gebrauch,
    // aus techn. Gruenden public
    //
    inline	void		SetView(SwView* pVw) { pView = pVw; }
    inline	SwView*		GetView() { return pView; }

    // Benutzereinstellungen modifizieren
    const SwMasterUsrPref *GetUsrPref(sal_Bool bWeb) const;
    const SwViewOption*	GetViewOption(sal_Bool bWeb);
    void				ApplyUsrPref(const SwViewOption &, SwView*,
                                     sal_uInt16 nDest = VIEWOPT_DEST_VIEW );

    // ConfigItems erzeugen
    SwModuleOptions*    GetModuleConfig()		{ return pModuleConfig;}
    SwPrintOptions* 	GetPrtOptions(sal_Bool bWeb);
    SwStdFontConfig*	GetStdFontConfig()		{ return pStdFontConfig; }
    SwDBConfig*			GetDBConfig();
    ColorConfig&		GetColorConfig();
    SvtCTLOptions&      GetCTLOptions();

    sal_Bool IsEmbeddedLoadSave() const 		{ return bEmbeddedLoadSave; }
    void SetEmbeddedLoadSave( sal_Bool bFlag )	{ bEmbeddedLoadSave = bFlag; }

    // Redlining
     sal_uInt16			GetRedlineAuthor();
     const String&		GetRedlineAuthor(sal_uInt16 nPos);
/*N*/ 	sal_uInt16			InsertRedlineAuthor(const String& rAuthor); //SW50.SDW

    sal_uInt16				GetRedlineMarkPos();

    // returne den definierten DocStat - WordDelimiter
    const String&		GetDocStatWordDelim() const;

    // Durchreichen der Metric von der ModuleConfig (fuer HTML-Export)
    sal_uInt16 GetMetric( sal_Bool bWeb ) const;

    // Update-Stati durchreichen
    sal_uInt16 GetLinkUpdMode( sal_Bool bWeb ) const;
    sal_uInt16 GetFldUpdateFlags( sal_Bool bWeb ) const;

    //hier wird der Pool angelegt und an der SfxShell gesetzt
    void	InitAttrPool();
    //Pool loeschen bevor es zu spaet ist
    void	RemoveAttrPool();

    inline ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XLinguServiceEventListener >
            GetLngSvcEvtListener();
    inline void	SetLngSvcEvtListener( ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XLinguServiceEventListener > & xLstnr);
    void    CreateLngSvcEvtListener();

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

#define SW_MOD() ( *(SwModule**) GetAppData(BF_SHL_WRITER))
SwView* 	GetActiveView();



} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
