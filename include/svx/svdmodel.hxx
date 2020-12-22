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

#pragma once

#include <functional>
#include <memory>
#include <com/sun/star/uno/Sequence.hxx>
#include <editeng/forbiddencharacterstable.hxx>
#include <editeng/outliner.hxx>
#include <rtl/ustring.hxx>
#include <tools/weakbase.h>
#include <svl/SfxBroadcaster.hxx>
#include <tools/degree.hxx>
#include <tools/fldunit.hxx>
#include <tools/fract.hxx>
#include <svl/hint.hxx>
#include <o3tl/enumarray.hxx>

#include <svl/style.hxx>
#include <svx/xtable.hxx>

class OutputDevice;
#include <svx/svdtypes.hxx>
#include <svx/svxdllapi.h>

#include <rtl/ref.hxx>
#include <deque>

#ifdef DBG_UTIL
// SdrObjectLifetimeWatchDog
#include <unordered_set>
#endif

#define DEGREE_CHAR u'\x00B0'   /* U+00B0 DEGREE SIGN */

class SdrOutliner;
class SdrLayerAdmin;
class SdrObjList;
class SdrObject;
class SdrPage;
class SdrPageView;
class SdrTextObj;
class SdrUndoAction;
class SdrUndoGroup;
class AutoTimer;
class SfxItemPool;
class SfxItemSet;
class SfxRepeatTarget;
class SfxStyleSheet;
class SfxUndoAction;
class SfxUndoManager;
class XBitmapList;
class XColorList;
class XDashList;
class XGradientList;
class XHatchList;
class XLineEndList;
class SvxForbiddenCharactersTable;
class SvNumberFormatter;
class SdrOutlinerCache;
class SdrUndoFactory;
class ImageMap;
class TextChain;
enum class CharCompressType;
namespace comphelper
{
    class IEmbeddedHelper;
    class LifecycleProxy;
}
namespace sfx2
{
    class LinkManager;
}


enum class SdrHintKind
{
    LayerChange,          // changed layer definition
    LayerOrderChange,     // order of layer changed (Insert/Remove/ChangePos)
    PageOrderChange,      // order of pages (object pages or master pages) changed (Insert/Remove/ChangePos)
    ObjectChange,         // object changed
    ObjectInserted,       // new object inserted
    ObjectRemoved,        // symbol object removed from list
    ModelCleared,         // deleted the whole model (no pages exist anymore). not impl.
    RefDeviceChange,      // RefDevice changed
    DefaultTabChange,     // Default tabulator width changed
    SwitchToPage,          // #94278# UNDO/REDO at an object evtl. on another page
    BeginEdit,             // Is called after the object has entered text edit mode
    EndEdit                // Is called after the object has left text edit mode
};

class SVXCORE_DLLPUBLIC SdrHint final : public SfxHint
{
private:
    SdrHintKind                             meHint;
    const SdrObject*                        mpObj;
    const SdrPage*                          mpPage;

public:
    explicit SdrHint(SdrHintKind eNewHint);
    explicit SdrHint(SdrHintKind eNewHint, const SdrObject& rNewObj);
    explicit SdrHint(SdrHintKind eNewHint, const SdrPage* pPage);
    explicit SdrHint(SdrHintKind eNewHint, const SdrObject& rNewObj, const SdrPage* pPage);

