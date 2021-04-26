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

#ifndef INCLUDED_SVL_SOURCE_INC_STRINGIO_HXX
#define INCLUDED_SVL_SOURCE_INC_STRINGIO_HXX

#include <rtl/ustring.hxx>

class SvStream;

/** Read in a Unicode string from a streamed byte string representation.

    @param rStream  Some (input) stream.  Its Stream/TargetCharSets must
    be set to correct values!

    @return  On success, returns the reconstructed Unicode string.
 */
OUString readByteString(SvStream& rStream);

/** Write a byte string representation of a Unicode string into a stream.

    @param rStream  Some (output) stream.  Its Stream/TargetCharSets must
    be set to correct values!

    @param rString  Some Unicode string.
 */
void writeByteString(SvStream& rStream, std::u16string_view rString);

#endif // INCLUDED_SVL_SOURCE_INC_STRINGIO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
