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

#ifndef _SDPAGE_HXX
#define _SDPAGE_HXX

#include <boost/shared_ptr.hpp>

#include <functional>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/presentation/FadeEffect.hpp>
#include <com/sun/star/office/XAnnotation.hpp>

#include <list>
#include <vector>
#include <svx/svdobj.hxx>
#include <svx/fmpage.hxx>
#include "fadedef.h"
#include "diadef.h"
#ifndef _PRESENTATION
#include "pres.hxx"
#endif
#include "shapelist.hxx"
#include "misc/scopelock.hxx"
#include "sddllapi.h"

namespace com { namespace sun { namespace star { namespace animations {
    class XAnimationNode;
} } } }

class SfxStyleSheet;
class SdDrawDocument;
class SdrTextObj;
class SdPageLink;
class StarBASIC;
class SfxItemSet;
struct StyleRequestData;
class SdPage;
class Paragraph;
class Outliner;
class SdStyleSheet;

namespace sd
{
    class MainSequence;
}

namespace boost
{
    template<class X> class shared_ptr;
}

namespace sd {

    struct SD_DLLPUBLIC HeaderFooterSettings
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

        bool operator==( const HeaderFooterSettings& rSettings ) const;
    };

    typedef std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::office::XAnnotation > > AnnotationVector;
}

namespace sd {
    class UndoAnimation;
    class UndoTransition;
    class UndoGeoObject;
    class UndoAttrObject;
}

class SD_DLLPUBLIC SdPage : public FmFormPage, public SdrObjUserCall
{
friend class SdGenericDrawPage;
friend class SdDrawPage;
friend class sd::UndoAnimation;
friend class sd::UndoTransition;
friend class ModifyPageUndoAction;
friend class sd::UndoGeoObject;
friend class sd::UndoAttrObject;

protected:
    PageKind    mePageKind;               ///< page type
    AutoLayout  meAutoLayout;             ///< AutoLayout
    sd::ShapeList maPresentationShapeList;///< presentation objects
    sd::ScopeLock maLockAutoLayoutArrangement;
    sal_Bool    mbSelected;               ///< selection identifier
    PresChange  mePresChange;             ///< manual / automatic / semi automatic
    sal_uInt32  mnTime;                   ///< Display time in seconds
    sal_Bool    mbSoundOn;                ///< with / without sound.
    sal_Bool    mbExcluded;               ///< will (not) be displayed during show.
    String      maLayoutName;             ///< Name of the layout
    String      maSoundFile;              ///< Path to sound file (MSDOS notation).
    bool        mbLoopSound;
    bool        mbStopSound;
    String      maCreatedPageName;        ///< generated page name by GetPageName.
    String      maFileName;               ///< file name.
    String      maBookmarkName;           ///< Bookmark name.
    sal_Bool    mbScaleObjects;           ///< Objects should be scaled
    sal_Bool    mbBackgroundFullSize;     ///< Background object to represent the whole page.
    rtl_TextEncoding meCharSet;           ///< Text encoding
    sal_uInt16  mnPaperBin;               ///< PaperBin
    Orientation meOrientation;            ///< Print orientation.
    SdPageLink* mpPageLink;               ///< Page link (at left sides only)

    sd::AnnotationVector    maAnnotations;

    /** holds the smil animation sequences for this page */
    ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > mxAnimationNode;

    /** a helper class to manipulate effects inside the main sequence */
    boost::shared_ptr< sd::MainSequence > mpMainSequence;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createUnoPage();

    SfxItemSet* mpItems;

    SfxItemSet* getOrCreateItems();

    sd::HeaderFooterSettings    maHeaderFooterSettings;

    // new transition settings
    sal_Int16 mnTransitionType;
    sal_Int16 mnTransitionSubtype;
    sal_Bool mbTransitionDirection;
    sal_Int32 mnTransitionFadeColor;
    double mfTransitionDuration;

public:
    TYPEINFO();

