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

#ifndef _CUI_FMSEARCH_HXX
#define _CUI_FMSEARCH_HXX

#include <com/sun/star/sdbc/XResultSet.hpp>

#include <svx/fmsearch.hxx> //CHINA001

#define _SVSTDARR_STRINGSDTOR
#include <svl/svstdarr.hxx>

#include <vcl/dialog.hxx>

#include <vcl/button.hxx>

#include <vcl/fixed.hxx>

#include <vcl/edit.hxx>
#include <vcl/combobox.hxx>
#include <vcl/lstbox.hxx>
#include <tools/link.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/stl_types.hxx>
#include <tools/string.hxx>

namespace svxform {
    class FmSearchConfigItem;
}

// ===================================================================================================
// = class FmSearchDialog - Dialog fuer Suchen in Formularen/Tabellen
// ===================================================================================================

struct FmSearchProgress;

class FmSearchEngine;

class FmSearchDialog : public ModalDialog
{
    friend class FmSearchEngine;

    // meine ganzen Controls
    FixedLine       m_flSearchFor;
    RadioButton     m_rbSearchForText;
    RadioButton     m_rbSearchForNull;
    RadioButton     m_rbSearchForNotNull;
    ComboBox        m_cmbSearchText;
    FixedLine       m_flWhere;
    FixedText       m_ftForm;
    ListBox         m_lbForm;
    RadioButton     m_rbAllFields;
    RadioButton     m_rbSingleField;
    ListBox         m_lbField;
    FixedLine       m_flOptions;
    FixedText       m_ftPosition;
    ListBox         m_lbPosition;
    CheckBox        m_cbUseFormat;
    CheckBox        m_cbCase;
    CheckBox        m_cbBackwards;
    CheckBox        m_cbStartOver;
    CheckBox        m_cbWildCard;
    CheckBox        m_cbRegular;
    CheckBox        m_cbApprox;
    PushButton      m_pbApproxSettings;
    CheckBox        m_aHalfFullFormsCJK;
    CheckBox        m_aSoundsLikeCJK;
    PushButton      m_aSoundsLikeCJKSettings;
    FixedLine       m_flState;
    FixedText       m_ftRecordLabel;
    FixedText       m_ftRecord;
    FixedText       m_ftHint;
    PushButton      m_pbSearchAgain;
    CancelButton    m_pbClose;
    HelpButton      m_pbHelp;
    String          m_sSearch;
    String          m_sCancel;

    Window*         m_pPreSearchFocus;

    Link    m_lnkFoundHandler;          // Handler fuer "gefunden"
    Link    m_lnkCanceledNotFoundHdl;   // Handler fuer Positionierung des Cursors

    Link    m_lnkContextSupplier;       // fuer Suche in verschiedenen Kontexten

    // ein Array, in dem ich mir fuer jeden Kontext das aktuell selektierte Feld merke
    ::std::vector<String> m_arrContextFields;

    // fuer die eigentliche Arbeit ...
    FmSearchEngine* m_pSearchEngine;

    Timer           m_aDelayedPaint;
        // siehe EnableSearchUI

    ::svxform::FmSearchConfigItem*      m_pConfig;
public:
    /** hiermit kann in verschiedenen Saetzen von Feldern gesucht werden. Es gibt eine Reihe von Kontexten, deren Namen in
        strContexts stehen (getrennt durch ';'), der Benutzer kann einen davon auswaehlen.
        Wenn der Benutzer einen Kontext auswaehlt, wird lnkContextSupplier aufgerufen, er bekommt einen Zeiger auf eine
        FmSearchContext-Struktur, die gefuellt werden muss.
        Fuer die Suche gilt dann :
        a) bei formatierter Suche wird der Iterator selber verwendet (wie beim ersten Constructor auch)
        b) bei formatierter Suche wird NICHT der FormatKey an den Fields des Iterators verwendet, sondern die entsprechende
            TextComponent wird gefragt (deshalb auch die Verwendung des originalen Iterator, durch dessen Move werden hoffentlich
            die hinter den TextComponent-Interfaces stehenden Controls geupdatet)
        c) bei nicht formatierter Suche wird ein Clone des Iterators verwendet (da ich hier die TextComponent-Interfaces nicht
            fragen muss)
        (natuerlich zwingend erforderlich : der String Nummer i in strUsedFields eines Kontexts muss mit dem Interface Nummer i
        in arrFields des Kontexts korrespondieren)
    */
    FmSearchDialog(Window* pParent, const String& strInitialText, const ::std::vector< String >& _rContexts, sal_Int16 nInitialContext,
        const Link& lnkContextSupplier);

