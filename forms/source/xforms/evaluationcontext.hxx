/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: evaluationcontext.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:17:42 $
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
