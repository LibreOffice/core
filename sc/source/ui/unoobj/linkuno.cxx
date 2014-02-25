/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <svl/smplhint.hxx>
#include <sfx2/linkmgr.hxx>
#include <vcl/svapp.hxx>

#include "linkuno.hxx"
#include "miscuno.hxx"
#include "convuno.hxx"
#include "docsh.hxx"
#include "docfunc.hxx"
#include "tablink.hxx"
#include "arealink.hxx"
#include "hints.hxx"
#include "unonames.hxx"
#include "rangeseq.hxx"
#include "token.hxx"
#include "scmatrix.hxx"
#include <documentlinkmgr.hxx>

#include <vector>
#include <climits>

using namespace com::sun::star;
using namespace formula;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::lang::IllegalArgumentException;
using ::com::sun::star::uno::RuntimeException;
using ::std::vector;



//  fuer Sheet- und Area-Links benutzt:
static const SfxItemPropertyMapEntry* lcl_GetSheetLinkMap()
{
    static const SfxItemPropertyMapEntry aSheetLinkMap_Impl[] =
    {
        {OUString(SC_UNONAME_FILTER),   0,  getCppuType((OUString*)0),    0, 0 },
        {OUString(SC_UNONAME_FILTOPT),  0,  getCppuType((OUString*)0),    0, 0 },
        {OUString(SC_UNONAME_LINKURL),  0,  getCppuType((OUString*)0),    0, 0 },
        {OUString(SC_UNONAME_REFDELAY), 0,  getCppuType((sal_Int32*)0),        0, 0 },
        {OUString(SC_UNONAME_REFPERIOD),    0,  getCppuType((sal_Int32*)0),        0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aSheetLinkMap_Impl;
}



SC_SIMPLE_SERVICE_INFO( ScAreaLinkObj, "ScAreaLinkObj", "com.sun.star.sheet.CellAreaLink" )
SC_SIMPLE_SERVICE_INFO( ScAreaLinksObj, "ScAreaLinksObj", "com.sun.star.sheet.CellAreaLinks" )
SC_SIMPLE_SERVICE_INFO( ScDDELinkObj, "ScDDELinkObj", "com.sun.star.sheet.DDELink" )
SC_SIMPLE_SERVICE_INFO( ScDDELinksObj, "ScDDELinksObj", "com.sun.star.sheet.DDELinks" )
SC_SIMPLE_SERVICE_INFO( ScSheetLinkObj, "ScSheetLinkObj", "com.sun.star.sheet.SheetLink" )
SC_SIMPLE_SERVICE_INFO( ScSheetLinksObj, "ScSheetLinksObj", "com.sun.star.sheet.SheetLinks" )



ScSheetLinkObj::ScSheetLinkObj(ScDocShell* pDocSh, const OUString& rName) :
    aPropSet( lcl_GetSheetLinkMap() ),
    pDocShell( pDocSh ),
    aFileName( rName )
{
    pDocShell->GetDocument()->AddUnoObject(*this);
}

ScSheetLinkObj::~ScSheetLinkObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScSheetLinkObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    //! notify if links in document are changed
    //  UpdateRef is not needed here

    if ( rHint.ISA( SfxSimpleHint ) )
    {
        if ( ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
            pDocShell = NULL;       // pointer is invalid
    }
    else if ( rHint.ISA( ScLinkRefreshedHint ) )
    {
        const ScLinkRefreshedHint& rLH = (const ScLinkRefreshedHint&) rHint;
        if ( rLH.GetLinkType() == SC_LINKREFTYPE_SHEET && rLH.GetUrl() == aFileName )
            Refreshed_Impl();
    }
}

ScTableLink* ScSheetLinkObj::GetLink_Impl() const
{
    if (pDocShell)
    {
        sfx2::LinkManager* pLinkManager = pDocShell->GetDocument()->GetLinkManager();
        size_t nCount = pLinkManager->GetLinks().size();
        for (size_t i=0; i<nCount; i++)
        {
            ::sfx2::SvBaseLink* pBase = *pLinkManager->GetLinks()[i];
            if (pBase->ISA(ScTableLink))
            {
                ScTableLink* pTabLink = (ScTableLink*)pBase;
                if ( pTabLink->GetFileName().equals(aFileName) )
                    return pTabLink;
            }
        }
    }
    return NULL;    // nicht gefunden
}

// XNamed

OUString SAL_CALL ScSheetLinkObj::getName() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return getFileName();   // Name ist der Dateiname (URL)
}

void SAL_CALL ScSheetLinkObj::setName( const OUString& aName ) throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    setFileName(aName);     // Name ist der Dateiname (URL)
}

// XRefreshable

void SAL_CALL ScSheetLinkObj::refresh() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    ScTableLink* pLink = GetLink_Impl();
    if (pLink)
        pLink->Refresh( pLink->GetFileName(), pLink->GetFilterName(), NULL, pLink->GetRefreshDelay() );
}

void SAL_CALL ScSheetLinkObj::addRefreshListener(
                                const uno::Reference<util::XRefreshListener >& xListener )
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference<util::XRefreshListener>* pObj =
            new uno::Reference<util::XRefreshListener>( xListener );
    aRefreshListeners.push_back( pObj );

    //  hold one additional ref to keep this object alive as long as there are listeners
    if ( aRefreshListeners.size() == 1 )
        acquire();
}

void SAL_CALL ScSheetLinkObj::removeRefreshListener(
                                const uno::Reference<util::XRefreshListener >& xListener )
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    size_t nCount = aRefreshListeners.size();
    for ( size_t n=nCount; n--; )
    {
        uno::Reference<util::XRefreshListener>& rObj = aRefreshListeners[n];
        if ( rObj == xListener )
        {
            aRefreshListeners.erase( aRefreshListeners.begin() + n );
            if ( aRefreshListeners.empty() )
                release();                          // release ref for listeners
            break;
        }
    }
}

void ScSheetLinkObj::Refreshed_Impl()
{
    lang::EventObject aEvent;
    aEvent.Source.set((cppu::OWeakObject*)this);
    for ( size_t n=0; n<aRefreshListeners.size(); n++ )
        aRefreshListeners[n]->refreshed( aEvent );
}

void ScSheetLinkObj::ModifyRefreshDelay_Impl( sal_Int32 nRefresh )
{
    ScTableLink* pLink = GetLink_Impl();
    if( pLink )
        pLink->SetRefreshDelay( (sal_uLong) nRefresh );
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScSheetLinkObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( aPropSet.getPropertyMap() ));
    return aRef;
}

