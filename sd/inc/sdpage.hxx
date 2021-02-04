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

#ifndef INCLUDED_SD_INC_SDPAGE_HXX
#define INCLUDED_SD_INC_SDPAGE_HXX

#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/document/XLinkTargetSupplier.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/drawing/XShapeBinder.hpp>
#include <com/sun/star/drawing/XShapeCombiner.hpp>
#include <com/sun/star/office/XAnnotation.hpp>
#include <com/sun/star/office/XAnnotationAccess.hpp>
#include <com/sun/star/presentation/FadeEffect.hpp>
#include <com/sun/star/presentation/XPresentationPage.hpp>

#include <memory>
#include <string_view>
#include <vector>
#include <editeng/flditem.hxx>
#include <svx/svdobj.hxx>
#include <svx/fmpage.hxx>
#include <xmloff/autolayout.hxx>
#include "diadef.h"
#include "pres.hxx"
#include "shapelist.hxx"
#include "misc/scopelock.hxx"
#include "sddllapi.h"
#include "unosrch.hxx"

namespace com::sun::star::animations {
    class XAnimationNode;
}

class SfxStyleSheet;
class SdDrawDocument;
class SdrTextObj;
class SdPageLink;
class SfxItemSet;
class Paragraph;
class Outliner;
class SdStyleSheet;
class Graphic;
class SdXImpressDocument;

namespace sd
{
    class MainSequence;
}

namespace sd {

    struct SD_DLLPUBLIC HeaderFooterSettings
    {
        bool mbHeaderVisible;
        OUString maHeaderText;

        bool mbFooterVisible;
        OUString maFooterText;

        bool mbSlideNumberVisible;

        bool mbDateTimeVisible;
        bool mbDateTimeIsFixed;
        OUString maDateTimeText;
        SvxDateFormat meDateFormat;
        SvxTimeFormat meTimeFormat;

        HeaderFooterSettings();

        bool operator==( const HeaderFooterSettings& rSettings ) const;
    };

    typedef std::vector< css::uno::Reference< css::office::XAnnotation > > AnnotationVector;
}

namespace sd {
    class UndoAnimation;
    class UndoTransition;
    class UndoGeoObject;
    class UndoAttrObject;
}

