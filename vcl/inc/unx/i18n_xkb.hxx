/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

