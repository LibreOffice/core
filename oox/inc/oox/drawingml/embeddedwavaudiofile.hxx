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

#ifndef OOX_DRAWINGML_EMBEDDEDWAVAUDIOFILE_HXX
#define OOX_DRAWINGML_EMBEDDEDWAVAUDIOFILE_HXX

#include <rtl/ustring.hxx>
#include <com/sun/star/xml/sax/XFastAttributeList.hpp>

#include "oox/core/fragmenthandler.hxx"

namespace oox { namespace drawingml {

    struct EmbeddedWAVAudioFile
    {
        EmbeddedWAVAudioFile()
            : mbBuiltIn(false)
            {
            }
        bool mbBuiltIn;
        OUString msName;
        OUString msEmbed;
    };

    void getEmbeddedWAVAudioFile(
            const ::oox::core::Relations& rRelations,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs,
            EmbeddedWAVAudioFile & aAudio );

} }


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
