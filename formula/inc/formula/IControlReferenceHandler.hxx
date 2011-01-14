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
        virtual void ShowReference(const String& _sRef) = 0;
        virtual void HideReference( sal_Bool bDoneRefMode = sal_True ) = 0;
        virtual void ReleaseFocus( RefEdit* pEdit, RefButton* pButton = NULL ) = 0;
        virtual void ToggleCollapsed( RefEdit* pEdit, RefButton* pButton = NULL ) = 0;
    };
} // formula
#endif // FORMULA_ICONTROL_REFERENCE_HANDLER_HXX_INCLUDED