void SAL_CALL ScSheetLinkObj::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    OUString aNameString(aPropertyName);
    OUString aValStr;
    if ( aNameString.equalsAscii( SC_UNONAME_LINKURL ) )
    {
        if ( aValue >>= aValStr )
            setFileName( aValStr );
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_FILTER ) )
    {
        if ( aValue >>= aValStr )
            setFilter( aValStr );
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_FILTOPT ) )
    {
        if ( aValue >>= aValStr )
            setFilterOptions( aValStr );
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_REFPERIOD ) )
    {
        sal_Int32 nRefresh = 0;
        if ( aValue >>= nRefresh )
            setRefreshDelay( nRefresh );
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_REFDELAY ) )
    {
        sal_Int32 nRefresh = 0;
        if ( aValue >>= nRefresh )
            setRefreshDelay( nRefresh );
    }
}

uno::Any SAL_CALL ScSheetLinkObj::getPropertyValue( const OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    OUString aNameString(aPropertyName);
    uno::Any aRet;
    if ( aNameString.equalsAscii( SC_UNONAME_LINKURL ) )
        aRet <<= getFileName();
    else if ( aNameString.equalsAscii( SC_UNONAME_FILTER ) )
        aRet <<= getFilter();
    else if ( aNameString.equalsAscii( SC_UNONAME_FILTOPT ) )
        aRet <<= getFilterOptions();
    else if ( aNameString.equalsAscii( SC_UNONAME_REFPERIOD ) )
        aRet <<= getRefreshDelay();
    else if ( aNameString.equalsAscii( SC_UNONAME_REFDELAY ) )
        aRet <<= getRefreshDelay();
    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScSheetLinkObj )

// internal:

OUString ScSheetLinkObj::getFileName(void) const
{
    SolarMutexGuard aGuard;
    return aFileName;
}

void ScSheetLinkObj::setFileName(const OUString& rNewName)
{
    SolarMutexGuard aGuard;
    ScTableLink* pLink = GetLink_Impl();
    if (pLink)
    {
        //  pLink->Refresh mit neuem Dateinamen bringt sfx2::LinkManager durcheinander
        //  darum per Hand die Tabellen umsetzen und Link per UpdateLinks neu erzeugen

        OUString aNewStr(ScGlobal::GetAbsDocName( rNewName, pDocShell ));

        //  zuerst Tabellen umsetzen

        ScDocument* pDoc = pDocShell->GetDocument();
        SCTAB nTabCount = pDoc->GetTableCount();
        for (SCTAB nTab=0; nTab<nTabCount; nTab++)
            if ( pDoc->IsLinked(nTab) && pDoc->GetLinkDoc(nTab) == aFileName )  // alte Datei
                pDoc->SetLink( nTab, pDoc->GetLinkMode(nTab), aNewStr,
                                pDoc->GetLinkFlt(nTab), pDoc->GetLinkOpt(nTab),
                                pDoc->GetLinkTab(nTab),
                                pDoc->GetLinkRefreshDelay(nTab) );  // nur Datei aendern

        //  Links updaten
        //! Undo !!!

        pLink = NULL;               // wird bei UpdateLinks ungueltig
        pDocShell->UpdateLinks();   // alter Link raus, evtl. neuen Link anlegen

        //  Daten kopieren

        aFileName = aNewStr;
        pLink = GetLink_Impl();     // neuer Link mit neuem Namen
        if (pLink)
            pLink->Update();        // inkl. Paint & Undo fuer Daten
    }
}

OUString ScSheetLinkObj::getFilter(void) const
{
    SolarMutexGuard aGuard;
    OUString aRet;
    ScTableLink* pLink = GetLink_Impl();
    if (pLink)
        aRet = pLink->GetFilterName();
    return aRet;
}

void ScSheetLinkObj::setFilter(const OUString& Filter)
{
    SolarMutexGuard aGuard;
    ScTableLink* pLink = GetLink_Impl();
    if (pLink)
    {
        OUString aFilterStr(Filter);
        pLink->Refresh( aFileName, aFilterStr, NULL, pLink->GetRefreshDelay() );
    }
}

OUString ScSheetLinkObj::getFilterOptions(void) const
{
    SolarMutexGuard aGuard;
    OUString aRet;
    ScTableLink* pLink = GetLink_Impl();
    if (pLink)
        aRet = pLink->GetOptions();
    return aRet;
}

void ScSheetLinkObj::setFilterOptions(const OUString& FilterOptions)
{
    SolarMutexGuard aGuard;
    ScTableLink* pLink = GetLink_Impl();
    if (pLink)
    {
        OUString aOptStr(FilterOptions);
        pLink->Refresh( aFileName, pLink->GetFilterName(), &aOptStr, pLink->GetRefreshDelay() );
    }
}

sal_Int32 ScSheetLinkObj::getRefreshDelay(void) const
{
    SolarMutexGuard aGuard;
    sal_Int32 nRet = 0;
    ScTableLink* pLink = GetLink_Impl();
    if (pLink)
        nRet = (sal_Int32) pLink->GetRefreshDelay();
    return nRet;
}

void ScSheetLinkObj::setRefreshDelay(sal_Int32 nRefreshDelay)
{
    SolarMutexGuard aGuard;
    ModifyRefreshDelay_Impl( nRefreshDelay );
}



ScSheetLinksObj::ScSheetLinksObj(ScDocShell* pDocSh) :
    pDocShell( pDocSh )
{
    pDocShell->GetDocument()->AddUnoObject(*this);
}

ScSheetLinksObj::~ScSheetLinksObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScSheetLinksObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    //  Referenz-Update interessiert hier nicht

    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       // ungueltig geworden
    }
}

// XSheetLinks

ScSheetLinkObj* ScSheetLinksObj::GetObjectByIndex_Impl(sal_Int32 nIndex)
{
    if (!pDocShell)
        return NULL;

    typedef boost::unordered_set<OUString, OUStringHash> StrSetType;
    StrSetType aNames;
    ScDocument* pDoc = pDocShell->GetDocument();
    SCTAB nTabCount = pDoc->GetTableCount();
    sal_Int32 nCount = 0;
    for (SCTAB nTab = 0; nTab < nTabCount; ++nTab)
    {
        if (!pDoc->IsLinked(nTab))
            continue;

        OUString aLinkDoc = pDoc->GetLinkDoc(nTab);
        if (aNames.insert(aLinkDoc).second)
        {
            // unique document name.
            if (nCount == nIndex)
                return new ScSheetLinkObj( pDocShell, aLinkDoc );
            ++nCount;
        }
    }

    return NULL;    // kein Dokument oder Index zu gross
}

