/*************************************************************************
 *
 *  $RCSfile: scuitphfedit.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-23 14:25:46 $
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

#ifndef SCUI_TPHFEDIT_HXX
#define SCUI_TPHFEDIT_HXX

#include "tphfedit.hxx"

//===================================================================
enum ScHFEntryId
{
    eNoneEntry  ,
    ePageEntry  ,
    ePagesEntry ,
    eSheetEntry ,
    eConfidentialEntry ,
    eFileNamePageEntry ,
    eExtFileNameEntry ,
    ePageSheetEntry  ,
    ePageFileNameEntry  ,
    ePageExtFileNameEntry  ,
    eUserNameEntry  ,
    eCreatedByEntry ,
    eEntryCount
};

class ScHeaderEditEngine;
class ScPatternAttr;
class EditView;
class EditTextObject;
class SvxFieldItem;
class ScAccessibleEditObject;


class ScHFEditPage : public SfxTabPage
{
public:
    virtual BOOL    FillItemSet ( SfxItemSet& rCoreSet );
    virtual void    Reset       ( const SfxItemSet& rCoreSet );

    void            SetNumType(SvxNumType eNumType);
    void            ClearTextAreas();

protected:
                ScHFEditPage( Window*           pParent,
                              USHORT            nResId,
                              const SfxItemSet& rCoreSet,
                              USHORT            nWhich, bool bHeader );
    virtual     ~ScHFEditPage();

private:
    FixedText       aFtLeft;
    ScEditWindow    aWndLeft;
    FixedText       aFtCenter;
    ScEditWindow    aWndCenter;
    FixedText       aFtRight;
    ScEditWindow    aWndRight;
    FixedText       maFtDefinedHF;
        ListBox                 maLbDefined;
    FixedText       maFtCustomHF;
    ImageButton     aBtnText;
    ScExtIButton    aBtnFile;
    ImageButton     aBtnTable;
    ImageButton     aBtnPage;
    ImageButton     aBtnLastPage;
    ImageButton     aBtnDate;
    ImageButton     aBtnTime;
    FixedLine       aFlInfo;
    FixedInfo       aFtInfo;
    ScPopupMenu     aPopUpFile;

    USHORT          nWhich;
    String          aCmdArr[6];

private:
#ifdef _TPHFEDIT_CXX
    void FillCmdArr();
    void InitPreDefinedList();
    void ProcessDefinedListSel(ScHFEntryId eSel, bool bTravelling = false);
    void InsertToDefinedList();
    void RemoveFromDefinedList();
    void SetSelectDefinedList();
    bool IsPageEntry(EditEngine*pEngine, EditTextObject* pTextObj);
    bool IsDateEntry(EditTextObject* pTextObj);
    bool IsExtFileNameEntry(EditTextObject* pTextObj);
    DECL_LINK( ListHdl_Impl, ListBox* );
    DECL_LINK( ClickHdl, ImageButton* );
    DECL_LINK( MenuHdl, ScExtIButton* );
#endif
};

//===================================================================

class ScRightHeaderEditPage : public ScHFEditPage
{
public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rCoreSet );
    static USHORT*      GetRanges();

private:
    ScRightHeaderEditPage( Window* pParent, const SfxItemSet& rSet );
};

//===================================================================

class ScLeftHeaderEditPage : public ScHFEditPage
{
public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rCoreSet );
    static USHORT*      GetRanges();

private:
    ScLeftHeaderEditPage( Window* pParent, const SfxItemSet& rSet );
};

//===================================================================

class ScRightFooterEditPage : public ScHFEditPage
{
public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rCoreSet );
    static USHORT*      GetRanges();

private:
    ScRightFooterEditPage( Window* pParent, const SfxItemSet& rSet );
};

//===================================================================

class ScLeftFooterEditPage : public ScHFEditPage
{
public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rCoreSet );
    static USHORT*      GetRanges();

private:
    ScLeftFooterEditPage( Window* pParent, const SfxItemSet& rSet );
};

#endif

