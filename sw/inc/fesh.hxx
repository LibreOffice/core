/*************************************************************************
 *
 *  $RCSfile: fesh.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-05 12:04:44 $
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
#ifndef _FESH_HXX
#define _FESH_HXX

#ifndef _EDITSH_HXX
#include <editsh.hxx>
#endif
#ifndef _ORNTENUM_HXX
#include <orntenum.hxx>
#endif
#ifndef _FLYENUM_HXX
#include <flyenum.hxx>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC_XSPELLCHECKER1_HPP_
#include <com/sun/star/linguistic/XSpellChecker1.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC_XHYPHENATOR_HPP_
#include <com/sun/star/linguistic/XHyphenator.hpp>
#endif

class SwFlyFrm;
class SvEmbeddedObject;
class SwTabCols;
class SvEmbeddedObjectRef;
class SvInPlaceObject;
class SvxBrushItem;
class SwTableAutoFmt;
class SwFrm;
class SwTabFrm;
class SwFmtFrmSize;
class SvxBorderLine;
class SvStorageStream;
class SdrObject;
class Color;
class Outliner;
class SotDataObject;
class SdrViewUserMarker;
class SwFrmFmt;
struct SwSortOptions;

enum FrmType
{
    //Fuer GetFrmType() und GetSelFrmType(). Der Return-Wert ist eine
    //Veroderung.
    FRMTYPE_NONE    = 0,
    FRMTYPE_PAGE    = 1,
    FRMTYPE_HEADER  = 2,
    FRMTYPE_FOOTER  = 4,
    FRMTYPE_BODY    = 8,
    FRMTYPE_COLUMN  = 16,
    FRMTYPE_TABLE   = 32,
    FRMTYPE_FLY_FREE    = 64,
    FRMTYPE_FLY_ATCNT   = 128,
    FRMTYPE_FLY_INCNT   = 256,
    FRMTYPE_FOOTNOTE    = 512,
    FRMTYPE_FTNPAGE     = 1024,
    FRMTYPE_FLY_ANY     = 2048,
    FRMTYPE_DRAWOBJ     = 4096,
    FRMTYPE_COLSECT     = 8192,
    FRMTYPE_COLSECTOUTTAB = 16384
};

#define FRMTYPE_ANYCOLSECT ( FRMTYPE_COLSECT | FRMTYPE_COLSECTOUTTAB )

enum DrawObjType
{
    DRAW_ANY,
    DRAW_CONTROL,
    DRAW_SIMPLE
};

enum FlyProtectType
{
     FLYPROTECT_CONTENT     = 1,        // kann verodert werden!
     FLYPROTECT_SIZE        = 2,
     FLYPROTECT_POS         = 4,
     FLYPROTECT_PARENT      = 8,        // nur Parents untersuchen
     FLYPROTECT_FIXED       = 16        // nur nicht aufhebbarer Schutz
                                        // z.B. durch OLE-Server, gilt auch
                                        // fuer Dialog
};

enum ObjCntType     //Fuer das Ermitteln des Cntnts per Positon (D&D)
{
    OBJCNT_NONE,
    OBJCNT_FLY,
    OBJCNT_GRF,
    OBJCNT_OLE,
    OBJCNT_SIMPLE,
    OBJCNT_CONTROL,
    OBJCNT_URLBUTTON,

    OBJCNT_GROUPOBJ,
    OBJCNT_DONTCARE     // nicht bestimmbar - unterschiedliche Objecte selektiert
};

//fuer GetAnyCurRect
enum CurRectType
{
    RECT_PAGE,                  //Rect der aktuellen Seite.
    RECT_PAGE_CALC,             //... Seite wird ggf. Formatiert
    RECT_PAGE_PRT,              //Rect der aktuellen PrtArea der Seite
    RECT_FRM,                   //Rect des aktuellen Rahmen
    RECT_FLY_EMBEDDED,          //Rect des aktuellen FlyFrm
    RECT_FLY_PRT_EMBEDDED,      //Rect der PrtArea des FlyFrm
    RECT_SECTION,               //Rect des aktuellen Bereichs
    RECT_OUTTABSECTION,         //Rect des aktuellen Bereichs,
                                // aber ausserhalb der Tabelle
    RECT_SECTION_PRT,           //Rect der aktuellen PrtArea des Bereichs
    RECT_OUTTABSECTION_PRT,     //Rect der aktuellen PrtArea des Bereichs,
                                // aber ausserhalb der Tabelle
    RECT_HEADERFOOTER,          //Rect des aktuellen Headers/Footer
    RECT_HEADERFOOTER_PRT       //Rect der PrtArea des aktuellen Headers/Footers
};

struct SwGetCurColNumPara
{
    const SwFrmFmt* pFrmFmt;
    const SwRect* pPrtRect, *pFrmRect;
    SwGetCurColNumPara() : pFrmFmt( 0 ), pPrtRect( 0 ), pFrmRect( 0 ) {}
};

#define SW_PASTESDR_INSERT      1
#define SW_PASTESDR_REPLACE     2
#define SW_PASTESDR_SETATTR     3


class SwFEShell : public SwEditShell
{
    SdrViewUserMarker *pChainFrom,
                      *pChainTo;

    SwFlyFrm *FindFlyFrm() const;
    SwFlyFrm *FindFlyFrm( const SvEmbeddedObject *pObj ) const;

    BOOL GotoFly( BOOL bNext, FlyCntType eType = FLYCNTTYPE_ALL );

    //Actions fuer alle Shells beenden und ChangeLink rufen.
    void EndAllActionAndCall();

    void Scroll( const Point &rPt );

    void ChangeOpaque( BYTE nLayerId );

    void GetStartEndCell( SwLayoutFrm *&prStart, SwLayoutFrm *&prEnd );

    const SwFrm *GetBox( const Point &rPt ) const;

    //0 == in keiner Spalte
    USHORT _GetCurColNum( const SwFrm *pFrm,
                          SwGetCurColNumPara* pPara ) const;

    void _GetTabCols( SwTabCols &rToFill, const SwFrm *pBox ) const;

    BOOL ImpEndCreate();

    ObjCntType GetObjCntType( const SdrObject& rObj ) const;

    // Methoden fuers kopieren von DrawObjecten
    BOOL CopyDrawSel( SwFEShell* pDestShell, const Point& rSttPt,
                                const Point& rInsPt, BOOL bIsMove );

public:
    TYPEINFO();
    SwFEShell( SwDoc *pDoc,
            ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic::XSpellChecker1 > &xSpell,
            ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic::XHyphenator > &xHyph,
               Window *pWin,
               SwRootFrm *pMaster = 0, const SwViewOption *pOpt = 0 );
    SwFEShell( SwEditShell *pShell, Window *pWin );
    ~SwFEShell();

    // Copy und Paste Methoden fuer das interne Clipboard
    BOOL Copy( SwDoc* pClpDoc, const String* pNewClpTxt = 0 );
    BOOL Paste( SwDoc* pClpDoc );
    // Copy-Methode fuer Drag&Drop
    BOOL Copy( SwFEShell*, const Point& rSttPt, const Point& rInsPt,
                BOOL bIsMove = FALSE );

    void SelectFlyFrm( SwFlyFrm& rFrm, BOOL bNew = FALSE );

    // befindet sich der selektierte Rahmen innerhalb eines anderen?
    const SwFrmFmt* IsFlyInFly();
    // Innerhalb eines des sichtbaren Ankers?
    Rectangle *IsAnchorAtPos( const Point &rPt ) const;

//SS fuer DrawObjekte und Rahmen-----------------------------

    //Wenn ein Objekt angegeben wurde, so wird genau diese Markiert (anstatt
    //ueber die Position zu suchen.
    BOOL SelectObj( const Point& rSelPt, BOOL bAddSelect = FALSE,
                    BOOL bEnterGroup = FALSE, SdrObject *pObj = 0 );
    void DelSelectedObj();

    //Selektion nach oben unten bewegen (Z-Order).
    //TRUE  == ganz nach oben/unten
    //FALSE == einen ueberholen
    void SelectionToTop   ( BOOL bTop = TRUE );
    void SelectionToBottom( BOOL bBottom = TRUE );

    short GetLayerId() const;   //1 Heaven, 0 Hell, -1 Uneindeutig
    void  SelectionToHeaven();  //Ueber dem Dokument
    void  SelectionToHell();    //Unter dem Dokument

    // folgende zwei Methoden returnen den enum SdrHdlKind, um sich ein
    // includen von SVDRAW.HXX zu ersparen als int deklariert.
    int IsObjSelectable( const Point& rPt );
    int IsInsideSelectedObj( const Point& rPt );

    //Returnwerte siehe oben FrmType.
    //pPt: Crsr bzw. DocPos; bStopAtFly: Bei Flys anhalten oder ueber den Anchor weitergehen
    // Obgleich (0,TRUE) eine Art Standard ist, sind die Parameter nicht defaultet, damit
    // bei jeder Benutzung insbesondere das bStopAtFly bewusst genutzt wird.
    USHORT GetFrmType( const Point *pPt, BOOL bStopAtFly ) const;
    USHORT GetSelFrmType() const;               //Selektion (Drawing)

    ObjCntType GetObjCntType( const Point &rPt, SdrObject *&rpObj ) const;
    ObjCntType GetObjCntTypeOfSelection( SdrObject** ppObj = 0 ) const;

    //Zum Anpassen der PosAttr bei Ankerwechseln.
    SwRect  GetObjRect() const;
    //Zum Verschieben von Flys mit der Tastatur
    SwRect  GetFlyRect() const;
    void CalcBoundRect( SwRect &rRect, RndStdIds nAnchorId,
                        SwRelationOrient eRelOrient = FRAME,
                        BOOL bMirror = FALSE,
                        Point* pRef = NULL,
                        Size* pPercent = NULL ) const;

    // Groesse von Drawobjekten setzen
    void SetObjRect( const SwRect& rRect );

    long BeginDrag( const Point *pPt, BOOL bProp );
    long Drag     ( const Point *pPt, BOOL bProp );
    long EndDrag  ( const Point *pPt, BOOL bProp );
    void BreakDrag();

    //Methoden fuer die Statuszeile.
    Point GetAnchorObjDiff() const; //Abstand zum Anker
    Point GetObjAbsPos()     const; //Absolute Position
    Size  GetObjSize()       const; //Groesse, ggf. die umschliessende

    //SS fuer die BriefUmschlaege: hole alle Seitengebundenen Objekte
    //und setze diese auf eine neue Seite.
    void GetPageObjs( SvPtrarr& rFillArr );
    void SetPageObjsNewPage( SvPtrarr& rFillArr, int nOffset = 1 );

    // zeige die aktuelle Selektion an ( ggfs. den Rahmen/DrawObject)
    virtual void MakeSelVisible();

    // returne das FrmFmt von dem evt. unter dem Point stehenden Object.
    // Das Object wird nicht selektiert!
    const SwFrmFmt* GetFmtFromObj( const Point& rPt, SwRect** pRectToFill = 0 ) const;
    // returns a format too, if the point is over the text of any fly
    const SwFrmFmt* GetFmtFromAnyObj( const Point& rPt ) const;

    //Welcher Schutz ist am selektierten Objekt gesetzt?
    BYTE IsSelObjProtected( FlyProtectType eType ) const;

    //Liefert neben der Grafik in rName bei gelinkten Grafiken den Namen mit
    //Pfad und sonst den Grafiknamen. rbLink ist TRU bei gelinkten Grafiken.
    const Graphic *GetGrfAtPos( const Point &rDocPos,
                                String &rName, BOOL &rbLink ) const;

//SS fuer Rahmen --------------------------------------------

    BOOL IsFrmSelected() const;
    BOOL GetFlyFrmAttr( SfxItemSet &rSet ) const;
    BOOL SetFlyFrmAttr( SfxItemSet &rSet );
    BOOL ResetFlyFrmAttr( USHORT nWhich, const SfxItemSet* pSet = 0 );
    const SwFrmFmt *NewFlyFrm( const SfxItemSet &rSet, BOOL bAnchValid = FALSE,
                         SwFrmFmt *pParent = 0 );
    void SetFlyPos( const Point &rAbsPos);
    Point FindAnchorPos( const Point &rAbsPos, BOOL bMoveIt = FALSE );

    SwFrmFmt* GetCurFrmFmt() const; //Wenn Rahmen, dann Rahmenvorlage, sonst 0
    void SetFrmFmt( SwFrmFmt *pFmt, BOOL bKeepOrient = FALSE, Point* pDocPos = 0 ); //Wenn Rahmen, dann Rahmenvorlage setzen

    // finde/loeschen den Fly, in dem der Cursor steht
    SwFrmFmt* WizzardGetFly();
    BOOL WizzardDelFly();

    //Selebstaendiges selektieren von Flys
    BOOL GotoNextFly(FlyCntType eType = FLYCNTTYPE_ALL)
                                { return GotoFly( TRUE, eType ); }
    BOOL GotoPrevFly(FlyCntType eType = FLYCNTTYPE_ALL)
                                { return GotoFly( FALSE, eType); }

    //iterieren ueber Flys - fuer Basic-Collections
    USHORT GetFlyCount(FlyCntType eType = FLYCNTTYPE_ALL) const;
    const SwFrmFmt* GetFlyNum(USHORT nIdx, FlyCntType eType = FLYCNTTYPE_ALL) const;

    //Wenn ein fly selectiert ist, zieht er den Crsr in den ersten CntntFrm
    const SwFrmFmt* SelFlyGrabCrsr();

    //Get FlyFrameFormat; fuer UI Macro Anbindung an Flys
    const SwFrmFmt* GetFlyFrmFmt() const;

    //OLE, Server fordert neue Groesse an, die gewuenschten Werte werden
    //als Rahmenattribute eingestellt. Wenn die Werte nicht erlaubt sind,
    //so wird von der Formatierung geclippt und eine Scalierung eingestellt.
    //siehe CalcAndSetScale().
    void RequestObjectResize( const SwRect &rRect, SvEmbeddedObject *pIPObj );

    //Der Client fuer das OleObject muss bezueglich der Scalierung auf dem
    //neuesten Stand gehalten werden. Impl in der WrtShell.
    //Wird ein Pointer auf eine Size uebergeben, so ist diese die aktuelle
    //Core-Groesse des Objectes. Anderfalls wird die Groesse per GetCurFlyRect()
    //besorgt.
    virtual void CalcAndSetScale( SvEmbeddedObjectRef xIPObj,
                                  const SwRect *pFlyPrtRect = 0,
                                  const SwRect *pFlyFrmRect = 0 ) = 0;

    //Objekte mit ActivateWhenVisible werden beim Paint Connected.
    //gerufen von notxtfrm::Paint, impl in wrtsh
    virtual void ConnectObj( SvInPlaceObjectRef xIPObj, const SwRect &rPrt,
                             const SwRect &rFrm ) = 0;

    //Sichbaren Bereich auf das Object setzen, wenn es noch nicht sichtbar ist.
    void MakeObjVisible( const SvEmbeddedObject *pIPObj ) const;

    // setze am selektierten FlyFrame einen Namen
    void SetFlyName( const String& rName );
    const String& GetFlyName() const;

    // erezeuge eindeutige Namen fuer Rahmen
    String GetUniqueGrfName() const;
    String GetUniqueOLEName() const;
    String GetUniqueFrameName() const;

    // springe zum benannten Rahmen (Grafik/OLE)
    BOOL GotoFly( const String& rName, FlyCntType eType = FLYCNTTYPE_ALL,
                    BOOL bSelFrame = TRUE );
    // steht an der Position eine Grafik mit einer URL ?
    const SwFrmFmt* IsURLGrfAtPos( const Point& rPt, String* pURL = 0,
                                    String *pTargetFrameName = 0,
                                    String *pURLDescription = 0 ) const;

    //Fuer das Chain wird immer der durch das Format spezifizierte Fly
    //mit dem durch den Point getroffenen verbunden.
    //In rRect wird das Rect des Flys geliefert (fuer Highlight desselben)
    int Chainable( SwRect &rRect, const SwFrmFmt &rSource, const Point &rPt ) const;
    int Chain( SwFrmFmt &rSource, const Point &rPt );
    void Unchain( SwFrmFmt &rFmt );
    void HideChainMarker();
    void SetChainMarker();

    Size GetGraphicDefaultSize() const;

//SS fuer DrawObjekte ---------------------

    //Temporaer um Bug zu umgehen.
    void CheckUnboundObjects();

    //Achtung: Uneindeutikeiten bei Mehrfachselektionen.
    BOOL GetObjAttr( SfxItemSet &rSet ) const;
    BOOL SetObjAttr( const SfxItemSet &rSet );

    BOOL GotoObj( BOOL bNext, DrawObjType eType = DRAW_ANY);

    ULONG ControlCount() const;
    BOOL  GotoControl( ULONG nIndex );

    //Setzen vom DragMode (z.B. Rotate), tut nix bei Rahmenselektion.
    void SetDragMode( UINT16 eSdrDragMode );

    USHORT IsObjSelected() const;   //Liefert gleich die Anzahl der Objekte,
                                    //zaehlt aber nicht die Objekte in Gruppen.

    void EndTextEdit();             //Loescht ggf. das Objekt.

    //Ankertyp des selektierten Objektes, -1 bei Uneindeutigkeit oder
    //Rahmenselektion; FLY_PAGE bzw. FLY_AT_CNTNT aus frmatr.hxx sonst.
    short GetAnchorId() const;

    //Erzeugen von DrawObjekten, beim Begin wird der Objekttyp mitgegeben.
    //Beim End kann ein Cmd mitgegeben werden, hier ist ggf.
    //SDRCREATE_RESTRAINTEND fuer Ende oder SDRCREATE_NEXTPOINT fuer ein
    //Polygon relevant. Nach dem RESTRAINTEND ist das Objekt erzeugt und
    //selektiert.
    //Mit BreakCreate wird der Vorgang abgebrochen, dann ist kein Objekt
    //mehr selektiert.
    BOOL BeginCreate( UINT16 eSdrObjectKind, const Point &rPos );
    BOOL BeginCreate( UINT16 eSdrObjectKind, UINT32 eObjInventor, const Point &);
    void MoveCreate ( const Point &rPos );
    BOOL EndCreate  ( UINT16 eSdrCreateCmd );
    void BreakCreate();
    BOOL IsDrawCreate() const;

    // Funktionen fÅr Rubberbox, um Draw-Objekte zu selektieren
    BOOL BeginMark( const Point &rPos );
    void MoveMark ( const Point &rPos );
    BOOL EndMark  ();
    void BreakMark();

    //Gruppe erzeugen, aufloesen, nix bei Rahmenselektion.
    BOOL IsGroupSelected();     //Kann auch eine Mischselektion sein!
    void GroupSelection();      //Hinterher ist die Gruppe selektiert.
    void UnGroupSelection();    //Die Einzelobjekte sind Selektiert
                                //Es koennen noch immer Gruppen dabei sein.

    BOOL IsGroupAllowed() const;

    void MirrorSelection( BOOL bHorizontal );   //Bei FALSE Vertikal

    //Umankern. erlaubt sind: FLY_PAGE und FLY_AT_CNTNT des enum RndStdIds aus
    //frmatr.hxx. Hier kein enum wg. Abhaengigkeiten
    //Der BOOL ist nur fuer internen Gebrauch! Anker wird nur - anhand der
    //aktuellen Dokumentposition - neu gesetzt aber nicht umgesetzt.
    void ChgAnchor( int eAnchorId, BOOL bSameOnly = FALSE,
                                   BOOL bPosCorr = TRUE );

    // hole die selectierten DrawObj als Grafik (MetaFile/Bitmap)
    // Return-Wert besagt ob konvertiert wurde!!
    BOOL GetDrawObjGraphic( ULONG nFmt, Graphic& rGrf ) const;

    void Paste( SvStorageStream& rStm, USHORT nAction, const Point* pPt = 0 );
    BOOL Paste( const Graphic &rGrf );
    BOOL Paste( SotDataObject& rObj, const Point& rPt );

    BOOL IsAlignPossible() const;
    void SetCalcFieldValueHdl(Outliner* pOutliner);

        //Einfuegen eines DrawObjectes. Das Object muss bereits im DrawModel
        // angemeldet sein.
    void Insert( SdrObject& rDrawObj, const SfxItemSet* pFlyAttrSet = 0,
                SwFrmFmt* = 0, const Point* = 0 );

    BOOL ReplaceSdrObj( const String& rGrfName, const String& rFltName,
                        const Graphic* pGrf = 0 );

//------------------------------------------

    //Auskunft ueber naechstliegenden Inhalt zum uebergebenen Point
    Point GetCntntPos( const Point& rPoint, BOOL bNext ) const;

    //Layout-Selektion Hiden/Zeigen und aufruf an die CrsrSh weiterreichen.
    void ShLooseFcs();
    void ShGetFcs( BOOL bUpdate = TRUE );

    //PageDescriptor-Schnittstelle
    void   ChgCurPageDesc( const SwPageDesc& );
    USHORT GetCurPageDesc( const BOOL bCalcFrm = TRUE ) const;
    USHORT GetPageDescCnt() const;
    SwPageDesc* FindPageDescByName( const String& rName,
                                    BOOL bGetFromPool = FALSE,
                                    USHORT* pPos = 0 );

    const SwPageDesc& GetPageDesc( USHORT i ) const;
    void  ChgPageDesc( USHORT i, const SwPageDesc& );
    // if inside all selection only one PageDesc, return this.
    // Otherwise return 0 pointer
    const SwPageDesc* GetSelectedPageDescs() const;

    const SwRect& GetAnyCurRect( CurRectType eType,
                                 const Point* pPt = 0,
                                 const SvEmbeddedObject *pObj = 0 ) const;

    //Seitennummer der Seite in der der Point liegt, 0 wenn keine
    //getroffen ist.
    USHORT GetPageNumber( const Point &rPoint ) const;
    BOOL GetPageNumber( long nYPos, BOOL bAtCrsrPos, USHORT& rPhyNum, USHORT& rVirtNum, String &rDisplay ) const;


    void Insert(const String& rGrfName,
                const String& rFltName,
                const Graphic* pGraphic = 0,
                const SfxItemSet* pFlyAttrSet = 0,
                const SfxItemSet* pGrfAttrSet = 0,
                SwFrmFmt* = 0 );

    void Insert(SvInPlaceObject *,
                const SfxItemSet* pFlyAttrSet = 0,
                const SfxItemSet* pGrfAttrSet = 0,
                SwFrmFmt* = 0 );

    //Attribute der Tabelle besorgen/setzen.
    void GetTblAttr( SfxItemSet & ) const;
    void SetTblAttr( const SfxItemSet & );

    //Tabelle vollstaendig selektiert?
    BOOL HasWholeTabSelection() const;
    //Ist der Inhalt einer Tabellenzelle oder mindestens eine Tabellenzelle
    //vollstaendig selektiert ist
    BOOL HasBoxSelection() const;

    BOOL InsertRow( USHORT nCnt, BOOL bBehind );
    BOOL InsertCol( USHORT nCnt, BOOL bBehind );  // 0 == am Ende
    BOOL DeleteCol();
    BOOL DeleteRow();

    BOOL DeleteTblSel();        //Aktuelle Selektion, ggf. die ganze Tabelle.

    USHORT MergeTab();          //Merged selektierte Tabellenteile zusammen
                                //Fehler ueber enum zurueck
    BOOL SplitTab( BOOL nVert = TRUE, USHORT nCnt = 1); //Zelle Vertikal oder
                                                        //Horizontal s plitten.
    BOOL Sort(const SwSortOptions&);    //Sortieren.

    void SetRowHeight( const SwFmtFrmSize &rSz );
    //Der Pointer muss vom Aufrufer zerstoert werden wenn != 0
    void GetRowHeight( SwFmtFrmSize *&rpSz ) const;

    void   SetBoxAlign( USHORT nOrient );
    USHORT GetBoxAlign() const;         //USHRT_MAX fuer uneindeutig!

    //Ausgleichen der Zeilenhoehen. Mit bTstOnly festellen ob mehr als eine
    //Zeile markiert ist.
    BOOL BalanceRowHeight( BOOL bTstOnly );

    void SetTabBorders( const SfxItemSet& rSet );
    void GetTabBorders(       SfxItemSet& rSet) const;
    void SetTabLineStyle(const Color* pColor, BOOL bSetLine = FALSE, const SvxBorderLine* pBorderLine = NULL);

    void SetTabBackground( const SvxBrushItem &rNew );
    void GetTabBackground( SvxBrushItem &rToFill ) const;

    void SetBoxBackground( const SvxBrushItem &rNew );
    BOOL GetBoxBackground( SvxBrushItem &rToFill ) const; //FALSE uneindeutig

    void SetRowBackground( const SvxBrushItem &rNew );
    BOOL GetRowBackground( SvxBrushItem &rToFill ) const; //FALSE uneindeutig

    BOOL IsMouseTabCol( const Point &rPt ) const;
    void GetMouseTabCols( SwTabCols &ToFill, const Point &rPt ) const;
    void SetMouseTabCols( const SwTabCols &rNew, BOOL bCurRowOnly,
                          const Point &rPt );
    void GetTabCols( SwTabCols &rToFill ) const; //Spalten- und Randinfo.
    void SetTabCols( const SwTabCols &rNew, BOOL bCurRowOnly = TRUE );

    void ProtectCells();    //Falls eine Tabselektion besteht, wird sie ver-
                            // nichtet, wenn der Cursor nicht in Readonly darf
    void UnProtectCells();  // auf die Tabellenselektin
    void UnProtectCells( const String& rTblName );  // diese Tabelle
    void UnProtectTbls();   //bei allen Tabellen in der Selektion den Schutz aufheben
    BOOL HasTblAnyProtection( const String* pTblName = 0,
                                BOOL* pFullTblProtection = 0 );

    void SetHeadlineRepeat( BOOL bSet );//TRUE: Headline soll widerholt werden.
    BOOL IsHeadlineRepeat() const;
    BOOL IsInRepeatedHeadline() const;
    BOOL IsInHeadline() const;

    //Stellt die Breiten der Zellen so ein, dass der Inhalt moeglichst
    //nicht umgebrochen werden muss.
    //bBalance sorgt fuer einen Ausgleich der markierten Spalten.
    void AdjustCellWidth( BOOL bBalance = FALSE );
    //Nicht erlaubt, wenn nur  leere Zellen selektiert sind.
    BOOL IsAdjustCellWidthAllowed( BOOL bBalance = FALSE ) const;

    //Ausgleich der Zellenbreiten, mit bTstOnly feststellen, ob mehr als
    //eine Zelle markiert ist.
    BOOL BalanceCellWidth( BOOL bTstOnly );

        // AutoFormat fuer die Tabelle/TabellenSelection
    BOOL SetTableAutoFmt( const SwTableAutoFmt& rNew );
        // Erfrage wie attributiert ist
    BOOL GetTableAutoFmt( SwTableAutoFmt& rGet );
        // aender eine  Zellenbreite/-Hoehe/Spaltenbreite/Zeilenhoehe
    BOOL SetColRowWidthHeight( USHORT eType, USHORT nDiff = 283 );
        // Autosumme
    BOOL GetAutoSum( String& rFml ) const;

    //Phy:  Tatsaechliche Seitenanzahl.
    //Virt: Vom User evtl. gesetzten Offset mit einbeziehen.
    USHORT  GetPhyPageNum();
    USHORT  GetVirtPageNum( const BOOL bCalcFrm = TRUE );

    // Setzt an der aktuellen Postion einen neuen Page Offset
    void SetNewPageOffset( USHORT nOffset );
    void SetPageOffset( USHORT nOffset );   //Aendert den letzten Page Offset
    USHORT GetPageOffset() const;           //Liefert den letzten Page Offset

    //SS fuer Beschriftungen
    void InsertLabel( const SwLabelType eType, const String &rTxt,
                      const BOOL bBefore, const USHORT nId,
                      const BOOL bCpyBrd = TRUE );

    //Das Lineal will auch noch etwas von uns wissen.
    USHORT GetCurColNum( SwGetCurColNumPara* pPara = 0 ) const; //0 == in keiner Spalte
    USHORT GetCurMouseColNum( const Point &rPt,
                            SwGetCurColNumPara* pPara = 0 ) const;
    USHORT GetCurTabColNum() const;     //0 == in keiner Tabelle
    USHORT GetCurMouseTabColNum( const Point &rPt ) const;
    USHORT GetCurOutColNum( SwGetCurColNumPara* pPara = 0 ) const;  // aktuelle aeussere Spalte
    // Die Breite des aktuellen Bereichs fuer Spaltendialog
    long GetSectionWidth( SwFmt& rFmt ) const;
};

#endif