ScSheetLinkObj* ScSheetLinksObj::GetObjectByName_Impl(const OUString& aName)
{
    //  Name ist der Dateiname

    if (pDocShell)
    {
        OUString aNameStr(aName);

        ScDocument* pDoc = pDocShell->GetDocument();
        SCTAB nTabCount = pDoc->GetTableCount();
        for (SCTAB nTab=0; nTab<nTabCount; nTab++)
            if (pDoc->IsLinked(nTab))
            {
                //! case-insensitiv ???
                OUString aLinkDoc = pDoc->GetLinkDoc( nTab );
                if ( aLinkDoc == aNameStr )
                    return new ScSheetLinkObj( pDocShell, aNameStr );
            }
    }

    return NULL;
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScSheetLinksObj::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, OUString("com.sun.star.sheet.SheetLinksEnumeration"));
}

// XIndexAccess

sal_Int32 SAL_CALL ScSheetLinksObj::getCount() throw(uno::RuntimeException)
{
    typedef boost::unordered_set<OUString, OUStringHash> StrSetType;

    SolarMutexGuard aGuard;
    if (!pDocShell)
        return 0;

    sal_Int32 nCount = 0;

    StrSetType aNames;
    ScDocument* pDoc = pDocShell->GetDocument();
    SCTAB nTabCount = pDoc->GetTableCount();
    for (SCTAB nTab = 0; nTab < nTabCount; ++nTab)
    {
        if (!pDoc->IsLinked(nTab))
            continue;

        OUString aLinkDoc = pDoc->GetLinkDoc(nTab);
        if (aNames.insert(aLinkDoc).second)
            ++nCount;
    }
    return nCount;
}

uno::Any SAL_CALL ScSheetLinksObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference<beans::XPropertySet> xLink(GetObjectByIndex_Impl(nIndex));
    if (xLink.is())
        return uno::makeAny(xLink);
    else
        throw lang::IndexOutOfBoundsException();
}

uno::Type SAL_CALL ScSheetLinksObj::getElementType() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return getCppuType((uno::Reference<beans::XPropertySet>*)0);
}

sal_Bool SAL_CALL ScSheetLinksObj::hasElements() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

uno::Any SAL_CALL ScSheetLinksObj::getByName( const OUString& aName )
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference<beans::XPropertySet> xLink(GetObjectByName_Impl(aName));
    if (xLink.is())
        return uno::makeAny(xLink);
    else
        throw container::NoSuchElementException();
//    return uno::Any();
}

sal_Bool SAL_CALL ScSheetLinksObj::hasByName( const OUString& aName )
                                        throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    //  Name ist der Dateiname

    if (pDocShell)
    {
        OUString aNameStr(aName);

        ScDocument* pDoc = pDocShell->GetDocument();
        SCTAB nTabCount = pDoc->GetTableCount();
        for (SCTAB nTab=0; nTab<nTabCount; nTab++)
            if (pDoc->IsLinked(nTab))
            {
                //! case-insensitiv ???
                OUString aLinkDoc(pDoc->GetLinkDoc( nTab ));
                if ( aLinkDoc == aNameStr )
                    return sal_True;
            }
    }
    return false;
}

uno::Sequence<OUString> SAL_CALL ScSheetLinksObj::getElementNames() throw(uno::RuntimeException)
{
    typedef boost::unordered_set<OUString, OUStringHash> StrSetType;

    SolarMutexGuard aGuard;
    //  Name ist der Dateiname

    if (!pDocShell)
        return uno::Sequence<OUString>();

    StrSetType aNames;
    ScDocument* pDoc = pDocShell->GetDocument();
    SCTAB nTabCount = pDoc->GetTableCount();

    sal_Int32 nLinkCount = getCount();
    uno::Sequence<OUString> aSeq(nLinkCount);
    OUString* pAry = aSeq.getArray();
    size_t nPos = 0;
    for (SCTAB nTab = 0; nTab < nTabCount; ++nTab)
    {
        if (!pDoc->IsLinked(nTab))
            continue;

        OUString aLinkDoc = pDoc->GetLinkDoc(nTab);
        if (aNames.insert(aLinkDoc).second)
            pAry[nPos++] = aLinkDoc;
    }
    OSL_ENSURE( nPos==static_cast<size_t>(nLinkCount), "verzaehlt" );
    return aSeq;
}



static ScAreaLink* lcl_GetAreaLink( ScDocShell* pDocShell, size_t nPos )
{
    if (pDocShell)
    {
        sfx2::LinkManager* pLinkManager = pDocShell->GetDocument()->GetLinkManager();
        size_t nTotalCount = pLinkManager->GetLinks().size();
        size_t nAreaCount = 0;
        for (size_t i=0; i<nTotalCount; i++)
        {
            ::sfx2::SvBaseLink* pBase = *pLinkManager->GetLinks()[i];
            if (pBase->ISA(ScAreaLink))
            {
                if ( nAreaCount == nPos )
                    return static_cast<ScAreaLink*>(pBase);
                ++nAreaCount;
            }
        }
    }
    return NULL;    // nicht gefunden
}

ScAreaLinkObj::ScAreaLinkObj(ScDocShell* pDocSh, size_t nP) :
    aPropSet( lcl_GetSheetLinkMap() ),
    pDocShell( pDocSh ),
    nPos( nP )
{
    pDocShell->GetDocument()->AddUnoObject(*this);
}

ScAreaLinkObj::~ScAreaLinkObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScAreaLinkObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    //! notify if links in document are changed
    //  UpdateRef is not needed here

    if ( rHint.ISA( SfxSimpleHint ) )
    {
        if ( ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
            pDocShell = NULL;       // pointer is invalid
    }
    else if ( rHint.ISA( ScLinkRefreshedHint ) )
    {
        const ScLinkRefreshedHint& rLH = (const ScLinkRefreshedHint&) rHint;
        if ( rLH.GetLinkType() == SC_LINKREFTYPE_AREA )
        {
            //  get this link to compare dest position
            ScAreaLink* pLink = lcl_GetAreaLink(pDocShell, nPos);
            if ( pLink && pLink->GetDestArea().aStart == rLH.GetDestPos() )
                Refreshed_Impl();
        }
    }
}

// XFileLink

