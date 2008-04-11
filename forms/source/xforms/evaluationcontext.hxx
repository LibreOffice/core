/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: evaluationcontext.hxx,v $
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

#ifndef _EVALUATIONCONTEXT_HXX
#define _EVALUATIONCONTEXT_HXX

#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/xforms/XModel.hpp>

namespace xforms
{


/** define the context for the evaluation of an XPath expression */
class EvaluationContext
{
public:
    EvaluationContext()
        : mxContextNode(),
          mxModel(),
          mxNamespaces(),
          mnContextPosition( 0 ),
          mnContextSize( 0 )
    { }

    EvaluationContext(
        const com::sun::star::uno::Reference<com::sun::star::xml::dom::XNode>& xContextNode,
        const com::sun::star::uno::Reference<com::sun::star::xforms::XModel>& xModel,
        const com::sun::star::uno::Reference<com::sun::star::container::XNameContainer>& xNamespaces,
        sal_Int32 nPosition,
        sal_Int32 nSize )
        : mxContextNode( xContextNode ),
          mxModel( xModel ),
          mxNamespaces( xNamespaces ),
          mnContextPosition( nPosition ),
          mnContextSize( nSize )
    { }

    com::sun::star::uno::Reference<com::sun::star::xml::dom::XNode> mxContextNode;
    com::sun::star::uno::Reference<com::sun::star::xforms::XModel> mxModel;
    com::sun::star::uno::Reference<com::sun::star::container::XNameContainer> mxNamespaces;

    sal_Int32 mnContextPosition;
    sal_Int32 mnContextSize;
};


} // namespace xforms

#endif
