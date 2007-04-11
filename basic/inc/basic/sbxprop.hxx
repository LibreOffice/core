/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sbxprop.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 12:55:27 $
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

#ifndef __SBX_SBXPROPERTY_HXX
#define __SBX_SBXPROPERTY_HXX

#ifndef _SBXVAR_HXX
#include <basic/sbxvar.hxx>
#endif

class SbxPropertyImpl;

class SbxProperty : public SbxVariable
{
    SbxPropertyImpl* mpSbxPropertyImpl; // Impl data

public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_PROPERTY,1);
    TYPEINFO();
    SbxProperty( const String& r, SbxDataType t )
    : SbxVariable( t ) { SetName( r ); }
    SbxProperty( const SbxProperty& r ) : SvRefBase( r ), SbxVariable( r ) {}
    SbxProperty& operator=( const SbxProperty& r )
    { SbxVariable::operator=( r ); return *this; }
    virtual SbxClassType GetClass() const;
};

#ifndef __SBX_SBXPROPERTYREF_HXX
#define __SBX_SBXPROPERTYREF_HXX

#ifndef SBX_PROPERTY_DECL_DEFINED
#define SBX_PROPERTY_DECL_DEFINED
SV_DECL_REF(SbxProperty)
#endif
SV_IMPL_REF(SbxProperty)

#endif

#endif