void ScAreaLinkObj::Modify_Impl( const OUString* pNewFile, const OUString* pNewFilter,
                                 const OUString* pNewOptions, const OUString* pNewSource,
                                 const table::CellRangeAddress* pNewDest )
{
    ScAreaLink* pLink = lcl_GetAreaLink(pDocShell, nPos);
    if (pLink)
    {
        OUString aFile    (pLink->GetFile());
        OUString aFilter  (pLink->GetFilter());
        OUString aOptions (pLink->GetOptions());
        OUString aSource  (pLink->GetSource());
        ScRange aDest   (pLink->GetDestArea());
        sal_uLong nRefresh  = pLink->GetRefreshDelay();

        //! Undo fuer Loeschen
        //! Undo zusammenfassen

        sfx2::LinkManager* pLinkManager = pDocShell->GetDocument()->GetLinkManager();
        pLinkManager->Remove( pLink );
        pLink = NULL;   // bei Remove geloescht

        sal_Bool bFitBlock = sal_True;          // verschieben, wenn durch Update Groesse geaendert
        if (pNewFile)
        {
            aFile = *pNewFile;
            aFile = ScGlobal::GetAbsDocName( aFile, pDocShell );    //! in InsertAreaLink?
        }
        if (pNewFilter)
            aFilter = *pNewFilter;
        if (pNewOptions)
            aOptions = *pNewOptions;
        if (pNewSource)
            aSource = *pNewSource;
        if (pNewDest)
        {
            ScUnoConversion::FillScRange( aDest, *pNewDest );
            bFitBlock = false;  // neuer Bereich angegeben -> keine Inhalte verschieben
        }
        pDocShell->GetDocFunc().InsertAreaLink( aFile, aFilter, aOptions, aSource,
                                                aDest, nRefresh, bFitBlock, true );
    }
}

void ScAreaLinkObj::ModifyRefreshDelay_Impl( sal_Int32 nRefresh )
{
    ScAreaLink* pLink = lcl_GetAreaLink( pDocShell, nPos );
    if( pLink )
        pLink->SetRefreshDelay( (sal_uLong) nRefresh );
}

// XRefreshable

void SAL_CALL ScAreaLinkObj::refresh()
    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    ScAreaLink* pLink = lcl_GetAreaLink(pDocShell, nPos);
    if (pLink)
        pLink->Refresh( pLink->GetFile(), pLink->GetFilter(), pLink->GetSource(), pLink->GetRefreshDelay() );
}

void SAL_CALL ScAreaLinkObj::addRefreshListener(
                                const uno::Reference<util::XRefreshListener >& xListener )
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference<util::XRefreshListener>* pObj =
            new uno::Reference<util::XRefreshListener>( xListener );
    aRefreshListeners.push_back( pObj );

    //  hold one additional ref to keep this object alive as long as there are listeners
    if ( aRefreshListeners.size() == 1 )
        acquire();
}

void SAL_CALL ScAreaLinkObj::removeRefreshListener(
                                const uno::Reference<util::XRefreshListener >& xListener )
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    size_t nCount = aRefreshListeners.size();
    for ( size_t n=nCount; n--; )
    {
        uno::Reference<util::XRefreshListener>& rObj = aRefreshListeners[n];
        if ( rObj == xListener )
        {
            aRefreshListeners.erase( aRefreshListeners.begin() + n );
            if ( aRefreshListeners.empty() )
                release();                          // release ref for listeners
            break;
        }

        if(n == 0)
            break;
    }
}

void ScAreaLinkObj::Refreshed_Impl()
{
    lang::EventObject aEvent;
    aEvent.Source.set((cppu::OWeakObject*)this);
    for ( size_t n=0; n<aRefreshListeners.size(); n++ )
        aRefreshListeners[n]->refreshed( aEvent );
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScAreaLinkObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( aPropSet.getPropertyMap() ));
    return aRef;
}

void SAL_CALL ScAreaLinkObj::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    OUString aNameString(aPropertyName);
    OUString aValStr;
    if ( aNameString.equalsAscii( SC_UNONAME_LINKURL ) )
    {
        if ( aValue >>= aValStr )
            setFileName( aValStr );
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_FILTER ) )
    {
        if ( aValue >>= aValStr )
            setFilter( aValStr );
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_FILTOPT ) )
    {
        if ( aValue >>= aValStr )
            setFilterOptions( aValStr );
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_REFPERIOD ) )
    {
        sal_Int32 nRefresh = 0;
        if ( aValue >>= nRefresh )
            setRefreshDelay( nRefresh );
    }
    else if ( aNameString.equalsAscii( SC_UNONAME_REFDELAY ) )
    {
        sal_Int32 nRefresh = 0;
        if ( aValue >>= nRefresh )
            setRefreshDelay( nRefresh );
    }
}

uno::Any SAL_CALL ScAreaLinkObj::getPropertyValue( const OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    OUString aNameString(aPropertyName);
    uno::Any aRet;
    if ( aNameString.equalsAscii( SC_UNONAME_LINKURL ) )
        aRet <<= getFileName();
    else if ( aNameString.equalsAscii( SC_UNONAME_FILTER ) )
        aRet <<= getFilter();
    else if ( aNameString.equalsAscii( SC_UNONAME_FILTOPT ) )
        aRet <<= getFilterOptions();
    else if ( aNameString.equalsAscii( SC_UNONAME_REFPERIOD ) )
        aRet <<= getRefreshDelay();
    else if ( aNameString.equalsAscii( SC_UNONAME_REFDELAY ) )
        aRet <<= getRefreshDelay();
    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScAreaLinkObj )

//  internal:

OUString ScAreaLinkObj::getFileName(void) const
{
    SolarMutexGuard aGuard;
    OUString aRet;
    ScAreaLink* pLink = lcl_GetAreaLink(pDocShell, nPos);
    if (pLink)
        aRet = pLink->GetFile();
    return aRet;
}

void ScAreaLinkObj::setFileName(const OUString& rNewName)
{
    SolarMutexGuard aGuard;
    Modify_Impl( &rNewName, NULL, NULL, NULL, NULL );
}

OUString ScAreaLinkObj::getFilter(void) const
{
    SolarMutexGuard aGuard;
    OUString aRet;
    ScAreaLink* pLink = lcl_GetAreaLink(pDocShell, nPos);
    if (pLink)
        aRet = pLink->GetFilter();
    return aRet;
}

void ScAreaLinkObj::setFilter(const OUString& Filter)
{
    SolarMutexGuard aGuard;
    Modify_Impl( NULL, &Filter, NULL, NULL, NULL );
}

OUString ScAreaLinkObj::getFilterOptions(void) const
{
    SolarMutexGuard aGuard;
    OUString aRet;
    ScAreaLink* pLink = lcl_GetAreaLink(pDocShell, nPos);
    if (pLink)
        aRet = pLink->GetOptions();
    return aRet;
}

void ScAreaLinkObj::setFilterOptions(const OUString& FilterOptions)
{
    SolarMutexGuard aGuard;
    Modify_Impl( NULL, NULL, &FilterOptions, NULL, NULL );
}

sal_Int32 ScAreaLinkObj::getRefreshDelay(void) const
{
    SolarMutexGuard aGuard;
    sal_Int32 nRet = 0;
    ScAreaLink* pLink = lcl_GetAreaLink(pDocShell, nPos);
    if (pLink)
        nRet = (sal_Int32) pLink->GetRefreshDelay();
    return nRet;
}

void ScAreaLinkObj::setRefreshDelay(sal_Int32 nRefreshDelay)
{
    SolarMutexGuard aGuard;
    ModifyRefreshDelay_Impl( nRefreshDelay );
}