class SD_DLLPUBLIC SdPage : public FmFormPage, public SdrObjUserCall,
                          public SdUnoSearchReplaceShape,
                          public css::drawing::XShapeCombiner,
                          public css::drawing::XShapeBinder,
                          public css::container::XNamed,
                          public css::beans::XPropertySet,
                          public css::beans::XMultiPropertySet,
                          public css::animations::XAnimationNodeSupplier,
                          public css::office::XAnnotationAccess,
                          public css::document::XLinkTargetSupplier
{
    SdPage& operator=(const SdPage&) = delete;
    SdPage(const SdPage&) = delete;

friend class SdGenericDrawPage;
friend class SdDrawPage;
friend class sd::UndoAnimation;
friend class sd::UndoTransition;
friend class ModifyPageUndoAction;
friend class sd::UndoGeoObject;
friend class sd::UndoAttrObject;

    PageKind    mePageKind;               ///< page type
    AutoLayout  meAutoLayout;             ///< AutoLayout
    sd::ShapeList maPresentationShapeList;///< presentation objects
    sd::ScopeLock maLockAutoLayoutArrangement;
    bool    mbSelected;               ///< selection identifier
    PresChange  mePresChange;             ///< manual / automatic / semi automatic
    double      mfTime;                   ///< Display time in seconds
    bool    mbSoundOn;                ///< with / without sound.
    bool    mbExcluded;               ///< will (not) be displayed during show.
    OUString    maLayoutName;             ///< Name of the layout
    OUString    maSoundFile;              ///< Path to sound file (MS-DOS notation).
    bool        mbLoopSound;
    bool        mbStopSound;
    OUString    maCreatedPageName;        ///< generated page name by GetPageName.
    OUString    maFileName;               ///< file name.
    OUString    maBookmarkName;           ///< Bookmark name.
    bool    mbScaleObjects;           ///< Objects should be scaled
    bool    mbBackgroundFullSize;     ///< Background object to represent the whole page.
    rtl_TextEncoding meCharSet;           ///< Text encoding
    sal_uInt16  mnPaperBin;               ///< PaperBin
    SdPageLink* mpPageLink;               ///< Page link (at left sides only)

    sd::AnnotationVector    maAnnotations;

    /** holds the smil animation sequences for this page */
    css::uno::Reference< css::animations::XAnimationNode > mxAnimationNode;

    /** a helper class to manipulate effects inside the main sequence */
    std::shared_ptr< sd::MainSequence > mpMainSequence;

    std::unique_ptr<SfxItemSet> mpItems;

    SfxItemSet* getOrCreateItems();

    sd::HeaderFooterSettings    maHeaderFooterSettings;

    // new transition settings
    sal_Int16 mnTransitionType;
    sal_Int16 mnTransitionSubtype;
    bool mbTransitionDirection;
    sal_Int32 mnTransitionFadeColor;
    double mfTransitionDuration;

    void lateInit(const SdPage& rSrcPage);

public:

    SdPage(SdXImpressDocument* pSfxModel, bool bMasterPage, const SvxItemPropertySet* _pSet);
    virtual ~SdPage() override;

    virtual rtl::Reference<SdrPage> CloneSdrPage(SdrModel& rTargetModel) const override;

    virtual void    SetSize(const Size& aSize) override;
    virtual void    SetBorder(sal_Int32 nLft, sal_Int32 nUpp, sal_Int32 nRgt, sal_Int32 Lwr) override;
    virtual void    SetLeftBorder(sal_Int32 nBorder) override;
    virtual void    SetRightBorder(sal_Int32 nBorder) override;
    virtual void    SetUpperBorder(sal_Int32 nBorder) override;
    virtual void    SetLowerBorder(sal_Int32 nBorder) override;
    virtual bool    IsReadOnly() const override;

    sd::ShapeList&  GetPresentationShapeList() { return maPresentationShapeList; }

    void EnsureMasterPageDefaultBackground();
    SdrObject*      CreatePresObj(PresObjKind eObjKind, bool bVertical, const ::tools::Rectangle& rRect);
    SdrObject*      CreateDefaultPresObj(PresObjKind eObjKind);
    void            DestroyDefaultPresObj(PresObjKind eObjKind);
    SdrObject*      GetPresObj(PresObjKind eObjKind, int nIndex = 1, bool bFuzzySearch = false );
    PresObjKind     GetPresObjKind(SdrObject* pObj) const;
    OUString        GetPresObjText(PresObjKind eObjKind) const;
    SfxStyleSheet* GetStyleSheetForMasterPageBackground() const;
    SfxStyleSheet*  GetStyleSheetForPresObj(PresObjKind eObjKind) const;
    sal_Int64       GetHashCode() const;
    bool            RestoreDefaultText( SdrObject* pObj );

    /** @return true if the given SdrObject is inside the presentation object list */
    bool            IsPresObj(const SdrObject* pObj);

    /** removes the given SdrObject from the presentation object list */
    void            RemovePresObj(const SdrObject* pObj);

    /** inserts the given SdrObject into the presentation object list */
    void            InsertPresObj(SdrObject* pObj, PresObjKind eKind );

    void            SetAutoLayout(AutoLayout eLayout, bool bInit=false, bool bCreate=false);
    AutoLayout      GetAutoLayout() const { return meAutoLayout; }
    void            CreateTitleAndLayout(bool bInit=false, bool bCreate=false);
    SdrObject*      InsertAutoLayoutShape(SdrObject* pObj, PresObjKind eObjKind, bool bVertical, const ::tools::Rectangle& rRect, bool bInit);

    virtual void       NbcInsertObject(SdrObject* pObj, size_t nPos=SAL_MAX_SIZE) override;
    virtual SdrObject* NbcRemoveObject(size_t nObjNum) override;
    virtual SdrObject* RemoveObject(size_t nObjNum) override;

    /** Also override ReplaceObject methods to realize when
    objects are removed with this mechanism instead of RemoveObject*/
    virtual SdrObject* ReplaceObject(SdrObject* pNewObj, size_t nObjNum) override;

    void        SetObjText(SdrTextObj* pObj, SdrOutliner* pOutliner, PresObjKind eObjKind, std::u16string_view rStr );

    void        SetPageKind(PageKind ePgType)        { mePageKind = ePgType; }
    PageKind    GetPageKind() const                  { return mePageKind; }

    void        SetSelected(bool bSel)               { mbSelected = bSel; }
    bool        IsSelected() const                   { return mbSelected; }

    void        SetFadeEffect(css::presentation::FadeEffect eNewEffect);
    css::presentation::FadeEffect  GetFadeEffect() const;

    void        SetPresChange(PresChange eChange)    { mePresChange = eChange; }
    PresChange  GetPresChange() const                { return mePresChange; }

    void        SetTime(double fNewTime)             { mfTime = fNewTime; }
    double      GetTime() const                      { return mfTime; }

    void        SetSound(bool bNewSoundOn)           { mbSoundOn = bNewSoundOn; }
    bool        IsSoundOn() const                    { return mbSoundOn; }

    void        SetExcluded(bool bNewExcluded)      { mbExcluded = bNewExcluded; }
    bool        IsExcluded() const                  { return mbExcluded; }

    bool        IsScaleObjects() const              { return mbScaleObjects; }

    void        SetSoundFile(const OUString& rStr)    { maSoundFile = rStr; }
    const OUString& GetSoundFile() const                { return maSoundFile; }

    void        SetLoopSound( bool bLoopSound ) { mbLoopSound = bLoopSound; }
    bool        IsLoopSound() const                 { return mbLoopSound; }

    void        SetStopSound( bool bStopSound ) { mbStopSound = bStopSound; }
    bool        IsStopSound() const             { return mbStopSound; }

    sal_Int16   getTransitionType() const { return mnTransitionType;}
    void        setTransitionType( sal_Int16 nTransitionType );

    sal_Int16   getTransitionSubtype() const { return mnTransitionSubtype;}
    void        setTransitionSubtype( sal_Int16 nTransitionSubtype );

    bool    getTransitionDirection() const { return mbTransitionDirection;}
    void        setTransitionDirection( bool bTransitionbDirection );

    sal_Int32   getTransitionFadeColor() const { return mnTransitionFadeColor;}
    void        setTransitionFadeColor( sal_Int32 nTransitionFadeColor );

    double      getTransitionDuration() const { return mfTransitionDuration;}
    void        setTransitionDuration( double fTransitionDuration );

    virtual void Changed(const SdrObject& rObj, SdrUserCallType eType,
                         const ::tools::Rectangle& rOldBoundRect) override;

    void             SetLayoutName(const OUString& aName);
    virtual OUString GetLayoutName() const override       { return maLayoutName; }

    void            SetFileName(const OUString& aName) { maFileName = aName; }
    const OUString& GetFileName() const       { return maFileName; }
    void            SetBookmarkName(const OUString& aName) { maBookmarkName = aName; }
    const OUString& GetBookmarkName() const       { return maBookmarkName; }

    void            ConnectLink();
    void            DisconnectLink();

    void            ScaleObjects(const Size& rNewPageSize, const ::tools::Rectangle& rNewBorderRect,
                         bool bScaleAllObj);

    const OUString& GetName() const;
    OUString const & GetRealName() const { return FmFormPage::GetName(); };

    void            SetPresentationLayout(std::u16string_view rLayoutName,
                                  bool bReplaceStyleSheets = true,
                                  bool bSetMasterPage = true,
                                  bool bReverseOrder = false);
    void            EndListenOutlineText();

    void    SetBackgroundFullSize( bool bIn );
    bool    IsBackgroundFullSize() const { return mbBackgroundFullSize; }

    void    SetPaperBin(sal_uInt16 nBin) { mnPaperBin = nBin; }
    sal_uInt16  GetPaperBin() const { return mnPaperBin; }
    virtual void        SetOrientation(Orientation eOrient) override;
    virtual Orientation GetOrientation() const override;

    virtual SfxStyleSheet* GetTextStyleSheetForObject( SdrObject* pObj ) const override;

    bool setAlienAttributes( const css::uno::Any& rAttributes );
    void getAlienAttributes( css::uno::Any& rAttributes );

    // XAnimationNodeSupplier
    /** @return the main animation node

        @throws css::uno::RuntimeException
    */
    virtual css::uno::Reference< css::animations::XAnimationNode > SAL_CALL getAnimationNode() override;

    /** sets the main animation node

        @throws css::uno::RuntimeException
    */
    void setAnimationNode( css::uno::Reference< css::animations::XAnimationNode > const & xNode );

    /// @return a helper class to manipulate effects inside the main sequence
    std::shared_ptr< sd::MainSequence > const & getMainSequence();

    /** quick check if this slide has an animation node.
        This can be used to have a cost free check if there are no animations ad this slide.
        If it returns true this does not mean that there are animations available.
    */
    bool hasAnimationNode() const;

    /// @return the SdPage implementation for the given XDrawPage or 0 if not available
    static SdPage* getImplementation( const css::uno::Reference< css::drawing::XDrawPage >& xPage );

    /** removes all custom animations for the given shape */
    void removeAnimations( const SdrObject* pObj );

    /** Notify that the object has been renamed and the animation effects has to update. */
    void notifyObjectRenamed(const SdrObject* pObj);

    /** Set the name of the page and broadcast a model change.
    */
    void SetName (const OUString& rName);

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
        bool bEdit ) override;

    /** callback from the sd::View when a new paragraph for one object on this page is created */
    void onParagraphInserted( ::Outliner* pOutliner, Paragraph const * pPara, SdrObject* pObj );

    /** callback from the sd::View when a paragraph from one object on this page is removed */
    void onParagraphRemoving( ::Outliner* pOutliner, Paragraph const * pPara, SdrObject* pObj );

    /** callback from the sd::View when an object just left text edit mode */
    void onEndTextEdit( SdrObject* pObj );

    /** @return the presentation style with the given helpid from this masterpage or this
        slides masterpage */
    SdStyleSheet* getPresentationStyle( sal_uInt32 nHelpId ) const;

    /** removes all empty presentation objects from this slide */
    void RemoveEmptyPresentationObjects();

    ::tools::Rectangle   GetTitleRect() const;
    ::tools::Rectangle   GetLayoutRect() const;

    static void CalculateHandoutAreas( SdDrawDocument& rModel, AutoLayout eLayout, bool bHorizontal, std::vector< ::tools::Rectangle >& rAreas );

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
    bool IsPrecious() const { return mbIsPrecious; }

    void createAnnotation( css::uno::Reference< css::office::XAnnotation >& xAnnotation );
    void addAnnotation( const css::uno::Reference< css::office::XAnnotation >& xAnnotation, int nIndex );
    // XAnnotationAccess
    virtual void SAL_CALL removeAnnotation( const css::uno::Reference< css::office::XAnnotation >& xAnnotation ) override;
    const sd::AnnotationVector& getAnnotations() const { return maAnnotations; }
    bool Equals(const SdPage&) const;
    virtual void dumpAsXml(xmlTextWriterPtr pWriter) const override;
    sal_uInt16 getPageId() const { return mnPageId; }

    /**
     Returns graphics objects from the page that can be prefetched before it's painted.
     The pointers are temporary and should not be kept.
    */
    void getGraphicsForPrefetch(std::vector<Graphic*>& graphics) const;

    static sal_uInt16 mnLastPageId;

