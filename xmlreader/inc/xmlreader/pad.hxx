/*************************************************************************
*
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
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

#ifndef INCLUDED_XMLREADER_PAD_HXX
#define INCLUDED_XMLREADER_PAD_HXX

#include "sal/config.h"

#include "rtl/strbuf.hxx"
#include "sal/types.h"
#include "xmlreader/detail/xmlreaderdllapi.hxx"
#include "xmlreader/span.hxx"

namespace xmlreader {

class OOO_DLLPUBLIC_XMLREADER Pad {
public:
    void add(char const * begin, sal_Int32 length);

    void addEphemeral(char const * begin, sal_Int32 length);

    void clear();

    Span get() const;

private:
    SAL_DLLPRIVATE void flushSpan();

    Span span_;
    rtl::OStringBuffer buffer_;
};

}

#endif
