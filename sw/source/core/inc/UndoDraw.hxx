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



#ifndef SW_UNDO_DRAW_HXX
#define SW_UNDO_DRAW_HXX

#include <undobj.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdobj.hxx>

struct SwUndoGroupObjImpl;
class SdrMark;
class SdrObject;
class SdrObjGroup;
class SdrUndoAction;
class SwDrawFrmFmt;



//--------------------------------------------------------------------
// ----------- Undo for Draw Objects ---------------------------------

class SwSdrUndo : public SwUndo
{
    SdrUndoAction* pSdrUndo;
    SdrObjectVector maSelectedSdrObjectVector; // all selected SdrObjects

public:
    SwSdrUndo( SdrUndoAction* , const SdrObjectVector& rSelectedSdrObjectVector );

    virtual ~SwSdrUndo();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );

    String GetComment() const;
};

class SwUndoDrawGroup : public SwUndo
{
    SwUndoGroupObjImpl* pObjArr;
    sal_uInt32 nSize;
    bool bDelFmt;

public:
    SwUndoDrawGroup( sal_uInt32 nCnt );

    virtual ~SwUndoDrawGroup();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );

    void AddObj( sal_uInt32 nPos, SwDrawFrmFmt*, SdrObject* );
    void SetGroupFmt( SwDrawFrmFmt* );
};

// --> OD 2006-11-01 #130889#
// Action "ungroup drawing object" is now splitted into three parts - see
// method <SwDoc::UnGroupSelection(..)>:
// - creation for <SwDrawFrmFmt> instances for the group members of the
//   selected group objects
// - intrinsic ungroup of the selected group objects
// - creation of <SwDrawContact> instances for the former group members and
//   connection to the Writer layout.
// Thus, two undo actions (instances of <SwUndo>) are needed:
// - Existing class <SwUndoDrawUnGroup> takes over the part for the formats.
// - New class <SwUndoDrawUnGroupConnectToLayout> takes over the part for
//   contact object.
class SwUndoDrawUnGroup : public SwUndo
{
    SwUndoGroupObjImpl* pObjArr;
    sal_uInt32 nSize;
    bool bDelFmt;

public:
    SwUndoDrawUnGroup( SdrObjGroup* );

    virtual ~SwUndoDrawUnGroup();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );

    void AddObj( sal_uInt32 nPos, SwDrawFrmFmt* );
};

// --> OD 2006-11-01 #130889#
class SwUndoDrawUnGroupConnectToLayout : public SwUndo
{
    private:
        std::vector< std::pair< SwDrawFrmFmt*, SdrObject* > > aDrawFmtsAndObjs;

    public:
        SwUndoDrawUnGroupConnectToLayout();

        virtual ~SwUndoDrawUnGroupConnectToLayout();

        virtual void UndoImpl( ::sw::UndoRedoContext & );
        virtual void RedoImpl( ::sw::UndoRedoContext & );

        void AddFmtAndObj( SwDrawFrmFmt* pDrawFrmFmt,
                           SdrObject* pDrawObject );
};
// <--


class SwUndoDrawDelete : public SwUndo
{
    SwUndoGroupObjImpl* pObjArr;
    SdrObjectVector maSdrObjectVector;  // MarkList for all selected SdrObjects
    sal_uInt32 nSize;
    bool bDelFmt;

public:
    SwUndoDrawDelete( sal_uInt32 nCnt );

    virtual ~SwUndoDrawDelete();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );

    void AddObj( sal_uInt32 nPos, SwDrawFrmFmt*, const SdrObject& );
};

#endif // SW_UNDO_DRAW_HXX