    virtual ~FmSearchDialog();

    /** der Found-Handler bekommt im "gefunden"-Fall einen Zeiger auf eine FmFoundRecordInformation-Struktur
        (dieser ist nur im Handler gueltig, wenn man sich also die Daten merken muss, nicht den Zeiger, sondern die
        Struktur kopieren)
        Dieser Handler MUSS gesetzt werden.
        Ausserdem sollte beachtet werden, dass waehrend des Handlers der Suchdialog immer noch modal ist
    */
    void SetFoundHandler(const Link& lnk) { m_lnkFoundHandler = lnk; }
    /**
        Wenn die Suche abgebrochen oder erfolglos beendet wurde, wird im Suchdialog immer der aktuelle Datensatz angezeigt
        Damit das mit der eventuellen Anzeige des Aufrufers synchron geht, existiert dieser Handler (der nicht undbedingt gesetzt
        werden muss).
        Der dem Handler uebergebene Zeiger zeigt auf eine FmFoundRecordInformation-Struktur, bei der aPosition und eventuell
        (bei Suche mit Kontexten) nContext gueltig sind.
    */
    void SetCanceledNotFoundHdl(const Link& lnk) { m_lnkCanceledNotFoundHdl = lnk; }

    inline void SetActiveField(const String& strField);

protected:
    virtual sal_Bool Close();

    void Init(const String& strVisibleFields, const String& strInitialText);
        // nur von den Constructoren aus zu verwenden

    void OnFound(const ::com::sun::star::uno::Any& aCursorPos, sal_Int16 nFieldPos);

    void EnableSearchUI(sal_Bool bEnable);
        // beim Suchen in einem eigenen Thread moechte ich natuerlich die UI zum Starten/Parameter-Setzen der Suche disablen
        // Bei bEnable == sal_False wird fuer alle betroffenen Controls das Painten kurz aus- und mittels m_aDelayedPaint nach
        // einer kurzen Weile wieder angeschaltet. Wenn inzwischen eine Anforderung mit bEnable==sal_True kommt, wird der Timer gestoppt
        // und das Painten gleich wieder angeschaltet. Als Konsequenz dieses umstaendlichen Vorgehens ist kein Flackern zu sehen,
        // wenn man schnell hintereinander aus- und wieder einschaltet.

    void EnableSearchForDependees(sal_Bool bEnable);

    void EnableControlPaint(sal_Bool bEnable);
        // enabled (disabled) fuer alle wichtigen Controls ihr Paint

    void InitContext(sal_Int16 nContext);

    void LoadParams();
    void SaveParams() const;

private:
    // Handler fuer die Controls
    DECL_LINK( OnClickedFieldRadios, Button* );
    DECL_LINK( OnClickedSearchAgain, Button* );
    DECL_LINK( OnClickedSpecialSettings, Button* );

    DECL_LINK( OnSearchTextModified, ComboBox* );

    DECL_LINK( OnPositionSelected, ListBox* );
    DECL_LINK( OnFieldSelected, ListBox* );

    DECL_LINK( OnCheckBoxToggled, CheckBox* );

    DECL_LINK( OnContextSelection, ListBox* );

    // Such-Fortschritt
    DECL_LINK( OnSearchProgress, FmSearchProgress* );

    DECL_LINK( OnDelayedPaint, void* );
        // siehe EnableSearchUI

    void implMoveControls(Control** _ppControls, sal_Int32 _nControls, sal_Int32 _nUp, Control* _pToResize);

    void initCommon( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >& _rxCursor );
};

inline void FmSearchDialog::SetActiveField(const String& strField)
{
    sal_uInt16 nInitialField = m_lbField.GetEntryPos(strField);
    if (nInitialField == COMBOBOX_ENTRY_NOTFOUND)
        nInitialField = 0;
    m_lbField.SelectEntryPos(nInitialField);
    LINK(this, FmSearchDialog, OnFieldSelected).Call(&m_lbField);
}

#endif // _CUI_FMSEARCH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
