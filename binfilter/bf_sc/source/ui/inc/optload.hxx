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
#ifndef _OPTLOAD_HXX
#define _OPTLOAD_HXX

#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
namespace binfilter {

class ScDocument;

class ScLoadOptPage : public SfxTabPage
{
private:
    FixedText	aLinkFT;
    RadioButton	aAlwaysRB;
    RadioButton	aRequestRB;
    RadioButton	aNeverRB;
    CheckBox	aDocOnlyCB;
    GroupBox	aLinkGB;
    ScDocument *pDoc;

    DECL_LINK(	UpdateHdl, CheckBox* );

public:
                        ScLoadOptPage( Window* pParent,
                                         const SfxItemSet& rSet );
                        ~ScLoadOptPage();

    static SfxTabPage*	Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    void				SetDocument(ScDocument*);

    virtual	BOOL 		FillItemSet( SfxItemSet& rSet );
    virtual	void 		Reset( const SfxItemSet& rSet );
};

} //namespace binfilter
#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
