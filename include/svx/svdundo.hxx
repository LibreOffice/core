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

#ifndef INCLUDED_SVX_SVDUNDO_HXX
#define INCLUDED_SVX_SVDUNDO_HXX

#include <sal/config.h>

#include <memory>
#include <vector>

#include <svl/solar.hrc>
#include <svl/undo.hxx>
#include <svl/style.hxx>
#include <tools/gen.hxx>
#include <svx/svdtypes.hxx>
#include <svx/svdsob.hxx>
#include <svx/svxdllapi.h>

class SfxItemSet;
class SfxPoolItem;
class SfxStyleSheet;
class SdrView;
class SdrPageView;
class SdrModel;
class SdrObject;
class SdrPage;
class SdrObjList;
class SdrLayer;
class SdrLayerAdmin;
class SdrObjGeoData;
class OutlinerParaObject;

/**
 * Abstract base class (ABC) for all UndoActions of DrawingEngine
 */

class SVX_DLLPUBLIC SdrUndoAction : public SfxUndoAction
{
protected:
    SdrModel&     rMod;
    ViewShellId   m_nViewShellId;

protected:
    SdrUndoAction(SdrModel& rNewMod);

public:
    virtual ~SdrUndoAction() override;

    virtual bool CanRepeat(SfxRepeatTarget& rView) const override;
    virtual void Repeat(SfxRepeatTarget& rView) override;

    virtual OUString GetRepeatComment(SfxRepeatTarget& rView) const override;
    virtual OUString GetSdrRepeatComment(SdrView& rView) const;

    virtual bool CanSdrRepeat(SdrView& rView) const;
    virtual void SdrRepeat(SdrView& rView);

    /// See SfxUndoAction::GetViewShellId().
    ViewShellId GetViewShellId() const override;
};

/**
 * Aggregation of arbitrarily many UndoActions to a single UndoAction.
 * Undo() executes the single UndoActions backwards (LIFO, first the last
 * ones attached), Redo() executes the UndoActions in the order they were
 * added (FIFO).
 */

class SVX_DLLPUBLIC SdrUndoGroup final : public SdrUndoAction
{
    std::vector<std::unique_ptr<SdrUndoAction>> maActions;

    // No expanded description of the Action (contains %O)
    OUString                    aComment;
    OUString                    aObjDescription;

    SdrRepeatFunc               eFunction;

public:
    SdrUndoGroup(SdrModel& rNewMod);
    virtual ~SdrUndoGroup() override;

    sal_Int32 GetActionCount() const { return maActions.size(); }
    SdrUndoAction* GetAction(sal_Int32 nNum) const { return maActions[nNum].get(); }
    void AddAction(std::unique_ptr<SdrUndoAction> pAct);

    void SetComment(const OUString& rStr) { aComment=rStr; }
    void SetObjDescription(const OUString& rStr) { aObjDescription=rStr; }
    virtual OUString GetComment() const override;
    virtual OUString GetSdrRepeatComment(SdrView& rView) const override;

    virtual void Undo() override;
    virtual void Redo() override;

    virtual bool CanSdrRepeat(SdrView& rView) const override;
    virtual void SdrRepeat(SdrView& rView) override;
    void SetRepeatFunction(SdrRepeatFunc eFunc) { eFunction=eFunc; }
};

/**
 * Abstract base class for all UndoActions that handle objects.
 */

class SVX_DLLPUBLIC SdrUndoObj : public SdrUndoAction
{
protected:
    SdrObject*                  pObj;

protected:
    SdrUndoObj(SdrObject& rNewObj);

    void ImpTakeDescriptionStr(const char* pStrCacheID, OUString& rStr, bool bRepeat = false) const;

    [[nodiscard]] static OUString GetDescriptionStringForObject( const SdrObject& _rForObject, const char* pStrCacheID, bool bRepeat = false );

    // #94278# new method for evtl. PageChange at UNDO/REDO
    void ImpShowPageOfThisObject();
};

/**
 * Changing the object attributes.
 * Create Action right before setting the new attributes.
 * Also for StyleSheets.
 */

