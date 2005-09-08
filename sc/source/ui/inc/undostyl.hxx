/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: undostyl.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 22:03:30 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_UNDOSTYL_HXX
#define SC_UNDOSTYL_HXX

#ifndef _RSCSFX_HXX
#include <rsc/rscsfx.hxx>
#endif

#ifndef SC_UNDOBASE_HXX
#include "undobase.hxx"
#endif

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
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

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
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

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

