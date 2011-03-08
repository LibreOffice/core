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

#ifndef _FMSRCIMP_HXX
#define _FMSRCIMP_HXX

#include <svx/fmtools.hxx>
#include "svx/svxdllapi.h"

/** === begin UNO includes === **/
#include <com/sun/star/awt/XCheckBox.hpp>
#include <com/sun/star/awt/XListBox.hpp>
#include <com/sun/star/awt/XTextComponent.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
/** === end UNO includes === **/

#include <comphelper/stl_types.hxx>
#include <cppuhelper/implbase1.hxx>
#include <osl/mutex.hxx>
#include <unotools/charclass.hxx>
#include <unotools/collatorwrapper.hxx>
#include <osl/thread.hxx>

#ifndef _SVSTDARR_ULONGS
#define _SVSTDARR_ULONGS
#include <svl/svstdarr.hxx>
#endif

// ===================================================================================================
// Hilfsstrukturen

SV_DECL_OBJARR(SvInt32Array, sal_Int32, 16, 16)

// ===================================================================================================
// = class FmSearchThread - wie der Name schon sagt
// ===================================================================================================

class FmSearchEngine;
class FmSearchThread : public ::osl::Thread
{
    FmSearchEngine*     m_pEngine;
    Link                m_aTerminationHdl;

    virtual void SAL_CALL run();
    virtual void SAL_CALL onTerminated();

public:
    FmSearchThread(FmSearchEngine* pEngine) : m_pEngine(pEngine) { }
    void setTerminationHandler(Link aHdl) { m_aTerminationHdl = aHdl; }
};

// ===================================================================================================
// = struct FmSearchProgress - diese Struktur bekommt der Owner der SearchEngine fuer Status-Updates
// = (und am Ende der Suche)
// ===================================================================================================

struct FmSearchProgress
{
    enum STATE { STATE_PROGRESS, STATE_PROGRESS_COUNTING, STATE_CANCELED, STATE_SUCCESSFULL, STATE_NOTHINGFOUND, STATE_ERROR };
        // (Bewegung auf neuen Datensatz; Fortschritt beim Zaehlen von Datensaetzen; abgebrochen; Datensatz gefunden;
        // nichts gefunden, irgendein nicht zu handelnder Fehler)
    STATE   aSearchState;

    // aktueller Datensatz - immer gueltig (ist zum Beispiel bei Abbrechen auch fuer das Weitersuchen interesant)
    sal_uInt32  nCurrentRecord;
    // Ueberlauf - nur gueltig bei STATE_PROGRESS
    sal_Bool    bOverflow;

    // die Position des Such-Cursors - bei STATE_SUCCESSFULL, STATE_CANCELED und STATE_NOTHING_FOUND gueltig
    ::com::sun::star::uno::Any  aBookmark;
    // das Feld, in dem der Text gefunden wurde - bei STATE_SUCCESSFULL gueltig
    sal_Int32   nFieldIndex;
};

// ===================================================================================================
// = class FmRecordCountListener - Hilfsklasse fuer FmSearchEngine, lauscht an einem Cursor und teilt
// =                                Aenderungem im RecordCount mit
// ===================================================================================================

class FmRecordCountListener : public ::cppu::WeakImplHelper1< ::com::sun::star::beans::XPropertyChangeListener>
{
// Atribute
    Link            m_lnkWhoWantsToKnow;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   m_xListening;

// Attribut-Zugriff
public:
    Link SetPropChangeHandler(const Link& lnk);

// Oprationen
public:
    FmRecordCountListener(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >& dbcCursor);
        // the set has to support the sdb::ResultSet service
    virtual ~FmRecordCountListener();

    //  DECLARE_UNO3_AGG_DEFAULTS(FmPropertyListener, UsrObject);
    //  virtual sal_Bool queryInterface(::com::sun::star::uno::Uik aUik, ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rOut);

// ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::beans::XPropertyChangeListener
    virtual void SAL_CALL propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException);

    void DisConnect();

private:
    void NotifyCurrentCount();

};

// ===================================================================================================
// = class FmSearchEngine - Impl-Klasse fuer FmSearchDialog
// ===================================================================================================

namespace svxform {
    // We have three possible control types we may search in, determined by the supported interfaces : ::com::sun::star::awt::XTextComponent, ::com::sun::star::awt::XListBox, ::com::sun::star::awt::XCheckBox.
    // While searching we don't want to do this distinction for every control in every round. So we need some helpers.
    class ControlTextWrapper
    {
        // attributes
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >   m_xControl;
        // attribute access
    public:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  getControl() const{ return m_xControl; }
    public:
        ControlTextWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xControl) { m_xControl = _xControl; }
        virtual ~ControlTextWrapper() { }

