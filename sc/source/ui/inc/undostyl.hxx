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



#ifndef SC_UNDOSTYL_HXX
#define SC_UNDOSTYL_HXX

#include <rsc/rscsfx.hxx>
#include "undobase.hxx"

class ScDocShell;
class SfxStyleSheetBase;

//----------------------------------------------------------------------------

class ScStyleSaveData
{
private:
    String          aName;
    String          aParent;
    SfxItemSet*     pItems;

public:
                        ScStyleSaveData();
                        ScStyleSaveData( const ScStyleSaveData& rOther );
                        ~ScStyleSaveData();
    ScStyleSaveData&    operator=( const ScStyleSaveData& rOther );

    void                InitFromStyle( const SfxStyleSheetBase* pSource );

    const String&       GetName() const     { return aName; }
    const String&       GetParent() const   { return aParent; }
    const SfxItemSet*   GetItems() const    { return pItems; }
};

class ScUndoModifyStyle: public ScSimpleUndo
{
private:
    SfxStyleFamily  eFamily;
    ScStyleSaveData aOldData;
    ScStyleSaveData aNewData;

    static void     DoChange( ScDocShell* pDocSh,
                                const String& rName, SfxStyleFamily eStyleFamily,
                                const ScStyleSaveData& rData );

public:
                    ScUndoModifyStyle( ScDocShell* pDocSh,
                                        SfxStyleFamily eFam,
                                        const ScStyleSaveData& rOld,
                                        const ScStyleSaveData& rNew );
    virtual         ~ScUndoModifyStyle();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;
};

class ScUndoApplyPageStyle: public ScSimpleUndo
{
public:
                    ScUndoApplyPageStyle( ScDocShell* pDocSh, const String& rNewStyle );
    virtual         ~ScUndoApplyPageStyle();

    void            AddSheetAction( SCTAB nTab, const String& rOld );

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;

private:
    struct ApplyStyleEntry
    {
        SCTAB           mnTab;
        String          maOldStyle;
        explicit        ApplyStyleEntry( SCTAB nTab, const String& rOldStyle );
    };
    typedef ::std::vector< ApplyStyleEntry > ApplyStyleVec;

    ApplyStyleVec   maEntries;
    String          maNewStyle;
};


#endif

