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
#ifndef _IDLC_ASTSTACK_HXX_
#define _IDLC_ASTSTACK_HXX_

#include <sal/types.h>

class AstScope;

class AstStack
{
public:
    AstStack();
    virtual ~AstStack();

    sal_uInt32 depth();
    AstScope* top();
    AstScope* bottom();
    AstScope* nextToTop();
    AstScope* topNonNull();
    AstStack* push(AstScope* pScope);
    void pop();
    void clear();

private:
    AstScope**  m_stack;
    sal_uInt32  m_size;
    sal_uInt32  m_top;
};

#endif // _IDLC_ASTSTACK_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
