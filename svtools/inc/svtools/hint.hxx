/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hint.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:20:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SFXHINT_HXX
#define _SFXHINT_HXX

#ifndef INCLUDED_SVLDLLAPI_H
#include "svtools/svldllapi.h"
#endif

#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif

class SVL_DLLPUBLIC SfxHint
{
public:
    TYPEINFO();

    virtual ~SfxHint();
};

//--------------------------------------------------------------------

#define DECL_PTRHINT(Visibility, Name, Type) \
        class Visibility Name: public SfxHint \
        { \
            Type* pObj; \
            BOOL  bIsOwner; \
        \
        public: \
            TYPEINFO(); \
            Name( Type* Object, BOOL bOwnedByHint = FALSE ); \
            ~Name(); \
        \
            Type* GetObject() const { return pObj; } \
            BOOL  IsOwner() const { return bIsOwner; } \
        }

#define IMPL_PTRHINT_AUTODELETE(Name, Type) \
        TYPEINIT1(Name, SfxHint);   \
        Name::Name( Type* pObject, BOOL bOwnedByHint ) \
            { pObj = pObject; bIsOwner = bOwnedByHint; } \
        Name::~Name() { if ( bIsOwner ) delete pObj; }

#define IMPL_PTRHINT(Name, Type) \
        TYPEINIT1(Name, SfxHint);   \
        Name::Name( Type* pObject, BOOL bOwnedByHint ) \
            { pObj = pObject; bIsOwner = bOwnedByHint; } \
        Name::~Name() {}


#endif

