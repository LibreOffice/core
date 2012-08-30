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
#ifndef SVX_CUIOPTGENRL_HXX
#define SVX_CUIOPTGENRL_HXX

// include ---------------------------------------------------------------

#include <svx/optgenrl.hxx>
#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>

#include <vector>
#include <boost/shared_ptr.hpp>

// forward ---------------------------------------------------------------

class Edit;

#define SfxGeneralTabPage   SvxGeneralTabPage

// class SvxGeneralTabPage -----------------------------------------------

class SvxGeneralTabPage : public SfxTabPage
{
    using TabPage::DeactivatePage;
private:
    // the horizontal line "Address"
    FixedLine           aAddrFrm;
    // the "Use data for document properties" checkbox (but where is it?)
    CheckBox            aUseDataCB;
    // rows
    struct Row;
    std::vector<boost::shared_ptr<Row> > vRows;
    // fields
    struct Field;
    std::vector<boost::shared_ptr<Field> > vFields;
    // "name" fields
    unsigned nNameRow;
    unsigned nShortNameField;

    DECL_LINK( ModifyHdl_Impl, Edit * );

    sal_Bool            GetAddress_Impl();
    void                SetAddress_Impl();

    void CreateControls ();
    void PositionControls ();
    void SetLinks ();
    void SetAccessibleNames ();

protected:
    virtual int         DeactivatePage( SfxItemSet* pSet );

public:
    SvxGeneralTabPage( Window* pParent, const SfxItemSet& rSet );
    ~SvxGeneralTabPage ();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual sal_Bool    FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

#endif // #ifndef SVX_CUIOPTGENRL_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
