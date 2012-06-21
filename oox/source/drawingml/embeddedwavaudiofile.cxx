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

#include "oox/drawingml/embeddedwavaudiofile.hxx"
#include "oox/helper/attributelist.hxx"

using ::rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;


namespace oox { namespace drawingml {


    // CT_EmbeddedWAVAudioFile
    void getEmbeddedWAVAudioFile( const Relations& rRelations,
            const Reference< XFastAttributeList >& xAttribs, EmbeddedWAVAudioFile & aAudio )
    {
        AttributeList attribs(xAttribs);

        OUString sId = xAttribs->getOptionalValue( R_TOKEN( embed ) );
        aAudio.msEmbed = rRelations.getFragmentPathFromRelId( sId );
        aAudio.mbBuiltIn = attribs.getBool( XML_builtIn, false );
        aAudio.msName = xAttribs->getOptionalValue( XML_name );
    }


} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
