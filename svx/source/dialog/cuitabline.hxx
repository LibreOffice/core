/*************************************************************************
 *
 *  $RCSfile: cuitabline.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 13:20:11 $
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
#ifndef _CUI_TAB_LINE_HXX
#define _CUI_TAB_LINE_HXX

// include ---------------------------------------------------------------
#include "tabline.hxx"
/*************************************************************************
|*
|* Transform-Tab-Dialog
|*
\************************************************************************/

class SvxLineTabDialog : public SfxTabDialog
{
private:
    SdrModel*           pDrawModel;
    const SdrObject*    pObj;

    const SfxItemSet&   rOutAttrs;

    XColorTable*        pColorTab;
    XDashList*          pDashList;
    XDashList*          pNewDashList;
    XLineEndList*       pLineEndList;
    XLineEndList*       pNewLineEndList;
    BOOL                bObjSelected;

    ChangeType          nLineEndListState;
    ChangeType          nDashListState;
    USHORT              nPageType;
    USHORT              nDlgType;
    USHORT              nPosDashLb;
    USHORT              nPosLineEndLb;

    virtual void        PageCreated( USHORT nId, SfxTabPage &rPage );

protected:
    virtual short       Ok();
#ifdef _SVX_TABLINE_CXX
    DECL_LINK( CancelHdl, void * );
    void                SavePalettes();
#endif

public:
    SvxLineTabDialog( Window* pParent, const SfxItemSet* pAttr,
                      SdrModel* pModel, const SdrObject* pObj = NULL,
                      BOOL bHasObj = TRUE );
    ~SvxLineTabDialog();

    void             SetNewDashList( XDashList* pInLst)
                        { pNewDashList = pInLst; }
    XDashList*       GetNewDashList() const { return pNewDashList; }
    const XDashList* GetDashList() const { return pDashList; }

    void                SetNewLineEndList( XLineEndList* pInLst)
                            { pNewLineEndList = pInLst; }
    XLineEndList*       GetNewLineEndList() const { return pNewLineEndList; }
    const XLineEndList* GetLineEndList() const { return pLineEndList; }
};

/*************************************************************************
|*
|* Linien-Tab-Page
|*
\************************************************************************/



#if SUPD>642
#else
//assert compatibility to removed NumMenuButton
class LineLocalExtMenuButton : public MenuButton
{
    Link            maClickHdl_Impl;
    public:
        LineLocalExtMenuButton(Window* pParent, const ResId& rResId) :
            MenuButton(pParent, rResId){}

};
#endif

class SvxLineTabPage : public SvxTabPage
{
private:
    FixedText           aFtLineStyle;
    LineLB              aLbLineStyle;
    FixedText           aFtColor;
    ColorLB             aLbColor;
    FixedText           aFtLineWidth;
    MetricField         aMtrLineWidth;
    FixedText           aFtTransparent;
    MetricField         aMtrTransparent;
    FixedLine           aFlLine;
    FixedText           aFtLineEndsStyle;
    LineEndLB           aLbStartStyle;
    FixedText           aFtLineEndsWidth;
    MetricField         aMtrStartWidth;
    TriStateBox         aTsbCenterStart;
    LineEndLB           aLbEndStyle;
    MetricField         aMtrEndWidth;
    TriStateBox         aTsbCenterEnd;
    CheckBox            aCbxSynchronize;
    FixedLine           aFlLineEnds;
    SvxXLinePreview     aCtlPreview;
    FixedLine           aFLSeparator;

    // #116827#
    FixedLine           maFLEdgeStyle;
    FixedText           maFTEdgeStyle;
    LineEndLB           maLBEdgeStyle;