// XAreaLink

OUString SAL_CALL ScAreaLinkObj::getSourceArea() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    OUString aRet;
    ScAreaLink* pLink = lcl_GetAreaLink(pDocShell, nPos);
    if (pLink)
        aRet = pLink->GetSource();
    return aRet;
}

void SAL_CALL ScAreaLinkObj::setSourceArea( const OUString& aSourceArea )
                                            throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    Modify_Impl( NULL, NULL, NULL, &aSourceArea, NULL );
}

table::CellRangeAddress SAL_CALL ScAreaLinkObj::getDestArea() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    table::CellRangeAddress aRet;
    ScAreaLink* pLink = lcl_GetAreaLink(pDocShell, nPos);
    if (pLink)
        ScUnoConversion::FillApiRange( aRet, pLink->GetDestArea() );
    return aRet;
}

void SAL_CALL ScAreaLinkObj::setDestArea( const table::CellRangeAddress& aDestArea )
                                            throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    Modify_Impl( NULL, NULL, NULL, NULL, &aDestArea );
}



ScAreaLinksObj::ScAreaLinksObj(ScDocShell* pDocSh) :
    pDocShell( pDocSh )
{
    pDocShell->GetDocument()->AddUnoObject(*this);
}

ScAreaLinksObj::~ScAreaLinksObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScAreaLinksObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    //  Referenz-Update interessiert hier nicht

    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       // ungueltig geworden
    }
}

// XAreaLinks

ScAreaLinkObj* ScAreaLinksObj::GetObjectByIndex_Impl(sal_Int32 nIndex)
{
    if ( pDocShell && nIndex >= 0 && nIndex < getCount() )
        return new ScAreaLinkObj( pDocShell, (size_t)nIndex );

    return NULL;    // nicht gefunden
}

void SAL_CALL ScAreaLinksObj::insertAtPosition( const table::CellAddress& aDestPos,
                                                const OUString& aFileName,
                                                const OUString& aSourceArea,
                                                const OUString& aFilter,
                                                const OUString& aFilterOptions )
                                            throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        OUString aFileStr   (aFileName);
        OUString aFilterStr (aFilter);
        OUString aOptionStr (aFilterOptions);
        OUString aSourceStr (aSourceArea);
        ScAddress aDestAddr( (SCCOL)aDestPos.Column, (SCROW)aDestPos.Row, aDestPos.Sheet );

        aFileStr = ScGlobal::GetAbsDocName( aFileStr, pDocShell );  //! in InsertAreaLink ???
        pDocShell->GetDocFunc().InsertAreaLink( aFileStr, aFilterStr, aOptionStr,
                                                aSourceStr, ScRange(aDestAddr),
                                                0, false, true ); // don't move contents
    }
}

void SAL_CALL ScAreaLinksObj::removeByIndex( sal_Int32 nIndex ) throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    ScAreaLink* pLink = lcl_GetAreaLink(pDocShell, (size_t)nIndex);
    if (pLink)
    {
        //! SetAddUndo oder so

        sfx2::LinkManager* pLinkManager = pDocShell->GetDocument()->GetLinkManager();
        pLinkManager->Remove( pLink );
    }
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScAreaLinksObj::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, OUString("com.sun.star.sheet.CellAreaLinksEnumeration"));
}

// XIndexAccess

sal_Int32 SAL_CALL ScAreaLinksObj::getCount() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    sal_Int32 nAreaCount = 0;
    if (pDocShell)
    {
        sfx2::LinkManager* pLinkManager = pDocShell->GetDocument()->GetLinkManager();
        size_t nTotalCount = pLinkManager->GetLinks().size();
        for (size_t i=0; i<nTotalCount; i++)
        {
            ::sfx2::SvBaseLink* pBase = *pLinkManager->GetLinks()[i];
            if (pBase->ISA(ScAreaLink))
                ++nAreaCount;
        }
    }
    return nAreaCount;
}

uno::Any SAL_CALL ScAreaLinksObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference<sheet::XAreaLink> xLink(GetObjectByIndex_Impl(nIndex));
    if (xLink.is())
        return uno::makeAny(xLink);
    else
        throw lang::IndexOutOfBoundsException();
}

uno::Type SAL_CALL ScAreaLinksObj::getElementType() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return getCppuType((uno::Reference<sheet::XAreaLink>*)0);
}

sal_Bool SAL_CALL ScAreaLinksObj::hasElements() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}



ScDDELinkObj::ScDDELinkObj(ScDocShell* pDocSh, const OUString& rA,
                            const OUString& rT, const OUString& rI) :
    pDocShell( pDocSh ),
    aAppl( rA ),
    aTopic( rT ),
    aItem( rI )
{
    pDocShell->GetDocument()->AddUnoObject(*this);
}

ScDDELinkObj::~ScDDELinkObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScDDELinkObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    //! notify if links in document are changed
    //  UpdateRef is not needed here

    if ( rHint.ISA( SfxSimpleHint ) )
    {
        if ( ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
            pDocShell = NULL;       // pointer is invalid
    }
    else if ( rHint.ISA( ScLinkRefreshedHint ) )
    {
        const ScLinkRefreshedHint& rLH = (const ScLinkRefreshedHint&) rHint;
        if ( rLH.GetLinkType() == SC_LINKREFTYPE_DDE &&
             rLH.GetDdeAppl()  == aAppl &&
             rLH.GetDdeTopic() == aTopic &&
             rLH.GetDdeItem()  == aItem )       //! mode is ignored
            Refreshed_Impl();
    }
}

// XNamed

static OUString lcl_BuildDDEName( const OUString& rAppl, const OUString& rTopic, const OUString& rItem )
{
    //  Appl|Topic!Item (wie Excel)
    OUString aRet = rAppl + "|" + rTopic + "!" + rItem;
    return aRet;
}

OUString SAL_CALL ScDDELinkObj::getName() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return lcl_BuildDDEName( aAppl, aTopic, aItem );
}

void SAL_CALL ScDDELinkObj::setName( const OUString& /* aName */ ) throw(uno::RuntimeException)
{
    //  name can't be changed (formulas wouldn't find the link)
    throw uno::RuntimeException();
}

// XDDELink

OUString SAL_CALL ScDDELinkObj::getApplication() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    //! Test, ob Link noch im Dokument enthalten?

    return aAppl;
}

OUString SAL_CALL ScDDELinkObj::getTopic() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    //! Test, ob Link noch im Dokument enthalten?

    return aTopic;
}

OUString SAL_CALL ScDDELinkObj::getItem() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    //! Test, ob Link noch im Dokument enthalten?

    return aItem;
}

// XRefreshable

