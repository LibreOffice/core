/*************************************************************************
 *
 *  $RCSfile: sdpage.hxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 14:54:46 $
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

#ifndef _SDPAGE_HXX
#define _SDPAGE_HXX

#include <list>
#include <functional>

#include <com/sun/star/uno/XInterface.hpp>
#ifndef _COM_SUN_STAR_PRESENTATION_FADEEFFECT_HPP_
#include <com/sun/star/presentation/FadeEffect.hpp>
#endif

#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif
#ifndef _FM_FMPAGE_HXX //autogen
#include <svx/fmpage.hxx>
#endif
#ifndef _SD_FADEDEF_H
#include "fadedef.h"
#endif
#ifndef _SD_DIADEF_H
#include <diadef.h>
#endif
#ifndef _PRESENTATION
#include "pres.hxx"
#endif

class SfxStyleSheet;
class SdDrawDocument;
class SdrTextObj;
class SdPageLink;
class StarBASIC;
class SfxItemSet;
struct StyleRequestData;
class SdPage;

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
    PRESOBJ_NOTES,
    PRESOBJ_HEADER,
    PRESOBJ_FOOTER,
    PRESOBJ_DATETIME,
    PRESOBJ_SLIDENUMBER
};

namespace sd {

    struct PresentationObjectDescriptor
    {
        SdrObject*  mpObject;
        PresObjKind meKind;

        PresentationObjectDescriptor( SdrObject* pObject, PresObjKind eKind ) : mpObject( pObject ), meKind( eKind ) {}

        bool operator==( const PresentationObjectDescriptor& r )
        {
            return (mpObject == r.mpObject) && (meKind == r.meKind);
        }
    };

    typedef std::list< PresentationObjectDescriptor > PresentationObjectList;

    /** this unary_function can be used with stl algorithms to find presentation objects of the same kind */
    struct isPresObjKind_func : public std::unary_function<PresentationObjectDescriptor, bool>
    {
        isPresObjKind_func( PresObjKind eKind ) : meKind( eKind ) {}
        bool operator() (PresentationObjectDescriptor x) { return x.meKind == meKind; }
        PresObjKind meKind;
    };

    /** this unary_function can be used with stl algorithms to remove presentation objects from its page */
    struct removePresObj_func : public std::unary_function< PresentationObjectDescriptor, void >
    {
        removePresObj_func( SdPage* pPage ) : mpPage( pPage ) {}
        void operator() (PresentationObjectDescriptor x);
        SdPage* mpPage;
    };

    struct HeaderFooterSettings
    {
        bool mbHeaderVisible;
        String maHeaderText;

        bool mbFooterVisible;
        String maFooterText;

        bool mbSlideNumberVisible;

        bool mbDateTimeVisible;
        bool mbDateTimeIsFixed;
        String maDateTimeText;
        int meDateTimeFormat;

        HeaderFooterSettings();

        bool operator==( const HeaderFooterSettings& rSettings );
    };
};

class SdPage : public FmFormPage, public SdrObjUserCall
{
friend class SdGenericDrawPage;

protected:
    PageKind    ePageKind;                // Seitentyp
    AutoLayout  eAutoLayout;              // AutoLayout
    sd::PresentationObjectList maPresObjList;             // Praesentationsobjekte
    BOOL        bOwnArrangement;          // Objekte werden intern angeordnet

    BOOL        bSelected;                // Selektionskennung
    FadeSpeed   eFadeSpeed;               // Ueberblendgeschwindigkeit
    ::com::sun::star::presentation::FadeEffect  eFadeEffect;              // Ueberblendeffekt
    PresChange  ePresChange;              // manuell/automatisch/halbautomatisch
    UINT32      nTime;                    // Anzeigedauer in Sekunden
    BOOL        bSoundOn;                 // mit/ohne Sound (TRUE/FALSE)
    BOOL        bExcluded;                // wird in der Show nicht/doch
                                          // angezeigt (TRUE/FALSE)
    String      aLayoutName;              // Name des Layouts
    String      aSoundFile;               // Pfad zum Soundfile (MSDOS-Notation)
    String      aCreatedPageName;         // von GetPageName erzeugter Seitenname
    String      aFileName;                // Filename
    String      aBookmarkName;            // Bookmarkname
    BOOL        bScaleObjects;            // Objekte sollen skaliert werden
    BOOL        bBackgroundFullSize;      // Hintergrundobjekt auf ganze Seite darstellen
    rtl_TextEncoding eCharSet;            // Text-Encoding
    USHORT      nPaperBin;                // PaperBin
    Orientation eOrientation;             // Print-Orientation
    SdPageLink* pPageLink;                // PageLink (nur bei gelinkten Seiten)

