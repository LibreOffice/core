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
#ifndef _SFX2_TIMESTAMP_HXX
#define _SFX2_TIMESTAMP_HXX

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif

namespace binfilter {

//______________________________________________________________________________________________________________________________
//  defines
//______________________________________________________________________________________________________________________________

#define TIMESTAMP_MAXLENGTH       		31														/// Max. length of valid timestamp in stream.

//______________________________________________________________________________________________________________________________
//  classes, structs
//______________________________________________________________________________________________________________________________

class TimeStamp
{
    //--------------------------------------------------------------------------------
    //  public method
    //--------------------------------------------------------------------------------
    public:

        //--------------------------------------------------------------------------------
        //  construct / destruct
        //--------------------------------------------------------------------------------

        TimeStamp (													) ;
        TimeStamp ( const String& rName								) ;
        TimeStamp ( const DateTime& rDateTime						) ;
        TimeStamp ( const String& rName, const DateTime& rDateTime	) ;

        //--------------------------------------------------------------------------------
        //  operator methods
        //--------------------------------------------------------------------------------

        const TimeStamp&	operator=	( const TimeStamp& rCopy	)	    ;
        BOOL				operator==	( const TimeStamp& rCompare ) const ;
        int					operator!=	( const TimeStamp& rCompare ) const ;

        //--------------------------------------------------------------------------------
        //  other methods
        //--------------------------------------------------------------------------------

        BOOL				IsValid	(							) const ;
        void				SetInvalid();
        BOOL				Load	( SvStream& rStream 		)       ;
        BOOL				Save 	( SvStream& rStream 		) const ;
        void				SetName ( const String& rName 		)       ;
        void				SetTime ( const DateTime& rDateTime )       ;
        const String&		GetName (							) const ;
        const DateTime&		GetTime (							) const ;

    //--------------------------------------------------------------------------------
    //  protected methods
    //--------------------------------------------------------------------------------
    protected:

    //--------------------------------------------------------------------------------
    //  private methods
    //--------------------------------------------------------------------------------
    private:

        void	impl_adjustName		( String& rName ) ;

        //--------------------------------------------------------------------------------
        //  debug methods
        //--------------------------------------------------------------------------------

        #ifdef DBG_UTIL

        BOOL	impl_debug_checkParameter	( const String& rString								) const ;
        BOOL	impl_debug_checkParameter	( const String& rString, const DateTime& rDateTime	) const ;
        BOOL	impl_debug_checkParameter	( const TimeStamp& rTimeStamp						) const ;
        BOOL	impl_debug_checkParameter	( SvStream& rSvStream								) const ;
        BOOL	impl_debug_checkParameter	( const DateTime& rDateTime							) const ;

        #endif // DBG_UTIL

    //--------------------------------------------------------------------------------
    //  private variables
    //--------------------------------------------------------------------------------
    private:

        String		m_sModifiedByName	;	/// Name of stamp
        DateTime	m_aModifiedDateTime	;	/// Time and date of stamp
} ;

}
#endif // _TIMESTAMP_HXX