void SAL_CALL ScDDELinkObj::refresh() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        sc::DocumentLinkManager& rMgr = pDocShell->GetDocument()->GetDocLinkManager();
        rMgr.updateDdeLink(aAppl, aTopic, aItem);
    }
}

void SAL_CALL ScDDELinkObj::addRefreshListener(
    const uno::Reference<util::XRefreshListener >& xListener )
        throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference<util::XRefreshListener>* pObj =
            new uno::Reference<util::XRefreshListener>( xListener );
    aRefreshListeners.push_back( pObj );

    //  hold one additional ref to keep this object alive as long as there are listeners
    if ( aRefreshListeners.size() == 1 )
        acquire();
}

void SAL_CALL ScDDELinkObj::removeRefreshListener(
                                const uno::Reference<util::XRefreshListener >& xListener )
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    size_t nCount = aRefreshListeners.size();
    for ( size_t n=nCount; n--; )
    {
        uno::Reference<util::XRefreshListener>& rObj = aRefreshListeners[n];
        if ( rObj == xListener )
        {
            aRefreshListeners.erase( aRefreshListeners.begin() + n );
            if ( aRefreshListeners.empty() )
                release();                          // release ref for listeners
            break;
        }
    }
}

// XDDELinkResults

uno::Sequence< uno::Sequence< uno::Any > > ScDDELinkObj::getResults(  )
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Sequence< uno::Sequence< uno::Any > > aReturn;
    bool bSuccess = false;

    if ( pDocShell )
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        if ( pDoc )
        {
            size_t nPos = 0;
            if ( pDoc->FindDdeLink( aAppl, aTopic, aItem, SC_DDE_IGNOREMODE, nPos ) )
            {
                const ScMatrix* pMatrix = pDoc->GetDdeLinkResultMatrix( nPos );
                if ( pMatrix )
                {
                    uno::Any aAny;
                    if ( ScRangeToSequence::FillMixedArray( aAny, pMatrix, true ) )
                    {
                        aAny >>= aReturn;
                    }
                }
                bSuccess = true;
            }
        }
    }

    if ( !bSuccess )
    {
        throw uno::RuntimeException( OUString(
            "ScDDELinkObj::getResults: failed to get results!" ),
            uno::Reference< uno::XInterface >() );
    }

    return aReturn;
}

void ScDDELinkObj::setResults( const uno::Sequence< uno::Sequence< uno::Any > >& aResults )
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    bool bSuccess = false;

    if ( pDocShell )
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        if ( pDoc )
        {
            size_t nPos = 0;
            if ( pDoc->FindDdeLink( aAppl, aTopic, aItem, SC_DDE_IGNOREMODE, nPos ) )
            {
                uno::Any aAny;
                aAny <<= aResults;
                ScMatrixRef xMatrix = ScSequenceToMatrix::CreateMixedMatrix( aAny );
                bSuccess = pDoc->SetDdeLinkResultMatrix( nPos, xMatrix );
            }
        }
    }

    if ( !bSuccess )
    {
        throw uno::RuntimeException( OUString(
            "ScDDELinkObj::setResults: failed to set results!" ),
            uno::Reference< uno::XInterface >() );
    }
}

void ScDDELinkObj::Refreshed_Impl()
{
    lang::EventObject aEvent;
    aEvent.Source.set((cppu::OWeakObject*)this);
    for ( size_t n=0; n<aRefreshListeners.size(); n++ )
        aRefreshListeners[n]->refreshed( aEvent );
}



ScDDELinksObj::ScDDELinksObj(ScDocShell* pDocSh) :
    pDocShell( pDocSh )
{
    pDocShell->GetDocument()->AddUnoObject(*this);
}

ScDDELinksObj::~ScDDELinksObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScDDELinksObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    //  Referenz-Update interessiert hier nicht

    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       // ungueltig geworden
    }
}

// XDDELinks

ScDDELinkObj* ScDDELinksObj::GetObjectByIndex_Impl(sal_Int32 nIndex)
{
    if (pDocShell)
    {
        OUString aAppl, aTopic, aItem;
        if ( pDocShell->GetDocument()->GetDdeLinkData( (size_t)nIndex, aAppl, aTopic, aItem ) )
            return new ScDDELinkObj( pDocShell, aAppl, aTopic, aItem );
    }
    return NULL;
}

ScDDELinkObj* ScDDELinksObj::GetObjectByName_Impl(const OUString& aName)
{
    if (pDocShell)
    {
        OUString aNamStr(aName);
        OUString aAppl, aTopic, aItem;

        ScDocument* pDoc = pDocShell->GetDocument();
        size_t nCount = pDoc->GetDocLinkManager().getDdeLinkCount();
        for (size_t i=0; i<nCount; i++)
        {
            pDoc->GetDdeLinkData( i, aAppl, aTopic, aItem );
            if ( lcl_BuildDDEName(aAppl, aTopic, aItem) == aNamStr )
                return new ScDDELinkObj( pDocShell, aAppl, aTopic, aItem );
        }
    }
    return NULL;
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScDDELinksObj::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, OUString("com.sun.star.sheet.DDELinksEnumeration"));
}

// XIndexAccess

sal_Int32 SAL_CALL ScDDELinksObj::getCount() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    sal_Int32 nAreaCount = 0;
    if (pDocShell)
        nAreaCount = pDocShell->GetDocument()->GetDocLinkManager().getDdeLinkCount();
    return nAreaCount;
}

uno::Any SAL_CALL ScDDELinksObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference<sheet::XDDELink> xLink(GetObjectByIndex_Impl(nIndex));
    if (xLink.is())
        return uno::makeAny(xLink);
    else
        throw lang::IndexOutOfBoundsException();
}

uno::Type SAL_CALL ScDDELinksObj::getElementType() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return getCppuType((uno::Reference<sheet::XDDELink>*)0);
}

sal_Bool SAL_CALL ScDDELinksObj::hasElements() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

uno::Any SAL_CALL ScDDELinksObj::getByName( const OUString& aName )
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference<sheet::XDDELink> xLink(GetObjectByName_Impl(aName));
    if (xLink.is())
        return uno::makeAny(xLink);
    else
        throw container::NoSuchElementException();
}

uno::Sequence<OUString> SAL_CALL ScDDELinksObj::getElementNames() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        OUString aAppl, aTopic, aItem;

        ScDocument* pDoc = pDocShell->GetDocument();
        size_t nCount = pDocShell->GetDocument()->GetDocLinkManager().getDdeLinkCount();
        uno::Sequence<OUString> aSeq(nCount);
        OUString* pAry = aSeq.getArray();

        for (size_t i=0; i<nCount; i++)
        {
            pDoc->GetDdeLinkData( i, aAppl, aTopic, aItem );
            pAry[i] = lcl_BuildDDEName(aAppl, aTopic, aItem);
        }
        return aSeq;
    }
    return uno::Sequence<OUString>();
}