    const SdrPage*   GetPage() const { return mpPage;}
    const SdrObject* GetObject() const { return mpObj;}
    SdrHintKind      GetKind() const { return meHint;}
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  SdrModel
//      DlgEdModel
//      FmFormModel
//          ScDrawLayer
//          SdDrawDocument
//          SwDrawModel
//      OReportModel

struct SdrModelImpl;

class SVXCORE_DLLPUBLIC SdrModel : public SfxBroadcaster, public tools::WeakBase
{
private:
#ifdef DBG_UTIL
    // SdrObjectLifetimeWatchDog:
    // Use maAllIncarnatedObjects to keep track of all SdrObjects incarnated using this SdrModel
    // (what is now possible after the paradigm change that a SdrObject stays at a single SdrModel
    // for it's whole lifetime).
    // The two methods are exclusive, debug-only, only-accessible-by SdrObject accesses to else
    // hidden/non-existing maAllIncarnatedObjects.
    // SdrObject::SdrObject uses impAddIncarnatedSdrObjectToSdrModel, while SdrObject::~SdrObject
    // uses impRemoveIncarnatedSdrObjectToSdrModel.
    // There are two places which may trigger SAL_WARN warnings:
    // - impRemoveIncarnatedSdrObjectToSdrModel when the to-be-removed SdrObject is not member of SdrModel
    // - SdrModel::~SdrModel after all SdrObjects *should* be cleaned-up.
    // SdrModel::~SdrModel will also - for convenience - Free the non-deleted SdrObjects if there
    // are any.
    // Using std::unordered_set will use quasi constant access times, so this watchdog will not
    // be expensive. Nonetheless, only use with debug code. It may be seductive to use this in
    // product code, too, especially if it will indeed trigger - but its intention is clearly
    // to find/identify MemoryLeaks caused by SdrObjects
    friend void impAddIncarnatedSdrObjectToSdrModel(const SdrObject& rSdrObject, SdrModel& rSdrModel);
    friend void impRemoveIncarnatedSdrObjectToSdrModel(const SdrObject& rSdrObject, SdrModel& rSdrModel);
    std::unordered_set< const SdrObject* >  maAllIncarnatedObjects;
#endif
protected:
    std::vector<SdrPage*> maMaPag;     // master pages
    std::vector<SdrPage*> maPages;
    std::function<void(std::unique_ptr<SdrUndoAction>)> m_aUndoLink;  // link to a NotifyUndo-Handler
    Size           m_aMaxObjSize; // e.g. for auto-growing text
    Fraction       m_aObjUnit;   // description of the coordinate units for ClipBoard, Drag&Drop, ...
    MapUnit        m_eObjUnit;   // see above
    FieldUnit      m_eUIUnit;      // unit, scale (e.g. 1/1000) for the UI (status bar) is set by ImpSetUIUnit()
    Fraction       m_aUIScale;     // see above
    OUString       m_aUIUnitStr;   // see above
    Fraction       m_aUIUnitFact;  // see above
    int            m_nUIUnitDecimalMark; // see above

    std::unique_ptr<SdrLayerAdmin> m_pLayerAdmin;
    SfxItemPool*    m_pItemPool;
    comphelper::IEmbeddedHelper*
                    m_pEmbeddedHelper; // helper for embedded objects to get rid of the SfxObjectShell
    std::unique_ptr<SdrOutliner> m_pDrawOutliner;  // an Outliner for outputting text
    std::unique_ptr<SdrOutliner> m_pHitTestOutliner;// an Outliner for the HitTest
    std::unique_ptr<SdrOutliner> m_pChainingOutliner; // an Outliner for chaining overflowing text
    sal_Int32       mnDefTextHgt;    // Default text height in logical units
    VclPtr<OutputDevice>  m_pRefOutDev;     // ReferenceDevice for the EditEngine
    rtl::Reference< SfxStyleSheetBasePool > mxStyleSheetPool;
    SfxStyleSheet*  m_pDefaultStyleSheet;
    SfxStyleSheet* mpDefaultStyleSheetForSdrGrafObjAndSdrOle2Obj; // #i119287#
    sfx2::LinkManager* m_pLinkManager;   // LinkManager
    std::unique_ptr<std::deque<std::unique_ptr<SfxUndoAction>>> m_pUndoStack;
    std::unique_ptr<std::deque<std::unique_ptr<SfxUndoAction>>> m_pRedoStack;
    std::unique_ptr<SdrUndoGroup> m_pCurrentUndoGroup;  // For multi-level
    sal_uInt16          m_nUndoLevel;                   // undo nesting
    bool                m_bMyPool:1;        // to clean up pMyPool from 303a
    bool                mbUndoEnabled:1;  // If false no undo is recorded or we are during the execution of an undo action
    bool                mbChanged:1;
    bool                m_bPagNumsDirty:1;
    bool                m_bMPgNumsDirty:1;
    bool                m_bTransportContainer:1;  // doc is temporary object container, no display (e.g. clipboard)
    bool                m_bReadOnly:1;
    bool                m_bTransparentTextFrames:1;
    bool                m_bSwapGraphics:1;
    bool                m_bPasteResize:1; // Objects are being resized due to Paste with different MapMode
    bool                m_bStarDrawPreviewMode:1;
    bool                mbDisableTextEditUsesCommonUndoManager:1;
    sal_uInt16          m_nDefaultTabulator;
    sal_uInt32          m_nMaxUndoCount;

