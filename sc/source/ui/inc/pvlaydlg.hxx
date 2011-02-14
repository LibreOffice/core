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

#ifndef SC_PVLAYDLG_HXX
#define SC_PVLAYDLG_HXX

#include <vector>
#include <memory>
#include <boost/shared_ptr.hpp>

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _SCRBAR_HXX //autogen
#include <vcl/scrbar.hxx>
#endif
#include <svtools/stdctrl.hxx>
#ifndef _MOREBTN_HXX //autogen
#include <vcl/morebtn.hxx>
#endif
#include "pivot.hxx"
#include "anyrefdg.hxx"
#include "fieldwnd.hxx"
#include "formula/funcutl.hxx"

/*==========================================================================*\

    Eine Instanz der Klasse ScPivotLayoutDlg ist ein (semi-)modaler
    Dialog, in dem mit der Maus Felder mit Spaltenueberschriften den
    drei Pivot-Kategorien "Spalte", "Zeile" und "Daten" zugeordnet
    werden koennen.

    Der Dialog erhaelt in der Struktur LabelData Informationen ueber
    diese Ueberschriften (Name, Art (Zahl/String) und Funktionsmaske).
    Weiterhin werden drei PivotFeld-Arrays uebergeben, mit denen die
    drei Kategorie-Fenster initialisiert werden. Ein Kategorie-Fenster
    wird durch eine Instanz der Klasse FieldWindow dargestellt. Ein
    solches Fenster ist fuer die Darstellung der Datenstrukturen am
    Schirm zustaendig. Es meldet Mausaktionen an den Dialog weiter und
    bietet entsprechende Methoden zur Veraenderung der Darstellung.
    Der Dialog sorgt fuer den Abgleich der interenen Datenstrukturen mit
    der Bildschirmdarstellung. Ein weiteres FieldWindow (Select) bietet
    alle Tabellenueberschriften zur Auswahl an, ist also "read-only".

\*==========================================================================*/

//============================================================================

class ScViewData;
class ScDocument;
class ScRangeData;
struct ScDPFuncData;
class ScDPObject;

//============================================================================

#define FUNC_COUNT 11

class ScDPLayoutDlg : public ScAnyRefDlg
{
public:
                            ScDPLayoutDlg(
                                SfxBindings* pB,
                                SfxChildWindow* pCW,
                                Window* pParent,
                                const ScDPObject& rDPObject );
    virtual                 ~ScDPLayoutDlg();

    virtual void            SetReference( const ScRange& rRef, ScDocument* pDoc );
    virtual sal_Bool            IsRefInputMode() const { return bRefInputMode; }
    virtual void            SetActive();
    virtual sal_Bool            Close();
    virtual void            StateChanged( StateChangedType nStateChange );

    void                    NotifyDoubleClick    ( ScDPFieldType eType, size_t nFieldIndex );
    PointerStyle            NotifyMouseButtonDown( ScDPFieldType eType, size_t nFieldIndex );
    void                    NotifyMouseButtonUp  ( const Point& rAt );
    PointerStyle            NotifyMouseMove      ( const Point& rAt );
    void                    NotifyFieldFocus     ( ScDPFieldType eType, sal_Bool bGotFocus );
    void                    NotifyMoveField      ( ScDPFieldType eToType );
    void                    NotifyRemoveField    ( ScDPFieldType eType, size_t nFieldIndex );
    sal_Bool                    NotifyMoveSlider     ( sal_uInt16 nKeyCode );   // return sal_True, if position changed

protected:
    virtual void            Deactivate();

private:
    typedef boost::shared_ptr< ScDPFuncData >   ScDPFuncDataRef;
    typedef std::vector< ScDPFuncDataRef >      ScDPFuncDataVec;
    typedef std::auto_ptr< ScDPObject >         ScDPObjectPtr;

    FixedLine               aFlLayout;
    FixedText               aFtPage;
    ScDPFieldWindow         aWndPage;
    FixedText               aFtCol;
    ScDPFieldWindow         aWndCol;
    FixedText               aFtRow;
    ScDPFieldWindow         aWndRow;
    FixedText               aFtData;
    ScDPFieldWindow         aWndData;
    ScDPFieldWindow         aWndSelect;
    ScrollBar               aSlider;
    FixedInfo               aFtInfo;

    FixedLine               aFlAreas;

    // DP source selection
    FixedText               aFtInArea;
    ::formula::RefEdit      aEdInPos;
    ::formula::RefButton    aRbInPos;

