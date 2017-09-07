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

#include "vbaovalshape.hxx"

using namespace com::sun::star;
using namespace ooo::vba;

/*
 * This is implemented as a new class in order to provide XTypeProvider
 * interface. This is needed by TypeOf ... Is ... basic operator.
 */

ScVbaOvalShape::ScVbaOvalShape( const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< drawing::XShape >& xShape, const uno::Reference< drawing::XShapes >& xShapes, const uno::Reference< frame::XModel >& xModel ) : OvalShapeImpl_BASE( uno::Reference< XHelperInterface >(), xContext, xShape, xShapes, xModel, ScVbaShape::getType( xShape )  )
{}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
