/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ScriptElement.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 12:30:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
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
