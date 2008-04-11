/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: mip.hxx,v $
 * $Revision: 1.4 $
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

#ifndef _MIP_HXX
#define _MIP_HXX

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
    rtl::OUString msTypeName;

    rtl::OUString msConstraintExplanation;

public:
    MIP();
    ~MIP();

    /// inherit from upper-level MIPs
    void inherit( const MIP& );

    /// join with same-level MIPs
    void join( const MIP& );

    void set( const MIP& );


    // has any info?
    bool hasInfo() const;

    // - type (static; default: xsd:string)
    //        (currently default implemented as empty string)
    bool hasTypeName() const;
    rtl::OUString getTypeName() const;
    void setTypeName( const rtl::OUString& );
    void resetTypeName();

    // - readonly (computed XPath; default: false; true if calculate exists)
    bool hasReadonly() const;
    bool isReadonly() const;
    void setReadonly( bool );
    void resetReadonly();

    // - required (computed XPath; default: false)
    bool hasRequired() const;
    bool isRequired() const;
    void setRequired( bool );
    void resetRequired();

    // - relevant (computed XPath; default: true)
    bool hasRelevant() const;
    bool isRelevant() const;
    void setRelevant( bool );
    void resetRelevant();

    // - constraing (computed XPath; default: true)
    bool hasConstraint() const;
    bool isConstraint() const;
    void setConstraint( bool );
    void resetConstraint();

    // explain _why_ a constraint failed
    void setConstraintExplanation( const rtl::OUString& );
    rtl::OUString getConstraintExplanation() const;

    // - calculate (computed XPath; default: has none (false))
    //   (for calculate, we only store whether a calculate MIP is present;
    //    the actual calculate value is handled my changing the instance
    //    directly)
    bool hasCalculate() const;
    void setHasCalculate( bool );
    void resetCalculate();

    // - minOccurs/maxOccurs (computed XPath; default: 0/inf)
    // - p3ptype (static; no default)

};

} // namespace xforms

#endif
