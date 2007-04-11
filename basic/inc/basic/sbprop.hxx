/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sbprop.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 12:52:36 $
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

#ifndef _SB_SBPROPERTY_HXX
#define _SB_SBPROPERTY_HXX

#ifndef __SBX_SBXPROPERTY_HXX //autogen
#include <basic/sbxprop.hxx>
#endif
#include <basic/sbdef.hxx>

class SbModule;

class SbProperty : public SbxProperty
{
    friend class SbiFactory;
    friend class SbModule;
    friend class SbProcedureProperty;
    SbModule* pMod;
    BOOL     bInvalid;
    SbProperty( const String&, SbxDataType, SbModule* );
    virtual ~SbProperty();
public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_BASICPROP,1);
    TYPEINFO();
    SbModule* GetModule() { return pMod; }
};

#ifndef __SB_SBPROPERTYREF_HXX
#define __SB_SBPROPERTYREF_HXX
SV_DECL_IMPL_REF(SbProperty)
#endif

class SbProcedureProperty : public SbxProperty
{
    bool mbSet;     // Flag for set command

    virtual ~SbProcedureProperty();

public:
    SbProcedureProperty( const String& r, SbxDataType t )
            : SbxProperty( r, t ) // , pMod( p )
            , mbSet( false )
    {}
    TYPEINFO();

    bool isSet( void )
        { return mbSet; }
    void setSet( bool bSet )
        { mbSet = bSet; }
};

#ifndef __SB_SBPROCEDUREPROPERTYREF_HXX
#define __SB_SBPROCEDUREPROPERTYREF_HXX
SV_DECL_IMPL_REF(SbProcedureProperty)
#endif

#endif
