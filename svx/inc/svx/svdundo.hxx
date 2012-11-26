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



#ifndef _SVDUNDO_HXX
#define _SVDUNDO_HXX

#ifndef _SOLAR_HRC
#include <svl/solar.hrc>
#endif
#include <svl/undo.hxx>
#include <tools/contnr.hxx>
#include <tools/gen.hxx>
#include <svx/svdtypes.hxx> // fuer enum RepeatFuncts
#include <svx/svdsob.hxx>
#include "svx/svxdllapi.h"
#include <basegfx/point/b2dpoint.hxx>
#include <vector>
#include <svl/style.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines

class SfxItemSet;
class SfxStyleSheet;
class SdrView;
class SdrModel;
class SdrObject;
class SdrPage;
class SdrObjList;
class SdrLayer;
class SdrLayerAdmin;
class SdrObjGeoData;
class OutlinerParaObject;

//////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrUndoAction : public SfxUndoAction
{
protected:
    SdrModel&                   mrModel;

    SdrUndoAction(SdrModel& rNewMod)
    :   mrModel(rNewMod)
    {
    }

public:
    virtual sal_Bool CanRepeat(SfxRepeatTarget& rView) const;
    virtual void Repeat(SfxRepeatTarget& rView);

    virtual String GetRepeatComment(SfxRepeatTarget& rView) const;
    virtual String GetSdrRepeatComment(SdrView& rView) const;

    virtual bool CanSdrRepeat(SdrView& rView) const;
    virtual void SdrRepeat(SdrView& rView);
};

//////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrUndoGroup : public SdrUndoAction
{
protected:
    typedef ::std::vector< SdrUndoAction* > SdrUndoActionContainerType;
    SdrUndoActionContainerType  maList;

    String                      maComment;
    String                      maObjDescription;
    SdrRepeatFunc               meFunction;

public:
    SdrUndoGroup(SdrModel& rNewMod);
    SdrUndoGroup(SdrModel& rNewMod,const String& rStr);
    virtual ~SdrUndoGroup();

    void Clear();
    sal_uInt32 GetActionCount() const { return maList.size(); }
    SdrUndoAction* GetAction(sal_uInt32 nNum) const;
    void AddAction(SdrUndoAction* pAct);
    void push_front( SdrUndoAction* pAct );

    void SetComment(const String& rStr) { maComment = rStr; }
    void SetObjDescription(const String& rStr) { maObjDescription = rStr; }
    virtual String GetComment() const;
    virtual String GetSdrRepeatComment(SdrView& rView) const;

    virtual void Undo();
    virtual void Redo();

    virtual bool CanSdrRepeat(SdrView& rView) const;
    virtual void SdrRepeat(SdrView& rView);
    void SetRepeatFunction(SdrRepeatFunc eFunc) { meFunction = eFunc; }
    SdrRepeatFunc GetRepeatFunction() const { return meFunction; }
};

//////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrUndoObj : public SdrUndoAction
{
protected:
    SdrObject&                  mrSdrObject;

protected:
    SdrUndoObj(SdrObject& rNewObj);

    void TakeMarkedDescriptionString(sal_uInt16 nStrCacheID, String& rStr, bool bRepeat = false) const;

    static void GetDescriptionStringForObject(const SdrObject& _rForObject, sal_uInt16 nStrCacheID, String& rStr, bool bRepeat = false);

    void ImpShowPageOfThisObject();
};

//////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrUndoAttrObj : public SdrUndoObj
{
protected:
    SfxItemSet*                 mpUndoItemSet;
    SfxItemSet*                 mpRedoItemSet;
    SfxItemSet*                 mpRepeatItemSet;

    rtl::Reference< SfxStyleSheetBase > mxUndoStyleSheet;
    rtl::Reference< SfxStyleSheetBase > mxRedoStyleSheet;

    // Bei Zuweisung von TextItems auf ein Zeichenobjekt mit Text:
    OutlinerParaObject*         mpTextUndoOPO;

    // The text rescue mechanism needs also to be implemented for redo actions.
    OutlinerParaObject*         mpTextRedoOPO;

    // Wenn sich um ein Gruppenobjekt handelt:
    SdrUndoGroup*               mpUndoGroup;

    /// bitfield
    bool                        mbStyleSheet : 1;
    bool                        mbHaveToTakeRedoSet : 1;

    // helper to ensure StyleSheet is in pool (provided by SdrModel from SdrObject)
    void ensureStyleSheetInStyleSheetPool(SfxStyleSheetBasePool& rStyleSheetPool, SfxStyleSheet& rSheet);

public:
    SdrUndoAttrObj(SdrObject& rNewObj, bool bStyleSheet1 = false, bool bSaveText = false);
    virtual ~SdrUndoAttrObj();
    void SetRepeatAttr(const SfxItemSet& rSet);
    virtual void Undo();
    virtual void Redo();

    virtual String GetComment() const;
    virtual String GetSdrRepeatComment(SdrView& rView) const;

    virtual void SdrRepeat(SdrView& rView);
    virtual bool CanSdrRepeat(SdrView& rView) const;
};

