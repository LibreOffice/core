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

#include "oox/drawingml/embeddedwavaudiofile.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/core/namespaces.hxx"

#include "tokens.hxx"

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

        OUString sId = xAttribs->getOptionalValue( NMSP_RELATIONSHIPS|XML_embed );
        aAudio.msEmbed = rRelations.getFragmentPathFromRelId( sId );
        aAudio.mbBuiltIn = attribs.getBool( XML_builtIn, false );
        aAudio.msName = xAttribs->getOptionalValue( XML_name );
    }


} }
