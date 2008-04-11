/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: mip.cxx,v $
 * $Revision: 1.5 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_forms.hxx"

#include "mip.hxx"


namespace xforms
{


MIP::MIP()
{
    resetReadonly();
    resetRequired();
    resetRelevant();
    resetConstraint();
    resetCalculate();
    resetTypeName();
}

MIP::~MIP() {}

void MIP::inherit( const MIP& rMip )
{
    if( ! mbHasReadonly )
    {
        mbHasReadonly = rMip.hasReadonly();
        mbReadonly = rMip.isReadonly();
    }
    if( ! mbHasRequired )
    {
        mbHasRequired = rMip.hasRequired();
        mbRequired = rMip.isRequired();
    }
    if( ! mbHasRelevant )
    {
        mbHasRelevant = rMip.hasRelevant();
        mbRelevant = rMip.isRelevant();
    }
    if( ! mbHasConstraint )
    {
        mbHasConstraint = rMip.hasConstraint();
        mbConstraint = rMip.isConstraint();
        msConstraintExplanation = rMip.getConstraintExplanation();
    }
    if( ! mbHasCalculate )
    {
        mbHasCalculate = rMip.hasCalculate();
    }
    if( ! mbHasTypeName )
    {
        mbHasTypeName = rMip.hasTypeName();
        msTypeName = rMip.getTypeName();
    }
}

void MIP::join( const MIP& rMip )
{
    // TODO: inherit only inheritable MIPs...
    inherit( rMip );
}

void MIP::set( const MIP& rMip )
{
    *this = rMip;
}


bool MIP::hasInfo() const
{
    return mbHasReadonly || mbHasRequired || mbHasRelevant
        || mbHasConstraint || mbHasCalculate || mbHasTypeName;
}

bool MIP::hasReadonly() const     { return mbHasReadonly; }
bool MIP::isReadonly() const      { return mbHasReadonly ? mbReadonly : mbHasCalculate; }
void MIP::setReadonly( bool b )   { mbHasReadonly = true; mbReadonly = b; }
void MIP::resetReadonly()         { mbHasReadonly = false; mbReadonly = false; }

bool MIP::hasRequired() const     { return mbHasRequired; }
bool MIP::isRequired() const      { return mbRequired; }
void MIP::setRequired( bool b )   { mbHasRequired = true; mbRequired = b; }
void MIP::resetRequired()         { mbHasRequired = false; mbRequired = false; }

bool MIP::hasRelevant() const     { return mbHasRelevant; }
bool MIP::isRelevant() const      { return mbRelevant; }
void MIP::setRelevant( bool b )   { mbHasRelevant = true; mbRelevant = b; }
void MIP::resetRelevant()         { mbHasRelevant = false; mbRelevant = true; }

bool MIP::hasConstraint() const   { return mbHasConstraint; }
bool MIP::isConstraint() const    { return mbConstraint; }
void MIP::setConstraint( bool b ) { mbHasConstraint = true; mbConstraint = b; msConstraintExplanation = rtl::OUString(); }
void MIP::resetConstraint()       { mbHasConstraint = false; mbConstraint = true; msConstraintExplanation = rtl::OUString(); }

void MIP::setConstraintExplanation( const rtl::OUString& s ) { msConstraintExplanation = s; }
rtl::OUString MIP::getConstraintExplanation() const { return msConstraintExplanation; }


bool MIP::hasCalculate() const    { return mbHasCalculate; }
void MIP::setHasCalculate( bool b ) { mbHasCalculate = b; }
void MIP::resetCalculate()        { mbHasCalculate = false; }

bool MIP::hasTypeName() const     { return mbHasTypeName; }
    rtl::OUString MIP::getTypeName() const      { return msTypeName; }
void MIP::setTypeName( const rtl::OUString& s ) { msTypeName = s; mbHasTypeName = true; }
void MIP::resetTypeName()         { msTypeName = rtl::OUString(); mbHasTypeName = false; }




} // namespace xforms