    //#58425# Symbole auf einer Linie (z.B. StarChart) ->
    SdrObjList*         pSymbolList; //a list of symbols to be shown in menu. Symbol at position SID_ATTR_SYMBOLTYPE is to be shown in preview. The list position is to be used cyclic.
    BOOL                bNewSize;
    Graphic             aAutoSymbolGraphic; //a graphic to be displayed in the preview in case that an automatic symbol is choosen
    long                nNumMenuGalleryItems;
    long                nSymbolType;
    SfxItemSet*         pSymbolAttr; //attributes for the shown symbols; only necessary if not equal to line properties
    FixedLine           aFlSymbol;
#if SUPD>642
    MenuButton          aSymbolMB;
#else
    LineLocalExtMenuButton  aSymbolMB;
#endif
    FixedText           aSymbolWidthFT;
    MetricField         aSymbolWidthMF;
    FixedText           aSymbolHeightFT;
    MetricField         aSymbolHeightMF;
    CheckBox            aSymbolRatioCB;
    List                aGrfNames;
    List                aGrfBrushItems;
    String              sNumCharFmtName;
    BOOL                bLastWidthModified;
    Size                aSymbolLastSize;
    Graphic             aSymbolGraphic;
    Size                aSymbolSize;
    BOOL                bSymbols;
    //Handler für Gallery-Popup-Menue-Button + Size
    DECL_LINK( GraphicHdl_Impl, MenuButton * );
    DECL_LINK( MenuCreateHdl_Impl, MenuButton * );
    DECL_STATIC_LINK( SvxLineTabPage, GraphicArrivedHdl_Impl, SvxBrushItem* );
    DECL_LINK( SizeHdl_Impl, MetricField * );
    DECL_LINK( RatioHdl_Impl, CheckBox * );
    // <- Symbole


    const SfxItemSet&   rOutAttrs;
    RECT_POINT          eRP;
    BOOL                bObjSelected;

    XOutdevItemPool*    pXPool;
    XOutputDevice       XOut;
    XLineStyleItem      aXLStyle;
    XLineWidthItem      aXWidth;
    XLineDashItem       aXDash;
    XLineColorItem      aXColor;
    XLineAttrSetItem    aXLineAttr;
    SfxItemSet&         rXLSet;

    XColorTable*        pColorTab;
    XDashList*          pDashList;
    XLineEndList*       pLineEndList;

    ChangeType*         pnLineEndListState;
    ChangeType*         pnDashListState;
//CHINA001  USHORT*             pPageType;
//CHINA001  USHORT*             pDlgType;
    UINT16             nPageType; //add CHINA001
    UINT16             nDlgType; //add CHINA001
    USHORT*             pPosDashLb;
    USHORT*             pPosLineEndLb;

    SfxMapUnit          ePoolUnit;

    // #63083#
    INT32               nActLineWidth;

#ifdef _SVX_TPLINE_CXX
    DECL_LINK( ClickInvisibleHdl_Impl, void * );
    DECL_LINK( ChangeStartHdl_Impl, void * );
    DECL_LINK( ChangeEndHdl_Impl, void * );
    DECL_LINK( ChangePreviewHdl_Impl, void * );
    DECL_LINK( ChangeTransparentHdl_Impl, void * );

    // #116827#
    DECL_LINK( ChangeEdgeStyleHdl_Impl, void * );

    BOOL FillXLSet_Impl();
#endif

    void FillListboxes();
public:

    void ShowSymbolControls(BOOL bOn);

    SvxLineTabPage( Window* pParent, const SfxItemSet& rInAttrs );
    virtual ~SvxLineTabPage();

    void    Construct();


    static  SfxTabPage* Create( Window*, const SfxItemSet& );
    static  USHORT*    GetRanges();

    virtual BOOL FillItemSet( SfxItemSet& );
    virtual void Reset( const SfxItemSet& );

    virtual void ActivatePage( const SfxItemSet& rSet );
    virtual int  DeactivatePage( SfxItemSet* pSet );

    virtual void PointChanged( Window* pWindow, RECT_POINT eRP );

    virtual void FillUserData();

