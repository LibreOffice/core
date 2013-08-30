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

#ifndef _SVDUNDO_HXX
#define _SVDUNDO_HXX

#include <svl/solar.hrc>
#include <svl/undo.hxx>
#include <svl/style.hxx>
#include <tools/gen.hxx>
#include <svx/svdtypes.hxx> // for enum RepeatFuncts
#include <svx/svdsob.hxx>
#include "svx/svxdllapi.h"

class SfxItemSet;
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
    SdrModel&                   rMod;

protected:
    SdrUndoAction(SdrModel& rNewMod)
    :   rMod(rNewMod)
    {}

public:
    TYPEINFO();
    virtual ~SdrUndoAction();

    virtual bool CanRepeat(SfxRepeatTarget& rView) const;
    virtual void Repeat(SfxRepeatTarget& rView);

    virtual OUString GetRepeatComment(SfxRepeatTarget& rView) const;
    virtual OUString GetSdrRepeatComment(SdrView& rView) const;

    virtual bool CanSdrRepeat(SdrView& rView) const;
    virtual void SdrRepeat(SdrView& rView);
};

/**
 * Aggregation of arbitrarily many UndoActions to a single UndoAction.
 * Undo() executes the single UndoActions backwards (LIFO, first the last
 * ones attached), Redo() executes the UndoActions in the order they were
 * added (FIFO).
 */

class SVX_DLLPUBLIC SdrUndoGroup : public SdrUndoAction
{
protected:
    std::vector<SdrUndoAction*> aBuf;

    // No expanded description of the Action (contains %O)
    OUString                    aComment;
    OUString                    aObjDescription;

    SdrRepeatFunc               eFunction;

public:
    SdrUndoGroup(SdrModel& rNewMod);
    virtual ~SdrUndoGroup();

    void Clear();
    sal_uIntPtr GetActionCount() const { return aBuf.size(); }
    SdrUndoAction* GetAction(sal_uIntPtr nNum) const { return aBuf[nNum]; }
    void AddAction(SdrUndoAction* pAct);

    void SetComment(const OUString& rStr) { aComment=rStr; }
    void SetObjDescription(const OUString& rStr) { aObjDescription=rStr; }
    virtual OUString GetComment() const;
    virtual OUString GetSdrRepeatComment(SdrView& rView) const;

    virtual void Undo();
    virtual void Redo();

    virtual bool CanSdrRepeat(SdrView& rView) const;
    virtual void SdrRepeat(SdrView& rView);
    void SetRepeatFunction(SdrRepeatFunc eFunc) { eFunction=eFunc; }
    SdrRepeatFunc GetRepeatFunction() const { return eFunction; }
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

    void ImpTakeDescriptionStr(sal_uInt16 nStrCacheID, OUString& rStr, bool bRepeat = false) const;

    static SAL_WARN_UNUSED_RESULT OUString GetDescriptionStringForObject( const SdrObject& _rForObject, sal_uInt16 nStrCacheID, bool bRepeat = false );

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
    SfxItemSet*                 pUndoSet;
    SfxItemSet*                 pRedoSet;
    SfxItemSet*                 pRepeatSet;

    // FIXME: Or should we better remember the StyleSheetNames?
    rtl::Reference< SfxStyleSheetBase > mxUndoStyleSheet;
    rtl::Reference< SfxStyleSheetBase > mxRedoStyleSheet;
    bool                        bStyleSheet;
    bool                        bHaveToTakeRedoSet;

    // When assigning TextItems to a drawing object with text:
    OutlinerParaObject*         pTextUndo;
    // #i8508#
    // The text rescue mechanism needs also to be implemented for redo actions.
    OutlinerParaObject*         pTextRedo;

    // If we have a group object:
    SdrUndoGroup*               pUndoGroup;