    SdPage(SdDrawDocument& rNewDoc, StarBASIC* pBasic, sal_Bool bMasterPage=sal_False);
    SdPage(const SdPage& rSrcPage);
    ~SdPage();
    virtual SdrPage* Clone() const;
    virtual SdrPage* Clone(SdrModel* pNewModel) const;

    virtual void    SetSize(const Size& aSize);
    virtual void    SetBorder(sal_Int32 nLft, sal_Int32 nUpp, sal_Int32 nRgt, sal_Int32 Lwr);
    virtual void    SetLftBorder(sal_Int32 nBorder);
    virtual void    SetRgtBorder(sal_Int32 nBorder);
    virtual void    SetUppBorder(sal_Int32 nBorder);
    virtual void    SetLwrBorder(sal_Int32 nBorder);
    virtual void    SetModel(SdrModel* pNewModel);
    virtual bool    IsReadOnly() const;

    sd::ShapeList&  GetPresentationShapeList() { return maPresentationShapeList; }

    void EnsureMasterPageDefaultBackground();
    SdrObject*      CreatePresObj(PresObjKind eObjKind, sal_Bool bVertical, const Rectangle& rRect, sal_Bool bInsert=sal_False);
    SdrObject*      CreateDefaultPresObj(PresObjKind eObjKind, bool bInsert);
    SdrObject*      GetPresObj(PresObjKind eObjKind, int nIndex = 1, bool bFuzzySearch = false );
    PresObjKind     GetPresObjKind(SdrObject* pObj) const;
    String          GetPresObjText(PresObjKind eObjKind) const;
    SfxStyleSheet* GetStyleSheetForMasterPageBackground() const;
    SfxStyleSheet*  GetStyleSheetForPresObj(PresObjKind eObjKind) const;
    bool            RestoreDefaultText( SdrObject* pObj );

    /** @return true if the given SdrObject is inside the presentation object list */
    bool            IsPresObj(const SdrObject* pObj);

    /** removes the given SdrObject from the presentation object list */
    void            RemovePresObj(const SdrObject* pObj);

    /** inserts the given SdrObject into the presentation object list */
    void            InsertPresObj(SdrObject* pObj, PresObjKind eKind );

    void            SetAutoLayout(AutoLayout eLayout, sal_Bool bInit=sal_False, sal_Bool bCreate=sal_False);
    AutoLayout      GetAutoLayout() const { return meAutoLayout; }
    void            CreateTitleAndLayout(sal_Bool bInit=sal_False, sal_Bool bCreate=sal_False);
    SdrObject*      InsertAutoLayoutShape(SdrObject* pObj, PresObjKind eObjKind, bool bVertical, Rectangle aRect, bool bInit );

    virtual void       NbcInsertObject(SdrObject* pObj, sal_uLong nPos=CONTAINER_APPEND,
                                       const SdrInsertReason* pReason=NULL);
    virtual SdrObject* NbcRemoveObject(sal_uLong nObjNum);
    virtual SdrObject* RemoveObject(sal_uLong nObjNum);

    /** Also overload ReplaceObject methods to realize when
    objects are removed with this mechanism instead of RemoveObject*/
    virtual SdrObject* NbcReplaceObject(SdrObject* pNewObj, sal_uLong nObjNum);
    virtual SdrObject* ReplaceObject(SdrObject* pNewObj, sal_uLong nObjNum);

    virtual void SetLinkData(const String& rLinkName, const String& rLinkData);

    void        SetObjText(SdrTextObj* pObj, SdrOutliner* pOutliner, PresObjKind eObjKind, const String& rStr );

    void        SetPageKind(PageKind ePgType)        { mePageKind = ePgType; }
    PageKind    GetPageKind() const                  { return mePageKind; }

    void        SetSelected(sal_Bool bSel)               { mbSelected = bSel; }
    sal_Bool        IsSelected() const                   { return mbSelected; }

