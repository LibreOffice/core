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

#ifndef INCLUDED_SC_SOURCE_UI_INC_DRAWUTIL_HXX
#define INCLUDED_SC_SOURCE_UI_INC_DRAWUTIL_HXX

#include "address.hxx"

namespace boost { template<typename T> class rational; }
class OutputDevice;
class ScDocument;

class ScDrawUtil
{
public:
    static void CalcScale( ScDocument* pDoc, SCTAB nTab,
                        SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                        OutputDevice* pDev, const boost::rational<long>& rZoomX, const boost::rational<long>& rZoomY,
                        double nPPTX, double nPPTY,
                        boost::rational<long>& rScaleX, boost::rational<long>& rScaleY );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