    // Helper to ensure StyleSheet is in pool (provided by SdrModel from SdrObject)
    void ensureStyleSheetInStyleSheetPool(SfxStyleSheetBasePool& rStyleSheetPool, SfxStyleSheet& rSheet);

public:
    SdrUndoAttrObj(SdrObject& rNewObj, bool bStyleSheet1 = false, bool bSaveText = false);
    virtual ~SdrUndoAttrObj();
    virtual void Undo();
    virtual void Redo();

    virtual OUString GetComment() const;
    virtual OUString GetSdrRepeatComment(SdrView& rView) const;

    virtual void SdrRepeat(SdrView& rView);
    virtual bool CanSdrRepeat(SdrView& rView) const;
};

/**
 * Only moving of an object.
 * Create Action right before moving.
 */

class SVX_DLLPUBLIC SdrUndoMoveObj : public SdrUndoObj
{
protected:
    Size                        aDistance;     // Distance by which we move

public:
    SdrUndoMoveObj(SdrObject& rNewObj): SdrUndoObj(rNewObj) {}
    SdrUndoMoveObj(SdrObject& rNewObj, const Size& rDist): SdrUndoObj(rNewObj),aDistance(rDist) {}
    virtual ~SdrUndoMoveObj();

    void SetDistance(const Size& rDist) { aDistance=rDist; }
    const Size& GetDistance() const { return aDistance; }

    virtual void Undo();
    virtual void Redo();

    virtual OUString GetComment() const;
    virtual OUString GetSdrRepeatComment(SdrView& rView) const;

    virtual void SdrRepeat(SdrView& rView);
    virtual bool CanSdrRepeat(SdrView& rView) const;
};

/**
 * Changing the geometry of an object.
 * Create Action right before the geometric transformation.
 */

class SVX_DLLPUBLIC SdrUndoGeoObj : public SdrUndoObj
{
protected:
    SdrObjGeoData*              pUndoGeo;
    SdrObjGeoData*              pRedoGeo;
    // If we have a group object:
    SdrUndoGroup*               pUndoGroup;

public:
    SdrUndoGeoObj(SdrObject& rNewObj);
    virtual ~SdrUndoGeoObj();

    virtual void Undo();
    virtual void Redo();

    virtual OUString GetComment() const;
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
    SdrView*                    pView;      // To be able to re-create the selection for a
    SdrPageView*                pPageView;  // for a ObjDel, Undo
    sal_uInt32                      nOrdNum;
    // It's possible that the object is re-assigned during a Undo/Redo.
    // The object is deleted in the dtor, if bOwner==TRUE
    ObjListListener*            m_pListener;

protected:
    SdrUndoObjList(SdrObject& rNewObj, bool bOrdNumDirect = false);
    virtual ~SdrUndoObjList();

    void SetView(SdrView* pView1, SdrPageView* pPageView1) { pView=pView1; pPageView=pPageView1; }
    bool IsOwner() { return bOwner; }
    void SetOwner(bool bNew);

private:
    sal_uInt32 GetOrdNum() const;
    void SetOrdNum(sal_uInt32 nOrdNum_);
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

    virtual void Undo();
    virtual void Redo();
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

    virtual void Undo();
    virtual void Redo();
};

/**
 * Deleting an Object.
 * Create Action before removing from ObjList.
 */

class SVX_DLLPUBLIC SdrUndoDelObj : public SdrUndoRemoveObj
{
public:
    SdrUndoDelObj(SdrObject& rNewObj, bool bOrdNumDirect = false)
    :   SdrUndoRemoveObj(rNewObj,bOrdNumDirect) { SetOwner(sal_True); }

    virtual void Undo();
    virtual void Redo();

    virtual OUString GetComment() const;
    virtual OUString GetSdrRepeatComment(SdrView& rView) const;

    virtual void SdrRepeat(SdrView& rView);
    virtual bool CanSdrRepeat(SdrView& rView) const;
};

/**
 * Inserting a NEW Object.
 * Create Actio after insertion into the ObjList.
 */

