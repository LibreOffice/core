/*************************************************************************
 *
 *  $RCSfile: undostyl.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:02 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
private:
    USHORT          nTab;
    String          aOldStyle;
    String          aNewStyle;

public:
                    TYPEINFO();
                    ScUndoApplyPageStyle( ScDocShell* pDocSh, USHORT nT,
                                        const String& rOld, const String& rNew );
    virtual         ~ScUndoApplyPageStyle();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual BOOL    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String  GetComment() const;
};


#endif