class SVX_DLLPUBLIC SdrUndoAttrObj : public SdrUndoObj
{
protected:
    std::unique_ptr<SfxItemSet> pUndoSet;
    std::unique_ptr<SfxItemSet> pRedoSet;

    // FIXME: Or should we better remember the StyleSheetNames?
    rtl::Reference< SfxStyleSheetBase > mxUndoStyleSheet;
    rtl::Reference< SfxStyleSheetBase > mxRedoStyleSheet;
    bool                        bStyleSheet;
    bool                        bHaveToTakeRedoSet;

    // When assigning TextItems to a drawing object with text:
    std::unique_ptr<OutlinerParaObject>
                                pTextUndo;
    // #i8508#
    // The text rescue mechanism needs also to be implemented for redo actions.
    std::unique_ptr<OutlinerParaObject>
                                pTextRedo;

    // If we have a group object:
    std::unique_ptr<SdrUndoGroup> pUndoGroup;

    // Helper to ensure StyleSheet is in pool (provided by SdrModel from SdrObject)
    static void ensureStyleSheetInStyleSheetPool(SfxStyleSheetBasePool& rStyleSheetPool, SfxStyleSheet& rSheet);

public:
    SdrUndoAttrObj(SdrObject& rNewObj, bool bStyleSheet1 = false, bool bSaveText = false);
    virtual ~SdrUndoAttrObj() override;
    virtual void Undo() override;
    virtual void Redo() override;

    virtual OUString GetComment() const override;
    virtual OUString GetSdrRepeatComment(SdrView& rView) const override;
};

/**
 * Only moving of an object.
 * Create Action right before moving.
 */

class SVX_DLLPUBLIC SdrUndoMoveObj final : public SdrUndoObj
{
    Size const                        aDistance;     // Distance by which we move

public:
    SdrUndoMoveObj(SdrObject& rNewObj, const Size& rDist): SdrUndoObj(rNewObj),aDistance(rDist) {}
    virtual ~SdrUndoMoveObj() override;

    virtual void Undo() override;
    virtual void Redo() override;

    virtual OUString GetComment() const override;
    virtual OUString GetSdrRepeatComment(SdrView& rView) const override;

    virtual void SdrRepeat(SdrView& rView) override;
    virtual bool CanSdrRepeat(SdrView& rView) const override;
};

/**
 * Changing the geometry of an object.
 * Create Action right before the geometric transformation.
 */

class SVX_DLLPUBLIC SdrUndoGeoObj : public SdrUndoObj
{
    std::unique_ptr<SdrObjGeoData>  pUndoGeo;
    std::unique_ptr<SdrObjGeoData>  pRedoGeo;
    // If we have a group object:
    std::unique_ptr<SdrUndoGroup>   pUndoGroup;
    /// If we have a table object, should its layout change?
    bool mbSkipChangeLayout;

public:
    SdrUndoGeoObj(SdrObject& rNewObj);
    virtual ~SdrUndoGeoObj() override;

    virtual void Undo() override;
    virtual void Redo() override;

    virtual OUString GetComment() const override;
    void SetSkipChangeLayout(bool bOn) { mbSkipChangeLayout=bOn; }
};

/**
 * Manipulation of an ObjList: New Object, DeleteObj, SetObjZLevel, Grouping, ...
 * Abstract base class.
 */

class SVX_DLLPUBLIC SdrUndoObjList : public SdrUndoObj {
    class ObjListListener;
    friend class ObjListListener;

private:
    bool                        bOwner;

protected:
    SdrObjList*                 pObjList;
    sal_uInt32                      nOrdNum;

protected:
    SdrUndoObjList(SdrObject& rNewObj, bool bOrdNumDirect);
    virtual ~SdrUndoObjList() override;

    bool IsOwner() { return bOwner; }
    void SetOwner(bool bNew);
};

/**
 * Removing an Object from an ObjectList.
 * To be used with corresponding Inserts within a UndoGroup.
 *
 * Create Action before removing from the ObjList.
 */

