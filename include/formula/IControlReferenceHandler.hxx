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
#ifndef FORMULA_ICONTROL_REFERENCE_HANDLER_HXX_INCLUDED
#define FORMULA_ICONTROL_REFERENCE_HANDLER_HXX_INCLUDED

#include "formula/formuladllapi.h"

namespace formula
{
    class RefEdit;
    class RefButton;
    class FORMULA_DLLPUBLIC SAL_NO_VTABLE IControlReferenceHandler
    {
    public:
        virtual void ShowReference(const OUString& _sRef) = 0;
        virtual void HideReference( sal_Bool bDoneRefMode = sal_True ) = 0;
        virtual void ReleaseFocus( RefEdit* pEdit, RefButton* pButton = NULL ) = 0;
        virtual void ToggleCollapsed( RefEdit* pEdit, RefButton* pButton = NULL ) = 0;

    protected:
        ~IControlReferenceHandler() {}
    };
} // formula
#endif // FORMULA_ICONTROL_REFERENCE_HANDLER_HXX_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
