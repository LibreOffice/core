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
#ifndef _SVX_MULTIPAT_HXX
#define _SVX_MULTIPAT_HXX

#include <vcl/dialog.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>

#include "radiobtnbox.hxx"

// define ----------------------------------------------------------------

// different delimiter for Unix (:) and Windows (;)

#ifdef UNX
#define CLASSPATH_DELIMITER ':'
#else
#define CLASSPATH_DELIMITER ';'
#endif

// forward ---------------------------------------------------------------

struct MultiPath_Impl;

// class SvxMultiPathDialog ----------------------------------------------

class SvxMultiPathDialog : public ModalDialog
{
protected:
    FixedLine                   aPathFL;
    ListBox                     aPathLB;
    SvxSimpleTableContainer m_aRadioLBContainer;
    svx::SvxRadioButtonListBox  aRadioLB;
    FixedText                   aRadioFT;
    PushButton                  aAddBtn;
    PushButton                  aDelBtn;
    OKButton                    aOKBtn;
    CancelButton                aCancelBtn;
    HelpButton                  aHelpButton;

    MultiPath_Impl* pImpl;

    DECL_LINK(AddHdl_Impl, void *);
    DECL_LINK(DelHdl_Impl, void *);
    DECL_LINK( SelectHdl_Impl, void * );
    DECL_LINK( CheckHdl_Impl, svx::SvxRadioButtonListBox * );

public:
    SvxMultiPathDialog( Window* pParent, sal_Bool bEmptyAllowed = sal_False );
    ~SvxMultiPathDialog();

    String          GetPath() const;
    void            SetPath( const String& rPath );
    void            SetClassPathMode();
    sal_Bool        IsClassPathMode() const;
    void            EnableRadioButtonMode();
};

#endif // #ifndef _SVX_MULTIPAT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