    std::unique_ptr<TextChain> m_pTextChain;


public:
    std::shared_ptr<SvxForbiddenCharactersTable> mpForbiddenCharactersTable;

    std::unique_ptr<SdrOutlinerCache> mpOutlinerCache;
    //get a vector of all the SdrOutliner belonging to the model
    std::vector<SdrOutliner*> GetActiveOutliners() const;
    std::unique_ptr<SdrModelImpl>       mpImpl;
    CharCompressType    mnCharCompressType;
    sal_uInt16          mnHandoutPageCount;
    bool                mbModelLocked;
    bool                mbKernAsianPunctuation;
    bool                mbAddExtLeading;
    bool                mbInDestruction;

    // Color, Dash, Line-End, Hatch, Gradient, Bitmap property lists ...
    o3tl::enumarray<XPropertyListType, XPropertyListRef> maProperties;

    sal_uInt16 getHandoutPageCount() const { return mnHandoutPageCount; }
    void setHandoutPageCount( sal_uInt16 nHandoutPageCount ) { mnHandoutPageCount = nHandoutPageCount; }

    // Adapt to given Size and Borders scaling all contained data, maybe
    // including PresObj's in higher derivations
    virtual void adaptSizeAndBorderForAllPages(
        const Size& rNewSize,
        tools::Long nLeft = 0,
        tools::Long nRight = 0,
        tools::Long nUpper = 0,
        tools::Long nLower = 0);

protected:
    virtual css::uno::Reference< css::uno::XInterface > createUnoModel();

private:
    SdrModel(const SdrModel& rSrcModel) = delete;
    void operator=(const SdrModel& rSrcModel) = delete;
    bool operator==(const SdrModel& rCmpModel) const = delete;
    SVX_DLLPRIVATE void ImpPostUndoAction(std::unique_ptr<SdrUndoAction> pUndo);
    SVX_DLLPRIVATE void ImpSetUIUnit();
    SVX_DLLPRIVATE void ImpSetOutlinerDefaults( SdrOutliner* pOutliner, bool bInit = false );
    SVX_DLLPRIVATE void ImpReformatAllTextObjects();
    SVX_DLLPRIVATE void ImpReformatAllEdgeObjects();
    SVX_DLLPRIVATE void ImpCreateTables(bool bDisablePropertyFiles);

    SVX_DLLPRIVATE void ImpCtor(
        SfxItemPool* pPool,
        ::comphelper::IEmbeddedHelper* pPers,
        bool bDisablePropertyFiles);

    // this is a weak reference to a possible living api wrapper for this model
    css::uno::Reference< css::uno::XInterface > mxUnoModel;

    // used to disable unique name checking during page move
    bool mbMakePageObjectsNamesUnique = true;

public:
    SVX_DLLPRIVATE virtual bool IsCreatingDataObj() const { return false; }
    bool     IsTransportContainer() const { return m_bTransportContainer; }
    bool     IsPasteResize() const        { return m_bPasteResize; }
    void     SetPasteResize(bool bOn) { m_bPasteResize=bOn; }
    // If a custom Pool is put here, the class will call methods
    // on it (Put(), Remove()). On disposal of SdrModel the pool
    // will be deleted with   delete.
    // If you give NULL instead, it will create an own pool (SdrItemPool)
    // which will also be disposed in the destructor.
    // If you do use a custom Pool, make sure you inherit from SdrItemPool,
    // if you want to use symbol objects inherited from SdrAttrObj.
    // If, however, you use objects inheriting from SdrObject you are free
    // to chose a pool of your liking.
    //
    // tdf#118731 a bDisablePropertyFiles of true will disable ability to load
    // XPropertyFiles describing defaults. Useful for UI preview widgets
    explicit SdrModel(
        SfxItemPool* pPool = nullptr,
        ::comphelper::IEmbeddedHelper* pPers = nullptr,
        bool bDisablePropertyFiles = false);
    virtual ~SdrModel() override;
    void ClearModel(bool bCalledFromDestructor);