class SVX_DLLPUBLIC SdrUndoNewObj : public SdrUndoInsertObj
{
public:
    SdrUndoNewObj(SdrObject& rNewObj, bool bOrdNumDirect = false)
    :   SdrUndoInsertObj(rNewObj,bOrdNumDirect) {}

    virtual void Undo();
    virtual void Redo();

    virtual OUString GetComment() const;

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

protected:
    SdrObjList*                 pObjList;
    sal_uInt32                      nOrdNum;
    SdrObject*                  pNewObj;

public:
    SdrUndoReplaceObj(SdrObject& rOldObj1, SdrObject& rNewObj1, bool bOrdNumDirect = false);
    virtual ~SdrUndoReplaceObj();

    virtual void Undo();
    virtual void Redo();

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
    SdrUndoCopyObj(SdrObject& rNewObj, bool bOrdNumDirect = false)
    :   SdrUndoNewObj(rNewObj,bOrdNumDirect) {}

    virtual OUString GetComment() const;
};

class SdrUndoObjOrdNum : public SdrUndoObj
{
protected:
    sal_uInt32                      nOldOrdNum;
    sal_uInt32                      nNewOrdNum;

public:
    SdrUndoObjOrdNum(SdrObject& rNewObj, sal_uInt32 nOldOrdNum1, sal_uInt32 nNewOrdNum1);

    virtual void Undo();
    virtual void Redo();

    virtual OUString GetComment() const;
};

//////////////////////////////////////////////////////////////////////////////
// #i11702#

class SVX_DLLPUBLIC SdrUndoObjectLayerChange : public SdrUndoObj
{
protected:
    SdrLayerID                  maOldLayer;
    SdrLayerID                  maNewLayer;

public:
    SdrUndoObjectLayerChange(SdrObject& rObj, SdrLayerID aOldLayer, SdrLayerID aNewLayer);

    virtual void Undo();
    virtual void Redo();
};

class SVX_DLLPUBLIC SdrUndoObjSetText : public SdrUndoObj
{
protected:
    OutlinerParaObject*         pOldText;
    OutlinerParaObject*         pNewText;
    bool                        bNewTextAvailable;
    sal_Bool                        bEmptyPresObj;
    sal_Int32                   mnText;

public:
    SdrUndoObjSetText(SdrObject& rNewObj, sal_Int32 nText );
    virtual ~SdrUndoObjSetText();

    bool IsDifferent() const { return pOldText!=pNewText; }
    void AfterSetText();

    virtual void Undo();
    virtual void Redo();

    virtual OUString GetComment() const;
    virtual OUString GetSdrRepeatComment(SdrView& rView) const;

    virtual void SdrRepeat(SdrView& rView);
    virtual bool CanSdrRepeat(SdrView& rView) const;
};

/**
 * Implement Title/Description Elements UI for Writer
 * text frames, graphics and embedded objects (#i73249#)
 */
class SdrUndoObjStrAttr : public SdrUndoObj
{
public:
    enum ObjStrAttrType
    {
        OBJ_NAME,
        OBJ_TITLE,
        OBJ_DESCRIPTION
    };

protected:
    const ObjStrAttrType meObjStrAttr;
    const OUString msOldStr;
    const OUString msNewStr;

public:
    SdrUndoObjStrAttr( SdrObject& rNewObj,
                       const ObjStrAttrType eObjStrAttr,
                       const OUString& sOldStr,
                       const OUString& sNewStr);

    virtual void Undo();
    virtual void Redo();

    virtual OUString GetComment() const;
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
    SdrLayer*                   pLayer;
    SdrLayerAdmin*              pLayerAdmin;
    sal_uInt16                      nNum;
    bool                        bItsMine;

protected:
    SdrUndoLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel);
    virtual ~SdrUndoLayer();
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

    virtual void Undo();
    virtual void Redo();

    virtual OUString GetComment() const;
};

/**
 * Deleting a Layer.
 * Create Action before the Remove.
 */