        virtual ::rtl::OUString getCurrentText() const = 0;
    };
    class SimpleTextWrapper : public ControlTextWrapper
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent >  m_xText;
    public:
        SimpleTextWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent >& _xText);
        virtual ::rtl::OUString getCurrentText() const;
    };
    class ListBoxWrapper : public ControlTextWrapper
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XListBox >  m_xBox;
    public:
        ListBoxWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XListBox >& _xBox);
        virtual ::rtl::OUString getCurrentText() const;
    };
    class CheckBoxWrapper : public ControlTextWrapper
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XCheckBox >  m_xBox;
    public:
        CheckBoxWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XCheckBox >& _xBox);
        virtual ::rtl::OUString getCurrentText() const;
    };
}

enum FMSEARCH_MODE { SM_BRUTE, SM_ALLOWSCHEDULE, SM_USETHREAD };

DECLARE_STL_VECTOR( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>, InterfaceArray);

class SVX_DLLPUBLIC FmSearchEngine
{
    friend class FmSearchThread;

    enum SEARCH_RESULT { SR_FOUND, SR_NOTFOUND, SR_ERROR, SR_CANCELED };
    enum SEARCHFOR_TYPE { SEARCHFOR_STRING, SEARCHFOR_NULL, SEARCHFOR_NOTNULL };

    // zugrundeliegende Daten
    CursorWrapper           m_xSearchCursor;
    SvInt32Array            m_arrFieldMapping;
        // da der Iterator durchaus mehr Spalten haben kann, als ich eigentlich verwalte (in meiner Feld-Listbox),
        // muss ich mir hier ein Mapping dieser ::com::sun::star::form-Schluessel auf die Indizies der entsprechenden Spalten im Iterator halten

    // der Formatter
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >  m_xFormatSupplier;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >        m_xFormatter;

    CharClass               m_aCharacterClassficator;
    CollatorWrapper         m_aStringCompare;

    // die Sammlung aller interesanten Felder (bzw. ihre ::com::sun::star::data::XDatabaseVariant-Interfaces und ihre FormatKeys)
    struct FieldInfo
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >          xContents;
        sal_uInt32              nFormatKey;
        sal_Bool                bDoubleHandling;
    };

    DECLARE_STL_VECTOR(FieldInfo, FieldCollection);
    FieldCollection             m_arrUsedFields;
    sal_Int32                   m_nCurrentFieldIndex;   // der letzte Parameter von RebuildUsedFields, ermoeglicht mir Checks in FormatField

    DECLARE_STL_VECTOR(svxform::ControlTextWrapper*, ControlTextSuppliers);
    ControlTextSuppliers    m_aControlTexts;

    sal_Bool                m_bUsingTextComponents;
    CursorWrapper           m_xOriginalIterator;
    CursorWrapper           m_xClonedIterator;

    // Daten fuer Entscheidung, in welchem Feld ich ein "Found" akzeptiere
    ::com::sun::star::uno::Any  m_aPreviousLocBookmark;             // Position, an der ich zuletzt fuendig war
    FieldCollectionIterator     m_iterPreviousLocField;             // dito Feld

    // Kommunikation mit dem Thread, der die eigentliche Suche durchfuehrt
    ::rtl::OUString             m_strSearchExpression;              // Hinrichtung
    SEARCHFOR_TYPE      m_eSearchForType;                   // dito
    SEARCH_RESULT       m_srResult;                         // Rueckrichtung

    // der Link, dem ich Fortschritte und Ergebnisse mitteile
    Link                m_aProgressHandler;
    sal_Bool            m_bSearchingCurrently : 1;      // laeuft gerade eine (asynchrone) Suche ?
    sal_Bool            m_bCancelAsynchRequest : 1;     // soll abgebrochen werden ?
    ::osl::Mutex        m_aCancelAsynchAccess;          // Zugriff auf m_bCancelAsynchRequest (eigentlich nur bei
                                                        // m_eMode == SM_USETHREAD interesant)
    FMSEARCH_MODE   m_eMode;                            // der aktuelle Modus
    // der aktuelle Modus

    // Parameter fuer die Suche
    sal_Bool    m_bFormatter : 1;       // Feldformatierung benutzen
    sal_Bool    m_bForward : 1;         // Richtung
    sal_Bool    m_bWildcard : 1;        // Platzhalter-Suche ?
    sal_Bool    m_bRegular : 1;         // regulaerer Ausdruck
    sal_Bool    m_bLevenshtein : 1;     // Levenshtein-Suche
    sal_Bool    m_bTransliteration : 1; // Levenshtein-Suche

    sal_Bool    m_bLevRelaxed : 1;      // Parameter fuer Levenshtein-Suche
    sal_uInt16  m_nLevOther;
    sal_uInt16  m_nLevShorter;
    sal_uInt16  m_nLevLonger;

    sal_uInt16  m_nPosition;            // wenn nicht regulaer oder lev, dann einer der MATCHING_...-Werte

    sal_Int32   m_nTransliterationFlags;

