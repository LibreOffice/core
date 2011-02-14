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
                    TYPEINFO();
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
                    TYPEINFO();
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