sal_Bool SAL_CALL ScDDELinksObj::hasByName( const OUString& aName )
                                        throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        OUString aNamStr(aName);
        OUString aAppl, aTopic, aItem;

        ScDocument* pDoc = pDocShell->GetDocument();
        size_t nCount = pDocShell->GetDocument()->GetDocLinkManager().getDdeLinkCount();
        for (size_t i=0; i<nCount; i++)
        {
            pDoc->GetDdeLinkData( i, aAppl, aTopic, aItem );
            if ( lcl_BuildDDEName(aAppl, aTopic, aItem) == aNamStr )
                return sal_True;
        }
    }
    return false;
}

// XDDELinks

uno::Reference< sheet::XDDELink > ScDDELinksObj::addDDELink(
    const OUString& aApplication, const OUString& aTopic,
    const OUString& aItem, ::com::sun::star::sheet::DDELinkMode nMode )
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference< sheet::XDDELink > xLink;

    if ( pDocShell )
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        if ( pDoc )
        {
            sal_uInt8 nMod = SC_DDE_DEFAULT;
            switch ( nMode )
            {
                case sheet::DDELinkMode_DEFAULT:
                    {
                        nMod = SC_DDE_DEFAULT;
                    }
                    break;
                case sheet::DDELinkMode_ENGLISH:
                    {
                        nMod = SC_DDE_ENGLISH;
                    }
                    break;
                case sheet::DDELinkMode_TEXT:
                    {
                        nMod = SC_DDE_TEXT;
                    }
                    break;
                default:
                    {
                    }
                    break;
            }

            if ( pDoc->CreateDdeLink( aApplication, aTopic, aItem, nMod, ScMatrixRef() ) )
            {
                const OUString aName( lcl_BuildDDEName( aApplication, aTopic, aItem ) );
                xLink.set( GetObjectByName_Impl( aName ) );
            }
        }
    }

    if ( !xLink.is() )
    {
        throw uno::RuntimeException( OUString(
            "ScDDELinksObj::addDDELink: cannot add DDE link!" ),
            uno::Reference< uno::XInterface >() );
    }

    return xLink;
}

// ============================================================================

ScExternalSheetCacheObj::ScExternalSheetCacheObj(ScExternalRefCache::TableTypeRef pTable, size_t nIndex) :
    mpTable(pTable),
    mnIndex(nIndex)
{
}

ScExternalSheetCacheObj::~ScExternalSheetCacheObj()
{
}

void SAL_CALL ScExternalSheetCacheObj::setCellValue(sal_Int32 nCol, sal_Int32 nRow, const Any& rValue)
    throw (IllegalArgumentException, RuntimeException)
{
    SolarMutexGuard aGuard;
    if (nRow < 0 || nCol < 0)
        throw IllegalArgumentException();

    ScExternalRefCache::TokenRef pToken;
    double fVal = 0.0;
    OUString aVal;
    if (rValue >>= fVal)
        pToken.reset(new FormulaDoubleToken(fVal));
    else if (rValue >>= aVal)
        pToken.reset(new FormulaStringToken(aVal));
    else
        // unidentified value type.
        return;

    mpTable->setCell(static_cast<SCCOL>(nCol), static_cast<SCROW>(nRow), pToken);
}

Any SAL_CALL ScExternalSheetCacheObj::getCellValue(sal_Int32 nCol, sal_Int32 nRow)
    throw (IllegalArgumentException, RuntimeException)
{
    SolarMutexGuard aGuard;
    if (nRow < 0 || nCol < 0)
        throw IllegalArgumentException();

    FormulaToken* pToken = mpTable->getCell(static_cast<SCCOL>(nCol), static_cast<SCROW>(nRow)).get();
    if (!pToken)
        throw IllegalArgumentException();

    Any aValue;
    switch (pToken->GetType())
    {
        case svDouble:
        {
            double fVal = pToken->GetDouble();
            aValue <<= fVal;
        }
        break;
        case svString:
        {
            OUString aVal = pToken->GetString().getString();
            aValue <<= aVal;
        }
        break;
        default:
            throw IllegalArgumentException();
    }
    return aValue;
}

Sequence< sal_Int32 > SAL_CALL ScExternalSheetCacheObj::getAllRows()
    throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    vector<SCROW> aRows;
    mpTable->getAllRows(aRows);
    size_t nSize = aRows.size();
    Sequence<sal_Int32> aRowsSeq(nSize);
    for (size_t i = 0; i < nSize; ++i)
        aRowsSeq[i] = aRows[i];

    return aRowsSeq;
}

Sequence< sal_Int32 > SAL_CALL ScExternalSheetCacheObj::getAllColumns(sal_Int32 nRow)
    throw (IllegalArgumentException, RuntimeException)
{
    SolarMutexGuard aGuard;
    if (nRow < 0)
        throw IllegalArgumentException();

    vector<SCCOL> aCols;
    mpTable->getAllCols(static_cast<SCROW>(nRow), aCols);
    size_t nSize = aCols.size();
    Sequence<sal_Int32> aColsSeq(nSize);
    for (size_t i = 0; i < nSize; ++i)
        aColsSeq[i] = aCols[i];

    return aColsSeq;
}

sal_Int32 SAL_CALL ScExternalSheetCacheObj::getTokenIndex()
        throw (RuntimeException)
{
    return static_cast< sal_Int32 >( mnIndex );
}

// ============================================================================

ScExternalDocLinkObj::ScExternalDocLinkObj(ScExternalRefManager* pRefMgr, sal_uInt16 nFileId) :
    mpRefMgr(pRefMgr), mnFileId(nFileId)
{
}

ScExternalDocLinkObj::~ScExternalDocLinkObj()
{
}

Reference< sheet::XExternalSheetCache > SAL_CALL ScExternalDocLinkObj::addSheetCache(
    const OUString& aSheetName, sal_Bool bDynamicCache )
        throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    size_t nIndex = 0;
    ScExternalRefCache::TableTypeRef pTable = mpRefMgr->getCacheTable(mnFileId, aSheetName, true, &nIndex);
    if (!bDynamicCache)
        // Set the whole table cached to prevent access to the source document.
        pTable->setWholeTableCached();

    Reference< sheet::XExternalSheetCache > aSheetCache(new ScExternalSheetCacheObj(pTable, nIndex));
    return aSheetCache;
}