    BOOL        InsertPresObj(SdrObject* pObj, PresObjKind eObjKind, BOOL bVertical,
                              Rectangle rRect, BOOL bInit, sd::PresentationObjectList& rObjList);

    void        AdjustBackgroundSize();
    Rectangle   GetTitleRect() const;
    Rectangle   GetLayoutRect() const;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createUnoPage();

    SfxItemSet* mpItems;

    SfxItemSet* getOrCreateItems();

    sd::HeaderFooterSettings    maHeaderFooterSettings;
public:
    TYPEINFO();

    SdPage(SdDrawDocument& rNewDoc, StarBASIC* pBasic, BOOL bMasterPage=FALSE);
    SdPage(const SdPage& rSrcPage);
    ~SdPage();
    virtual SdrPage* Clone() const;

    virtual void    SetSize(const Size& aSize);
    virtual void    SetBorder(INT32 nLft, INT32 nUpp, INT32 nRgt, INT32 Lwr);
    virtual void    SetLftBorder(INT32 nBorder);
    virtual void    SetRgtBorder(INT32 nBorder);
    virtual void    SetUppBorder(INT32 nBorder);
    virtual void    SetLwrBorder(INT32 nBorder);
    virtual void    SetModel(SdrModel* pNewModel);
    virtual FASTBOOL IsReadOnly() const;

    sd::PresentationObjectList& GetPresObjList() { return maPresObjList; }
    SdrObject*      CreatePresObj(PresObjKind eObjKind, BOOL bVertical, const Rectangle& rRect, BOOL bInsert=FALSE);
    SdrObject*      CreateDefaultPresObj(PresObjKind eObjKind, bool bInsert);
    SdrObject*      GetPresObj(PresObjKind eObjKind, int nIndex = 1);
    PresObjKind     GetPresObjKind(SdrObject* pObj);
    String          GetPresObjText(PresObjKind eObjKind);
    SfxStyleSheet*  GetStyleSheetForPresObj(PresObjKind eObjKind);

    /** returns true if the given SdrObject is inside the presentation object list */
    bool            IsPresObj(const SdrObject* pObj);

    /** removes the given SdrObject from the presentation object list */
    void            RemovePresObj(const SdrObject* pObj);

    /** inserts the given SdrObject into the presentation object list */
    void            InsertPresObj(SdrObject* pObj, PresObjKind eKind );

    sd::PresentationObjectList::iterator FindPresObj(const SdrObject* pObj);

    void            SetAutoLayout(AutoLayout eLayout, BOOL bInit=FALSE, BOOL bCreate=FALSE);
    AutoLayout      GetAutoLayout() const { return eAutoLayout; }
    void            CreateTitleAndLayout(BOOL bInit=FALSE, BOOL bCreate=FALSE);

    virtual void       NbcInsertObject(SdrObject* pObj, ULONG nPos=CONTAINER_APPEND,
                                       const SdrInsertReason* pReason=NULL);
    virtual SdrObject* NbcRemoveObject(ULONG nObjNum);
    virtual SdrObject* RemoveObject(ULONG nObjNum);

    // #95876# Also overload ReplaceObject methods to realize when
    // objects are removed with this mechanism instead of RemoveObject
    virtual SdrObject* NbcReplaceObject(SdrObject* pNewObj, ULONG nObjNum);
    virtual SdrObject* ReplaceObject(SdrObject* pNewObj, ULONG nObjNum);

    virtual void SetLinkData(const String& rLinkName, const String& rLinkData);

    void        SetObjText(SdrTextObj* pObj, SdrOutliner* pOutliner, PresObjKind eObjKind, const String& rStr );

    void        SetPageKind(PageKind ePgType)        { ePageKind = ePgType; }
    PageKind    GetPageKind() const                  { return ePageKind; }

