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

#ifndef _SDPAGE_HXX
#define _SDPAGE_HXX

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/presentation/FadeEffect.hpp>

#ifndef _SVDOBJ_HXX //autogen
#include <bf_svx/svdobj.hxx>
#endif
#ifndef _FM_FMPAGE_HXX //autogen
#include <bf_svx/fmpage.hxx>
#endif
#ifndef _SD_FADEDEF_H
#include <bf_sd/fadedef.h>
#endif
#ifndef _SD_DIADEF_H
#include <bf_sd/diadef.h>
#endif
#ifndef _PRESENTATION
#include <bf_sd/pres.hxx>
#endif
namespace binfilter {

class SfxStyleSheet;
class SfxItemSet;
class StarBASIC;
class SdDrawDocument;
class SdrTextObj;
struct StyleRequestData;

enum PresObjKind
{
    PRESOBJ_NONE,
    PRESOBJ_TITLE,
    PRESOBJ_OUTLINE,
    PRESOBJ_TEXT,
    PRESOBJ_GRAPHIC,
    PRESOBJ_OBJECT,
    PRESOBJ_CHART,
    PRESOBJ_ORGCHART,
    PRESOBJ_TABLE,
    PRESOBJ_IMAGE,
    PRESOBJ_BACKGROUND,
    PRESOBJ_PAGE,
    PRESOBJ_HANDOUT,
    PRESOBJ_NOTES
};

class SdPage : public FmFormPage, public SdrObjUserCall
{
friend class SdGenericDrawPage;

protected:
    PageKind	ePageKind;				  // Seitentyp
    AutoLayout	eAutoLayout;			  // AutoLayout
    List		aPresObjList;			  // Praesentationsobjekte
    BOOL		bOwnArrangement;		  // Objekte werden intern angeordnet

    BOOL		bSelected;				  // Selektionskennung
    FadeSpeed	eFadeSpeed; 			  // Ueberblendgeschwindigkeit
    ::com::sun::star::presentation::FadeEffect	eFadeEffect;			  // Ueberblendeffekt
    PresChange	ePresChange;			  // manuell/automatisch/halbautomatisch
    UINT32		nTime;					  // Anzeigedauer in Sekunden
    BOOL		bSoundOn;				  // mit/ohne Sound (TRUE/FALSE)
    BOOL		bExcluded;				  // wird in der Show nicht/doch
                                          // angezeigt (TRUE/FALSE)
    String		aLayoutName;			  // Name des Layouts
    String      aSoundFile;               // Pfad zum Soundfile (MSDOS-Notation)
    String      aCreatedPageName;         // von GetPageName erzeugter Seitenname
    String      aFileName;                // Filename
    String		aBookmarkName;			  // Bookmarkname
    BOOL		bScaleObjects;			  // Objekte sollen skaliert werden
    BOOL		bBackgroundFullSize;	  // Hintergrundobjekt auf ganze Seite darstellen
    rtl_TextEncoding eCharSet;            // Text-Encoding
    USHORT      nPaperBin;                // PaperBin
    Orientation eOrientation;             // Print-Orientation

    BOOL		InsertPresObj(SdrObject* pObj, PresObjKind eObjKind, BOOL bVertical,
                              Rectangle rRect, BOOL bInit, List& rObjList);

    void		AdjustBackgroundSize();
    Rectangle   GetTitleRect() const;
    Rectangle   GetLayoutRect() const;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createUnoPage();

    SfxItemSet*	mpItems;
    SfxItemSet* getOrCreateItems();

public:
    TYPEINFO();

    SdPage(SdDrawDocument& rNewDoc, StarBASIC* pBasic, BOOL bMasterPage=FALSE);
    SdPage(const SdPage& rSrcPage);
    ~SdPage();
     virtual SdrPage* Clone() const;

    virtual void	SetSize(const Size& aSize);
    virtual void	SetBorder(INT32 nLft, INT32 nUpp, INT32 nRgt, INT32 Lwr);
    virtual void	SetLftBorder(INT32 nBorder);
    virtual void	SetRgtBorder(INT32 nBorder);
    virtual void	SetUppBorder(INT32 nBorder);
    virtual void	SetLwrBorder(INT32 nBorder);
    virtual void	SetModel(SdrModel* pNewModel);

