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

#ifndef INCLUDED_SDEXT_SOURCE_PDFIMPORT_INC_XMLEMITTER_HXX
#define INCLUDED_SDEXT_SOURCE_PDFIMPORT_INC_XMLEMITTER_HXX

#include "pdfihelper.hxx"
#include <memory>

namespace pdfi
{
    /** Output interface to ODF

        Should be easy to implement using either SAX events or plain ODF
     */
    class XmlEmitter
    {
    public:
        virtual ~XmlEmitter() {}

        /** Open up a tag with the given properties
         */
        virtual void beginTag( const char* pTag, const PropertyMap& rProperties ) = 0;
        /** Write PCTEXT as-is to output
         */
        virtual void write( const OUString& rString ) = 0;
        /** Close previously opened tag
         */
        virtual void endTag( const char* pTag ) = 0;
    };

    typedef std::shared_ptr<XmlEmitter> XmlEmitterSharedPtr;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
