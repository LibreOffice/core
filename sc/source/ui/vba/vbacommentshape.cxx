/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright IBM Corporation 2009, 2010.
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "vbacommentshape.hxx"

using namespace com::sun::star;
using namespace ooo::vba;

ScVbaCommentShape::ScVbaCommentShape( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< drawing::XShape >& xShape, const uno::Reference< excel::XComment >& xComment,
    const uno::Reference< drawing::XShapes >& xShapes, const uno::Reference< frame::XModel >& xModel, sal_Int32 nType ) : ScVbaShape( xParent, xContext, xShape, xShapes, xModel, nType )
{
    m_xComment.set( xComment, uno::UNO_QUERY );
}

void SAL_CALL ScVbaCommentShape::Delete() throw (uno::RuntimeException)
{
    if ( m_xComment.is() )
    {
        m_xComment->Delete();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