class SVX_DLLPUBLIC SdrUndoRemoveObj : public SdrUndoObjList
{
public:
    SdrUndoRemoveObj(SdrObject& rNewObj, bool bOrdNumDirect = false)
    : SdrUndoObjList(rNewObj,bOrdNumDirect) {}

    virtual void Undo() override;
    virtual void Redo() override;

    virtual ~SdrUndoRemoveObj() override;
};

/**
 * Inserting Objects into a ObjectList.
 * Use with corresponding Removes within an UndoGroup.
 * Create Action before removal from ObjList.
 */

class SVX_DLLPUBLIC SdrUndoInsertObj : public SdrUndoObjList
{
public:
    SdrUndoInsertObj(SdrObject& rNewObj, bool bOrdNumDirect = false)
    :   SdrUndoObjList(rNewObj,bOrdNumDirect) {}

    virtual void Undo() override;
    virtual void Redo() override;
};

/**
 * Deleting an Object.
 * Create Action before removing from ObjList.
 */

class SVX_DLLPUBLIC SdrUndoDelObj : public SdrUndoRemoveObj
{
public:
    SdrUndoDelObj(SdrObject& rNewObj, bool bOrdNumDirect = false);

    virtual void Undo() override;
    virtual void Redo() override;

    virtual OUString GetComment() const override;
    virtual OUString GetSdrRepeatComment(SdrView& rView) const override;

    virtual void SdrRepeat(SdrView& rView) override;
    virtual bool CanSdrRepeat(SdrView& rView) const override;
};

/**
 * Inserting a NEW Object.
 * Create Action after insertion into the ObjList.
 */

class SVX_DLLPUBLIC SdrUndoNewObj : public SdrUndoInsertObj
{
public:
    SdrUndoNewObj(SdrObject& rNewObj, bool bOrdNumDirect = false)
    :   SdrUndoInsertObj(rNewObj,bOrdNumDirect) {}

    virtual void Undo() override;
    virtual void Redo() override;

    virtual OUString GetComment() const override;

    static  OUString GetComment(const SdrObject& _rForObject);
};

/**
 * Replacing an Object.
 * Create Action before Replace in ObjList.
 */

class SVX_DLLPUBLIC SdrUndoReplaceObj : public SdrUndoObj
{
    bool                        bOldOwner;
    bool                        bNewOwner;

    SdrObjList*                 pObjList;
    sal_uInt32                      nOrdNum;
    SdrObject*                  pNewObj;

public:
    SdrUndoReplaceObj(SdrObject& rOldObj1, SdrObject& rNewObj1, bool bOrdNumDirect);
    virtual ~SdrUndoReplaceObj() override;

    virtual void Undo() override;
    virtual void Redo() override;

    bool IsNewOwner() { return bNewOwner; }
    void SetNewOwner(bool bNew);

    bool IsOldOwner() { return bOldOwner; }
    void SetOldOwner(bool bNew);
};

/**
 * Copying an Object.
 * Create Action before inserting into the ObjList.
 */

class SdrUndoCopyObj : public SdrUndoNewObj
{
public:
    SdrUndoCopyObj(SdrObject& rNewObj, bool bOrdNumDirect)
    :   SdrUndoNewObj(rNewObj,bOrdNumDirect) {}

    virtual OUString GetComment() const override;
};

class SdrUndoObjOrdNum final : public SdrUndoObj
{
    sal_uInt32 const                      nOldOrdNum;
    sal_uInt32 const                      nNewOrdNum;

public:
    SdrUndoObjOrdNum(SdrObject& rNewObj, sal_uInt32 nOldOrdNum1, sal_uInt32 nNewOrdNum1);

    virtual void Undo() override;
    virtual void Redo() override;

    virtual OUString GetComment() const override;
};


// #i11702#

class SVX_DLLPUBLIC SdrUndoObjectLayerChange final : public SdrUndoObj
{
    SdrLayerID const                  maOldLayer;
    SdrLayerID const                  maNewLayer;

public:
    SdrUndoObjectLayerChange(SdrObject& rObj, SdrLayerID aOldLayer, SdrLayerID aNewLayer);