    void        SetFadeEffect(::com::sun::star::presentation::FadeEffect eNewEffect);
    ::com::sun::star::presentation::FadeEffect  GetFadeEffect() const;

    void        SetPresChange(PresChange eChange)    { mePresChange = eChange; }
    PresChange  GetPresChange() const                { return mePresChange; }

    void        SetTime(sal_uInt32 nNewTime)             { mnTime = nNewTime; }
    sal_uInt32      GetTime() const                      { return mnTime; }

    void        SetSound(sal_Bool bNewSoundOn)           { mbSoundOn = bNewSoundOn; }
    sal_Bool        IsSoundOn() const                    { return mbSoundOn; }

    void        SetExcluded(sal_Bool bNewExcluded)      { mbExcluded = bNewExcluded; }
    sal_Bool        IsExcluded() const                  { return mbExcluded; }

    void        SetScaleObjects(sal_Bool bScale)        { mbScaleObjects = bScale; }
    sal_Bool        IsScaleObjects() const              { return mbScaleObjects; }

    void        SetSoundFile(const String& rStr)    { maSoundFile = rStr; }
    String      GetSoundFile() const                { return maSoundFile; }

    void        SetLoopSound( bool bLoopSound ) { mbLoopSound = bLoopSound; }
    bool        IsLoopSound() const                 { return mbLoopSound; }

    void        SetStopSound( bool bStopSound ) { mbStopSound = bStopSound; }
    bool        IsStopSound() const             { return mbStopSound; }

    sal_Int16   getTransitionType() const;
    void        setTransitionType( sal_Int16 nTransitionType );

    sal_Int16   getTransitionSubtype() const;
    void        setTransitionSubtype( sal_Int16 nTransitionSubtype );

    sal_Bool    getTransitionDirection() const;
    void        setTransitionDirection( sal_Bool bTransitionbDirection );

    sal_Int32   getTransitionFadeColor() const;
    void        setTransitionFadeColor( sal_Int32 nTransitionFadeColor );

    double      getTransitionDuration() const;
    void        setTransitionDuration( double fTranstionDuration );

    virtual void Changed(const SdrObject& rObj, SdrUserCallType eType,
                         const Rectangle& rOldBoundRect);

    void            SetLayoutName(String aName);
    virtual String  GetLayoutName() const       { return maLayoutName; }

    void            SetFileName(const String& aName) { maFileName = aName; }
    virtual String  GetFileName() const       { return maFileName; }
    void            SetBookmarkName(const String& aName) { maBookmarkName = aName; }
    virtual String  GetBookmarkName() const       { return maBookmarkName; }
    SdPageLink*     GetLink() { return mpPageLink; }

    void            ConnectLink();
    void            DisconnectLink();

    void    ScaleObjects(const Size& rNewPageSize, const Rectangle& rNewBorderRect,
                         sal_Bool bScaleAllObj);

    const String&   GetName() const;
    String          GetRealName() const { return FmFormPage::GetName(); };

    void    SetPresentationLayout(const String& rLayoutName,
                                  sal_Bool bReplaceStyleSheets = sal_True,
                                  sal_Bool bSetMasterPage = sal_True,
                                  sal_Bool bReverseOrder = sal_False);
    void    EndListenOutlineText();

    void    SetBackgroundFullSize( sal_Bool bIn );
    sal_Bool    IsBackgroundFullSize() const { return mbBackgroundFullSize; }

    rtl_TextEncoding GetCharSet() { return(meCharSet); }

    void    SetPaperBin(sal_uInt16 nBin) { mnPaperBin = nBin; }
    sal_uInt16  GetPaperBin() const { return mnPaperBin; }
    virtual void        SetOrientation(Orientation eOrient);
    virtual Orientation GetOrientation() const;

    virtual SfxStyleSheet* GetTextStyleSheetForObject( SdrObject* pObj ) const;