    void        SetSelected(BOOL bSel)               { bSelected = bSel; }
    BOOL        IsSelected() const                   { return bSelected; }

    void        SetFadeSpeed(FadeSpeed eNewSpeed)    { eFadeSpeed = eNewSpeed; }
    FadeSpeed   GetFadeSpeed() const                 { return eFadeSpeed; }

    void        SetFadeEffect(::com::sun::star::presentation::FadeEffect eNewEffect) { eFadeEffect = eNewEffect; }
    ::com::sun::star::presentation::FadeEffect  GetFadeEffect() const                { return eFadeEffect; }

    void        SetPresChange(PresChange eChange)    { ePresChange = eChange; }
    PresChange  GetPresChange() const                { return ePresChange; }

    void        SetTime(UINT32 nNewTime)             { nTime = nNewTime; }
    UINT32      GetTime() const                      { return nTime; }

    void        SetSound(BOOL bNewSoundOn)           { bSoundOn = bNewSoundOn; }
    BOOL        IsSoundOn() const                    { return bSoundOn; }

    void        SetExcluded(BOOL bNewExcluded)      { bExcluded = bNewExcluded; }
    BOOL        IsExcluded() const                  { return bExcluded; }

    void        SetScaleObjects(BOOL bScale)        { bScaleObjects = bScale; }
    BOOL        IsScaleObjects() const              { return bScaleObjects; }

    void        SetSoundFile(const String& rStr)    { aSoundFile = rStr; }
    String      GetSoundFile() const                { return aSoundFile; }

#ifndef SVX_LIGHT
    // Virtuelle Methoden von SdrObjUserCall
    virtual void Changed(const SdrObject& rObj, SdrUserCallType eType,
                         const Rectangle& rOldBoundRect);

    virtual void RequestBasic();

    virtual void WriteData(SvStream& rOut) const;
#endif
    virtual void ReadData(const SdrIOHeader& rHead, SvStream& rIn);

    void            SetLayoutName(String aName);
    virtual String  GetLayoutName() const       { return aLayoutName; }

    void            SetFileName(const String& aName) { aFileName = aName; }
    virtual String  GetFileName() const       { return aFileName; }
    void            SetBookmarkName(const String& aName) { aBookmarkName = aName; }
    virtual String  GetBookmarkName() const       { return aBookmarkName; }
    SdPageLink*     GetLink() { return pPageLink; }

#ifndef SVX_LIGHT
    void            ConnectLink();
    void            DisconnectLink();

    void    ScaleObjects(const Size& rNewPageSize, const Rectangle& rNewBorderRect,
                         BOOL bScaleAllObj);
#endif

    const String&   GetName();
    String          GetRealName() const { return aPageName; };

    void    SetPresentationLayout(const String& rLayoutName,
                                  BOOL bReplaceStyleSheets = TRUE,
                                  BOOL bSetMasterPage = TRUE,
                                  BOOL bReverseOrder = FALSE);
    void    EndListenOutlineText();

    void    SetBackgroundFullSize( BOOL bIn );
    BOOL    IsBackgroundFullSize() const { return bBackgroundFullSize; }

    rtl_TextEncoding GetCharSet() { return(eCharSet); }

    void    SetPaperBin(USHORT nBin) { nPaperBin = nBin; }
    USHORT  GetPaperBin() const { return nPaperBin; }
    virtual void        SetOrientation(Orientation eOrient);
    virtual Orientation GetOrientation() const;

    virtual SfxStyleSheet* GetTextStyleSheetForObject( SdrObject* pObj ) const;

    sal_Bool setAlienAttributes( const com::sun::star::uno::Any& rAttributes );
    void getAlienAttributes( com::sun::star::uno::Any& rAttributes );

    sd::HeaderFooterSettings& getHeaderFooterSettings();

    /** this method returns true if the object from the ViewObjectContact should
        be visible on this page while rendering.
        bEdit selects if visibility test is for an editing view or a final render,
        like printing.
    */
    virtual bool checkVisibility(
        ::sdr::contact::ViewObjectContact& rOriginal,
        ::sdr::contact::DisplayInfo& rDisplayInfo,
        bool bEdit );
};

#endif     // _SDPAGE_HXX




