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

#ifndef INCLUDED_BASEBMP_DEBUG_HXX
#define INCLUDED_BASEBMP_DEBUG_HXX

#include <iostream>
#include <memory>
#include <basebmp/basebmpdllapi.h>

namespace basebmp
{
    class BitmapDevice;

    /** Dump content of BitmapDevice to given output stream.

        @param rDevice
        Device whose content should be dumped.

        @param rOutputStream
        Stream to write output to.

        Used in vcl/headless/svpgdi.cxx when OSL_DEBUG_LEVEL > 2

        Use like this:
<pre>
        #include <basebmp/debug.hxx>
        #include <iostream>
        #include <fstream>

        std::ofstream output("/tmp/my_test.dump");
        debugDump( pMyDevice, output );
</pre>
    */
    void BASEBMP_DLLPUBLIC debugDump( const std::shared_ptr< BitmapDevice >& rDevice,
                                      ::std::ostream&                          rOutputStream );
}

#endif /* INCLUDED_BASEBMP_DEBUG_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
