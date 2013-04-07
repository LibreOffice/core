/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


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
void MIP::setConstraint( bool b ) { mbHasConstraint = true; mbConstraint = b; msConstraintExplanation = OUString(); }
void MIP::resetConstraint()       { mbHasConstraint = false; mbConstraint = true; msConstraintExplanation = OUString(); }

void MIP::setConstraintExplanation( const OUString& s ) { msConstraintExplanation = s; }
OUString MIP::getConstraintExplanation() const { return msConstraintExplanation; }


bool MIP::hasCalculate() const    { return mbHasCalculate; }
void MIP::setHasCalculate( bool b ) { mbHasCalculate = b; }
void MIP::resetCalculate()        { mbHasCalculate = false; }

bool MIP::hasTypeName() const     { return mbHasTypeName; }
    OUString MIP::getTypeName() const      { return msTypeName; }
void MIP::setTypeName( const OUString& s ) { msTypeName = s; mbHasTypeName = true; }
void MIP::resetTypeName()         { msTypeName = OUString(); mbHasTypeName = false; }




} // namespace xforms

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