    void    SetColorTable( XColorTable* pColTab ) { pColorTab = pColTab; }
    void    SetDashList( XDashList* pDshLst ) { pDashList = pDshLst; }
    void    SetLineEndList( XLineEndList* pLneEndLst) { pLineEndList = pLneEndLst; }
    void    SetObjSelected( BOOL bHasObj ) { bObjSelected = bHasObj; }

    void    SetPageType( UINT16 nInType ) { nPageType = nInType; }//CHINA001 void    SetPageType( USHORT* pInType ) { pPageType = pInType; }
    void    SetDlgType( UINT16 nInType ) { nDlgType = nInType; }    //CHINA001 void    SetDlgType( USHORT* pInType ) { pDlgType = pInType; }
    void    SetPosDashLb( USHORT* pInPos ) { pPosDashLb = pInPos; }
    void    SetPosLineEndLb( USHORT* pInPos ) { pPosLineEndLb = pInPos; }

    void    SetLineEndChgd( ChangeType* pIn ) { pnLineEndListState = pIn; }
    void    SetDashChgd( ChangeType* pIn ) { pnDashListState = pIn; }
    virtual void PageCreated (SfxAllItemSet aSet); //add CHINA001
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
};

/*************************************************************************
|*
|* Linien-Definitions-Tab-Page
|*
\************************************************************************/

class SvxLineDefTabPage : public SfxTabPage
{
private:
    FixedLine           aFlDefinition;
    FixedText           aFTLinestyle;
    LineLB              aLbLineStyles;
    FixedText           aFtType;
    ListBox             aLbType1;
    ListBox             aLbType2;
    FixedText           aFtNumber;
    NumericField        aNumFldNumber1;
    NumericField        aNumFldNumber2;
    FixedText           aFtLength;
    MetricField         aMtrLength1;
    MetricField         aMtrLength2;
    FixedText           aFtDistance;
    MetricField         aMtrDistance;
    CheckBox            aCbxSynchronize;
    PushButton          aBtnAdd;
    PushButton          aBtnModify;
    PushButton          aBtnDelete;
    ImageButton         aBtnLoad;
    ImageButton         aBtnSave;
    SvxXLinePreview     aCtlPreview;

    const SfxItemSet&   rOutAttrs;
    XDash               aDash;
    BOOL                bObjSelected;

    XOutdevItemPool*    pXPool;
    XOutputDevice       XOut;
    XLineStyleItem      aXLStyle;
    XLineWidthItem      aXWidth;
    XLineDashItem       aXDash;
    XLineColorItem      aXColor;
    XLineAttrSetItem    aXLineAttr;
    SfxItemSet&         rXLSet;

    XDashList*          pDashList;

    ChangeType*         pnDashListState;
    USHORT*             pPageType;
    USHORT*             pDlgType;
    USHORT*             pPosDashLb;

    SfxMapUnit          ePoolUnit;
    FieldUnit           eFUnit;

#ifdef _SVX_TPLNEDEF_CXX
    void FillDash_Impl();
    void FillDialog_Impl();

    DECL_LINK( ClickAddHdl_Impl, void * );
    DECL_LINK( ClickModifyHdl_Impl, void * );
    DECL_LINK( ClickDeleteHdl_Impl, void * );
    DECL_LINK( SelectLinestyleHdl_Impl, void * );
    DECL_LINK( ChangePreviewHdl_Impl, void * );
    DECL_LINK( ChangeNumber1Hdl_Impl, void * );
    DECL_LINK( ChangeNumber2Hdl_Impl, void * );
    DECL_LINK( ClickLoadHdl_Impl, void * );
    DECL_LINK( ClickSaveHdl_Impl, void * );
    DECL_LINK( ChangeMetricHdl_Impl, void * );
    DECL_LINK( SelectTypeHdl_Impl, void * );

    void CheckChanges_Impl();
#endif

public:
    SvxLineDefTabPage( Window* pParent, const SfxItemSet& rInAttrs  );

    void    Construct();