    // Override this to enable the Swap/LoadOnDemand of graphics.
    // If rbDeleteAfterUse is set to sal_True the SvStream instance from
    // the caller will be disposed after use.
    // If this method returns NULL, a temporary file will be allocated for
    // swapping.
    // The stream from which the model was loaded or in which is was saved last
    // needs to be delivered
    virtual css::uno::Reference<
                css::embed::XStorage> GetDocumentStorage() const;
    css::uno::Reference<
            css::io::XInputStream >
        GetDocumentStream(OUString const& rURL,
                ::comphelper::LifecycleProxy const & rProxy) const;
    // Change the template attributes of the symbol objects to hard attributes
    void BurnInStyleSheetAttributes();
    // If you inherit from SdrPage you also need to inherit from SdrModel
    // and implement both VM AllocPage() and AllocModel()...
    virtual SdrPage*  AllocPage(bool bMasterPage);
    virtual SdrModel* AllocModel() const;

    // Changes on the layers set the modified flag and broadcast on the model!
    const SdrLayerAdmin& GetLayerAdmin() const                  { return *m_pLayerAdmin; }
    SdrLayerAdmin&       GetLayerAdmin()                        { return *m_pLayerAdmin; }

    const SfxItemPool&   GetItemPool() const                    { return *m_pItemPool; }
    SfxItemPool&         GetItemPool()                          { return *m_pItemPool; }

    SdrOutliner&         GetDrawOutliner(const SdrTextObj* pObj=nullptr) const;

    SdrOutliner&         GetHitTestOutliner() const { return *m_pHitTestOutliner; }
    const SdrTextObj*    GetFormattingTextObj() const;
    // put the TextDefaults (Font,Height,Color) in a Set
    void                 SetTextDefaults() const;
    static void          SetTextDefaults( SfxItemPool* pItemPool, sal_Int32 nDefTextHgt );

    SdrOutliner&         GetChainingOutliner(const SdrTextObj* pObj) const;
    TextChain *          GetTextChain() const;

    // ReferenceDevice for the EditEngine
    void                 SetRefDevice(OutputDevice* pDev);
    OutputDevice*        GetRefDevice() const                   { return m_pRefOutDev.get(); }
    // If a new MapMode is set on the RefDevice (or similar)
    void                 RefDeviceChanged(); // not yet implemented
    // default font height in logical units
    void                 SetDefaultFontHeight(sal_Int32 nVal);
    // default tabulator width for the EditEngine
    void                 SetDefaultTabulator(sal_uInt16 nVal);
    sal_uInt16           GetDefaultTabulator() const            { return m_nDefaultTabulator; }

    // The DefaultStyleSheet will be used in every symbol object which is inserted
    // in this model and does not have a StyleSheet set.
    SfxStyleSheet*       GetDefaultStyleSheet() const             { return m_pDefaultStyleSheet; }
    void                 SetDefaultStyleSheet(SfxStyleSheet* pDefSS) { m_pDefaultStyleSheet = pDefSS; }

    // #i119287# default StyleSheet for SdrGrafObj and SdrOle2Obj
    SfxStyleSheet* GetDefaultStyleSheetForSdrGrafObjAndSdrOle2Obj() const { return mpDefaultStyleSheetForSdrGrafObjAndSdrOle2Obj; }
    void SetDefaultStyleSheetForSdrGrafObjAndSdrOle2Obj(SfxStyleSheet* pDefSS) { mpDefaultStyleSheetForSdrGrafObjAndSdrOle2Obj = pDefSS; }

