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

#ifndef INCLUDED_SD_CUSTOMSCALEDIALOG_HXX
#define INCLUDED_SD_CUSTOMSCALEDIALOG_HXX

#include <sddllapi.h>

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/field.hxx>
#include <vcl/msgbox.hxx>

namespace sd
{
class SD_DLLPUBLIC SdCustomScaleDialog : public ModalDialog
{
    VclPtr<PushButton> pOk_btn;
    VclPtr<PushButton> pCancel_btn;
    VclPtr<NumericField> m_pHorizontalScale;
    VclPtr<NumericField> m_pVerticalScale;
    VclPtr<CheckBox> m_CBUniformScale;

    DECL_LINK(OkHdl, Button*, void);
    DECL_LINK(CancelHdl, Button*, void);
    DECL_LINK(UniformHdl, Button*, void);

public:
    SdCustomScaleDialog(Window* pWindow, sal_uInt16 scaleX, sal_uInt16 scaleY,
                        bool bIsUniformlyScaled);
    virtual ~SdCustomScaleDialog() override;

    bool IsScaledUniformly() const;
    sal_uInt16 getHorizontalScale() const;
    sal_uInt16 getVerticalScale() const;
    virtual void dispose() override;
};

} // end of namespace sd

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
