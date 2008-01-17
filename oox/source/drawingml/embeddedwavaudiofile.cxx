/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: embeddedwavaudiofile.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
    void getEmbeddedWAVAudioFile( const FragmentHandlerRef &xHandler, const Reference< XFastAttributeList >& xAttribs,
                                                                EmbeddedWAVAudioFile & aAudio )
    {
        AttributeList attribs(xAttribs);

        OUString sId = xAttribs->getOptionalValue( NMSP_RELATIONSHIPS|XML_embed );
        aAudio.msLink = xHandler->getRelations().getTargetFromRelId( sId );
        aAudio.mbBuiltIn = attribs.getBool( XML_builtIn, false );
        aAudio.msName = xAttribs->getOptionalValue( XML_name );
    }


} }