    sfx2::LinkManager*   GetLinkManager()                         { return m_pLinkManager; }
    void                 SetLinkManager( sfx2::LinkManager* pLinkMgr ) { m_pLinkManager = pLinkMgr; }

    ::comphelper::IEmbeddedHelper*     GetPersist() const               { return m_pEmbeddedHelper; }
    void                 SetPersist( ::comphelper::IEmbeddedHelper *p ) { m_pEmbeddedHelper = p; }

    // Unit for the symbol coordination
    // Default is 1 logical unit = 1/100mm (Unit=MapUnit::Map100thMM, Fract=(1,1)).
    // Examples:
    //   MapUnit::MapPoint,    Fraction(72,1)    : 1 log Einh = 72 Point   = 1 Inch
    //   MapUnit::MapPoint,    Fraction(1,20)    : 1 log Einh = 1/20 Point = 1 Twip
    //   MapUnit::MapTwip,     Fraction(1,1)     : 1 log Einh = 1 Twip
    //   MapUnit::Map100thMM, Fraction(1,10)    : 1 log Einh = 1/1000mm
    //   MapUnit::MapMM,       Fraction(1000,1)  : 1 log Einh = 1000mm     = 1m
    //   MapUnit::MapCM,       Fraction(100,1)   : 1 log Einh = 100cm      = 1m
    //   MapUnit::MapCM,       Fraction(100000,1): 1 log Einh = 100000cm   = 1km
    // (FWIW: you cannot represent light years).
    // The scaling unit is needed for the Engine to serve the Clipboard
    // with the correct sizes.
    MapUnit          GetScaleUnit() const                       { return m_eObjUnit; }
    void             SetScaleUnit(MapUnit eMap);
    const Fraction&  GetScaleFraction() const                   { return m_aObjUnit; }
    void             SetScaleFraction(const Fraction& rFrac);
    // Setting both simultaneously performs a little better
    void             SetScaleUnit(MapUnit eMap, const Fraction& rFrac);

    // maximal size e.g. for auto growing texts
    const Size&      GetMaxObjSize() const                      { return m_aMaxObjSize; }
    void             SetMaxObjSize(const Size& rSiz)            { m_aMaxObjSize=rSiz; }

    // For the View! to display sane numbers in the status bar: Default is mm.
    void             SetUIUnit(FieldUnit eUnit);
    FieldUnit        GetUIUnit() const                          { return m_eUIUnit; }
    // The scale of the drawing. Default 1/1.
    void             SetUIScale(const Fraction& rScale);
    const Fraction&  GetUIScale() const                         { return m_aUIScale; }
    // Setting both simultaneously performs a little better
    void             SetUIUnit(FieldUnit eUnit, const Fraction& rScale);

    static OUString  GetUnitString(FieldUnit eUnit);
    OUString         GetMetricString(tools::Long nVal, bool bNoUnitChars = false, sal_Int32 nNumDigits = -1) const;
    static OUString  GetAngleString(Degree100 nAngle);
    static OUString  GetPercentString(const Fraction& rVal);

    // RecalcPageNums is ordinarily only called by the Page.
    bool             IsPagNumsDirty() const                     { return m_bPagNumsDirty; };
    bool             IsMPgNumsDirty() const                     { return m_bMPgNumsDirty; };
    void             RecalcPageNums(bool bMaster);
    // After the Insert the Page belongs to the SdrModel.
    virtual void     InsertPage(SdrPage* pPage, sal_uInt16 nPos=0xFFFF);
    virtual void     DeletePage(sal_uInt16 nPgNum);
    // Remove means transferring ownership to the caller (opposite of Insert)
    virtual SdrPage* RemovePage(sal_uInt16 nPgNum);
    virtual void     MovePage(sal_uInt16 nPgNum, sal_uInt16 nNewPos);
    const SdrPage*   GetPage(sal_uInt16 nPgNum) const;
    SdrPage*         GetPage(sal_uInt16 nPgNum);
    sal_uInt16       GetPageCount() const;
    virtual void     PageListChanged();

