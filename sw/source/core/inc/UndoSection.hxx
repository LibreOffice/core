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

#ifndef SW_UNDO_SECTION_HXX
#define SW_UNDO_SECTION_HXX

#include <undobj.hxx>

#include <memory>

#include <swdllapi.h>


class SfxItemSet;
class SwTxtNode;
class SwSectionData;
class SwSectionFmt;
class SwTOXBase;


//------------ Undo of Insert-/Delete-Section ----------------------

class SwUndoInsSection : public SwUndo, private SwUndRng
{
private:
    const ::std::auto_ptr<SwSectionData> m_pSectionData;
    const ::std::auto_ptr<SwTOXBase> m_pTOXBase; /// set iff section is TOX
    const ::std::auto_ptr<SfxItemSet> m_pAttrSet;
    ::std::auto_ptr<SwHistory> m_pHistory;
    ::std::auto_ptr<SwRedlineData> m_pRedlData;
    sal_uLong m_nSectionNodePos;
    bool m_bSplitAtStart : 1;
    bool m_bSplitAtEnd : 1;
    bool m_bUpdateFtn : 1;

    void Join( SwDoc& rDoc, sal_uLong nNode );

public:
    SwUndoInsSection(SwPaM const&, SwSectionData const&,
        SfxItemSet const*const pSet, SwTOXBase const*const pTOXBase);

    virtual ~SwUndoInsSection();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

    void SetSectNdPos(sal_uLong const nPos)     { m_nSectionNodePos = nPos; }
    void SaveSplitNode(SwTxtNode *const pTxtNd, bool const bAtStart);
    void SetUpdtFtnFlag(bool const bFlag)   { m_bUpdateFtn = bFlag; }
};

SW_DLLPRIVATE SwUndo * MakeUndoDelSection(SwSectionFmt const&);

SW_DLLPRIVATE SwUndo * MakeUndoUpdateSection(SwSectionFmt const&, bool const);

#endif // SW_UNDO_SECTION_HXX

