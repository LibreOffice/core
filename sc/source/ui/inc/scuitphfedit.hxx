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
    virtual sal_Bool    FillItemSet ( SfxItemSet& rCoreSet );
    virtual void    Reset       ( const SfxItemSet& rCoreSet );

    void            SetNumType(SvxNumType eNumType);
    void            ClearTextAreas();

protected:
                ScHFEditPage( Window*           pParent,
                              sal_uInt16            nResId,
                              const SfxItemSet& rCoreSet,
                              sal_uInt16            nWhich, bool bHeader );
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

    sal_uInt16          nWhich;
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
    static sal_uInt16*      GetRanges();

private:
    ScRightHeaderEditPage( Window* pParent, const SfxItemSet& rSet );
};

//===================================================================

class ScLeftHeaderEditPage : public ScHFEditPage
{
public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rCoreSet );
    static sal_uInt16*      GetRanges();

private:
    ScLeftHeaderEditPage( Window* pParent, const SfxItemSet& rSet );
};

//===================================================================

class ScRightFooterEditPage : public ScHFEditPage
{
public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rCoreSet );
    static sal_uInt16*      GetRanges();

private:
    ScRightFooterEditPage( Window* pParent, const SfxItemSet& rSet );
};

//===================================================================

class ScLeftFooterEditPage : public ScHFEditPage
{
public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rCoreSet );
    static sal_uInt16*      GetRanges();

private:
    ScLeftFooterEditPage( Window* pParent, const SfxItemSet& rSet );
};

#endif

