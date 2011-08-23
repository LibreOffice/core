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

#ifndef _OWNLIST_HXX
#define _OWNLIST_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "bf_svtools/svtdllapi.h"
#endif

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _TOOLS_OWNLIST_HXX //autogen
#include <tools/ownlist.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

namespace com { namespace sun { namespace star {
    namespace beans {
        struct PropertyValue;
    }
}}}

namespace binfilter
{


//=========================================================================
class SvCommand
/*	[Beschreibung]

    Enth"alt einen String, welcher das Kommando angibt und eine weiteren
    String, der das Argument des Kommandos bildet. W"urde solch ein
    Kommando "uber die Kommandozeile angegeben werden, s"ahe es wie folgt
    aus: Kommando = Argument.
*/
{
    String	aCommand;
    String	aArgument;
public:
                    SvCommand() {}
                    SvCommand( const String & rCommand, const String & rArg )
                    {
                        aCommand = rCommand;
                        aArgument = rArg;
                    }
    const String & GetCommand() const { return aCommand; }
    const String & GetArgument() const { return aArgument; }

    friend SvStream& operator >> ( SvStream& rStm, SvCommand & rThis )
                    {
                        rStm.ReadByteString( rThis.aCommand, gsl_getSystemTextEncoding() );
                        rStm.ReadByteString( rThis.aArgument, gsl_getSystemTextEncoding() );
                        return rStm;
                    }
    friend SvStream& operator << ( SvStream& rStm, const SvCommand & rThis )
                    {
                        rStm.WriteByteString( rThis.aCommand, gsl_getSystemTextEncoding() );
                        rStm.WriteByteString( rThis.aArgument, gsl_getSystemTextEncoding() );
                        return rStm;
                    }
};

//=========================================================================
class SvCommandList
/*	[Beschreibung]

    Die Liste enth"alt Objekte vom Typ SvCommand. Wird ein Objekt
    eingef"ugt, dann wird es kopiert und das neue Objekt wird
    in die Liste gestellt.
*/
{
                    PRV_SV_DECL_OWNER_LIST(SvCommandList,SvCommand);
    SvCommand &		Append( const String & rCommand, const String & rArg );

     friend SvStream& operator >> ( SvStream& rStm, SvCommandList & );
     friend SvStream& operator << ( SvStream&, const SvCommandList & );
};

}

#endif // _OWNLIST_HXX
