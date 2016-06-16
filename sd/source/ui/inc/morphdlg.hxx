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

#ifndef INCLUDED_SD_SOURCE_UI_INC_MORPHDLG_HXX
#define INCLUDED_SD_SOURCE_UI_INC_MORPHDLG_HXX

#include <vcl/button.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/dialog.hxx>

class SdrObject;

namespace sd {

class MorphDlg
    : public ModalDialog
{
public:
    MorphDlg (
        vcl::Window* pParent,
        const SdrObject* pObj1,
        const SdrObject* pObj2);
    virtual ~MorphDlg() override;
    virtual void dispose() override;

    void            SaveSettings() const;
    sal_uInt16      GetFadeSteps() const { return (sal_uInt16) m_pMtfSteps->GetValue(); }
    bool            IsAttributeFade() const { return m_pCbxAttributes->IsChecked(); }
    bool            IsOrientationFade() const { return m_pCbxOrientation->IsChecked(); }

private:
    VclPtr<NumericField>   m_pMtfSteps;
    VclPtr<CheckBox>       m_pCbxAttributes;
    VclPtr<CheckBox>       m_pCbxOrientation;

    void            LoadSettings();
};

#endif

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