    virtual void Undo() override;
    virtual void Redo() override;
};

class SVX_DLLPUBLIC SdrUndoObjSetText : public SdrUndoObj
{
protected:
    std::unique_ptr<OutlinerParaObject>
                                pOldText;
    std::unique_ptr<OutlinerParaObject>
                                pNewText;
    bool                        bNewTextAvailable;
    bool                        bEmptyPresObj;
    sal_Int32 const             mnText;

public:
    SdrUndoObjSetText(SdrObject& rNewObj, sal_Int32 nText );
    virtual ~SdrUndoObjSetText() override;

    bool IsDifferent() const { return pOldText!=pNewText; }
    void AfterSetText();

    virtual void Undo() override;
    virtual void Redo() override;

    virtual OUString GetComment() const override;
    virtual OUString GetSdrRepeatComment(SdrView& rView) const override;

    virtual void SdrRepeat(SdrView& rView) override;
    virtual bool CanSdrRepeat(SdrView& rView) const override;
};

/**
 * Implement Title/Description Elements UI for Writer
 * text frames, graphics and embedded objects (#i73249#)
 */
class SdrUndoObjStrAttr final : public SdrUndoObj
{
public:
    enum class ObjStrAttrType
    {
        Name,
        Title,
        Description
    };

    SdrUndoObjStrAttr( SdrObject& rNewObj,
                       const ObjStrAttrType eObjStrAttr,
                       const OUString& sOldStr,
                       const OUString& sNewStr);

    virtual void Undo() override;
    virtual void Redo() override;

    virtual OUString GetComment() const override;

private:
    const ObjStrAttrType meObjStrAttr;
    const OUString msOldStr;
    const OUString msNewStr;

};


/*
 * Layer
 */

/**
 * Abstract base class for all UndoActions that have something to do with SdrLayer.
 */

class SdrUndoLayer : public SdrUndoAction
{
protected:
    SdrLayer* const             pLayer;
    SdrLayerAdmin*              pLayerAdmin;
    sal_uInt16 const            nNum;
    bool                        bItsMine;

protected:
    SdrUndoLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel);
    virtual ~SdrUndoLayer() override;
};

/**
 * Inserting a new Layer.
 * Create Action after Insertion.
 */

class SdrUndoNewLayer : public SdrUndoLayer
{
public:
    SdrUndoNewLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel)
    : SdrUndoLayer(nLayerNum,rNewLayerAdmin,rNewModel) {}

    virtual void Undo() override;
    virtual void Redo() override;

    virtual OUString GetComment() const override;
};

/**
 * Deleting a Layer.
 * Create Action before the Remove.
 */

class SdrUndoDelLayer : public SdrUndoLayer
{
public:
    SdrUndoDelLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel)
    : SdrUndoLayer(nLayerNum,rNewLayerAdmin,rNewModel) { bItsMine=true; }

    virtual void Undo() override;
    virtual void Redo() override;

    virtual OUString GetComment() const override;
};

/*
 * Pages
 */

/**
 * ABC for all UndoActions that have something to do with SdrPages.
 */

class SVX_DLLPUBLIC SdrUndoPage : public SdrUndoAction
{
protected:
    SdrPage&                    mrPage;

protected:
    void ImpInsertPage(sal_uInt16 nNum);
    void ImpRemovePage(sal_uInt16 nNum);
    void ImpMovePage(sal_uInt16 nOldNum, sal_uInt16 nNewNum);

protected:
    SdrUndoPage(SdrPage& rNewPg);

    static void ImpTakeDescriptionStr(const char* pStrCacheID, OUString& rStr);
};

/**
 * ABC for manipulation of a PageList: New Page, DeletePage, MovePage (ChangePageNum).
 */

class SVX_DLLPUBLIC SdrUndoPageList : public SdrUndoPage
{
protected:
    sal_uInt16                      nPageNum;

    // It's possible that the object is re-assigned during a Undo/Redo.
    // The Page is deleted in the dtor, if bItsMine==TRUE
    bool                        bItsMine;

protected:
    SdrUndoPageList(SdrPage& rNewPg);
    virtual ~SdrUndoPageList() override;
};