class SdrUndoDelLayer : public SdrUndoLayer
{
public:
    SdrUndoDelLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel)
    : SdrUndoLayer(nLayerNum,rNewLayerAdmin,rNewModel) { bItsMine=sal_True; }

    virtual void Undo();
    virtual void Redo();

    virtual OUString GetComment() const;
};

/**
 * Moving a Layer.
 * Create Action before the Move.
 */

class SdrUndoMoveLayer : public SdrUndoLayer
{
    sal_uInt16                      nNeuPos;

public:
    SdrUndoMoveLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel, sal_uInt16 nNeuPos1)
    :   SdrUndoLayer(nLayerNum,rNewLayerAdmin,rNewModel), nNeuPos(nNeuPos1) {}

    virtual void Undo();
    virtual void Redo();

    virtual OUString GetComment() const;
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

    void ImpTakeDescriptionStr(sal_uInt16 nStrCacheID, OUString& rStr, sal_uInt16 n=0, bool bRepeat = false) const;
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
    virtual ~SdrUndoPageList();
};

/**
 * Deleting a Page.
 * Create Action before removing from the List.
 */

class SVX_DLLPUBLIC SdrUndoDelPage : public SdrUndoPageList
{
    // When deleting a MasterPage, we remember all relations of the
    // Character Page with the MasterPage in this UndoGroup.
    SdrUndoGroup*               pUndoGroup;

public:
    SdrUndoDelPage(SdrPage& rNewPg);
    virtual ~SdrUndoDelPage();

    virtual void Undo();
    virtual void Redo();

    virtual OUString GetComment() const;
    virtual OUString GetSdrRepeatComment(SdrView& rView) const;

    virtual void SdrRepeat(SdrView& rView);
    virtual bool CanSdrRepeat(SdrView& rView) const;
};

/**
 * Inserting a new Page.
 * Create Action after inserting into the List.
 */

class SVX_DLLPUBLIC SdrUndoNewPage : public SdrUndoPageList
{
public:
    SdrUndoNewPage(SdrPage& rNewPg): SdrUndoPageList(rNewPg) {}

    virtual void Undo();
    virtual void Redo();

    virtual OUString GetComment() const;
};

/**
 * Copying a Page.
 * Create Action after inserting into the List.
 */

class SdrUndoCopyPage : public SdrUndoNewPage
{
public:
    SdrUndoCopyPage(SdrPage& rNewPg): SdrUndoNewPage(rNewPg) {}

    virtual OUString GetComment() const;
    virtual OUString GetSdrRepeatComment(SdrView& rView) const;

    virtual void SdrRepeat(SdrView& rView);
    virtual bool CanSdrRepeat(SdrView& rView) const;
};

/**
 * Moving a Page within the List.
 * Create Action before moving the Page.
 */

class SVX_DLLPUBLIC SdrUndoSetPageNum : public SdrUndoPage
{
protected:
    sal_uInt16                      nOldPageNum;
    sal_uInt16                      nNewPageNum;

public:
    SdrUndoSetPageNum(SdrPage& rNewPg, sal_uInt16 nOldPageNum1, sal_uInt16 nNewPageNum1)
    :   SdrUndoPage(rNewPg),nOldPageNum(nOldPageNum1),nNewPageNum(nNewPageNum1) {}

    virtual void Undo();
    virtual void Redo();

    virtual OUString GetComment() const;
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
    sal_Bool                    mbOldHadMasterPage;
    SetOfByte                   maOldSet;
    sal_uInt16                  maOldMasterPageNumber;

protected:
    SdrUndoPageMasterPage(SdrPage& rChangedPage);

public:
    SVX_DLLPUBLIC virtual ~SdrUndoPageMasterPage();
};

/**
 * Removal of a MasterPage from a Character Page.
 * Create Action before removing the MasterPageDescriptor.
 */

class SdrUndoPageRemoveMasterPage : public SdrUndoPageMasterPage
{
public:
    SdrUndoPageRemoveMasterPage(SdrPage& rChangedPage);