    sal_Bool setAlienAttributes( const com::sun::star::uno::Any& rAttributes );
    void getAlienAttributes( com::sun::star::uno::Any& rAttributes );

    /** @return the main animation node */
    ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > getAnimationNode() throw (::com::sun::star::uno::RuntimeException);

    /** sets the main animation node */
    void setAnimationNode( ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode ) throw (::com::sun::star::uno::RuntimeException);

    /// @return a helper class to manipulate effects inside the main sequence
    boost::shared_ptr< sd::MainSequence > getMainSequence();

    /** quick check if this slide has an animation node.
        This can be used to have a cost free check if there are no animations ad this slide.
        If it returns true this does not mean that there are animations available.
    */
    bool hasAnimationNode() const;

    /// @return the SdPage implementation for the given XDrawPage or 0 if not available
    static SdPage* getImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xPage );

    /** removes all custom animations for the given shape */
    void removeAnimations( const SdrObject* pObj );

    /** Set the name of the page and broadcast a model change.
    */
    virtual void SetName (const String& rName);

    const sd::HeaderFooterSettings& getHeaderFooterSettings() const;
    void setHeaderFooterSettings( const sd::HeaderFooterSettings& rNewSettings );

    /**
        @return true if the object from the ViewObjectContact should
        be visible on this page while rendering.

        bEdit selects if visibility test is for an editing view or a final render,
        like printing.
    */
    virtual bool checkVisibility(
        const sdr::contact::ViewObjectContact& rOriginal,
        const sdr::contact::DisplayInfo& rDisplayInfo,
        bool bEdit );

    /** callback from the sd::View when a new paragraph for one object on this page is created */
    void onParagraphInserted( ::Outliner* pOutliner, Paragraph* pPara, SdrObject* pObj );

    /** callback from the sd::View when a paragraph from one object on this page is removed */
    void onParagraphRemoving( ::Outliner* pOutliner, Paragraph* pPara, SdrObject* pObj );

    /** callback from the sd::View when an object just left text edit mode */
    void onEndTextEdit( SdrObject* pObj );

    /** @return the presentation style with the given helpid from this masterpage or this
        slides masterpage */
    SdStyleSheet* getPresentationStyle( sal_uInt32 nHelpId ) const;

    /** removes all empty presentation objects from this slide */
    void RemoveEmptyPresentationObjects();

    Rectangle   GetTitleRect() const;
    Rectangle   GetLayoutRect() const;

    static void CalculateHandoutAreas( SdDrawDocument& rModel, AutoLayout eLayout, bool bHorizontal, std::vector< Rectangle >& rAreas );

    /** Set the "precious" flag to the given value.
    */
    void SetPrecious (const bool bIsPrecious);

    /** The "precious" flag is used for master pages to prevent some unused
        master pages from being deleted automatically.  For pages
        other than master pages this flag can be ignored.
        @return
            When this method returns <TRUE/> for a master page then this
            master page should not be deleted automatically.
    */
    bool IsPrecious (void) const;

    void createAnnotation( ::com::sun::star::uno::Reference< ::com::sun::star::office::XAnnotation >& xAnnotation );
    void addAnnotation( const ::com::sun::star::uno::Reference< ::com::sun::star::office::XAnnotation >& xAnnotation, int nIndex = -1 );
    void removeAnnotation( const ::com::sun::star::uno::Reference< ::com::sun::star::office::XAnnotation >& xAnnotation );
    const sd::AnnotationVector& getAnnotations() const { return maAnnotations; }
    bool hasAnnotations() const { return !maAnnotations.empty(); }

private:
    bool mbIsPrecious;

    /** clone the animations from this and set them to rTargetPage
    */
    void cloneAnimations( SdPage& rTargetPage ) const;

    /** called before a shape is removed or replaced from this slide */
    void onRemoveObject( SdrObject* pObject );
};

#endif     // _SDPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
