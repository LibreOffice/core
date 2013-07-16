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

#ifndef SC_FOPTMGR_HXX
#define SC_FOPTMGR_HXX

#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>

//----------------------------------------------------------------------------

class FixedInfo;
namespace formula
{
    class RefButton;
}
class MoreButton;
struct ScQueryParam;
class ScDocument;
class ScViewData;

//----------------------------------------------------------------------------

class ScFilterOptionsMgr
{
public:
            ScFilterOptionsMgr( ScViewData*         ptrViewData,
                                const ScQueryParam& refQueryData,
                                MoreButton&         refBtnMore,
                                CheckBox&           refBtnCase,
                                CheckBox&           refBtnRegExp,
                                CheckBox&           refBtnHeader,
                                CheckBox&           refBtnUnique,
                                CheckBox&           refBtnCopyResult,
                                CheckBox&           refBtnDestPers,
                                ListBox&            refLbCopyArea,
                                Edit&               refEdCopyArea,
                                formula::RefButton&     refRbCopyArea,
                                FixedText&          refFtDbAreaLabel,
                                FixedInfo&          refFtDbArea,
                                FixedLine&          refFlOptions,
                                const String&       refStrUndefined );
            ~ScFilterOptionsMgr();

    sal_Bool    VerifyPosStr ( const String& rPosStr ) const;

private:
    ScViewData*     pViewData;
    ScDocument*     pDoc;

    MoreButton&     rBtnMore;
    CheckBox&       rBtnCase;
    CheckBox&       rBtnRegExp;
    CheckBox&       rBtnHeader;
    CheckBox&       rBtnUnique;
    CheckBox&       rBtnCopyResult;
    CheckBox&       rBtnDestPers;
    ListBox&        rLbCopyPos;
    Edit&           rEdCopyPos;
    formula::RefButton& rRbCopyPos;
    FixedText&      rFtDbAreaLabel;
    FixedInfo&      rFtDbArea;
    FixedLine&      rFlOptions;

    const String&   rStrUndefined;

    const ScQueryParam& rQueryData;

#ifdef _FOPTMGR_CXX
private:
    void Init();

    // Handler:
    DECL_LINK( EdPosModifyHdl,      Edit* );
    DECL_LINK( LbPosSelHdl,         ListBox* );
    DECL_LINK( BtnCopyResultHdl,    CheckBox* );
#endif
};



#endif // SC_FOPTMGR_HXX


#ifndef SC_NEWFOPTMGR_HXX
#define SC_NEWFOPTMGR_HXX

#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
//#include <vcl/layout.hxx>

//----------------------------------------------------------------------------

namespace formula
{
    class RefButton;
}
struct ScQueryParam;
class ScDocument;
class ScViewData;

//----------------------------------------------------------------------------

class ScNewFilterOptionsMgr
{
public:
            ScNewFilterOptionsMgr( ScViewData*         ptrViewData,
                                const ScQueryParam& refQueryData,
                                CheckBox*           refBtnCase,
                                CheckBox*           refBtnRegExp,
                                CheckBox*           refBtnHeader,
                                CheckBox*           refBtnUnique,
                                CheckBox*           refBtnCopyResult,
                                CheckBox*           refBtnDestPers,
                                ListBox*            refLbCopyArea,
                                Edit*               refEdCopyArea,
                                formula::RefButton*     refRbCopyArea,
                                FixedText*          refFtDbAreaLabel,
                                FixedText*          refFtDbArea,
                                const String&       refStrUndefined );
            ~ScNewFilterOptionsMgr();

private:
    ScViewData*     pViewData;
    ScDocument*     pDoc;

    CheckBox*       pBtnCase;
    CheckBox*       pBtnRegExp;
    CheckBox*       pBtnHeader;
    CheckBox*       pBtnUnique;
    CheckBox*       pBtnCopyResult;
    CheckBox*       pBtnDestPers;
    ListBox*        pLbCopyArea;
    Edit*           pEdCopyArea;
    formula::RefButton* pRbCopyArea;
    FixedText*      pFtDbAreaLabel;
    FixedText*      pFtDbArea;

    const String&   rStrUndefined;

    const ScQueryParam& rQueryData;

#ifdef _NEWFOPTMGR_CXX
private:
    void Init();

    // Handler:
    DECL_LINK( EdAreaModifyHdl,     Edit* );
    DECL_LINK( LbAreaSelHdl,        ListBox* );
    DECL_LINK( BtnCopyResultHdl,    CheckBox* );
#endif
};


#endif // SC_NEWFOPTMGR_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