    List*           GetPresObjList() { return &aPresObjList; }
    SdrObject*      CreatePresObj(PresObjKind eObjKind, BOOL bVertical, const Rectangle& rRect, BOOL bInsert=FALSE);
    SdrObject*		GetPresObj(PresObjKind eObjKind, USHORT nIndex = 1);
    PresObjKind     GetPresObjKind(SdrObject* pObj);
    String          GetPresObjText(PresObjKind eObjKind);
    SfxStyleSheet*	GetStyleSheetForPresObj(PresObjKind eObjKind);
    void			SetAutoLayout(AutoLayout eLayout, BOOL bInit=FALSE, BOOL bAPICall=FALSE);
    AutoLayout		GetAutoLayout() const { return eAutoLayout; }
    void            CreateTitleAndLayout(BOOL bInit=FALSE, BOOL bAPICall=FALSE);

    virtual void       NbcInsertObject(SdrObject* pObj, ULONG nPos=CONTAINER_APPEND,
                                       const SdrInsertReason* pReason=NULL);
    virtual SdrObject* RemoveObject(ULONG nObjNum);

    // #95876# Also overload ReplaceObject methods to realize when
    // objects are removed with this mechanism instead of RemoveObject


    void		SetObjText(SdrTextObj* pObj, SdrOutliner* pOutliner, PresObjKind eObjKind, const String& rStr );

    void		SetPageKind(PageKind ePgType)		 { ePageKind = ePgType; }
    PageKind	GetPageKind() const 				 { return ePageKind; }

    void		SetSelected(BOOL bSel)				 { bSelected = bSel; }
    BOOL		IsSelected() const					 { return bSelected; }

    void		SetFadeSpeed(FadeSpeed eNewSpeed)	 { eFadeSpeed = eNewSpeed; }
    FadeSpeed	GetFadeSpeed() const				 { return eFadeSpeed; }

    void		SetFadeEffect(::com::sun::star::presentation::FadeEffect eNewEffect) { eFadeEffect = eNewEffect; }
    ::com::sun::star::presentation::FadeEffect	GetFadeEffect() const				 { return eFadeEffect; }

    void		SetPresChange(PresChange eChange)	 { ePresChange = eChange; }
    PresChange	GetPresChange() const				 { return ePresChange; }

    void		SetTime(UINT32 nNewTime)			 { nTime = nNewTime; }
    UINT32		GetTime() const 					 { return nTime; }

    void		SetSound(BOOL bNewSoundOn)			 { bSoundOn = bNewSoundOn; }
    BOOL		IsSoundOn() const					 { return bSoundOn; }

    void		SetExcluded(BOOL bNewExcluded)		{ bExcluded = bNewExcluded; }
    BOOL		IsExcluded() const					{ return bExcluded; }

    BOOL		IsScaleObjects() const				{ return bScaleObjects; }

    void		SetSoundFile(const String& rStr)	{ aSoundFile = rStr; }
    String		GetSoundFile() const				{ return aSoundFile; }

#ifndef SVX_LIGHT
    // Virtuelle Methoden von SdrObjUserCall
    virtual void Changed(const SdrObject& rObj, SdrUserCallType eType,
                         const Rectangle& rOldBoundRect);


    virtual void WriteData(SvStream& rOut) const;
#endif
    virtual void ReadData(const SdrIOHeader& rHead, SvStream& rIn);

    void			SetLayoutName(String aName);
    virtual String	GetLayoutName() const		{ return aLayoutName; }

    void			SetFileName(const String& aName) { aFileName = aName; }
    virtual String	GetFileName() const 	  { return aFileName; }
    void			SetBookmarkName(const String& aName) { aBookmarkName = aName; }
    virtual String	GetBookmarkName() const 	  { return aBookmarkName; }

    void    ScaleObjects(const Size& rNewPageSize, const Rectangle& rNewBorderRect, BOOL bScaleAllObj);

    const String&   GetName();
    String			GetRealName() const { return aPageName; };

    void	EndListenOutlineText();

    void	SetBackgroundFullSize( BOOL bIn );
    BOOL	IsBackgroundFullSize() const { return bBackgroundFullSize; }

    rtl_TextEncoding GetCharSet() { return(eCharSet); }

    void    SetPaperBin(USHORT nBin) { nPaperBin = nBin; }
    USHORT  GetPaperBin() const { return nPaperBin; }
    virtual void        SetOrientation(Orientation eOrient);
    virtual Orientation GetOrientation() const;

    virtual SfxStyleSheet* GetTextStyleSheetForObject( SdrObject* pObj ) const;

#ifndef SVX_LIGHT
    sal_Bool setAlienAttributes( const ::com::sun::star::uno::Any& rAttributes );
    void getAlienAttributes( ::com::sun::star::uno::Any& rAttributes );
#endif
};

} //namespace binfilter
#endif	   // _SDPAGE_HXX