    // Masterpages
    virtual void     InsertMasterPage(SdrPage* pPage, sal_uInt16 nPos=0xFFFF);
    void             DeleteMasterPage(sal_uInt16 nPgNum);
    // Remove means transferring ownership to the caller (opposite of Insert)
    virtual SdrPage* RemoveMasterPage(sal_uInt16 nPgNum);
    void             MoveMasterPage(sal_uInt16 nPgNum, sal_uInt16 nNewPos);
    const SdrPage*   GetMasterPage(sal_uInt16 nPgNum) const;
    SdrPage*         GetMasterPage(sal_uInt16 nPgNum);
    sal_uInt16       GetMasterPageCount() const;
    virtual void     MasterPageListChanged();

    // modified flag. Is set automatically when something changes on the Pages
    // symbol objects. You need to reset it yourself, however, e.g. on Save().
    bool IsChanged() const { return mbChanged; }
    virtual void SetChanged(bool bFlg = true);

    // If set to sal_True, graphics from graphics objects will:
    // - not be loaded immediately when loading a document,
    //   but only once they are needed (e.g. displayed).
    // - be pruned from memory if they are not needed.
    // For that to work, override the virtual method GetDocumentStream().
    // Default=FALSE. Flag is not persistent.
    bool            IsSwapGraphics() const { return m_bSwapGraphics; }
    void            SetSwapGraphics();

    // Text frames without filling can be select with a mouse click by default (sal_False).
    // With this flag set to true you can hit them only in the area in which text is to be
    // found.
    bool            IsPickThroughTransparentTextFrames() const  { return m_bTransparentTextFrames; }
    void            SetPickThroughTransparentTextFrames(bool bOn) { m_bTransparentTextFrames=bOn; }

    // Can the model be changed at all?
    // Is only evaluated by the possibility methods of the View.
    // Direct manipulations on the model, ... do not respect this flag.
    // Override this and return the appropriate ReadOnly status
    // of the files, i.e. true or false. (Method is called multiple
    // times, so use one flag only!)
    virtual bool IsReadOnly() const;
    void     SetReadOnly(bool bYes);

    // Mixing two SdrModels. Mind that rSourceModel is not const.
    // The pages will not be copied but moved, when inserted.
    // rSourceModel may very well be empty afterwards.
    // nFirstPageNum,nLastPageNum: The pages to take from rSourceModel
    // nDestPos..................: position to insert
    // bMergeMasterPages.........: sal_True = needed MasterPages will be taken
    //                                   from rSourceModel
    //                             sal_False= the MasterPageDescriptors of
    //                                   the pages of the rSourceModel will be
    //                                   mapped on the existing  MasterPages.
    // bUndo.....................: An undo action is generated for the merging.
    //                             Undo is only for the target model, not for the
    //                             rSourceModel.
    // bTreadSourceAsConst.......: sal_True=the SourceModel will not be changed,
    //                             so pages will be copied.
    virtual void Merge(SdrModel& rSourceModel,
               sal_uInt16 nFirstPageNum, sal_uInt16 nLastPageNum,
               sal_uInt16 nDestPos,
               bool bMergeMasterPages, bool bAllMasterPages,
               bool bUndo = true, bool bTreadSourceAsConst = false);

    // Behaves like Merge(SourceModel=DestModel,nFirst,nLast,nDest,sal_False,sal_False,bUndo,!bMoveNoCopy);
    void CopyPages(sal_uInt16 nFirstPageNum, sal_uInt16 nLastPageNum,
                   sal_uInt16 nDestPos,
                   bool bUndo, bool bMoveNoCopy);

