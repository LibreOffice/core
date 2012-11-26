/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SDPAGE_HXX
#define _SDPAGE_HXX

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

#ifndef INCLUDED_FUNCTIONAL
#include <functional>
#define INCLUDED_FUNCTIONAL
#endif
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/presentation/FadeEffect.hpp>
#include <com/sun/star/office/XAnnotation.hpp>

#include <list>
#include <functional>
#include <vector>
#include <svx/svdobj.hxx>
#ifndef _FM_FMPAGE_HXX //autogen
#include <svx/fmpage.hxx>
#endif
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

//////////////////////////////////////////////////////////////////////////////

// try to find the SdPage connected to the SdrObject
SdPage* findConnectionToSdrObject(const SdrObject* pSdrObject);

// set/reset connection between SdrObject and SdPage
SD_DLLPUBLIC void establishConnectionToSdrObject(SdrObject* pSdrObject, SdPage* pSdPage);
SD_DLLPUBLIC void resetConnectionToSdrObject(SdrObject* pSdrObject);

//////////////////////////////////////////////////////////////////////////////

class SD_DLLPUBLIC SdPage : public FmFormPage, public SfxListener
{
private:
friend class SdGenericDrawPage;
friend class SdDrawPage;
friend class sd::UndoAnimation;
friend class sd::UndoTransition;
friend class ModifyPageUndoAction;
friend class sd::UndoGeoObject;
friend class sd::UndoAttrObject;

protected:
    PageKind    mePageKind;               // Seitentyp
    AutoLayout  meAutoLayout;             // AutoLayout
    sd::ShapeList maPresentationShapeList;            // Praesentationsobjekte
    sd::ScopeLock maLockAutoLayoutArrangement;
    bool        mbSelected;               // Selektionskennung
    PresChange  mePresChange;             // manuell/automatisch/halbautomatisch
    sal_uInt32      mnTime;                   // Anzeigedauer in Sekunden
    bool        mbSoundOn;                // mit/ohne Sound (true/false)
    bool        mbExcluded;               // wird in der Show nicht/doch
                                          // angezeigt (true/false)
    String      maLayoutName;             // Name des Layouts
    String      maSoundFile;               // Pfad zum Soundfile (MSDOS-Notation)
    bool        mbLoopSound;
    bool        mbStopSound;
    String      maCreatedPageName;         // von GetPageName erzeugter Seitenname
    String      maFileName;                // Filename
    String      maBookmarkName;           // Bookmarkname
    bool        mbScaleObjects;           // Objekte sollen skaliert werden
    bool        mbBackgroundFullSize;     // Hintergrundobjekt auf ganze Seite darstellen
    rtl_TextEncoding meCharSet;            // Text-Encoding
    sal_uInt16      mnPaperBin;                // PaperBin
    Orientation meOrientation;             // Print-Orientation
    SdPageLink* mpPageLink;               // PageLink (nur bei gelinkten Seiten)

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

    /// method to copy all data from given source
    virtual void copyDataFromSdrPage(const SdrPage& rSource);

public:
    /// create a copy, evtl. with a different target model (if given)
    virtual SdrPage* CloneSdrPage(SdrModel* pTargetModel = 0) const;

    SdPage(SdDrawDocument& rNewDoc, StarBASIC* pBasic, bool bMasterPage = false);
    ~SdPage();

    virtual void SetPageScale(const basegfx::B2DVector& aNewScale);

    sd::ShapeList&  GetPresentationShapeList() { return maPresentationShapeList; }

    void EnsureMasterPageDefaultBackground();
    SdrObject*      CreatePresObj(PresObjKind eObjKind, bool bVertical, const basegfx::B2DRange& rRange, bool bInsert=false);
    SdrObject*      CreateDefaultPresObj(PresObjKind eObjKind, bool bInsert);
    SdrObject*      GetPresObj(PresObjKind eObjKind, int nIndex = 1, bool bFuzzySearch = false );
    PresObjKind     GetPresObjKind(const SdrObject* pObj) const;
    String          GetPresObjText(PresObjKind eObjKind) const;
    SfxStyleSheet* GetStyleSheetForMasterPageBackground() const;
    SfxStyleSheet*  GetStyleSheetForPresObj(PresObjKind eObjKind) const;
    bool            RestoreDefaultText( SdrObject* pObj );

    /** returns true if the given SdrObject is inside the presentation object list */
    bool            IsPresObj(const SdrObject* pObj);

    /** removes the given SdrObject from the presentation object list */
    void            RemovePresObj(const SdrObject* pObj);

    /** inserts the given SdrObject into the presentation object list */
    void            InsertPresObj(SdrObject* pObj, PresObjKind eKind );

    void            SetAutoLayout(AutoLayout eLayout, bool bInit=false, bool bCreate=false);
    AutoLayout      GetAutoLayout() const { return meAutoLayout; }
    void            CreateTitleAndLayout(bool bInit=false, bool bCreate=false);
    SdrObject*      InsertAutoLayoutShape(SdrObject* pObj, PresObjKind eObjKind, bool bVertical, const basegfx::B2DRange& rRange, bool bInit );

    // react on content change
    virtual void handleContentChange(const SfxHint& rHint);

    virtual void SetLinkData(const String& rLinkName, const String& rLinkData);

    void        SetObjText(SdrTextObj* pObj, SdrOutliner* pOutliner, PresObjKind eObjKind, const String& rStr );