// -------------
// Memberzugriff
private:
    SVX_DLLPRIVATE sal_Bool CancelRequested();      // liefert eine durch m_aCancelAsynchAccess gesicherte Auswertung von m_bCancelAsynchRequest

public:
    void        SetCaseSensitive(sal_Bool bSet);
    sal_Bool    GetCaseSensitive() const;

    void        SetFormatterUsing(sal_Bool bSet);   // das ist etwas umfangreicher, deshalb kein hier inline ....
    sal_Bool    GetFormatterUsing() const           { return m_bFormatter; }

    void        SetDirection(sal_Bool bForward)     { m_bForward = bForward; }
    sal_Bool    GetDirection() const                { return m_bForward; }

    void        SetWildcard(sal_Bool bSet)          { m_bWildcard = bSet; }
    sal_Bool    GetWildcard() const                 { return m_bWildcard; }

    void        SetRegular(sal_Bool bSet)           { m_bRegular = bSet; }
    sal_Bool    GetRegular() const                  { return m_bRegular; }

    void        SetLevenshtein(sal_Bool bSet)       { m_bLevenshtein = bSet; }
    sal_Bool    GetLevenshtein() const              { return m_bLevenshtein; }

    void        SetIgnoreWidthCJK(sal_Bool bSet);
    sal_Bool    GetIgnoreWidthCJK() const;

    void        SetTransliteration(sal_Bool bSet)   { m_bTransliteration = bSet; }
    sal_Bool    GetTransliteration() const          { return m_bTransliteration; }

    void        SetLevRelaxed(sal_Bool bSet)        { m_bLevRelaxed = bSet; }
    sal_Bool    GetLevRelaxed() const               { return m_bLevRelaxed; }
    void        SetLevOther(sal_uInt16 nHowMuch)    { m_nLevOther = nHowMuch; }
    sal_uInt16  GetLevOther() const                 { return m_nLevOther; }
    void        SetLevShorter(sal_uInt16 nHowMuch)  { m_nLevShorter = nHowMuch; }
    sal_uInt16  GetLevShorter() const               { return m_nLevShorter; }
    void        SetLevLonger(sal_uInt16 nHowMuch)   { m_nLevLonger = nHowMuch; }
    sal_uInt16  GetLevLonger() const                { return m_nLevLonger; }
        // die ganzen Lev-Werte werden nur bei  m_bLevenshtein==sal_True beachtet

    void        SetTransliterationFlags(sal_Int32 _nFlags)  { m_nTransliterationFlags = _nFlags; }
    sal_Int32   GetTransliterationFlags() const             { return m_nTransliterationFlags; }

    void    SetPosition(sal_uInt16 nValue)      { m_nPosition = nValue; }
    sal_uInt16  GetPosition() const             { return m_nPosition; }
        // Position wird bei m_bWildCard==sal_True nicht beachtet

    FMSEARCH_MODE GetSearchMode() const { return m_eMode; }

public:
    /** zwei Constructoren, beide analog zu denen des FmSearchDialog, Erklaerung siehe also dort ....
        xCursor muss jeweils den ::com::sun::star::data::DatabaseCursor-Service implementieren.
        wenn eMode == SM_USETHREAD, sollte ein ProgressHandler gesetzt sein, da dann die Ergebnisuebermittlung ueber diesen
        Handler erfolgt.
        Ist eMode != SM_USETHREAD, kehren SearchNext und StarOver nicht zurueck, bevor die Suche (erfolgreich oder nicht) beendet
        wurde, dann kann man das Ergebnis danach abfragen. Ist zusaetzlich der ProgressHandler gesetzt, wird dieser fuer jeden neuen
        Datensatz sowie am Ende der Suche aufgerufen.
    */
    FmSearchEngine(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >& xCursor,
        const ::rtl::OUString& strVisibleFields,
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >& xFormat,
        FMSEARCH_MODE eMode);
    FmSearchEngine(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >& xCursor,
        const ::rtl::OUString& strVisibleFields,
        const InterfaceArray& arrFields,
        FMSEARCH_MODE eMode);

    virtual ~FmSearchEngine();

    /** der Link wird fuer jeden Datensatz und nach Beendigung der Suche aufgerufen, Parameter ist ein Zeiger auf
        eine FmSearchProgress-Struktur
        der Handler sollte auf jeden Fall Thread-sicher sein
    */
    void SetProgressHandler(Link aHdl) { m_aProgressHandler = aHdl; }

    /// das naechste Vorkommen suchen (Werte fuer nDirection siehe DIRECTION_*-defines)
    void SearchNext(const ::rtl::OUString& strExpression);
    /// analogous, search for "NULL" (_bSearchForNull==sal_True) or "not NULL"
    void SearchNextSpecial(sal_Bool _bSearchForNull);
    /// das naechste Vorkommen suchen, abhaengig von nDirection wird dabei am Anfang oder am Ende neu begonnen
    void StartOver(const ::rtl::OUString& strExpression);
    /// analogous, search for "NULL" (_bSearchForNull==sal_True) or "not NULL"
    void StartOverSpecial(sal_Bool _bSearchForNull);
    /// die Angaben ueber letzte Fundstelle invalidieren
    void InvalidatePreviousLoc();

    /** baut m_arrUsedFields neu auf (nFieldIndex==-1 bedeutet alle Felder, ansonsten gibt es den Feldindex an)
        wenn bForce nicht gesetzt ist, passiert bei nFieldIndex == m_nCurrentFieldIndex nichts
        (ruft InvalidatePreviousLoc auf)
    */
    void RebuildUsedFields(sal_Int32 nFieldIndex, sal_Bool bForce = sal_False);
    ::rtl::OUString FormatField(sal_Int32 nWhich);

    /// kehrt sofort zurueck; nachdem wirklich abgebrochen wurde, wird der ProgressHandler mit STATE_CANCELED aufgerufen
    void CancelSearch();

    /** nur gueltig, wenn nicht gerade eine (asynchrone) Suche laeuft, die naechste Suche wird dann auf dem neuen Iterator
        mit den neuen Parametern durchgefuehrt
    */
    sal_Bool SwitchToContext(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >& xCursor, const ::rtl::OUString& strVisibleFields, const InterfaceArray& arrFields,
        sal_Int32 nFieldIndex);