//////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrUndoGeoObj : public SdrUndoObj
{
protected:
    SdrObjGeoData*              mpUndoSdrObjGeoData;
    SdrObjGeoData*              mpRedoSdrObjGeoData;
    SdrUndoGroup*               mpUndoGroup;

public:
    SdrUndoGeoObj(SdrObject& rNewObj);
    virtual ~SdrUndoGeoObj();

    virtual void Undo();
    virtual void Redo();

    virtual String GetComment() const;
};

//////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrUndoObjList : public SdrUndoObj
{
protected:
    SdrObjList*                 mpObjList;
    sal_uInt32                  mnOrdNum;

    /// bitfield
    bool                        mbOwner : 1;

    SdrUndoObjList(SdrObject& rNewObj);
    virtual ~SdrUndoObjList();

    bool IsOwner() { return mbOwner; }
    void SetOwner(bool bNew);
};

//////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrUndoRemoveObj : public SdrUndoObjList
{
public:
    SdrUndoRemoveObj(SdrObject& rNewObj)
    :   SdrUndoObjList(rNewObj)
    {
    }

    virtual void Undo();
    virtual void Redo();
};

//////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrUndoInsertObj : public SdrUndoObjList
{
public:
    SdrUndoInsertObj(SdrObject& rNewObj)
    :   SdrUndoObjList(rNewObj)
    {
    }

    virtual void Undo();
    virtual void Redo();
};

//////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrUndoDelObj : public SdrUndoRemoveObj
{
public:
    SdrUndoDelObj(SdrObject& rNewObj)
    :   SdrUndoRemoveObj(rNewObj)
    {
        SetOwner(true);
    }

    virtual void Undo();
    virtual void Redo();

    virtual String GetComment() const;
    virtual String GetSdrRepeatComment(SdrView& rView) const;

    virtual void SdrRepeat(SdrView& rView);
    virtual bool CanSdrRepeat(SdrView& rView) const;
};

//////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrUndoNewObj : public SdrUndoInsertObj
{
public:
    SdrUndoNewObj(SdrObject& rNewObj)
    :   SdrUndoInsertObj(rNewObj)
    {
    }

    virtual void Undo();
    virtual void Redo();

    virtual String GetComment() const;
    static  String GetComment( const SdrObject& _rForObject );
};

//////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrUndoReplaceObj : public SdrUndoObj
{
protected:
    SdrObjList*                 mpObjList;
    sal_uInt32                  mnOrdNum;
    SdrObject&                  mrNewSdrObject;

    /// bitfield
    bool                        mbOldOwner : 1;
    bool                        mbNewOwner : 1;

public:
    SdrUndoReplaceObj(SdrObject& rOldObj1, SdrObject& rNewObj1);
    virtual ~SdrUndoReplaceObj();

    virtual void Undo();
    virtual void Redo();

    bool IsNewOwner() { return mbNewOwner; }
    void SetNewOwner(bool bNew);

    bool IsOldOwner() { return mbOldOwner; }
    void SetOldOwner(bool bNew);
};

//////////////////////////////////////////////////////////////////////////////

class SdrUndoCopyObj : public SdrUndoNewObj
{
public:
    SdrUndoCopyObj(SdrObject& rNewObj)
    :   SdrUndoNewObj(rNewObj)
    {
    }

    virtual String GetComment() const;
};

//////////////////////////////////////////////////////////////////////////////

class SdrUndoObjOrdNum : public SdrUndoObj
{
protected:
    sal_uInt32                      mnOldOrdNum;
    sal_uInt32                      mnNewOrdNum;

public:
    SdrUndoObjOrdNum(SdrObject& rNewObj, sal_uInt32 nOldOrdNum1, sal_uInt32 nNewOrdNum1);

    virtual void Undo();
    virtual void Redo();

    virtual String   GetComment() const;
};

//////////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrUndoObjSetText : public SdrUndoObj
{
protected:
    OutlinerParaObject*         mpOldText;
    OutlinerParaObject*         mpNewText;
    sal_Int32                   mnText;

    /// bitfield
    bool                        mbNewTextAvailable : 1;
    bool                        mbEmptyPresObj : 1;

public:
    SdrUndoObjSetText(SdrObject& rNewObj, sal_Int32 nText );
    virtual ~SdrUndoObjSetText();

    bool IsDifferent() const { return mpOldText != mpNewText; }
    void AfterSetText();

    virtual void Undo();
    virtual void Redo();

    virtual String GetComment() const;
    virtual String GetSdrRepeatComment(SdrView& rView) const;

    virtual void SdrRepeat(SdrView& rView);
    virtual bool CanSdrRepeat(SdrView& rView) const;
};