    void        SetPageKind(PageKind ePgType)        { mePageKind = ePgType; }
    PageKind    GetPageKind() const                  { return mePageKind; }

    void        SetSelected(bool bSel)               { mbSelected = bSel; }
    bool        IsSelected() const                   { return mbSelected; }

    void        SetFadeEffect(::com::sun::star::presentation::FadeEffect eNewEffect);
    ::com::sun::star::presentation::FadeEffect  GetFadeEffect() const;

    void        SetPresChange(PresChange eChange)    { mePresChange = eChange; }
    PresChange  GetPresChange() const                { return mePresChange; }

    void        SetTime(sal_uInt32 nNewTime)             { mnTime = nNewTime; }
    sal_uInt32      GetTime() const                      { return mnTime; }

    void        SetSound(bool bNewSoundOn)           { mbSoundOn = bNewSoundOn; }
    bool        IsSoundOn() const                    { return mbSoundOn; }

    void        SetExcluded(bool bNewExcluded)      { mbExcluded = bNewExcluded; }
    bool        IsExcluded() const                  { return mbExcluded; }

    void        SetScaleObjects(bool bScale)        { mbScaleObjects = bScale; }
    bool        IsScaleObjects() const              { return mbScaleObjects; }

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

    // virtual notifyer from SfxListener
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint);

    // new central SdrObject change handler, called from Notify
    void HandleChanged(const SdrObject& rObj, SdrHintKind eHint);

    void            SetLayoutName(String aName);
    virtual String  GetLayoutName() const       { return maLayoutName; }

    void            SetFileName(const String& aName) { maFileName = aName; }
    virtual String  GetFileName() const       { return maFileName; }
    void            SetBookmarkName(const String& aName) { maBookmarkName = aName; }
    virtual String  GetBookmarkName() const       { return maBookmarkName; }
    SdPageLink*     GetLink() { return mpPageLink; }

    void            ConnectLink();
    void            DisconnectLink();

    void ScaleObjects(const basegfx::B2DVector& rNewPageSize, double fLeft, double fTop, double fRight, double fBottom, bool bScaleAllObj);

    const String&   GetName() const;
    String          GetRealName() const { return FmFormPage::GetName(); };

    void    SetPresentationLayout(const String& rLayoutName,
                                  bool bReplaceStyleSheets = true,
                                  bool bSetMasterPage = true,
                                  bool bReverseOrder = false);
    void    EndListenOutlineText();

    void    SetBackgroundFullSize( bool bIn );
    bool    IsBackgroundFullSize() const { return mbBackgroundFullSize; }

    rtl_TextEncoding GetCharSet() { return(meCharSet); }

    void    SetPaperBin(sal_uInt16 nBin) { mnPaperBin = nBin; }
    sal_uInt16  GetPaperBin() const { return mnPaperBin; }
    virtual void        SetOrientation(Orientation eOrient);
    virtual Orientation GetOrientation() const;

    virtual SfxStyleSheet* GetTextStyleSheetForObject( SdrObject* pObj ) const;

    sal_Bool setAlienAttributes( const com::sun::star::uno::Any& rAttributes );
    void getAlienAttributes( com::sun::star::uno::Any& rAttributes );

    /** returns the main animation node */
    ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > getAnimationNode() throw (::com::sun::star::uno::RuntimeException);

    /** sets the main animation node */
    void setAnimationNode( ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode ) throw (::com::sun::star::uno::RuntimeException);

    /** returns a helper class to manipulate effects inside the main sequence */
    boost::shared_ptr< sd::MainSequence > getMainSequence();

    /** quick check if this slide has an animation node.
        This can be used to have a cost free check if there are no animations ad this slide.
        If it returns true this does not mean that there are animations available.
    */
    bool hasAnimationNode() const;

    /** returns the SdPage implementation for the given XDrawPage or 0 if not available */
    static SdPage* getImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xPage );

    /** removes all custom animations for the given shape */
    void removeAnimations( const SdrObject* pObj );

    /** Set the name of the page and broadcast a model change.
    */
    virtual void SetName (const String& rName);

    const sd::HeaderFooterSettings& getHeaderFooterSettings() const;
    void setHeaderFooterSettings( const sd::HeaderFooterSettings& rNewSettings );

    /** this method returns true if the object from the ViewObjectContact should
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

    /** returns the presentation style with the given helpid from this masterpage or this
        slides masterpage */
    SdStyleSheet* getPresentationStyle( sal_uInt32 nHelpId ) const;

    /** removes all empty presentation objects from this slide */
    void RemoveEmptyPresentationObjects();

    basegfx::B2DRange GetTitleRange() const;
    basegfx::B2DRange GetLayoutRange() const;

    static void CalculateHandoutAreas( SdDrawDocument& rModel, AutoLayout eLayout, bool bHorizontal, std::vector< basegfx::B2DRange >& rAreas );

    /** Set the "precious" flag to the given value.
    */
    void SetPrecious (const bool bIsPrecious);

    /** The "precious" flag is used for master pages to prevent some unused
        master pages from being deleted automatically.  For pages
        other than master pages this flag can be ignored.
        @return
            When this method returns <true/> for a master page then this
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

    /** called before a shape is removed from this slide */
    void onRemoveObject( const SdrObject* pObject );

    /** called after a shape is inserted to this slide */
    void onInsertObject( const SdrObject* pObject );
};

#endif     // _SDPAGE_HXX
