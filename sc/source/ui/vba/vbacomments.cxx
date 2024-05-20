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
#include "vbacomments.hxx"
#include "vbacomment.hxx"

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/sheet/XSheetAnnotation.hpp>
#include <com/sun/star/table/XCellRange.hpp>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

static uno::Any AnnotationToComment( const uno::Any& aSource, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< frame::XModel >& xModel )
{
    uno::Reference< sheet::XSheetAnnotation > xAnno( aSource, uno::UNO_QUERY_THROW );
    uno::Reference< container::XChild > xChild( xAnno, uno::UNO_QUERY_THROW );
    uno::Reference< table::XCellRange > xCellRange( xChild->getParent(), uno::UNO_QUERY_THROW );

    // #FIXME needs to find the correct Parent
    return uno::Any( uno::Reference< excel::XComment > (
        new ScVbaComment( uno::Reference< XHelperInterface >(), xContext, xModel, xCellRange ) ) );
}

namespace {

class CommentEnumeration : public EnumerationHelperImpl
{
    css::uno::Reference< css::frame::XModel > mxModel;
public:
    /// @throws uno::RuntimeException
    CommentEnumeration(
            const uno::Reference< XHelperInterface >& xParent,
            const uno::Reference< uno::XComponentContext >& xContext,
            const uno::Reference< container::XEnumeration >& xEnumeration,
            const uno::Reference< frame::XModel >& xModel ) :
        EnumerationHelperImpl( xParent, xContext, xEnumeration ),
        mxModel( xModel, uno::UNO_SET_THROW )
    {}

    virtual uno::Any SAL_CALL nextElement() override
    {
        return AnnotationToComment( m_xEnumeration->nextElement(), m_xContext, mxModel );
    }

};

}

ScVbaComments::ScVbaComments(
        const uno::Reference< XHelperInterface >& xParent,
        const uno::Reference< uno::XComponentContext > & xContext,
        const uno::Reference< frame::XModel >& xModel,
        const uno::Reference< container::XIndexAccess >& xIndexAccess  ) :
    ScVbaComments_BASE( xParent, xContext, xIndexAccess ),
    mxModel( xModel, uno::UNO_SET_THROW )
{
}

// public helper functions

uno::Reference< container::XEnumeration >
ScVbaComments::createEnumeration()
{
    uno::Reference< container::XEnumerationAccess > xEnumAccess( m_xIndexAccess, uno::UNO_QUERY_THROW );
    return new CommentEnumeration( mxParent, mxContext, xEnumAccess->createEnumeration(), mxModel );
}

uno::Any
ScVbaComments::createCollectionObject( const css::uno::Any& aSource )
{
    return AnnotationToComment( aSource,  mxContext, mxModel );
}

uno::Type
ScVbaComments::getElementType()
{
    return cppu::UnoType<excel::XComment>::get();
}

OUString
ScVbaComments::getServiceImplName()
{
    return u"ScVbaComments"_ustr;
}

css::uno::Sequence<OUString>
ScVbaComments::getServiceNames()
{
    static uno::Sequence< OUString > const sNames
    {
        u"ooo.vba.excel.Comments"_ustr
    };
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
