/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ScriptElement.cxx,v $
 * $Revision: 1.10 $
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
