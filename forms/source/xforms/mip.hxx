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

#ifndef INCLUDED_FORMS_SOURCE_XFORMS_MIP_HXX
#define INCLUDED_FORMS_SOURCE_XFORMS_MIP_HXX

#include <rtl/ustring.hxx>

namespace xforms
{

/** represents the XForms *m*odel *i*tem *p*roperties (MIPs) for a
 * given XNode in the instance data at a given point in time.  The
 * values will not be updated; for updated values new MIP objects have
 * to be created/queried. */
class MIP
{
    bool mbHasReadonly;
    bool mbReadonly;

    bool mbHasRequired;
    bool mbRequired;

    bool mbHasRelevant;
    bool mbRelevant;

    bool mbHasConstraint;
    bool mbConstraint;

    bool mbHasCalculate;

    bool mbHasTypeName;
    OUString msTypeName;

    OUString msConstraintExplanation;

public:
    MIP();
    ~MIP();

    /// inherit from upper-level MIPs
    void inherit( const MIP& );

    /// join with same-level MIPs
    void join( const MIP& );


    // - type (static; default: xsd:string)
    //        (currently default implemented as empty string)
    bool hasTypeName() const { return mbHasTypeName; }
    OUString getTypeName() const { return msTypeName; }
    void setTypeName( const OUString& );
    void resetTypeName();

    // - readonly (computed XPath; default: false; true if calculate exists)
    bool hasReadonly() const { return mbHasReadonly; }
    bool isReadonly() const;
    void setReadonly( bool );
    void resetReadonly();

    // - required (computed XPath; default: false)
    bool hasRequired() const { return mbHasRequired; }
    bool isRequired() const { return mbRequired; }
    void setRequired( bool );
    void resetRequired();

    // - relevant (computed XPath; default: true)
    bool hasRelevant() const { return mbHasRelevant; }
    bool isRelevant() const { return mbRelevant; }
    void setRelevant( bool );
    void resetRelevant();

    // - constraint (computed XPath; default: true)
    bool hasConstraint() const { return mbHasConstraint; }
    bool isConstraint() const { return mbConstraint; }
    void setConstraint( bool );
    void resetConstraint();

    // explain _why_ a constraint failed
    void setConstraintExplanation( const OUString& );
    OUString getConstraintExplanation() const { return msConstraintExplanation; }

    // - calculate (computed XPath; default: has none (false))
    //   (for calculate, we only store whether a calculate MIP is present;
    //    the actual calculate value is handled my changing the instance
    //    directly)
    bool hasCalculate() const { return mbHasCalculate; }
    void setHasCalculate( bool );
    void resetCalculate();

    // - minOccurs/maxOccurs (computed XPath; default: 0/inf)
    // - p3ptype (static; no default)

};

} // namespace xforms

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