    virtual void Undo();
    virtual void Redo();

    virtual OUString GetComment() const;
};

/**
 * Changing the MasterPageDescriptor (e.g. change of the VisibleLayer).
 * Create Action before changing the MasterPageDescriptors.
 */

class SVX_DLLPUBLIC SdrUndoPageChangeMasterPage : public SdrUndoPageMasterPage
{
protected:
    sal_Bool                    mbNewHadMasterPage;
    SetOfByte                   maNewSet;
    sal_uInt16                  maNewMasterPageNumber;

public:
    SdrUndoPageChangeMasterPage(SdrPage& rChangedPage);

    virtual void Undo();
    virtual void Redo();

    virtual OUString GetComment() const;
};

///////////////////////////////////////////////////////////////////////

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
    virtual SdrUndoAction* CreateUndoMoveObject( SdrObject& rObject );
    virtual SdrUndoAction* CreateUndoMoveObject( SdrObject& rObject, const Size& rDist );
    virtual SdrUndoAction* CreateUndoGeoObject( SdrObject& rObject );
    virtual SdrUndoAction* CreateUndoAttrObject( SdrObject& rObject, bool bStyleSheet1 = false, bool bSaveText = false );
    virtual SdrUndoAction* CreateUndoRemoveObject( SdrObject& rObject, bool bOrdNumDirect = false);
    virtual SdrUndoAction* CreateUndoInsertObject( SdrObject& rObject, bool bOrdNumDirect = false);
    virtual SdrUndoAction* CreateUndoDeleteObject( SdrObject& rObject, bool bOrdNumDirect = false);
    virtual SdrUndoAction* CreateUndoNewObject( SdrObject& rObject, bool bOrdNumDirect = false);
    virtual SdrUndoAction* CreateUndoCopyObject( SdrObject& rObject, bool bOrdNumDirect = false);

    virtual SdrUndoAction* CreateUndoObjectOrdNum( SdrObject& rObject, sal_uInt32 nOldOrdNum1, sal_uInt32 nNewOrdNum1);

    virtual SdrUndoAction* CreateUndoReplaceObject( SdrObject& rOldObject, SdrObject& rNewObject, bool bOrdNumDirect = false );
    virtual SdrUndoAction* CreateUndoObjectLayerChange( SdrObject& rObject, SdrLayerID aOldLayer, SdrLayerID aNewLayer );
    virtual SdrUndoAction* CreateUndoObjectSetText( SdrObject& rNewObj, sal_Int32 nText );

    // Implement Title/Description Elements UI for Writer text frames, graphics and embedded objects (#i73249#)
    virtual SdrUndoAction* CreateUndoObjectStrAttr( SdrObject& rObject,
                                                    SdrUndoObjStrAttr::ObjStrAttrType eObjStrAttrType,
                                                    const OUString& sOldStr,
                                                    const OUString& sNewStr );

    // Layer
    virtual SdrUndoAction* CreateUndoNewLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel);
    virtual SdrUndoAction* CreateUndoDeleteLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel);
    virtual SdrUndoAction* CreateUndoMoveLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel, sal_uInt16 nNeuPos1);

    // Page
    virtual SdrUndoAction*  CreateUndoDeletePage(SdrPage& rPage);
    virtual SdrUndoAction* CreateUndoNewPage(SdrPage& rPage);
    virtual SdrUndoAction* CreateUndoCopyPage(SdrPage& rPage);
    virtual SdrUndoAction* CreateUndoSetPageNum(SdrPage& rNewPg, sal_uInt16 nOldPageNum1, sal_uInt16 nNewPageNum1);

    // Master page
    virtual SdrUndoAction* CreateUndoPageRemoveMasterPage(SdrPage& rChangedPage);
    virtual SdrUndoAction* CreateUndoPageChangeMasterPage(SdrPage& rChangedPage);
};

#endif //_SVDUNDO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
