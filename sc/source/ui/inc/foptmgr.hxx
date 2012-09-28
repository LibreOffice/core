/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
