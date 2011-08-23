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
#ifndef _SFXCNCLHINT_HXX
#define _SFXCNCLHINT_HXX

#ifndef _SFXHINT_HXX
#include <bf_svtools/hint.hxx>
#endif

#ifndef _SFXCANCEL_HXX
#include <bf_svtools/cancel.hxx>
#endif

#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif

namespace binfilter {

#define SFXCANCELHINT_REMOVED		1

class SfxCancelHint: public SfxHint
{
private:
    SfxCancellable* pCancellable;
    USHORT			nAction;
public:
    TYPEINFO();
    SfxCancelHint( SfxCancellable*, USHORT nAction );
    USHORT GetAction() const { return nAction; }
    const SfxCancellable& GetCancellable() const { return *pCancellable; }
};

}

#endif