private:
    bool mbIsPrecious;

    // page id of this page
    sal_uInt16 mnPageId;

    /** clone the animations from this and set them to rTargetPage
     *  TTTT: Order is strange, should be the other way around by
     *  convention/convenience and makes usage a little dangerous...
    */
    void cloneAnimations( SdPage& rTargetPage ) const;

    /** called before a shape is removed or replaced from this slide */
    void onRemoveObject( SdrObject* pObject );

private:
    SdXImpressDocument* mpDocModel;
    bool      mbIsImpressDocument;
    sal_Int16 mnTempPageNumber; // for printing handouts

protected:
    friend class SdXImpressDocument;

    const SvxItemPropertySet*   mpPropSet;

    /// @throws css::lang::IllegalArgumentException
    virtual void setBackground( const css::uno::Any& rValue );
    /// @throws std::exception
    virtual void getBackground( css::uno::Any& rValue );

    OUString getBookmarkURL() const;
    void setBookmarkURL( OUString const & rURL );

    void SetWidth( sal_Int32 nWidth );
    void SetHeight( sal_Int32 nHeight );

    bool IsImpressDocument() const;

    virtual void disposing() throw() override;

    css::uno::Any getNavigationOrder();
    void setNavigationOrder( const css::uno::Any& rValue );

    /// @throws css::uno::RuntimeException
    void throwIfDisposed() const;

