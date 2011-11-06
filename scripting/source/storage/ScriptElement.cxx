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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_scripting.hxx"

#include "ScriptElement.hxx"
#include <util/util.hxx>

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

typedef ::std::vector < ::std::pair < ::rtl::OUString, bool > >  dependencies_vec;
typedef ::std::vector < ::std::pair < ::rtl::OUString, ::rtl::OUString > > deliveries_vec;

namespace scripting_impl
{

//*************************************************************************
/**
    Construct a ScriptElement from a ScriptData object

    @param sII
    the ScriptDataObject
*/
ScriptElement::ScriptElement( ScriptData & sII ) :
        XMLElement( OUSTR( "parcel" ) ),
        m_sII( sII )
{
    OSL_TRACE( "ScriptElement ctor called\n" );

    addAttribute( OUSTR( "language" ), sII.language );
    addAttribute( OUSTR( "xmlns:parcel" ), OUSTR( "scripting.dtd" ) );
    XMLElement* xScriptElt = new XMLElement( OUSTR( "script" ) );
    xScriptElt->addAttribute( OUSTR( "language" ), sII.language );
    Reference < xml::sax::XAttributeList > xal( xScriptElt );
    addSubElement( xal );

    strpair_map::const_iterator mp_it = sII.locales.begin();
    strpair_map::const_iterator mp_itend = sII.locales.end();

    for( ; mp_it != mp_itend; ++mp_it )
    {
        XMLElement* xel = new XMLElement( OUSTR( "locale" ) );
        xel->addAttribute( OUSTR( "lang" ), mp_it->first );

        {
            XMLElement* subxel = new XMLElement( OUSTR( "displayname" ) );
            subxel->addAttribute( OUSTR( "value" ), mp_it->second.first );
            Reference < xml::sax::XAttributeList > subxattl( subxel );
            xel->addSubElement( subxattl );
        }
        {
            XMLElement* subxel = new XMLElement( OUSTR( "description" ),
                                     mp_it->second.second );
            Reference< xml::sax::XAttributeList > subxattl( subxel );
            xel->addSubElement( subxattl );
        }

        Reference < xml::sax::XAttributeList > xal( xel );
        xScriptElt->addSubElement( xal );
    }

    {
        XMLElement* xel = new XMLElement( OUSTR( "functionname" ) );
        xel->addAttribute( OUSTR( "value" ), sII.functionname );
        Reference < xml::sax::XAttributeList > xal( xel );
        xScriptElt->addSubElement( xal );
    }

    {
        XMLElement* xel = new XMLElement( OUSTR( "logicalname" ) );
        xel->addAttribute( OUSTR( "value" ), sII.logicalname );
        Reference < xml::sax::XAttributeList > xal( xel );
        xScriptElt->addSubElement( xal );
    }

    props_vec::const_iterator vp_it = sII.languagedepprops.begin();
    props_vec::const_iterator vp_itend = sII.languagedepprops.end();

    if ( vp_it != vp_itend )
    {
        XMLElement* xel = new XMLElement( OUSTR( "languagedepprops" ) );

        for( ; vp_it != vp_itend ; ++vp_it )
        {
            XMLElement* subxel =  new XMLElement( OUSTR( "prop" ) );
            subxel->addAttribute( OUSTR( "name" ), vp_it->first );
            subxel->addAttribute( OUSTR( "value" ), vp_it->second );
            Reference < xml::sax::XAttributeList > subxattl( subxel );
            xel->addSubElement( subxattl );
        }

        Reference < xml::sax::XAttributeList > xal( xel );
        xScriptElt->addSubElement( xal );
    }

    filesets_map::const_iterator fm_it = sII.filesets.begin();
    filesets_map::const_iterator fm_itend = sII.filesets.end();

    for( ; fm_it != fm_itend; ++fm_it )
    {
        XMLElement* xel = new XMLElement( OUSTR( "fileset" ) );
        xel->addAttribute( OUSTR( "name" ), fm_it->first );

        vp_it = fm_it->second.first.begin();
        vp_itend = fm_it->second.first.end();

        for( ; vp_it != vp_itend; ++vp_it )
        {
            XMLElement* subxel = new XMLElement( OUSTR( "prop" ) );
            subxel->addAttribute( OUSTR( "name" ), vp_it->first );
            subxel->addAttribute( OUSTR("value"), vp_it->second );
            Reference < xml::sax::XAttributeList > subxattl( subxel );
            xel->addSubElement( subxattl );
       }

       strpairvec_map::const_iterator sm_it = fm_it->second.second.begin();
       strpairvec_map::const_iterator sm_itend = fm_it->second.second.end();

       if( sm_it != sm_itend )
       {
           // was there a purpose for contstructing this
           // XMLElement* subxel = new XMLElement( OUSTR( "file" ) );
           xel->addAttribute( OUSTR( "name" ), sm_it->first );

       }
    }
}

//*************************************************************************
ScriptElement::~ScriptElement() SAL_THROW(())
{
}

} // namespace scripting_impl