    // BegUndo() / EndUndo() enables you to group arbitrarily many UndoActions
    // arbitrarily deeply. As comment for the UndoAction the first BegUndo(String) of all
    // nestings will be used.
    // In that case the NotifyUndoActionHdl will be called on the last EndUndo().
    // No UndoAction will be generated for an empty group.
    // All direct modifications on the SdrModel do not create an UndoActions.
    // Actions on the SdrView however do generate those.
    void BegUndo();                       // open Undo group
    void BegUndo(const OUString& rComment); // open Undo group
    void BegUndo(const OUString& rComment, const OUString& rObjDescr, SdrRepeatFunc eFunc); // open Undo group
    void EndUndo();                       // close Undo group
    void AddUndo(std::unique_ptr<SdrUndoAction> pUndo);
    sal_uInt16 GetUndoBracketLevel() const                       { return m_nUndoLevel; }
    // only after the first BegUndo or before the last EndUndo:
    void SetUndoComment(const OUString& rComment);
    void SetUndoComment(const OUString& rComment, const OUString& rObjDescr);

    // The Undo management is only done if NotifyUndoAction-Handler is not set.
    // Default is 16. Minimal MaxUndoActionCount is 1.
    void  SetMaxUndoActionCount(sal_uInt32 nCount);
    void  ClearUndoBuffer();

    bool HasUndoActions() const;
    bool HasRedoActions() const;
    void Undo();
    void Redo();
    void Repeat(SfxRepeatTarget&);

    // The application can set a handler here which collects the UndoActions einsammelt.
    // The handler has the following signature:
    //   void NotifyUndoActionHdl(SfxUndoAction* pUndoAction);
    // When calling the handler ownership is transferred;
    // The UndoAction belongs to the Handler, not the SdrModel.
    void        SetNotifyUndoActionHdl(const std::function<void(std::unique_ptr<SdrUndoAction>)>& rLink)  { m_aUndoLink=rLink; }

    /** application can set its own undo manager, BegUndo, EndUndo and AddUndoAction
        calls are routed to this interface if given */
    void SetSdrUndoManager( SfxUndoManager* pUndoManager );
    SfxUndoManager* GetSdrUndoManager() const;

    /** applications can set their own undo factory to override creation of
        undo actions. The SdrModel will become owner of the given SdrUndoFactory
        and delete it upon its destruction. */
    void SetSdrUndoFactory( SdrUndoFactory* pUndoFactory );

    /** returns the models undo factory. This must be used to create
        undo actions for this model. */
    SdrUndoFactory& GetSdrUndoFactory() const;

    // Accessor methods for Palettes, Lists and Tables
    // FIXME: this badly needs re-factoring...
    const XPropertyListRef& GetPropertyList( XPropertyListType t ) const { return maProperties[ t ]; }
    void             SetPropertyList( XPropertyListRef const & p ) { maProperties[ p->Type() ] = p; }

    // friendlier helpers
    XDashListRef     GetDashList() const     { return XPropertyList::AsDashList(GetPropertyList( XPropertyListType::Dash )); }
    XHatchListRef    GetHatchList() const    { return XPropertyList::AsHatchList(GetPropertyList( XPropertyListType::Hatch )); }
    XColorListRef    GetColorList() const    { return XPropertyList::AsColorList(GetPropertyList( XPropertyListType::Color )); }
    XBitmapListRef   GetBitmapList() const   { return XPropertyList::AsBitmapList(GetPropertyList( XPropertyListType::Bitmap )); }
    XPatternListRef  GetPatternList() const  { return XPropertyList::AsPatternList(GetPropertyList( XPropertyListType::Pattern )); }
    XLineEndListRef  GetLineEndList() const  { return XPropertyList::AsLineEndList(GetPropertyList( XPropertyListType::LineEnd )); }
    XGradientListRef GetGradientList() const { return XPropertyList::AsGradientList(GetPropertyList( XPropertyListType::Gradient )); }

    // The DrawingEngine only references the StyleSheetPool, whoever
    // made it needs to delete it.
    SfxStyleSheetBasePool* GetStyleSheetPool() const         { return mxStyleSheetPool.get(); }
    void SetStyleSheetPool(SfxStyleSheetBasePool* pPool)     { mxStyleSheetPool=pPool; }

    void    SetStarDrawPreviewMode(bool bPreview);
    bool    IsStarDrawPreviewMode() const { return m_bStarDrawPreviewMode; }

