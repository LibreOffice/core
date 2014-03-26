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
#ifndef INCLUDED_SC_SOURCE_UI_INC_OPTLOAD_HXX
#define INCLUDED_SC_SOURCE_UI_INC_OPTLOAD_HXX

#include <sfx2/tabdlg.hxx>

#include <vcl/group.hxx>
#include <vcl/fixed.hxx>

class ScDocument;

class ScLoadOptPage : public SfxTabPage
{
private:
    FixedText   aLinkFT;
    RadioButton aAlwaysRB;
    RadioButton aRequestRB;
    RadioButton aNeverRB;
    CheckBox    aDocOnlyCB;
    GroupBox    aLinkGB;
    ScDocument *pDoc;

    DECL_LINK(  UpdateHdl, CheckBox* );

public:
                        ScLoadOptPage( Window* pParent,
                                         const SfxItemSet& rSet );
                        ~ScLoadOptPage();

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    void                SetDocument(ScDocument*);

    virtual bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
