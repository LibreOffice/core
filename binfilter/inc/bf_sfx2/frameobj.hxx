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

#ifndef _FRAMEOBJ_HXX
#define _FRAMEOBJ_HXX

#ifndef _IPOBJ_HXX //autogen
#include <bf_so3/ipobj.hxx>
#endif
namespace binfilter {

//=========================================================================
struct SfxFrameObject_Impl;
class SfxFrameDescriptor;

struct SfxFrameObjectFactoryPtr
{
    // Ist n"otig, da im SO2_DECL_BASIC_CLASS_DLL-Macro ein Pointer auf
    // eine exportierbare struct/class "ubergeben werden mu\s
    SotFactory *pSfxFrameObjectFactory;
    SfxFrameObjectFactoryPtr();
};

class SfxFrameObject : public SvInPlaceObject
/*	[Beschreibung]

*/
{
    SfxFrameObject_Impl*	pImpl;

#if _SOLAR__PRIVATE
    DECL_LINK(				NewObjectHdl_Impl, Timer* );
#endif

protected:
    BOOL					ConstructFrame();
                            // Protokoll

                            // Datenaustausch

                            // Laden speichern
    virtual BOOL    		Load( SvStorage * );
    virtual BOOL    		Save();
    virtual BOOL    		SaveAs( SvStorage * );
    virtual BOOL    		SaveCompleted( SvStorage * );

                            ~SfxFrameObject();
public:

    static SfxFrameObjectFactoryPtr*
                            GetFactoryPtr();

                            // Macro mu\s exportiert werden, sonst geht das
                            // SO2-RTTI schief, da GetFactoryAdress() die
                            // Factory-Adresse der Basisklasse liefert.
                            // Da das zweite Argument in einer Inline-Funktion
                            // verwendet wird, mu\s dieses exportierbare
                            // Funktionsaufrufe verwenden, also nicht z.B.
                            // SFX_APP()->Get_Impl()
                            SO2_DECL_BASIC_CLASS_DLL(SfxFrameObject, GetFactoryPtr())

                            SfxFrameObject();

    void					SetFrameDescriptor( const SfxFrameDescriptor*);
    const SfxFrameDescriptor*
                            GetFrameDescriptor() const;

#if _SOLAR__PRIVATE
#endif
};

inline SfxFrameObjectFactoryPtr::SfxFrameObjectFactoryPtr()
    : pSfxFrameObjectFactory(0) 	// sonst funzt ClassFactory() nicht!
{
    // hier Factory nicht erzeugen, da GetFactoryPtr() erst nach Beenden dieses
    // ctors einen vern"unftigen Wert liefern kann!
}

SO2_DECL_IMPL_REF(SfxFrameObject)


}//end of namespace binfilter
#endif
