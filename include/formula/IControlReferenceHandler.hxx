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
#ifndef INCLUDED_FORMULA_ICONTROLREFERENCEHANDLER_HXX
#define INCLUDED_FORMULA_ICONTROLREFERENCEHANDLER_HXX

#include <formula/formuladllapi.h>
#include <rtl/ustring.hxx>

namespace formula
{
    class RefEdit;
    class RefButton;
    class FORMULA_DLLPUBLIC SAL_NO_VTABLE IControlReferenceHandler
    {
    public:
        virtual void ShowReference(const OUString& _sRef) = 0;
        virtual void HideReference( bool bDoneRefMode = true ) = 0;
        virtual void ReleaseFocus( RefEdit* pEdit, RefButton* pButton = nullptr ) = 0;
        virtual void ToggleCollapsed( RefEdit* pEdit, RefButton* pButton = nullptr ) = 0;

    protected:
        ~IControlReferenceHandler() {}
    };
} // formula
#endif // INCLUDED_FORMULA_ICONTROLREFERENCEHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