    static  SfxTabPage* Create( Window*, const SfxItemSet& );
    virtual BOOL FillItemSet( SfxItemSet& );
    virtual void Reset( const SfxItemSet & );

    virtual void ActivatePage( const SfxItemSet& rSet );
    virtual int  DeactivatePage( SfxItemSet* pSet );

    void    SetDashList( XDashList* pDshLst ) { pDashList = pDshLst; }
    void    SetObjSelected( BOOL bHasObj ) { bObjSelected = bHasObj; }

    void    SetPageType( USHORT* pInType ) { pPageType = pInType; }
    void    SetDlgType( USHORT* pInType ) { pDlgType = pInType; }
    void    SetPosDashLb( USHORT* pInPos ) { pPosDashLb = pInPos; }

    void    SetDashChgd( ChangeType* pIn ) { pnDashListState = pIn; }

    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
};

/*************************************************************************
|*
|* Linienenden-Definitions-Tab-Page
|*
\************************************************************************/

class SvxLineEndDefTabPage : public SfxTabPage
{
private:
    FixedLine           aFlTip;
    FixedText           aFTTitle;
    Edit                aEdtName;
    FixedText           aFTLineEndStyle;
    LineEndLB           aLbLineEnds;
    PushButton          aBtnAdd;
    PushButton          aBtnModify;
    PushButton          aBtnDelete;
    ImageButton         aBtnLoad;
    ImageButton         aBtnSave;
    FixedInfo           aFiTip;
    SvxXLinePreview     aCtlPreview;

    const SfxItemSet&   rOutAttrs;
    const SdrObject*    pPolyObj;
    BOOL                bObjSelected;

    XOutdevItemPool*    pXPool;
    XOutputDevice       XOut;
    XLineStyleItem      aXLStyle;
    XLineWidthItem      aXWidth;
    XLineColorItem      aXColor;
    XLineAttrSetItem    aXLineAttr;
    SfxItemSet&         rXLSet;

    XLineEndList*       pLineEndList;

    ChangeType*         pnLineEndListState;
    USHORT*             pPageType;
    USHORT*             pDlgType;
    USHORT*             pPosLineEndLb;

#ifdef _SVX_TPLNEEND_CXX
    DECL_LINK( ClickAddHdl_Impl, void * );
    DECL_LINK( ClickModifyHdl_Impl, void * );
    DECL_LINK( ClickDeleteHdl_Impl, void * );
    DECL_LINK( ClickLoadHdl_Impl, void * );
    DECL_LINK( ClickSaveHdl_Impl, void * );
    DECL_LINK( SelectLineEndHdl_Impl, void * );
    long ChangePreviewHdl_Impl( void* p );

    void CheckChanges_Impl();
#endif

public:
    SvxLineEndDefTabPage( Window* pParent, const SfxItemSet& rInAttrs );
    ~SvxLineEndDefTabPage();

    void    Construct();

    static  SfxTabPage* Create( Window*, const SfxItemSet& );
    virtual BOOL FillItemSet( SfxItemSet& );
    virtual void Reset( const SfxItemSet & );

    virtual void ActivatePage( const SfxItemSet& rSet );
    virtual int  DeactivatePage( SfxItemSet* pSet );

    void    SetLineEndList( XLineEndList* pInList ) { pLineEndList = pInList; }
    void    SetPolyObj( const SdrObject* pObj ) { pPolyObj = pObj; }
    void    SetObjSelected( BOOL bHasObj ) { bObjSelected = bHasObj; }

    void    SetPageType( USHORT* pInType ) { pPageType = pInType; }
    void    SetDlgType( USHORT* pInType ) { pDlgType = pInType; }
    void    SetPosLineEndLb( USHORT* pInPos ) { pPosLineEndLb = pInPos; }

    void    SetLineEndChgd( ChangeType* pIn ) { pnLineEndListState = pIn; }

    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
};


#endif // _CUI_TAB_LINE_HXX

