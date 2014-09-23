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
#ifndef INCLUDED_CUI_SOURCE_INC_SRCHXTRA_HXX
#define INCLUDED_CUI_SOURCE_INC_SRCHXTRA_HXX

#include <vcl/field.hxx>
#include <svtools/ctrltool.hxx>
#include <sfx2/tabdlg.hxx>

#include <svx/checklbx.hxx>
#include <svx/srchdlg.hxx>

class SvxSearchFormatDialog : public SfxTabDialog
{
public:
    SvxSearchFormatDialog( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual ~SvxSearchFormatDialog();

protected:
    virtual void    PageCreated( sal_uInt16 nId, SfxTabPage &rPage ) SAL_OVERRIDE;

private:
    FontList*  m_pFontList;
    sal_uInt16 m_nNamePageId;
    sal_uInt16 m_nParaStdPageId;
    sal_uInt16 m_nParaAlignPageId;
    sal_uInt16 m_nBackPageId;
};

// class SvxSearchFormatDialog -------------------------------------------

class SvxSearchAttributeDialog : public ModalDialog
{
public:
    SvxSearchAttributeDialog( vcl::Window* pParent, SearchAttrItemList& rLst,
                              const sal_uInt16* pWhRanges );

private:
    SvxCheckListBox*    m_pAttrLB;
    OKButton*           m_pOKBtn;

    SearchAttrItemList& rList;

    DECL_LINK(OKHdl, void *);
};

// class SvxSearchSimilarityDialog ---------------------------------------

class SvxSearchSimilarityDialog : public ModalDialog
{
private:
    NumericField*        m_pOtherFld;
    NumericField*        m_pLongerFld;
    NumericField*        m_pShorterFld;
    CheckBox*            m_pRelaxBox;

public:
    SvxSearchSimilarityDialog(  vcl::Window* pParent,
                                bool bRelax,
                                sal_uInt16 nOther,
                                sal_uInt16 nShorter,
                                sal_uInt16 nLonger );

    sal_uInt16  GetOther()      { return (sal_uInt16)m_pOtherFld->GetValue(); }
    sal_uInt16  GetShorter()    { return (sal_uInt16)m_pShorterFld->GetValue(); }
    sal_uInt16  GetLonger()     { return (sal_uInt16)m_pLongerFld->GetValue(); }
    bool    IsRelaxed()     { return m_pRelaxBox->IsChecked(); }
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