    bool GetDisableTextEditUsesCommonUndoManager() const { return mbDisableTextEditUsesCommonUndoManager; }
    void SetDisableTextEditUsesCommonUndoManager(bool bNew) { mbDisableTextEditUsesCommonUndoManager = bNew; }

    css::uno::Reference< css::uno::XInterface > const & getUnoModel();
    void setUnoModel( const css::uno::Reference< css::uno::XInterface >& xModel );

    // these functions are used by the api to disable repaints during a
    // set of api calls.
    bool isLocked() const { return mbModelLocked; }
    void setLock( bool bLock );

    void            SetForbiddenCharsTable( const std::shared_ptr<SvxForbiddenCharactersTable>& xForbiddenChars );
    const std::shared_ptr<SvxForbiddenCharactersTable>& GetForbiddenCharsTable() const { return mpForbiddenCharactersTable;}

    void SetCharCompressType( CharCompressType nType );
    CharCompressType GetCharCompressType() const { return mnCharCompressType; }

    void SetKernAsianPunctuation( bool bEnabled );
    bool IsKernAsianPunctuation() const { return mbKernAsianPunctuation; }

    void SetAddExtLeading( bool bEnabled );
    bool IsAddExtLeading() const { return mbAddExtLeading; }

    // tdf#99729 compatibility flag
    void SetAnchoredTextOverflowLegacy(bool bEnabled);
    bool IsAnchoredTextOverflowLegacy() const;

    void ReformatAllTextObjects();

    std::unique_ptr<SdrOutliner> createOutliner( OutlinerMode nOutlinerMode );
    void disposeOutliner( std::unique_ptr<SdrOutliner> pOutliner );

    bool IsWriter() const { return !m_bMyPool; }

    // Used as a fallback in *::ReadUserDataSequence() to process common properties
    void ReadUserDataSequenceValue(const css::beans::PropertyValue *pValue);
    void WriteUserDataSequence(css::uno::Sequence < css::beans::PropertyValue >& rValues);

    /** returns the numbering type that is used to format page fields in drawing shapes */
    virtual SvxNumType GetPageNumType() const;

    /** copies the items from the source set to the destination set. Both sets must have
        same ranges but can have different pools. */
    static void MigrateItemSet( const SfxItemSet* pSourceSet, SfxItemSet* pDestSet, SdrModel* pNewModel );

    bool IsInDestruction() const { return mbInDestruction;}

    static const css::uno::Sequence< sal_Int8 >& getUnoTunnelId();

    /** enables (true) or disables (false) recording of undo actions
        If undo actions are added while undo is disabled, they are deleted.
        Disabling undo does not clear the current undo buffer! */
    void EnableUndo( bool bEnable );

    /** returns true if undo is currently enabled
        This returns false if undo was disabled using EnableUndo( false ) and
        also during the runtime of the Undo() and Redo() methods. */
    bool IsUndoEnabled() const;

    // used to prevent object name change during page move
    bool DoesMakePageObjectsNamesUnique() const { return mbMakePageObjectsNamesUnique; }
    void DoMakePageObjectsNamesUnique(bool bDo) { mbMakePageObjectsNamesUnique = bDo; }

    virtual void dumpAsXml(xmlTextWriterPtr pWriter) const;
};

/*
            +-----------+
            | SdrModel  |
            +--+------+-+
               |      +-----------+
          +----+-----+            |
          |   ...    |            |
     +----+---+ +----+---+  +-----+--------+
     |SdrPage | |SdrPage |  |SdrLayerAdmin |
     +---+----+ +-+--+--++  +---+-------+--+
         |        |  |  |       |       +-------------------+
    +----+----+           +-----+-----+             +-------+-------+
    |   ...   |           |    ...    |             |      ...      |
+---+---+ +---+---+  +----+----+ +----+----+  +-----+------+ +------+-----+
|SdrObj | |SdrObj |  |SdrLayer | |SdrLayer |  |SdrLayerSet | |SdrLayerSet |
+-------+ +-------+  +---------+ +---------+  +------------+ +------------+
This class: SdrModel is the head of the data models for the StarView Drawing Engine.

///////////////////////////////////////////////////////////////////////////////////////////////// */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
