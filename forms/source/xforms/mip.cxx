/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mip.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 00:04:14 $
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
