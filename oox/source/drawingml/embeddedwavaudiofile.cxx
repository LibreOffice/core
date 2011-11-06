/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
