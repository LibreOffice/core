/*************************************************************************
 *
 *  $RCSfile: linkuno.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: dr $ $Date: 2001-04-05 10:50:56 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include <svtools/smplhint.hxx>
#include <svx/linkmgr.hxx>

#include "linkuno.hxx"
#include "miscuno.hxx"
#include "convuno.hxx"
#include "docsh.hxx"
#include "docfunc.hxx"
#include "collect.hxx"
#include "tablink.hxx"
#include "arealink.hxx"
#include "unoguard.hxx"
#include "hints.hxx"
#include "unonames.hxx"

using namespace com::sun::star;

//------------------------------------------------------------------------

//  fuer Sheet- und Area-Links benutzt:
const SfxItemPropertyMap* lcl_GetSheetLinkMap()
{
    static SfxItemPropertyMap aSheetLinkMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_FILTER),   0,  &getCppuType((rtl::OUString*)0),    0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_FILTOPT),  0,  &getCppuType((rtl::OUString*)0),    0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_LINKURL),  0,  &getCppuType((rtl::OUString*)0),    0, 0 },
        {0,0,0,0}
    };
    return aSheetLinkMap_Impl;
}

//------------------------------------------------------------------------

SV_IMPL_PTRARR( XRefreshListenerArr_Impl, XRefreshListenerPtr );

SC_SIMPLE_SERVICE_INFO( ScAreaLinkObj, "ScAreaLinkObj", "com.sun.star.sheet.CellAreaLink" )
SC_SIMPLE_SERVICE_INFO( ScAreaLinksObj, "ScAreaLinksObj", "com.sun.star.sheet.CellAreaLinks" )
SC_SIMPLE_SERVICE_INFO( ScDDELinkObj, "ScDDELinkObj", "com.sun.star.sheet.DDELink" )
SC_SIMPLE_SERVICE_INFO( ScDDELinksObj, "ScDDELinksObj", "com.sun.star.sheet.DDELinks" )
SC_SIMPLE_SERVICE_INFO( ScSheetLinkObj, "ScSheetLinkObj", "com.sun.star.sheet.SheetLink" )
SC_SIMPLE_SERVICE_INFO( ScSheetLinksObj, "ScSheetLinksObj", "com.sun.star.sheet.SheetLinks" )

//------------------------------------------------------------------------

ScSheetLinkObj::ScSheetLinkObj(ScDocShell* pDocSh, const String& rName) :
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

void ScSheetLinkObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
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
        SvxLinkManager* pLinkManager = pDocShell->GetDocument()->GetLinkManager();
        USHORT nCount = pLinkManager->GetLinks().Count();
        for (USHORT i=0; i<nCount; i++)
        {
            ::so3::SvBaseLink* pBase = *pLinkManager->GetLinks()[i];
            if (pBase->ISA(ScTableLink))
            {
                ScTableLink* pTabLink = (ScTableLink*)pBase;
                if ( pTabLink->GetFileName() == aFileName )
                    return pTabLink;
            }
        }
    }
    return NULL;    // nicht gefunden
}

// XNamed

rtl::OUString SAL_CALL ScSheetLinkObj::getName() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return getFileName();   // Name ist der Dateiname (URL)
}

void SAL_CALL ScSheetLinkObj::setName( const rtl::OUString& aName ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    setFileName(aName);     // Name ist der Dateiname (URL)
}

// XRefreshable

void SAL_CALL ScSheetLinkObj::refresh() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScTableLink* pLink = GetLink_Impl();
    if (pLink)
        pLink->Refresh( pLink->GetFileName(), pLink->GetFilterName() );
}

void SAL_CALL ScSheetLinkObj::addRefreshListener(
                                const uno::Reference<util::XRefreshListener >& xListener )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference<util::XRefreshListener>* pObj =
            new uno::Reference<util::XRefreshListener>( xListener );
    aRefreshListeners.Insert( pObj, aRefreshListeners.Count() );

    //  hold one additional ref to keep this object alive as long as there are listeners
    if ( aRefreshListeners.Count() == 1 )
        acquire();
}

void SAL_CALL ScSheetLinkObj::removeRefreshListener(
                                const uno::Reference<util::XRefreshListener >& xListener )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    USHORT nCount = aRefreshListeners.Count();
    for ( USHORT n=nCount; n--; )
    {
        uno::Reference<util::XRefreshListener>* pObj = aRefreshListeners[n];
        if ( *pObj == xListener )
        {
            aRefreshListeners.DeleteAndDestroy( n );
            if ( aRefreshListeners.Count() == 0 )
                release();                          // release ref for listeners
            break;
        }
    }
}

void ScSheetLinkObj::Refreshed_Impl()
{
    lang::EventObject aEvent;
    aEvent.Source = (cppu::OWeakObject*)this;
    for ( USHORT n=0; n<aRefreshListeners.Count(); n++ )
        (*aRefreshListeners[n])->refreshed( aEvent );
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScSheetLinkObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( aPropSet.getPropertyMap() );
    return aRef;
}

void SAL_CALL ScSheetLinkObj::setPropertyValue(
                        const rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aNameString = aPropertyName;
    rtl::OUString aValStr;
    if ( aNameString.EqualsAscii( SC_UNONAME_LINKURL ) )
    {
        if ( aValue >>= aValStr )
            setFileName( aValStr );
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_FILTER ) )
    {
        if ( aValue >>= aValStr )
            setFilter( aValStr );
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_FILTOPT ) )
    {
        if ( aValue >>= aValStr )
            setFilterOptions( aValStr );
    }
}

uno::Any SAL_CALL ScSheetLinkObj::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aNameString = aPropertyName;
    uno::Any aRet;
    if ( aNameString.EqualsAscii( SC_UNONAME_LINKURL ) )
        aRet <<= getFileName();
    else if ( aNameString.EqualsAscii( SC_UNONAME_FILTER ) )
        aRet <<= getFilter();
    else if ( aNameString.EqualsAscii( SC_UNONAME_FILTOPT ) )
        aRet <<= getFilterOptions();
    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScSheetLinkObj )

// internal:

rtl::OUString ScSheetLinkObj::getFileName(void) const
{
    ScUnoGuard aGuard;
    return aFileName;
}

void ScSheetLinkObj::setFileName(const rtl::OUString& rNewName)
{
    ScUnoGuard aGuard;
    ScTableLink* pLink = GetLink_Impl();
    if (pLink)
    {
        //  pLink->Refresh mit neuem Dateinamen bringt SvxLinkManager durcheinander
        //  darum per Hand die Tabellen umsetzen und Link per UpdateLinks neu erzeugen

        String aNewStr = rNewName;
        aNewStr = ScGlobal::GetAbsDocName( aNewStr, pDocShell );

        //  zuerst Tabellen umsetzen

        ScDocument* pDoc = pDocShell->GetDocument();
        USHORT nTabCount = pDoc->GetTableCount();
        for (USHORT nTab=0; nTab<nTabCount; nTab++)
            if ( pDoc->IsLinked(nTab) && pDoc->GetLinkDoc(nTab) == aFileName )  // alte Datei
                pDoc->SetLink( nTab, pDoc->GetLinkMode(nTab), aNewStr,
                                pDoc->GetLinkFlt(nTab), pDoc->GetLinkOpt(nTab),
                                pDoc->GetLinkTab(nTab) );   // nur Datei aendern

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

rtl::OUString ScSheetLinkObj::getFilter(void) const
{
    ScUnoGuard aGuard;
    rtl::OUString aRet;
    ScTableLink* pLink = GetLink_Impl();
    if (pLink)
        aRet = pLink->GetFilterName();
    return aRet;
}

void ScSheetLinkObj::setFilter(const rtl::OUString& Filter)
{
    ScUnoGuard aGuard;
    ScTableLink* pLink = GetLink_Impl();
    if (pLink)
    {
        String aFilterStr = Filter;
        pLink->Refresh( aFileName, aFilterStr );
    }
}

rtl::OUString ScSheetLinkObj::getFilterOptions(void) const
{
    ScUnoGuard aGuard;
    rtl::OUString aRet;
    ScTableLink* pLink = GetLink_Impl();
    if (pLink)
        aRet = pLink->GetOptions();
    return aRet;
}

void ScSheetLinkObj::setFilterOptions(const rtl::OUString& FilterOptions)
{
    ScUnoGuard aGuard;
    ScTableLink* pLink = GetLink_Impl();
    if (pLink)
    {
        String aOptStr = FilterOptions;
        pLink->Refresh( aFileName, pLink->GetFilterName(), &aOptStr );
    }
}

//------------------------------------------------------------------------

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

void ScSheetLinksObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    //  Referenz-Update interessiert hier nicht

    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       // ungueltig geworden
    }
}

// XSheetLinks

ScSheetLinkObj* ScSheetLinksObj::GetObjectByIndex_Impl(INT32 nIndex)
{
    if (pDocShell)
    {
        INT32 nCount = 0;
        StrCollection aNames;   // um doppelte wegzulassen
        ScDocument* pDoc = pDocShell->GetDocument();
        USHORT nTabCount = pDoc->GetTableCount();
        for (USHORT nTab=0; nTab<nTabCount; nTab++)
            if (pDoc->IsLinked(nTab))
            {
                String aLinkDoc = pDoc->GetLinkDoc( nTab );
                StrData* pData = new StrData(aLinkDoc);
                if (aNames.Insert(pData))
                {
                    if ( nCount == nIndex )
                        return new ScSheetLinkObj( pDocShell, aLinkDoc );
                    ++nCount;
                }
                else
                    delete pData;
            }
    }
    return NULL;    // kein Dokument oder Index zu gross
}

ScSheetLinkObj* ScSheetLinksObj::GetObjectByName_Impl(const rtl::OUString& aName)
{
    //  Name ist der Dateiname

    if (pDocShell)
    {
        String aNameStr = aName;

        ScDocument* pDoc = pDocShell->GetDocument();
        USHORT nTabCount = pDoc->GetTableCount();
        for (USHORT nTab=0; nTab<nTabCount; nTab++)
            if (pDoc->IsLinked(nTab))
            {
                //! case-insensitiv ???
                String aLinkDoc = pDoc->GetLinkDoc( nTab );
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
    ScUnoGuard aGuard;
    return new ScIndexEnumeration(this);
}

// XIndexAccess

sal_Int32 SAL_CALL ScSheetLinksObj::getCount() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    INT32 nCount = 0;
    if (pDocShell)
    {
        StrCollection aNames;   // um doppelte wegzulassen
        ScDocument* pDoc = pDocShell->GetDocument();
        USHORT nTabCount = pDoc->GetTableCount();
        for (USHORT nTab=0; nTab<nTabCount; nTab++)
            if (pDoc->IsLinked(nTab))
            {
                String aLinkDoc = pDoc->GetLinkDoc( nTab );
                StrData* pData = new StrData(aLinkDoc);
                if (aNames.Insert(pData))
                    ++nCount;
                else
                    delete pData;
            }
    }
    return nCount;
}

uno::Any SAL_CALL ScSheetLinksObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference<beans::XPropertySet> xLink = GetObjectByIndex_Impl(nIndex);
    uno::Any aAny;
    if (xLink.is())
        aAny <<= xLink;
    else
        throw lang::IndexOutOfBoundsException();
    return aAny;
}

uno::Type SAL_CALL ScSheetLinksObj::getElementType() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return getCppuType((uno::Reference<beans::XPropertySet>*)0);
}

sal_Bool SAL_CALL ScSheetLinksObj::hasElements() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return ( getCount() != 0 );
}

uno::Any SAL_CALL ScSheetLinksObj::getByName( const rtl::OUString& aName )
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference<beans::XPropertySet> xLink = GetObjectByName_Impl(aName);
    uno::Any aAny;
    if (xLink.is())
        aAny <<= xLink;
    else
        throw container::NoSuchElementException();
    return aAny;
}

sal_Bool SAL_CALL ScSheetLinksObj::hasByName( const rtl::OUString& aName )
                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    //  Name ist der Dateiname

    if (pDocShell)
    {
        String aNameStr = aName;

        ScDocument* pDoc = pDocShell->GetDocument();
        USHORT nTabCount = pDoc->GetTableCount();
        for (USHORT nTab=0; nTab<nTabCount; nTab++)
            if (pDoc->IsLinked(nTab))
            {
                //! case-insensitiv ???
                String aLinkDoc = pDoc->GetLinkDoc( nTab );
                if ( aLinkDoc == aNameStr )
                    return TRUE;
            }
    }
    return FALSE;
}

uno::Sequence<rtl::OUString> SAL_CALL ScSheetLinksObj::getElementNames() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    //  Name ist der Dateiname

    if (pDocShell)
    {
        StrCollection aNames;   // um doppelte wegzulassen
        ScDocument* pDoc = pDocShell->GetDocument();
        USHORT nTabCount = pDoc->GetTableCount();
        String aName;

        INT32 nLinkCount = getCount();
        uno::Sequence<rtl::OUString> aSeq(nLinkCount);
        rtl::OUString* pAry = aSeq.getArray();
        USHORT nPos = 0;
        for (USHORT nTab=0; nTab<nTabCount; nTab++)
        {
            if (pDoc->IsLinked(nTab))
            {
                String aLinkDoc = pDoc->GetLinkDoc( nTab );
                StrData* pData = new StrData(aLinkDoc);
                if (aNames.Insert(pData))
                    pAry[nPos++] = aLinkDoc;
                else
                    delete pData;
            }
        }
        DBG_ASSERT( nPos==nLinkCount, "verzaehlt" );
        return aSeq;
    }
    return uno::Sequence<rtl::OUString>();
}

//------------------------------------------------------------------------

ScAreaLink* lcl_GetAreaLink( ScDocShell* pDocShell, USHORT nPos )
{
    if (pDocShell)
    {
        SvxLinkManager* pLinkManager = pDocShell->GetDocument()->GetLinkManager();
        USHORT nTotalCount = pLinkManager->GetLinks().Count();
        USHORT nAreaCount = 0;
        for (USHORT i=0; i<nTotalCount; i++)
        {
            ::so3::SvBaseLink* pBase = *pLinkManager->GetLinks()[i];
            if (pBase->ISA(ScAreaLink))
            {
                if ( nAreaCount == nPos )
                    return (ScAreaLink*)pBase;
                ++nAreaCount;
            }
        }
    }
    return NULL;    // nicht gefunden
}

ScAreaLinkObj::ScAreaLinkObj(ScDocShell* pDocSh, USHORT nP) :
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

void ScAreaLinkObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
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

void ScAreaLinkObj::Modify_Impl( const rtl::OUString* pNewFile, const rtl::OUString* pNewFilter,
                                 const rtl::OUString* pNewOptions, const rtl::OUString* pNewSource,
                                 const table::CellRangeAddress* pNewDest )
{
    ScAreaLink* pLink = lcl_GetAreaLink(pDocShell, nPos);
    if (pLink)
    {
        String aFile    = pLink->GetFile();
        String aFilter  = pLink->GetFilter();
        String aOptions = pLink->GetOptions();
        String aSource  = pLink->GetSource();
        ScRange aDest   = pLink->GetDestArea();
        ULONG nRefresh  = pLink->GetRefreshDelay();

        //! Undo fuer Loeschen
        //! Undo zusammenfassen

        SvxLinkManager* pLinkManager = pDocShell->GetDocument()->GetLinkManager();
        pLinkManager->Remove( pLink );
        pLink = NULL;   // bei Remove geloescht

        BOOL bFitBlock = TRUE;          // verschieben, wenn durch Update Groesse geaendert
        if (pNewFile)
        {
            aFile = String( *pNewFile );
            aFile = ScGlobal::GetAbsDocName( aFile, pDocShell );    //! in InsertAreaLink?
        }
        if (pNewFilter)
            aFilter = String( *pNewFilter );
        if (pNewOptions)
            aOptions = String( *pNewOptions );
        if (pNewSource)
            aSource = String( *pNewSource );
        if (pNewDest)
        {
            ScUnoConversion::FillScRange( aDest, *pNewDest );
            bFitBlock = FALSE;  // neuer Bereich angegeben -> keine Inhalte verschieben
        }

        ScDocFunc aFunc(*pDocShell);
        aFunc.InsertAreaLink( aFile, aFilter, aOptions, aSource, aDest, nRefresh, bFitBlock, TRUE );
    }
}

void ScAreaLinkObj::ModifyRefreshDelay_Impl( sal_Int32 nRefresh )
{
    ScAreaLink* pLink = lcl_GetAreaLink( pDocShell, nPos );
    if( pLink )
        pLink->SetRefreshDelay( (ULONG) nRefresh );
}

// XRefreshable

void SAL_CALL ScAreaLinkObj::refresh() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScAreaLink* pLink = lcl_GetAreaLink(pDocShell, nPos);
    if (pLink)
        pLink->Refresh( pLink->GetFile(), pLink->GetFilter(), pLink->GetSource(), pLink->GetRefreshDelay() );
}

void SAL_CALL ScAreaLinkObj::addRefreshListener(
                                const uno::Reference<util::XRefreshListener >& xListener )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference<util::XRefreshListener>* pObj =
            new uno::Reference<util::XRefreshListener>( xListener );
    aRefreshListeners.Insert( pObj, aRefreshListeners.Count() );

    //  hold one additional ref to keep this object alive as long as there are listeners
    if ( aRefreshListeners.Count() == 1 )
        acquire();
}

void SAL_CALL ScAreaLinkObj::removeRefreshListener(
                                const uno::Reference<util::XRefreshListener >& xListener )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    USHORT nCount = aRefreshListeners.Count();
    for ( USHORT n=nCount; n--; )
    {
        uno::Reference<util::XRefreshListener>* pObj = aRefreshListeners[n];
        if ( *pObj == xListener )
        {
            aRefreshListeners.DeleteAndDestroy( n );
            if ( aRefreshListeners.Count() == 0 )
                release();                          // release ref for listeners
            break;
        }
    }
}

void ScAreaLinkObj::Refreshed_Impl()
{
    lang::EventObject aEvent;
    aEvent.Source = (cppu::OWeakObject*)this;
    for ( USHORT n=0; n<aRefreshListeners.Count(); n++ )
        (*aRefreshListeners[n])->refreshed( aEvent );
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScAreaLinkObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( aPropSet.getPropertyMap() );
    return aRef;
}

void SAL_CALL ScAreaLinkObj::setPropertyValue(
                        const rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aNameString = aPropertyName;
    rtl::OUString aValStr;
    if ( aNameString.EqualsAscii( SC_UNONAME_LINKURL ) )
    {
        if ( aValue >>= aValStr )
            setFileName( aValStr );
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_FILTER ) )
    {
        if ( aValue >>= aValStr )
            setFilter( aValStr );
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_FILTOPT ) )
    {
        if ( aValue >>= aValStr )
            setFilterOptions( aValStr );
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_REFDELAY ) )
    {
        sal_Int32 nRefresh;
        if ( aValue >>= nRefresh )
            setRefreshDelay( nRefresh );
    }
}

uno::Any SAL_CALL ScAreaLinkObj::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aNameString = aPropertyName;
    uno::Any aRet;
    if ( aNameString.EqualsAscii( SC_UNONAME_LINKURL ) )
        aRet <<= getFileName();
    else if ( aNameString.EqualsAscii( SC_UNONAME_FILTER ) )
        aRet <<= getFilter();
    else if ( aNameString.EqualsAscii( SC_UNONAME_FILTOPT ) )
        aRet <<= getFilterOptions();
    else if ( aNameString.EqualsAscii( SC_UNONAME_REFDELAY ) )
        aRet <<= getRefreshDelay();
    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScAreaLinkObj )

//  internal:

rtl::OUString ScAreaLinkObj::getFileName(void) const
{
    ScUnoGuard aGuard;
    rtl::OUString aRet;
    ScAreaLink* pLink = lcl_GetAreaLink(pDocShell, nPos);
    if (pLink)
        aRet = pLink->GetFile();
    return aRet;
}

void ScAreaLinkObj::setFileName(const rtl::OUString& rNewName)
{
    ScUnoGuard aGuard;
    Modify_Impl( &rNewName, NULL, NULL, NULL, NULL );
}

rtl::OUString ScAreaLinkObj::getFilter(void) const
{
    ScUnoGuard aGuard;
    rtl::OUString aRet;
    ScAreaLink* pLink = lcl_GetAreaLink(pDocShell, nPos);
    if (pLink)
        aRet = pLink->GetFilter();
    return aRet;
}

void ScAreaLinkObj::setFilter(const rtl::OUString& Filter)
{
    ScUnoGuard aGuard;
    Modify_Impl( NULL, &Filter, NULL, NULL, NULL );
}

rtl::OUString ScAreaLinkObj::getFilterOptions(void) const
{
    ScUnoGuard aGuard;
    rtl::OUString aRet;
    ScAreaLink* pLink = lcl_GetAreaLink(pDocShell, nPos);
    if (pLink)
        aRet = pLink->GetOptions();
    return aRet;
}

void ScAreaLinkObj::setFilterOptions(const rtl::OUString& FilterOptions)
{
    ScUnoGuard aGuard;
    Modify_Impl( NULL, NULL, &FilterOptions, NULL, NULL );
}

sal_Int32 ScAreaLinkObj::getRefreshDelay(void) const
{
    ScUnoGuard aGuard;
    sal_Int32 nRet;
    ScAreaLink* pLink = lcl_GetAreaLink(pDocShell, nPos);
    if (pLink)
        nRet = (sal_Int32) pLink->GetRefreshDelay();
    return nRet;
}

void ScAreaLinkObj::setRefreshDelay(sal_Int32 nRefreshDelay)
{
    ScUnoGuard aGuard;
    ModifyRefreshDelay_Impl( nRefreshDelay );
}

// XAreaLink

rtl::OUString SAL_CALL ScAreaLinkObj::getSourceArea() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    rtl::OUString aRet;
    ScAreaLink* pLink = lcl_GetAreaLink(pDocShell, nPos);
    if (pLink)
        aRet = pLink->GetSource();
    return aRet;
}

void SAL_CALL ScAreaLinkObj::setSourceArea( const rtl::OUString& aSourceArea )
                                            throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    Modify_Impl( NULL, NULL, NULL, &aSourceArea, NULL );
}

table::CellRangeAddress SAL_CALL ScAreaLinkObj::getDestArea() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    table::CellRangeAddress aRet;
    ScAreaLink* pLink = lcl_GetAreaLink(pDocShell, nPos);
    if (pLink)
        ScUnoConversion::FillApiRange( aRet, pLink->GetDestArea() );
    return aRet;
}

void SAL_CALL ScAreaLinkObj::setDestArea( const table::CellRangeAddress& aDestArea )
                                            throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    Modify_Impl( NULL, NULL, NULL, NULL, &aDestArea );
}

//------------------------------------------------------------------------

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

void ScAreaLinksObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    //  Referenz-Update interessiert hier nicht

    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       // ungueltig geworden
    }
}

// XAreaLinks

ScAreaLinkObj* ScAreaLinksObj::GetObjectByIndex_Impl(INT32 nIndex)
{
    if ( pDocShell && nIndex >= 0 && nIndex < getCount() )
        return new ScAreaLinkObj( pDocShell, (USHORT)nIndex );

    return NULL;    // nicht gefunden
}

void SAL_CALL ScAreaLinksObj::insertAtPosition( const table::CellAddress& aDestPos,
                                                const rtl::OUString& aFileName,
                                                const rtl::OUString& aSourceArea,
                                                const rtl::OUString& aFilter,
                                                const rtl::OUString& aFilterOptions )
                                            throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (pDocShell)
    {
        String aFileStr   = aFileName;
        String aFilterStr = aFilter;
        String aOptionStr = aFilterOptions;
        String aSourceStr = aSourceArea;
        ScAddress aDestAddr( (USHORT)aDestPos.Column, (USHORT)aDestPos.Row, aDestPos.Sheet );

        aFileStr = ScGlobal::GetAbsDocName( aFileStr, pDocShell );  //! in InsertAreaLink ???

        ScDocFunc aFunc(*pDocShell);
        aFunc.InsertAreaLink( aFileStr, aFilterStr, aOptionStr,
                                aSourceStr, ScRange(aDestAddr),
                                0, FALSE, TRUE );                   // keine Inhalte verschieben
    }
}

void SAL_CALL ScAreaLinksObj::removeByIndex( sal_Int32 nIndex ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScAreaLink* pLink = lcl_GetAreaLink(pDocShell, (USHORT)nIndex);
    if (pLink)
    {
        //! SetAddUndo oder so

        SvxLinkManager* pLinkManager = pDocShell->GetDocument()->GetLinkManager();
        pLinkManager->Remove( pLink );
    }
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScAreaLinksObj::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScIndexEnumeration(this);
}

// XIndexAccess

sal_Int32 SAL_CALL ScAreaLinksObj::getCount() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    INT32 nAreaCount = 0;
    if (pDocShell)
    {
        SvxLinkManager* pLinkManager = pDocShell->GetDocument()->GetLinkManager();
        USHORT nTotalCount = pLinkManager->GetLinks().Count();
        for (USHORT i=0; i<nTotalCount; i++)
        {
            ::so3::SvBaseLink* pBase = *pLinkManager->GetLinks()[i];
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
    ScUnoGuard aGuard;
    uno::Reference<sheet::XAreaLink> xLink = GetObjectByIndex_Impl(nIndex);
    uno::Any aAny;
    if (xLink.is())
        aAny <<= xLink;
    else
        throw lang::IndexOutOfBoundsException();
    return aAny;
}

uno::Type SAL_CALL ScAreaLinksObj::getElementType() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return getCppuType((uno::Reference<sheet::XAreaLink>*)0);
}

sal_Bool SAL_CALL ScAreaLinksObj::hasElements() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return ( getCount() != 0 );
}

//------------------------------------------------------------------------

ScDDELinkObj::ScDDELinkObj(ScDocShell* pDocSh, const String& rA,
                            const String& rT, const String& rI) :
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

void ScDDELinkObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
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

String lcl_BuildDDEName( const String& rAppl, const String& rTopic, const String& rItem )
{
    //  Appl|Topic!Item (wie Excel)
    String aRet = rAppl;
    aRet += '|';
    aRet += rTopic;
    aRet += '!';
    aRet += rItem;
    return aRet;
}

rtl::OUString SAL_CALL ScDDELinkObj::getName() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return lcl_BuildDDEName( aAppl, aTopic, aItem );
}

void SAL_CALL ScDDELinkObj::setName( const rtl::OUString& aName ) throw(uno::RuntimeException)
{
    //  name can't be changed (formulas wouldn't find the link)
    throw uno::RuntimeException();
}

// XDDELink

rtl::OUString SAL_CALL ScDDELinkObj::getApplication() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    //! Test, ob Link noch im Dokument enthalten?

    return aAppl;
}

rtl::OUString SAL_CALL ScDDELinkObj::getTopic() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    //! Test, ob Link noch im Dokument enthalten?

    return aTopic;
}

rtl::OUString SAL_CALL ScDDELinkObj::getItem() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    //! Test, ob Link noch im Dokument enthalten?

    return aItem;
}

// XRefreshable

void SAL_CALL ScDDELinkObj::refresh() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        BOOL bOk = pDoc->UpdateDdeLink( aAppl, aTopic, aItem );
        //! Fehler abfragen
    }
}

void SAL_CALL ScDDELinkObj::addRefreshListener(
                                const uno::Reference<util::XRefreshListener >& xListener )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference<util::XRefreshListener>* pObj =
            new uno::Reference<util::XRefreshListener>( xListener );
    aRefreshListeners.Insert( pObj, aRefreshListeners.Count() );

    //  hold one additional ref to keep this object alive as long as there are listeners
    if ( aRefreshListeners.Count() == 1 )
        acquire();
}

void SAL_CALL ScDDELinkObj::removeRefreshListener(
                                const uno::Reference<util::XRefreshListener >& xListener )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    USHORT nCount = aRefreshListeners.Count();
    for ( USHORT n=nCount; n--; )
    {
        uno::Reference<util::XRefreshListener>* pObj = aRefreshListeners[n];
        if ( *pObj == xListener )
        {
            aRefreshListeners.DeleteAndDestroy( n );
            if ( aRefreshListeners.Count() == 0 )
                release();                          // release ref for listeners
            break;
        }
    }
}

void ScDDELinkObj::Refreshed_Impl()
{
    lang::EventObject aEvent;
    aEvent.Source = (cppu::OWeakObject*)this;
    for ( USHORT n=0; n<aRefreshListeners.Count(); n++ )
        (*aRefreshListeners[n])->refreshed( aEvent );
}

//------------------------------------------------------------------------

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

void ScDDELinksObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    //  Referenz-Update interessiert hier nicht

    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       // ungueltig geworden
    }
}

// XDDELinks

ScDDELinkObj* ScDDELinksObj::GetObjectByIndex_Impl(INT32 nIndex)
{
    if (pDocShell)
    {
        String aAppl, aTopic, aItem;
        if ( nIndex <= USHRT_MAX &&
                pDocShell->GetDocument()->GetDdeLinkData( (USHORT)nIndex, aAppl, aTopic, aItem ) )
            return new ScDDELinkObj( pDocShell, aAppl, aTopic, aItem );
    }
    return NULL;
}

ScDDELinkObj* ScDDELinksObj::GetObjectByName_Impl(const rtl::OUString& aName)
{
    if (pDocShell)
    {
        String aNamStr = aName;
        String aAppl, aTopic, aItem;

        ScDocument* pDoc = pDocShell->GetDocument();
        USHORT nCount = pDoc->GetDdeLinkCount();
        for (USHORT i=0; i<nCount; i++)
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
    ScUnoGuard aGuard;
    return new ScIndexEnumeration(this);
}

// XIndexAccess

sal_Int32 SAL_CALL ScDDELinksObj::getCount() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    INT32 nAreaCount = 0;
    if (pDocShell)
        nAreaCount = pDocShell->GetDocument()->GetDdeLinkCount();
    return nAreaCount;
}

uno::Any SAL_CALL ScDDELinksObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference<sheet::XDDELink> xLink = GetObjectByIndex_Impl(nIndex);
    uno::Any aAny;
    if (xLink.is())
        aAny <<= xLink;
    else
        throw lang::IndexOutOfBoundsException();
    return aAny;
}

uno::Type SAL_CALL ScDDELinksObj::getElementType() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return getCppuType((uno::Reference<sheet::XDDELink>*)0);
}

sal_Bool SAL_CALL ScDDELinksObj::hasElements() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return ( getCount() != 0 );
}

uno::Any SAL_CALL ScDDELinksObj::getByName( const rtl::OUString& aName )
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference<sheet::XDDELink> xLink = GetObjectByName_Impl(aName);
    uno::Any aAny;
    if (xLink.is())
        aAny <<= xLink;
    else
        throw container::NoSuchElementException();
    return aAny;
}

uno::Sequence<rtl::OUString> SAL_CALL ScDDELinksObj::getElementNames() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (pDocShell)
    {
        String aAppl, aTopic, aItem;

        ScDocument* pDoc = pDocShell->GetDocument();
        USHORT nCount = pDoc->GetDdeLinkCount();
        uno::Sequence<rtl::OUString> aSeq(nCount);
        rtl::OUString* pAry = aSeq.getArray();

        for (USHORT i=0; i<nCount; i++)
        {
            pDoc->GetDdeLinkData( i, aAppl, aTopic, aItem );
            pAry[i] = lcl_BuildDDEName(aAppl, aTopic, aItem);
        }
        return aSeq;
    }
    return uno::Sequence<rtl::OUString>();
}

sal_Bool SAL_CALL ScDDELinksObj::hasByName( const rtl::OUString& aName )
                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (pDocShell)
    {
        String aNamStr = aName;
        String aAppl, aTopic, aItem;

        ScDocument* pDoc = pDocShell->GetDocument();
        USHORT nCount = pDoc->GetDdeLinkCount();
        for (USHORT i=0; i<nCount; i++)
        {
            pDoc->GetDdeLinkData( i, aAppl, aTopic, aItem );
            if ( lcl_BuildDDEName(aAppl, aTopic, aItem) == aNamStr )
                return TRUE;
        }
    }
    return FALSE;
}

//------------------------------------------------------------------------