public:

    SdXImpressDocument* GetModel() const;

    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw();
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;

    // this is called whenever a SdrObject must be created for an empty api shape wrapper
    virtual SdrObject *CreateSdrObject_( const css::uno::Reference< css::drawing::XShape >& xShape ) override;

    // SvxFmDrawPage
    virtual css::uno::Reference<css::drawing::XShape>  CreateShape(SdrObject *pObj) const override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire() throw() override { FmFormPage::acquire(); }
    virtual void SAL_CALL release() throw() override { FmFormPage::release(); }

    // XShapeCombiner
    virtual css::uno::Reference< css::drawing::XShape > SAL_CALL combine( const css::uno::Reference< css::drawing::XShapes >& xShapes ) override;
    virtual void SAL_CALL split( const css::uno::Reference< css::drawing::XShape >& xGroup ) override;

    // XShapeBinder
    virtual css::uno::Reference< css::drawing::XShape > SAL_CALL bind( const css::uno::Reference< css::drawing::XShapes >& xShapes ) override;
    virtual void SAL_CALL unbind( const css::uno::Reference< css::drawing::XShape >& xShape ) override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    // XMultiPropertySet
    virtual void SAL_CALL setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& aValues ) override;
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL getPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames ) override;
    virtual void SAL_CALL addPropertiesChangeListener( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertiesChangeListener( const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
    virtual void SAL_CALL firePropertiesChangeEvent( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;

    // XLinkTargetSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getLinks(  ) override;

    // XServiceInfo
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XAnnotationAccess:
    virtual css::uno::Reference< css::office::XAnnotation > SAL_CALL createAndInsertAnnotation() override;
    virtual css::uno::Reference< css::office::XAnnotationEnumeration > SAL_CALL createAnnotationEnumeration() override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/

class SdNotMasterPage final : public css::drawing::XMasterPageTarget,
                   public css::presentation::XPresentationPage,
                   public SdPage
{
private:
    css::uno::Sequence< css::uno::Type > maTypeSequence;

    virtual void setBackground( const css::uno::Any& rValue ) override;
    virtual void getBackground( css::uno::Any& rValue ) override;
public:
    SdNotMasterPage(SdXImpressDocument* pSfxModel);
    virtual ~SdNotMasterPage() throw() override;

    UNO3_GETIMPLEMENTATION_DECL( SdDrawPage )

    static OUString getPageApiName( SdPage const * pPage );
    static OUString getPageApiNameFromUiName( const OUString& rUIName );
    static OUString getUiNameFromPageApiName( const OUString& rApiName );

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire() throw() override { SdPage::acquire(); }
    virtual void SAL_CALL release() throw() override { SdPage::release(); }

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XMasterPageTarget
    virtual css::uno::Reference< css::drawing::XDrawPage > SAL_CALL getMasterPage(  ) override;
    virtual void SAL_CALL setMasterPage( const css::uno::Reference< css::drawing::XDrawPage >& xMasterPage ) override;

    // XPresentationPage
    virtual css::uno::Reference< css::drawing::XDrawPage > SAL_CALL getNotesPage(  ) override;

    // XNamed
    virtual OUString SAL_CALL getName(  ) override;
    virtual void SAL_CALL setName( const OUString& aName ) override;

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override ;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // XShapes
    virtual void SAL_CALL add( const css::uno::Reference< css::drawing::XShape >& xShape ) override;
    virtual void SAL_CALL remove( const css::uno::Reference< css::drawing::XShape >& xShape ) override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/

class SdMasterPage final : public css::presentation::XPresentationPage,
                     public SdPage
{
private:
    css::uno::Sequence< css::uno::Type > maTypeSequence;

    virtual void setBackground( const css::uno::Any& rValue ) override;
    virtual void getBackground( css::uno::Any& rValue ) override;

public:
    SdMasterPage(SdXImpressDocument* pSfxModel);
    virtual ~SdMasterPage() throw() override;

    UNO3_GETIMPLEMENTATION_DECL(SdMasterPage)

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire() throw() override { SdPage::acquire(); }
    virtual void SAL_CALL release() throw() override { SdPage::release(); }

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override ;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // XPresentationPage
    virtual css::uno::Reference< css::drawing::XDrawPage > SAL_CALL getNotesPage(  ) override;

    // XNamed
    virtual OUString SAL_CALL getName(  ) override;
    virtual void SAL_CALL setName( const OUString& aName ) override;

    // XShapes
    virtual void SAL_CALL add( const css::uno::Reference< css::drawing::XShape >& xShape ) override;
    virtual void SAL_CALL remove( const css::uno::Reference< css::drawing::XShape >& xShape ) override;
};

#endif // INCLUDED_SD_INC_SDPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
