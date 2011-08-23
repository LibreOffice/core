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
#ifndef _SFXSMPLHINT_HXX
#define _SFXSMPLHINT_HXX

#ifndef _SFXHINT_HXX
#include <bf_svtools/hint.hxx>
#endif

#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif

namespace binfilter
{


#define SFX_HINT_DYING				0x00000001
#define SFX_HINT_NAMECHANGED		0x00000002
#define SFX_HINT_TITLECHANGED		0x00000004
#define SFX_HINT_DATACHANGED		0x00000008
#define SFX_HINT_DOCCHANGED 		0x00000010
#define SFX_HINT_UPDATEDONE			0x00000020
#define SFX_HINT_DEINITIALIZING		0x00000040
#define SFX_HINT_MODECHANGED		0x00000080
#define SFX_HINT_CANCELLABLE		0x00000100
#define SFX_HINT_DATAAVAILABLE  	0x00000200
#define SFX_HINT_SAVECOMPLETED 	    0x00000400
#define SFX_HINT_RELEASEREF			0x00000800
#define SFX_HINT_COLORS_CHANGED     0x00001000
#define SFX_HINT_CTL_SETTINGS_CHANGED   0x00002000
#define SFX_HINT_ACCESSIBILITY_CHANGED	0x00004000
#define SFX_HINT_VIEWCREATED		0x00008000
#define SFX_HINT_USER00 			0x00010000
#define SFX_HINT_USER01 			0x00020000
#define SFX_HINT_USER02 			0x00040000
#define SFX_HINT_USER03 			0x00080000
#define SFX_HINT_USER04 			0x00100000
#define SFX_HINT_USER05 			0x00200000
#define SFX_HINT_USER06 			0x00400000
#define SFX_HINT_USER07 			0x00800000
#define SFX_HINT_USER08 			0x01000000
#define SFX_HINT_USER09 			0x02000000
#define SFX_HINT_USER10 			0x04000000
#define SFX_HINT_USER11 			0x08000000
#define SFX_HINT_USER12 			0x10000000
#define SFX_HINT_USER13 			0x20000000
#define SFX_HINT_UNDO_OPTIONS_CHANGED 0x40000000
#define SFX_HINT_USER_OPTIONS_CHANGED 0x80000000
#define SFX_HINT_ALL				0xFFFFFFFF

class  SfxSimpleHint: public SfxHint
{
private:
    ULONG nId;
public:
    TYPEINFO();
    SfxSimpleHint( ULONG nId );
    ULONG GetId() const { return nId; }
};

//--------------------------------------------------------------------

#define DECL_OBJHINT(Name, Type) \
        class Name: public SfxSimpleHint \
        { \
            Type  aObj; \
        \
        public: \
            TYPEINFO(); \
            Name( USHORT nId, const Type& rObject ); \
            ~Name(); \
            const Type& GetObject() const { return aObj; } \
        }

#define IMPL_OBJHINT(Name, Type) \
        TYPEINIT1(Name, SfxSimpleHint);	\
        Name::Name( USHORT nID, const Type& rObject ): \
            SfxSimpleHint( nID ), aObj(rObject) \
            { } \
        Name::~Name() {}

}

#endif
