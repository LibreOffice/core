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

#include "oox/drawingml/chart/modelbase.hxx"

#include "oox/helper/attributelist.hxx"

namespace oox {
namespace drawingml {
namespace chart {

NumberFormat::NumberFormat() :
    mbSourceLinked( true )
{
}

void NumberFormat::setAttributes( const AttributeList& rAttribs )
{
    mbSourceLinked = rAttribs.getBool( XML_sourceLinked, true);
    maFormatCode = rAttribs.getString( XML_formatCode, OUString() );
}

LayoutModel::LayoutModel() :
    mfX( 0.0 ),
    mfY( 0.0 ),
    mfW( 0.0 ),
    mfH( 0.0 ),
    mnXMode( XML_factor ),
    mnYMode( XML_factor ),
    mnWMode( XML_factor ),
    mnHMode( XML_factor ),
    mnTarget( XML_outer ),
    mbAutoLayout( true )
{
}

LayoutModel::~LayoutModel()
{
}

}
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
