/*************************************************************************
 *
 *  $RCSfile: mip.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 10:53:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