//////////////////////////////////////////////////////////////////////////////

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
    const String msOldStr;
    const String msNewStr;

public:
    SdrUndoObjStrAttr( SdrObject& rNewObj, const ObjStrAttrType eObjStrAttr, const String& sOldStr, const String& sNewStr);

    virtual void Undo();
    virtual void Redo();

    virtual String GetComment() const;
};

//////////////////////////////////////////////////////////////////////////////

class SdrUndoLayer : public SdrUndoAction
{
protected:
    SdrLayer*                   mpLayer;
    SdrLayerAdmin&              mrLayerAdmin;
    sal_uInt16                  mnLayerNum;

    /// bitfield
    bool                        mbItsMine : 1;

    SdrUndoLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel);
    virtual ~SdrUndoLayer();
};

//////////////////////////////////////////////////////////////////////////////

class SdrUndoNewLayer : public SdrUndoLayer
{
public:
    SdrUndoNewLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel)
    :   SdrUndoLayer(nLayerNum, rNewLayerAdmin, rNewModel)
    {
    }

    virtual void Undo();
    virtual void Redo();

    virtual String GetComment() const;
};

//////////////////////////////////////////////////////////////////////////////

class SdrUndoDelLayer : public SdrUndoLayer
{
public:
    SdrUndoDelLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel)
    :   SdrUndoLayer(nLayerNum, rNewLayerAdmin, rNewModel)
{
        mbItsMine = true;
    }

    virtual void Undo();
    virtual void Redo();

    virtual String GetComment() const;
};

//////////////////////////////////////////////////////////////////////////////

//class SdrUndoMoveLayer : public SdrUndoLayer
//{
//private:
//  sal_uInt16                      mnNewLayerPos;
//
//public:
//  SdrUndoMoveLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel, sal_uInt16 nNeuPos1)
//  :   SdrUndoLayer(nLayerNum, rNewLayerAdmin, rNewModel),
//      mnNewLayerPos(nNeuPos1)
//  {
//  }
//
//  virtual void Undo();
//  virtual void Redo();
//
//  virtual String GetComment() const;
//};

//////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrUndoPage : public SdrUndoAction
{
protected:
    SdrPage&                    mrPage;

    void ImpInsertPage(sal_uInt32 nNum);
    void ImpRemovePage(sal_uInt32 nNum);
    void ImpMovePage(sal_uInt32 nOldNum, sal_uInt32 nNewNum);

protected:
    SdrUndoPage(SdrPage& rNewPg);

    void TakeMarkedDescriptionString(sal_uInt16 nStrCacheID, String& rStr, sal_uInt16 n = 0, bool bRepeat = false) const;
};

//////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrUndoPageList : public SdrUndoPage
{
protected:
    sal_uInt32              mnPageNum;

    /// bitfield
    bool                    mbItsMine : 1;

protected:
    SdrUndoPageList(SdrPage& rNewPg);
    virtual ~SdrUndoPageList();
};

//////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrUndoDelPage : public SdrUndoPageList
{
private:
    SdrUndoGroup*               mpUndoGroup;

public:
    SdrUndoDelPage(SdrPage& rNewPg);
    virtual ~SdrUndoDelPage();

    virtual void Undo();
    virtual void Redo();

    virtual String GetComment() const;
    virtual String GetSdrRepeatComment(SdrView& rView) const;

    virtual void SdrRepeat(SdrView& rView);
    virtual bool CanSdrRepeat(SdrView& rView) const;
};

//////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrUndoNewPage : public SdrUndoPageList
{
public:
    SdrUndoNewPage(SdrPage& rNewPg)
    :   SdrUndoPageList(rNewPg)
    {
    }

    virtual void Undo();
    virtual void Redo();

    virtual String GetComment() const;
};

//////////////////////////////////////////////////////////////////////////////

class SdrUndoCopyPage : public SdrUndoNewPage
{
public:
    SdrUndoCopyPage(SdrPage& rNewPg)
    :   SdrUndoNewPage(rNewPg)
    {
    }

    virtual String GetComment() const;
    virtual String GetSdrRepeatComment(SdrView& rView) const;

    virtual void SdrRepeat(SdrView& rView);
    virtual bool CanSdrRepeat(SdrView& rView) const;
};