/**
 * Deleting a Page.
 * Create Action before removing from the List.
 */

class SVX_DLLPUBLIC SdrUndoDelPage : public SdrUndoPageList
{
    // When deleting a MasterPage, we remember all relations of the
    // Character Page with the MasterPage in this UndoGroup.
    std::unique_ptr<SdrUndoGroup>  pUndoGroup;
    std::unique_ptr<SfxPoolItem>   mpFillBitmapItem;
    bool                           mbHasFillBitmap;

public:
    SdrUndoDelPage(SdrPage& rNewPg);
    virtual ~SdrUndoDelPage() override;

    virtual void Undo() override;
    virtual void Redo() override;

    virtual OUString GetComment() const override;
    virtual OUString GetSdrRepeatComment(SdrView& rView) const override;

    virtual void SdrRepeat(SdrView& rView) override;
    virtual bool CanSdrRepeat(SdrView& rView) const override;

private:
    void queryFillBitmap(const SfxItemSet &rItemSet);
    void clearFillBitmap();
    void restoreFillBitmap();
};

/**
 * Inserting a new Page.
 * Create Action after inserting into the List.
 */

class SVX_DLLPUBLIC SdrUndoNewPage : public SdrUndoPageList
{
public:
    SdrUndoNewPage(SdrPage& rNewPg): SdrUndoPageList(rNewPg) {}

    virtual void Undo() override;
    virtual void Redo() override;

    virtual OUString GetComment() const override;
};

/**
 * Copying a Page.
 * Create Action after inserting into the List.
 */

class SdrUndoCopyPage : public SdrUndoNewPage
{
public:
    SdrUndoCopyPage(SdrPage& rNewPg): SdrUndoNewPage(rNewPg) {}

    virtual OUString GetComment() const override;
    virtual OUString GetSdrRepeatComment(SdrView& rView) const override;

    virtual void SdrRepeat(SdrView& rView) override;
    virtual bool CanSdrRepeat(SdrView& rView) const override;
};

/**
 * Moving a Page within the List.
 * Create Action before moving the Page.
 */

class SVX_DLLPUBLIC SdrUndoSetPageNum final : public SdrUndoPage
{
    sal_uInt16 const                      nOldPageNum;
    sal_uInt16 const                      nNewPageNum;

public:
    SdrUndoSetPageNum(SdrPage& rNewPg, sal_uInt16 nOldPageNum1, sal_uInt16 nNewPageNum1)
    :   SdrUndoPage(rNewPg),nOldPageNum(nOldPageNum1),nNewPageNum(nNewPageNum1) {}

    virtual void Undo() override;
    virtual void Redo() override;

    virtual OUString GetComment() const override;
};


/*
 * Masterpages
 */

/**
 * ABC for all UndoActions that have something to do with
 * MasterPage relationships.
 */

class SdrUndoPageMasterPage : public SdrUndoPage
{
protected:
    bool const                  mbOldHadMasterPage;
    SdrLayerIDSet               maOldSet;
    sal_uInt16                  maOldMasterPageNumber;

protected:
    SdrUndoPageMasterPage(SdrPage& rChangedPage);

public:
    SVX_DLLPUBLIC virtual ~SdrUndoPageMasterPage() override;
};

/**
 * Removal of a MasterPage from a Character Page.
 * Create Action before removing the MasterPageDescriptor.
 */

class SdrUndoPageRemoveMasterPage : public SdrUndoPageMasterPage
{
public:
    SdrUndoPageRemoveMasterPage(SdrPage& rChangedPage);

    virtual void Undo() override;
    virtual void Redo() override;

    virtual OUString GetComment() const override;
};

/**
 * Changing the MasterPageDescriptor (e.g. change of the VisibleLayer).
 * Create Action before changing the MasterPageDescriptors.
 */

