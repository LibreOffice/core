/*************************************************************************
 *
 *  $RCSfile: SwAppletImpl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mtg $ $Date: 2001-02-27 17:04:33 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Martin Gallwey (gallwey@sun.com)
 *
 *
 ************************************************************************/

#ifndef _SW_APPLET_IMPL_HXX
#include <SwAppletImpl.hxx>
#endif

/* Some MIB magic...*/

sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_hidden, "HIDDEN" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_HIDDEN_false, "FALSE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_archive, "ARCHIVE" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_archives, "ARCHIVES" );
sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DEF( sHTML_O_object, "OBJECT" );

USHORT SwApplet_Impl::GetOptionType( const String& rName, BOOL bApplet )
{
    USHORT nType = bApplet ? SWHTML_OPTTYPE_PARAM : SWHTML_OPTTYPE_TAG;

    switch( rName.GetChar(0) )
    {
    case 'A':
    case 'a':
        if( rName.EqualsIgnoreCaseAscii( sHTML_O_align ) ||
            rName.EqualsIgnoreCaseAscii( sHTML_O_alt ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        else if( bApplet &&
                 (rName.EqualsIgnoreCaseAscii( sHTML_O_archive ) ||
                 rName.EqualsIgnoreCaseAscii( sHTML_O_archives )) )
            nType = SWHTML_OPTTYPE_TAG;
        break;
    case 'C':
    case 'c':
        if( rName.EqualsIgnoreCaseAscii( sHTML_O_class ) ||
            (bApplet && (rName.EqualsIgnoreCaseAscii( sHTML_O_code ) ||
                         rName.EqualsIgnoreCaseAscii( sHTML_O_codebase ))) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'H':
    case 'h':
        if( rName.EqualsIgnoreCaseAscii( sHTML_O_height ) ||
            rName.EqualsIgnoreCaseAscii( sHTML_O_hspace ) ||
            (!bApplet && rName.EqualsIgnoreCaseAscii( sHTML_O_hidden )) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'I':
    case 'i':
        if( rName.EqualsIgnoreCaseAscii( sHTML_O_id ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'M':
    case 'm':
        if( bApplet && rName.EqualsIgnoreCaseAscii( sHTML_O_mayscript ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'N':
    case 'n':
        if( rName.EqualsIgnoreCaseAscii( sHTML_O_name ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'O':
    case 'o':
        if( bApplet && rName.EqualsIgnoreCaseAscii( sHTML_O_object ) )
            nType = SWHTML_OPTTYPE_TAG;
        break;
    case 'S':
    case 's':
        if( rName.EqualsIgnoreCaseAscii( sHTML_O_style ) ||
            (!bApplet && rName.EqualsIgnoreCaseAscii( sHTML_O_src )) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'T':
    case 't':
        if( !bApplet && rName.EqualsIgnoreCaseAscii( sHTML_O_type ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'V':
    case 'v':
        if( rName.EqualsIgnoreCaseAscii( sHTML_O_vspace ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'W':
    case 'w':
        if( rName.EqualsIgnoreCaseAscii( sHTML_O_width ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    }

    return nType;
}
SwApplet_Impl::SwApplet_Impl( SfxItemPool& rPool, USHORT nWhich1, USHORT nWhich2 ) :
        aItemSet( rPool, nWhich1, nWhich2 )
{
}

void SwApplet_Impl::CreateApplet( const String& rCode, const String& rName,
                                      BOOL bMayScript, const String& rCodeBase)
                                      //const String& rAlt )
{
    SvStorageRef pStor = new SvStorage( String(), STREAM_STD_READWRITE );
    xApplet = new SvAppletObject();
    xApplet->DoInitNew( pStor );

    xApplet->EnableSetModified( FALSE );
    xApplet->SetClass( rCode );
    xApplet->SetName( rName );
    xApplet->SetMayScript( bMayScript );
    xApplet->SetDocBase( INetURLObject::GetBaseURL() );

    String sCodeBase;
    if( rCodeBase.Len() )
    {
        INetURLObject aTmpURL;

        INetProtocol eProt = aTmpURL.CompareProtocolScheme( rCodeBase );
        if( eProt==INET_PROT_NOT_VALID &&
            rCodeBase.Search( ':' ) != STRING_NOTFOUND  )
        {
            // The codebase contains an unknown protocol rather than
            // a relative URL.
            sCodeBase = rCodeBase;
        }
        else
        {
            sCodeBase = INetURLObject::RelToAbs( rCodeBase );
        }
    }
    else
    {
        INetURLObject aTmpURL( INetURLObject::GetBaseURL() );
        sCodeBase = aTmpURL.GetPartBeforeLastName();
    }

    xApplet->SetCodeBase( sCodeBase );

    //sAlt = rAlt;
}
SwApplet_Impl::~SwApplet_Impl()
{
    xApplet.Clear();
}
void SwApplet_Impl::FinishApplet()
{
    xApplet->SetCommandList( aCommandList );
    xApplet->EnableSetModified( TRUE );
}

void SwApplet_Impl::AppendParam( const String& rName, const String& rValue )
{
    aCommandList.Append( rName, rValue );
}