//////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrUndoSetPageNum : public SdrUndoPage
{
protected:
    sal_uInt32                      mnOldPageNum;
    sal_uInt32                      mnNewPageNum;

public:
    SdrUndoSetPageNum(SdrPage& rNewPg, sal_uInt32 nOldPageNum1, sal_uInt32 nNewPageNum1)
    :   SdrUndoPage(rNewPg),
        mnOldPageNum(nOldPageNum1),
        mnNewPageNum(nNewPageNum1)
    {
    }

    virtual void Undo();
    virtual void Redo();

    virtual String GetComment() const;
};

//////////////////////////////////////////////////////////////////////////////

class SdrUndoPageMasterPage : public SdrUndoPage
{
protected:
    SetOfByte                   maOldSet;
    sal_uInt32                  maOldMasterPageNumber;

    /// bitfield
    bool                        mbOldHadMasterPage : 1;

    SdrUndoPageMasterPage(SdrPage& rChangedPage);

public:
    SVX_DLLPUBLIC virtual ~SdrUndoPageMasterPage();
};

//////////////////////////////////////////////////////////////////////////////

class SdrUndoPageRemoveMasterPage : public SdrUndoPageMasterPage
{
public:
    SdrUndoPageRemoveMasterPage(SdrPage& rChangedPage);

    virtual void Undo();
    virtual void Redo();

    virtual String GetComment() const;
};

//////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrUndoPageChangeMasterPage : public SdrUndoPageMasterPage
{
protected:
    SetOfByte                   maNewSet;
    sal_uInt16                  maNewMasterPageNumber;

    /// bitfield
    bool                        mbNewHadMasterPage : 1;

public:
    SdrUndoPageChangeMasterPage(SdrPage& rChangedPage);

    virtual void Undo();
    virtual void Redo();

    virtual String GetComment() const;
};

///////////////////////////////////////////////////////////////////////

/** the SdrUndoFactory can be set and retrieved from the SdrModel.
    It is used by the drawing layer implementations to create undo actions.
    It can be used by applications to create application specific undo actions.
*/

class SVX_DLLPUBLIC SdrUndoFactory
{
public:
    // shapes
    virtual ~SdrUndoFactory();
    virtual SdrUndoAction* CreateUndoGeoObject( SdrObject& rObject );
    virtual SdrUndoAction* CreateUndoAttrObject( SdrObject& rObject, bool bStyleSheet1 = false, bool bSaveText = false );
    virtual SdrUndoAction* CreateUndoRemoveObject( SdrObject& rObject);
    virtual SdrUndoAction* CreateUndoInsertObject( SdrObject& rObject);
    virtual SdrUndoAction* CreateUndoDeleteObject( SdrObject& rObject);
    virtual SdrUndoAction* CreateUndoNewObject( SdrObject& rObject);
    virtual SdrUndoAction* CreateUndoCopyObject( SdrObject& rObject);

    virtual SdrUndoAction* CreateUndoObjectOrdNum( SdrObject& rObject, sal_uInt32 nOldOrdNum1, sal_uInt32 nNewOrdNum1);

    virtual SdrUndoAction* CreateUndoReplaceObject( SdrObject& rOldObject, SdrObject& rNewObject );
    virtual SdrUndoAction* CreateUndoObjectLayerChange( SdrObject& rObject, SdrLayerID aOldLayer, SdrLayerID aNewLayer );
    virtual SdrUndoAction* CreateUndoObjectSetText( SdrObject& rNewObj, sal_Int32 nText );

    virtual SdrUndoAction* CreateUndoObjectStrAttr( SdrObject& rObject, SdrUndoObjStrAttr::ObjStrAttrType eObjStrAttrType, String sOldStr, String sNewStr );

    // layer
    virtual SdrUndoAction* CreateUndoNewLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel);
    virtual SdrUndoAction* CreateUndoDeleteLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel);
//  virtual SdrUndoAction* CreateUndoMoveLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel, sal_uInt16 nNeuPos1);

    // page
    virtual SdrUndoAction*  CreateUndoDeletePage(SdrPage& rPage);
    virtual SdrUndoAction* CreateUndoNewPage(SdrPage& rPage);
    virtual SdrUndoAction* CreateUndoCopyPage(SdrPage& rPage);
    virtual SdrUndoAction* CreateUndoSetPageNum(SdrPage& rNewPg, sal_uInt16 nOldPageNum1, sal_uInt16 nNewPageNum1);

    // master page
    virtual SdrUndoAction* CreateUndoPageRemoveMasterPage(SdrPage& rChangedPage);
    virtual SdrUndoAction* CreateUndoPageChangeMasterPage(SdrPage& rChangedPage);
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDUNDO_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