    // DP output location
    ListBox                 aLbOutPos;
    FixedText               aFtOutArea;
    formula::RefEdit        aEdOutPos;
    formula::RefButton      aRbOutPos;

    CheckBox                aBtnIgnEmptyRows;
    CheckBox                aBtnDetectCat;
    CheckBox                aBtnTotalCol;
    CheckBox                aBtnTotalRow;
    CheckBox                aBtnFilter;
    CheckBox                aBtnDrillDown;

    OKButton                aBtnOk;
    CancelButton            aBtnCancel;
    HelpButton              aBtnHelp;
    PushButton              aBtnRemove;
    PushButton              aBtnOptions;
    MoreButton              aBtnMore;

    const String            aStrUndefined;
    const String            aStrNewTable;
    std::vector< String >   aFuncNameArr;

    ScDPFieldType           eDnDFromType;
    size_t                  nDnDFromIndex;
    sal_Bool                    bIsDrag;

    ::formula::RefEdit*     pEditActive;

    Rectangle               aRectPage;
    Rectangle               aRectRow;
    Rectangle               aRectCol;
    Rectangle               aRectData;
    Rectangle               aRectSelect;

    ScDPLabelDataVec        aLabelDataArr; // (nCol, Feldname, Zahl/Text)

    ScDPFieldType           eLastActiveType;        /// Type of last active area.
    size_t                  nOffset;                /// Offset of first field in TYPE_SELECT area.

    ScDPFuncDataVec         aSelectArr;
    ScDPFuncDataVec         aPageArr;
    ScDPFuncDataVec         aColArr;
    ScDPFuncDataVec         aRowArr;
    ScDPFuncDataVec         aDataArr;

    ScDPObjectPtr           xDlgDPObject;
    ScRange                 aOldRange;
    ScPivotParam            thePivotData;
    ScViewData*             pViewData;
    ScDocument*             pDoc;
    sal_Bool                    bRefInputMode;

private:
    ScDPFieldWindow&        GetFieldWindow  ( ScDPFieldType eType );
    void                    Init            ();
    void                    InitWndSelect   ( const ::std::vector<ScDPLabelDataRef>& rLabels );
    void                    InitWnd         ( PivotField* pArr, long nCount, ScDPFieldType eType );
    void                    InitFocus       ();
    void                    InitFields      ();
    void                    CalcWndSizes    ();
    Point                   DlgPos2WndPos   ( const Point& rPt, Window& rWnd );
    ScDPLabelData*          GetLabelData    ( SCsCOL nCol, size_t* pPos = NULL );
    String                  GetLabelString  ( SCsCOL nCol );
    bool                    IsOrientationAllowed( SCsCOL nCol, ScDPFieldType eType );
    String                  GetFuncString   ( sal_uInt16& rFuncMask, sal_Bool bIsValue = sal_True );
    sal_Bool                    Contains        ( ScDPFuncDataVec* pArr, SCsCOL nCol, size_t& nAt );
    void                    Remove          ( ScDPFuncDataVec* pArr, size_t nAt );
    void                    Insert          ( ScDPFuncDataVec* pArr, const ScDPFuncData& rFData, size_t nAt );

    void                    AddField        ( size_t nFromIndex,
                                              ScDPFieldType eToType, const Point& rAtPos );
    void                    MoveField       ( ScDPFieldType eFromType, size_t nFromIndex,
                                              ScDPFieldType eToType, const Point&  rAtPos );
    void                    RemoveField     ( ScDPFieldType eRemType, size_t nRemIndex );

    sal_Bool                    GetPivotArrays  ( PivotField*   pPageArr,
                                              PivotField*   pColArr,
                                              PivotField*   pRowArr,
                                              PivotField*   pDataArr,
                                              sal_uInt16&       rPageCount,
                                              sal_uInt16&       rColCount,
                                              sal_uInt16&       rRowCount,
                                              sal_uInt16&       rDataCount );

    void                    UpdateSrcRange();

    // Handler
    DECL_LINK( ClickHdl, PushButton * );
    DECL_LINK( ScrollHdl, ScrollBar * );
    DECL_LINK( SelAreaHdl, ListBox * );
    DECL_LINK( MoreClickHdl, MoreButton * );
    DECL_LINK( EdModifyHdl, Edit * );
    DECL_LINK( EdInModifyHdl, Edit * );
    DECL_LINK( OkHdl, OKButton * );
    DECL_LINK( CancelHdl, CancelButton * );
    DECL_LINK( GetFocusHdl, Control* );
};



#endif // SC_PVLAYDLG_HXX