class SVX_DLLPUBLIC SdrUndoPageChangeMasterPage final : public SdrUndoPageMasterPage
{
    bool                        mbNewHadMasterPage;
    SdrLayerIDSet                   maNewSet;
    sal_uInt16                  maNewMasterPageNumber;

public:
    SdrUndoPageChangeMasterPage(SdrPage& rChangedPage);

    virtual void Undo() override;
    virtual void Redo() override;

    virtual OUString GetComment() const override;
};


/**
 * The SdrUndoFactory can be set and retrieved from the SdrModel.
 * It is used by the drawing layer implementations to create undo actions.
 * It can be used by applications to create application specific undo actions.
 */
class SVX_DLLPUBLIC SdrUndoFactory
{
public:
    // Shapes
    virtual ~SdrUndoFactory();
    virtual std::unique_ptr<SdrUndoAction> CreateUndoMoveObject( SdrObject& rObject, const Size& rDist );
    virtual std::unique_ptr<SdrUndoAction> CreateUndoGeoObject( SdrObject& rObject );
    virtual std::unique_ptr<SdrUndoAction> CreateUndoAttrObject( SdrObject& rObject, bool bStyleSheet1 = false, bool bSaveText = false );
    virtual std::unique_ptr<SdrUndoAction> CreateUndoRemoveObject( SdrObject& rObject, bool bOrdNumDirect = false);
    virtual std::unique_ptr<SdrUndoAction> CreateUndoInsertObject( SdrObject& rObject, bool bOrdNumDirect = false);
    virtual std::unique_ptr<SdrUndoAction> CreateUndoDeleteObject( SdrObject& rObject, bool bOrdNumDirect = false);
    virtual std::unique_ptr<SdrUndoAction> CreateUndoNewObject( SdrObject& rObject, bool bOrdNumDirect = false);
    virtual std::unique_ptr<SdrUndoAction> CreateUndoCopyObject( SdrObject& rObject, bool bOrdNumDirect = false);

    virtual std::unique_ptr<SdrUndoAction> CreateUndoObjectOrdNum( SdrObject& rObject, sal_uInt32 nOldOrdNum1, sal_uInt32 nNewOrdNum1);

    virtual std::unique_ptr<SdrUndoAction> CreateUndoReplaceObject( SdrObject& rOldObject, SdrObject& rNewObject, bool bOrdNumDirect = false );
    virtual std::unique_ptr<SdrUndoAction> CreateUndoObjectLayerChange( SdrObject& rObject, SdrLayerID aOldLayer, SdrLayerID aNewLayer );
    virtual std::unique_ptr<SdrUndoAction> CreateUndoObjectSetText( SdrObject& rNewObj, sal_Int32 nText );

    // Implement Title/Description Elements UI for Writer text frames, graphics and embedded objects (#i73249#)
    static std::unique_ptr<SdrUndoAction> CreateUndoObjectStrAttr( SdrObject& rObject,
                                                    SdrUndoObjStrAttr::ObjStrAttrType eObjStrAttrType,
                                                    const OUString& sOldStr,
                                                    const OUString& sNewStr );

    // Layer
    virtual std::unique_ptr<SdrUndoAction> CreateUndoNewLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel);
    virtual std::unique_ptr<SdrUndoAction> CreateUndoDeleteLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel);

    // Page
    virtual std::unique_ptr<SdrUndoAction> CreateUndoDeletePage(SdrPage& rPage);
    virtual std::unique_ptr<SdrUndoAction> CreateUndoNewPage(SdrPage& rPage);
    virtual std::unique_ptr<SdrUndoAction> CreateUndoCopyPage(SdrPage& rPage);
    virtual std::unique_ptr<SdrUndoAction> CreateUndoSetPageNum(SdrPage& rNewPg, sal_uInt16 nOldPageNum1, sal_uInt16 nNewPageNum1);

    // Master page
    virtual std::unique_ptr<SdrUndoAction> CreateUndoPageRemoveMasterPage(SdrPage& rChangedPage);
    virtual std::unique_ptr<SdrUndoAction> CreateUndoPageChangeMasterPage(SdrPage& rChangedPage);
};

#endif // INCLUDED_SVX_SVDUNDO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
