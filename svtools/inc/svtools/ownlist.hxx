/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ownlist.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:30:35 $
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

#ifndef _OWNLIST_HXX
#define _OWNLIST_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
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

//=========================================================================
class SvCommand
/*  [Beschreibung]

    Enth"alt einen String, welcher das Kommando angibt und eine weiteren
    String, der das Argument des Kommandos bildet. W"urde solch ein
    Kommando "uber die Kommandozeile angegeben werden, s"ahe es wie folgt
    aus: Kommando = Argument.
*/
{
    String  aCommand;
    String  aArgument;
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
class SVT_DLLPUBLIC SvCommandList
/*  [Beschreibung]

    Die Liste enth"alt Objekte vom Typ SvCommand. Wird ein Objekt
    eingef"ugt, dann wird es kopiert und das neue Objekt wird
    in die Liste gestellt.
*/
{
                    PRV_SV_DECL_OWNER_LIST(SvCommandList,SvCommand);
    SvCommand &     Append( const String & rCommand, const String & rArg );
    BOOL            AppendCommands( const String & rCmd, USHORT * pEaten );
    String          GetCommands() const;

    BOOL FillFromSequence( const com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue >& );
    void FillSequence( com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue >& );

    SVT_DLLPUBLIC friend SvStream& operator >> ( SvStream& rStm, SvCommandList & );
    SVT_DLLPUBLIC friend SvStream& operator << ( SvStream&, const SvCommandList & );
};

#endif // _OWNLIST_HXX
