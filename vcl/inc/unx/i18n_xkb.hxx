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

#ifndef _SAL_I18N_XKBDEXTENSION_HXX
#define _SAL_I18N_XKBDEXTENSION_HXX

#include <sal/types.h>
#include <vclpluginapi.h>

class VCLPLUG_GEN_PUBLIC SalI18N_KeyboardExtension
{
private:

    sal_Bool            mbUseExtension;
    sal_uInt32          mnDefaultGroup;
    sal_uInt32          mnGroup;
    int                 mnEventBase;
    int                 mnErrorBase;
    Display*            mpDisplay;

public:

                        SalI18N_KeyboardExtension( Display *pDisplay );
    inline              ~SalI18N_KeyboardExtension();

    inline sal_Bool     UseExtension() const ;      // server and client support the
                                                    // extension
    inline void         UseExtension( sal_Bool bState );// used to disable the Extension

    void                Dispatch( XEvent *pEvent ); // keep track of group changes

    sal_uInt32          LookupKeysymInGroup(    sal_uInt32 nKeyCode,
                                                  sal_uInt32 nShiftState,
                                                  sal_uInt32 nGroup ) const ;

    inline sal_uInt32   LookupKeysymInDefaultGroup(
                                                sal_uInt32 nKeyCode,
                                                sal_uInt32 nShiftState ) const ;
    inline sal_uInt32   GetGroup() const ;          // the current keyboard group
    inline sal_uInt32   GetDefaultGroup() const ;   // base group, usually group 1
    inline int          GetEventBase() const ;

protected:

                        SalI18N_KeyboardExtension(); // disabled
};

inline
SalI18N_KeyboardExtension::~SalI18N_KeyboardExtension()
{
}

inline sal_Bool
SalI18N_KeyboardExtension::UseExtension() const
{
    return mbUseExtension;
}

inline void
SalI18N_KeyboardExtension::UseExtension( sal_Bool bState )
{
    mbUseExtension = mbUseExtension && bState;
}

inline sal_uInt32
SalI18N_KeyboardExtension::LookupKeysymInDefaultGroup( sal_uInt32 nKeyCode,
                                                       sal_uInt32 nShiftState ) const
{
    return LookupKeysymInGroup( nKeyCode, nShiftState, mnDefaultGroup );
}

inline sal_uInt32
SalI18N_KeyboardExtension::GetGroup() const
{
    return mnGroup;
}

inline sal_uInt32
SalI18N_KeyboardExtension::GetDefaultGroup() const
{
    return mnDefaultGroup;
}

inline int
SalI18N_KeyboardExtension::GetEventBase() const
{
    return mnEventBase;
}

#endif // _SAL_I18N_XKBDEXTENSION_HXX