Any SAL_CALL ScExternalDocLinkObj::getByName(const OUString &aName)
        throw (container::NoSuchElementException, lang::WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    size_t nIndex = 0;
    ScExternalRefCache::TableTypeRef pTable = mpRefMgr->getCacheTable(mnFileId, aName, false, &nIndex);
    if (!pTable)
        throw container::NoSuchElementException();

    Reference< sheet::XExternalSheetCache > aSheetCache(new ScExternalSheetCacheObj(pTable, nIndex));

    Any aAny;
    aAny <<= aSheetCache;
    return aAny;
}

Sequence< OUString > SAL_CALL ScExternalDocLinkObj::getElementNames()
        throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    vector<OUString> aTabNames;
    mpRefMgr->getAllCachedTableNames(mnFileId, aTabNames);

    // #i116940# be consistent with getByName: include only table names which have a cache already
    vector<OUString> aValidNames;
    for (vector<OUString>::iterator aIter = aTabNames.begin(); aIter != aTabNames.end(); ++aIter)
        if (mpRefMgr->getCacheTable(mnFileId, *aIter, false))
            aValidNames.push_back(*aIter);

    size_t n = aValidNames.size();
    Sequence<OUString> aSeq(n);
    for (size_t i = 0; i < n; ++i)
        aSeq[i] = aValidNames[i];
    return aSeq;
}

sal_Bool SAL_CALL ScExternalDocLinkObj::hasByName(const OUString &aName)
        throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    // #i116940# be consistent with getByName: allow only table names which have a cache already
    ScExternalRefCache::TableTypeRef pTable = mpRefMgr->getCacheTable(mnFileId, aName, false);
    return (pTable.get() != NULL);
}

sal_Int32 SAL_CALL ScExternalDocLinkObj::getCount()
        throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    // #i116940# be consistent with getByName: count only table names which have a cache already
    return getElementNames().getLength();
}

Any SAL_CALL ScExternalDocLinkObj::getByIndex(sal_Int32 nApiIndex)
        throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;

    // #i116940# Can't use nApiIndex as index for the ref manager, because the API counts only
    // the entries which have a cache already. Quick solution: Use getElementNames.
    Sequence< OUString > aNames( getElementNames() );
    if (nApiIndex < 0 || nApiIndex >= aNames.getLength())
        throw lang::IndexOutOfBoundsException();

    size_t nIndex = 0;
    ScExternalRefCache::TableTypeRef pTable = mpRefMgr->getCacheTable(mnFileId, aNames[nApiIndex], false, &nIndex);
    if (!pTable)
        throw lang::IndexOutOfBoundsException();

    Reference< sheet::XExternalSheetCache > aSheetCache(new ScExternalSheetCacheObj(pTable, nIndex));

    Any aAny;
    aAny <<= aSheetCache;
    return aAny;
}

Reference< container::XEnumeration > SAL_CALL ScExternalDocLinkObj::createEnumeration()
        throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    Reference< container::XEnumeration > aRef(
        new ScIndexEnumeration(this, OUString(
            "com.sun.star.sheet.ExternalDocLink")));
    return aRef;
}

uno::Type SAL_CALL ScExternalDocLinkObj::getElementType()
        throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    return getCppuType(static_cast<Reference<sheet::XExternalDocLink>*>(0));
}

sal_Bool SAL_CALL ScExternalDocLinkObj::hasElements()
        throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    // #i116940# be consistent with getByName: count only table names which have a cache already
    return ( getElementNames().getLength() > 0 );
}

sal_Int32 SAL_CALL ScExternalDocLinkObj::getTokenIndex()
        throw (RuntimeException)
{
    return static_cast<sal_Int32>(mnFileId);
}

// ============================================================================

ScExternalDocLinksObj::ScExternalDocLinksObj(ScDocShell* pDocShell) :
    mpDocShell(pDocShell),
    mpRefMgr(pDocShell->GetDocument()->GetExternalRefManager())
{
}

ScExternalDocLinksObj::~ScExternalDocLinksObj()
{
}

Reference< sheet::XExternalDocLink > SAL_CALL ScExternalDocLinksObj::addDocLink(
    const OUString& aDocName )
        throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    sal_uInt16 nFileId = mpRefMgr->getExternalFileId(aDocName);
    Reference< sheet::XExternalDocLink > aDocLink(new ScExternalDocLinkObj(mpRefMgr, nFileId));
    return aDocLink;
}

Any SAL_CALL ScExternalDocLinksObj::getByName(const OUString &aName)
        throw (container::NoSuchElementException, lang::WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    if (!mpRefMgr->hasExternalFile(aName))
        throw container::NoSuchElementException();

    sal_uInt16 nFileId = mpRefMgr->getExternalFileId(aName);
    Reference< sheet::XExternalDocLink > aDocLink(new ScExternalDocLinkObj(mpRefMgr, nFileId));

    Any aAny;
    aAny <<= aDocLink;
    return aAny;
}

Sequence< OUString > SAL_CALL ScExternalDocLinksObj::getElementNames()
        throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    sal_uInt16 n = mpRefMgr->getExternalFileCount();
    Sequence<OUString> aSeq(n);
    for (sal_uInt16 i = 0; i < n; ++i)
    {
        const OUString* pName = mpRefMgr->getExternalFileName(i);
        aSeq[i] = pName ? *pName : OUString();
    }

    return aSeq;
}

sal_Bool SAL_CALL ScExternalDocLinksObj::hasByName(const OUString &aName)
        throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    return mpRefMgr->hasExternalFile(aName);
}

sal_Int32 SAL_CALL ScExternalDocLinksObj::getCount()
        throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    return mpRefMgr->getExternalFileCount();
}

Any SAL_CALL ScExternalDocLinksObj::getByIndex(sal_Int32 nIndex)
        throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    if (nIndex > ::std::numeric_limits<sal_uInt16>::max() || nIndex < ::std::numeric_limits<sal_uInt16>::min())
        throw lang::IndexOutOfBoundsException();

    sal_uInt16 nFileId = static_cast<sal_uInt16>(nIndex);

    if (!mpRefMgr->hasExternalFile(nFileId))
        throw lang::IndexOutOfBoundsException();

    Reference< sheet::XExternalDocLink > aDocLink(new ScExternalDocLinkObj(mpRefMgr, nFileId));
    Any aAny;
    aAny <<= aDocLink;
    return aAny;
}

Reference< container::XEnumeration > SAL_CALL ScExternalDocLinksObj::createEnumeration()
        throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    Reference< container::XEnumeration > aRef(
        new ScIndexEnumeration(this, OUString(
            "com.sun.star.sheet.ExternalDocLinks")));
    return aRef;
}

uno::Type SAL_CALL ScExternalDocLinksObj::getElementType()
        throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    return getCppuType(static_cast<Reference<sheet::XExternalDocLinks>*>(0));
}

sal_Bool SAL_CALL ScExternalDocLinksObj::hasElements()
        throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    return mpRefMgr->getExternalFileCount() > 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