protected:
    void Init(const ::rtl::OUString& strVisibleFields);

    void SearchNextImpl();
        // diese Impl-Methode laeuft im SearchThread

    // start a thread-search (or call SearchNextImpl directly, depending on the search mode)
    void ImplStartNextSearch();

private:
    SVX_DLLPRIVATE void clearControlTexts();
    SVX_DLLPRIVATE void fillControlTexts(const InterfaceArray& arrFields);

    // three methods implementing a complete search loop (null/not null, wildcard, SearchText)
    // (they all have some code in common, but with this solution we have do do a distinction only once per search (before
    // starting the loop), not in every loop step
    SVX_DLLPRIVATE SEARCH_RESULT SearchSpecial(sal_Bool _bSearchForNull, sal_Int32& nFieldPos, FieldCollectionIterator& iterFieldLoop,
        const FieldCollectionIterator& iterBegin, const FieldCollectionIterator& iterEnd);
    SVX_DLLPRIVATE SEARCH_RESULT SearchWildcard(const ::rtl::OUString& strExpression, sal_Int32& nFieldPos, FieldCollectionIterator& iterFieldLoop,
        const FieldCollectionIterator& iterBegin, const FieldCollectionIterator& iterEnd);
    SVX_DLLPRIVATE SEARCH_RESULT SearchRegularApprox(const ::rtl::OUString& strExpression, sal_Int32& nFieldPos, FieldCollectionIterator& iterFieldLoop,
        const FieldCollectionIterator& iterBegin, const FieldCollectionIterator& iterEnd);

    SVX_DLLPRIVATE void PropagateProgress(sal_Bool _bDontPropagateOverflow);
        // ruft den ProgressHandler mit STATE_PROGRESS und der aktuellen Position des SearchIterators auf

    // helpers, die ich mehrmals brauche
    SVX_DLLPRIVATE sal_Bool MoveCursor();
        // bewegt m_xSearchIterator unter Beachtung von Richtung/Ueberlauf Cursor
    SVX_DLLPRIVATE sal_Bool MoveField(sal_Int32& nPos, FieldCollectionIterator& iter, const FieldCollectionIterator& iterBegin, const FieldCollectionIterator& iterEnd);
        // bewegt den Iterator unter Beachtung von Richtung/Ueberlauf Iterator/Ueberlauf Cursor
    SVX_DLLPRIVATE void BuildAndInsertFieldInfo(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& xAllFields, sal_Int32 nField);
        // baut eine FieldInfo zum Feld Nummer nField (in xAllFields) auf und fuegt sie zu m_arrUsedFields hinzu
        // xAllFields muss den DatabaseRecord-Service unterstuetzen
    SVX_DLLPRIVATE ::rtl::OUString FormatField(const FieldInfo& rField);
        // formatiert das Feld mit dem NumberFormatter

    SVX_DLLPRIVATE sal_Bool HasPreviousLoc() { return m_aPreviousLocBookmark.hasValue(); }

    DECL_LINK(OnSearchTerminated, FmSearchThread*);
        // wird vom SuchThread benutzt, nach Rueckkehr aus diesem Handler loescht sich der Thread selber
    DECL_LINK(OnNewRecordCount, void*);
};

#endif // _FMSRCIMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
