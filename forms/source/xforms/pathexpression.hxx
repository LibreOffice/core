/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: pathexpression.hxx,v $
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

#ifndef _PATHEXPRESSION_HXX
#define _PATHEXPRESSION_HXX


// include for parent class
#include "computedexpression.hxx"

// includes for member variables
#include <vector>

// forward declaractions
namespace com { namespace sun { namespace star
{
    namespace xml { namespace dom
    {
        class XNodeList;
        namespace events { class XEventListener; }
    } }
} } }



namespace xforms
{

/** PathExpression represents an XPath Expression and caches results */
class PathExpression : public ComputedExpression
{
public:
    typedef std::vector<com::sun::star::uno::Reference<com::sun::star::xml::dom::XNode> > NodeVector_t;

private:
    /// the node-list result from the last bind (cached from mxResult)
    NodeVector_t maNodes;

protected:
    /// get expression for evaluation
    const rtl::OUString _getExpressionForEvaluation() const;


public:
    PathExpression();
    ~PathExpression();

    /// set the expression string
    /// (overridden to do remove old listeners)
    /// (also defines simple expressions)
    void setExpression( const rtl::OUString& rExpression );


    /// evaluate the expression relative to the content node.
    bool evaluate( const xforms::EvaluationContext& rContext );


    // get the result of this expression as node/node list/...
    com::sun::star::uno::Reference<com::sun::star::xml::dom::XNode> getNode() const;
    const NodeVector_t getNodeList() const;
    com::sun::star::uno::Reference<com::sun::star::xml::dom::XNodeList> getXNodeList() const;

};

} // namespace xforms

#endif
