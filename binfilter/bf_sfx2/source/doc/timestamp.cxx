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

// MARKER(update_precomp.py): autogen include statement, do not remove

//______________________________________________________________________________________________________________________________
//  my own includes
//______________________________________________________________________________________________________________________________

#include "timestamp.hxx"

#include <tools/debug.hxx>

#define TIMESTAMP_INVALID_DATETIME    	( DateTime ( Date ( 1, 1, 1601 ), Time ( 0, 0, 0 ) ) )	/// Invalid value for date and time to create invalid instance of TimeStamp.

/*******************************************************************************************************************
 *
 *	@short		default constructor
 *	@descr		Take default values for member initialization.
 *
 *	@ATTENTION	We use default constructor for member !!! And the default of "DateTime" is the current date and time.
 *
 ******************************************************************************************************************/
namespace binfilter {

TimeStamp::TimeStamp ()
        :	m_sModifiedByName	()
        ,	m_aModifiedDateTime	()
{
}

/*******************************************************************************************************************
 *
 *	@short		overloaded constructor
 *	@descr		Take default value for date and time. Name can be set.
 *
 *	@ATTENTION	We use default constructor for member !!! And the default of "DateTime" is the current date and time.
 *
 ******************************************************************************************************************/

TimeStamp::TimeStamp ( const String& rName )
        :   m_sModifiedByName   ( rName	)
        ,	m_aModifiedDateTime	(		)
{
    // Safe impossible cases
    DBG_ASSERT ( impl_debug_checkParameter ( rName ), "TimeStamp::TimeStamp(String)\nInvalid parameter detected!\n" ) ;

    // Make valid name.
    impl_adjustName ( m_sModifiedByName ) ;
}

/*******************************************************************************************************************
 *
 *	@short		overloaded constructor
 *	@descr		Take default value for name. Date and time can be set.
 *
 ******************************************************************************************************************/

TimeStamp::TimeStamp ( const DateTime& rDateTime )
        :	m_sModifiedByName	(			)
        ,   m_aModifiedDateTime ( rDateTime	)
{
    // Safe impossible cases
    DBG_ASSERT ( impl_debug_checkParameter ( rDateTime ), "TimeStamp::TimeStamp(DateTime)\nInvalid parameter detected!\n" ) ;
}

/*******************************************************************************************************************
 *
 *	@short		overloaded constructor
 *	@descr		All member can be set.
 *
 ******************************************************************************************************************/

TimeStamp::TimeStamp ( const String& rName, const DateTime& rDateTime )
        :   m_sModifiedByName   ( rName		)
        ,   m_aModifiedDateTime ( rDateTime )
{
    // Safe impossible cases
    DBG_ASSERT ( impl_debug_checkParameter ( rName, rDateTime ), "TimeStamp::TimeStamp(String,DateTime)\nInvalid parameter detected!\n" ) ;

    // Make valid name.
    impl_adjustName ( m_sModifiedByName ) ;
}

//______________________________________________________________________________________________________________________________
//  operator methods
//______________________________________________________________________________________________________________________________

/*******************************************************************************************************************
 *
 *	@short		Set one instance to another.
 *
 ******************************************************************************************************************/

const TimeStamp& TimeStamp::operator= ( const TimeStamp& rCopy )
{
    // Safe impossible cases
    DBG_ASSERT ( impl_debug_checkParameter ( rCopy ), "TimeStamp::operator=()\nInvalid parameter detected!\n" ) ;

    // Save new values.
    m_sModifiedByName	=	rCopy.m_sModifiedByName		;
    m_aModifiedDateTime	=	rCopy.m_aModifiedDateTime	;

    // Adjust name.
    impl_adjustName ( m_sModifiedByName ) ;

    // Return pointer to this instance.
    return *this ;
}

/*******************************************************************************************************************
 *
 *	@short		Compare two instances of this class.
 *	@return		TRUE on equal / FALSE on non equal
 *
 ******************************************************************************************************************/

BOOL TimeStamp::operator== ( const TimeStamp& rCompare ) const
{
    // Safe impossible cases
    DBG_ASSERT ( impl_debug_checkParameter ( rCompare ), "TimeStamp::operator==()\nInvalid parameter detected!\n" ) ;

    // Compare member of this two objects.
    // Safe result for better debug ! (to se value)
    BOOL bResult =  (
                        ( m_sModifiedByName     ==  rCompare.m_sModifiedByName      )   &&
                        ( m_aModifiedDateTime   ==  rCompare.m_aModifiedDateTime    )
                    ) ;

    return bResult ;
}

/*******************************************************************************************************************
 *
 *	@short		Compare two instances of this class.
 *	@return		TRUE on non equal / FALSE on equal
 *
 ******************************************************************************************************************/

int TimeStamp::operator!= ( const TimeStamp& rCompare ) const
{
    // Safe impossible cases
    DBG_ASSERT ( impl_debug_checkParameter ( rCompare ), "TimeStamp::operator!=()\nInvalid parameter detected!\n" ) ;

    // Use operator "==" !!!
    return ( ! operator== ( rCompare ) ) ;
}

//______________________________________________________________________________________________________________________________
//  other methods
//______________________________________________________________________________________________________________________________

/*******************************************************************************************************************
 *
 *	@short		Check state of object.
 *	@return		TRUE on OK / FALSE on error
 *
 ******************************************************************************************************************/

BOOL TimeStamp::IsValid () const
{
    // Timestamp is only valid, if member "m_aModifiedDateTime" not the default.
    // The name can have a defaultvalue!
    // And its better to safe this result in a local variable ... for better debug! (to see value)
    BOOL bResult = ( m_aModifiedDateTime != TIMESTAMP_INVALID_DATETIME ) ;
    return bResult && m_aModifiedDateTime.IsValid();
}


/*******************************************************************************************************************
 *
 *	@short      Load a timestamp from stream.
 *	@return		TRUE on OK / FALSE on error
 *
 ******************************************************************************************************************/

BOOL TimeStamp::Load ( SvStream& rStream )
{
    // Safe impossible cases
    DBG_ASSERT ( impl_debug_checkParameter ( rStream ), "TimeStamp::Load()\nInvalid parameter detected!\n" ) ;

    // Load name from stream.
    CharSet eConvertCharSet = rStream.GetStreamCharSet();
    DBG_ASSERT( !(eConvertCharSet == RTL_TEXTENCODING_DONTKNOW)	, "TimeStamp::Load()\nCharSet of stream is unknown. Can't convert bytestring to UniCode!\n" );
    DBG_ASSERT( !(eConvertCharSet == ((rtl_TextEncoding)9))		, "TimeStamp::Load()\nCharSet SYSTEM is obsolete. Can't convert bytestring to UniCode!\n"	);
    rStream.ReadByteString( m_sModifiedByName, eConvertCharSet );

    // Skip name in stream.

    // Follow operation "USHORT nSkip = ..." is right !!!
    // This fix old bugs of StarOffice versions before 5.1 ...
    // These versions write SfxStamps. And this implementation use USHORT to!
    // Follow subtraction will produce an overflow, if length of string greater then max. value.
    // We must keep ALL bytes of written strings - it can be up to 64K !!! ... :-(
    USHORT nSkip = TIMESTAMP_MAXLENGTH - m_sModifiedByName.Len () ;

    rStream.SeekRel ( nSkip ) ;

    // Adjust name.
    impl_adjustName ( m_sModifiedByName ) ;

    // Load date and time from stream.
    long nDate ;
    long nTime ;

    rStream >> nDate >> nTime ;

    // Safe date and time in member.
    m_aModifiedDateTime = DateTime ( Date ( nDate ), Time ( nTime ) ) ;

    // There was errors ...
    if ( rStream.GetError () != SVSTREAM_OK )
    {
        // Reset object to default => "IsValid()" return now FALSE !!!
        SetInvalid () ;
        // And return with ERROR.
        return FALSE ;
    }

    // OK.
    return TRUE ;
}

/*******************************************************************************************************************
 *
 *	@short		Save a timestamp to stream.
 *	@return		TRUE on OK / FALSE on error
 *
 ******************************************************************************************************************/

BOOL TimeStamp::Save ( SvStream& rStream ) const
{
    // Safe impossible cases
    DBG_ASSERT ( impl_debug_checkParameter ( rStream ), "TimeStamp::Save()\nInvalid parameter detected!\n" ) ;

    // DEFINE "TIMESTAMP_MAXLENGTH" is to short for this operation.
    DBG_ASSERT ( !( m_sModifiedByName.Len () > TIMESTAMP_MAXLENGTH ), "TimeStamp::Save()\n\"TIMESTAMP_MAXLENGTH\" is to short! Please change this." ) ;

    // Save name in stream.
    CharSet eConvertCharSet = rStream.GetStreamCharSet();
    DBG_ASSERT( !(eConvertCharSet == RTL_TEXTENCODING_DONTKNOW)	, "TimeStamp::Save()\nCharSet of stream is unknown. Can't convert bytestring to UniCode!\n" );
    DBG_ASSERT( !(eConvertCharSet == ((rtl_TextEncoding)9))		, "TimeStamp::Save()\nCharSet SYSTEM is obsolete. Can't convert bytestring to UniCode!\n"	);
    rStream.WriteByteString( m_sModifiedByName, eConvertCharSet );

    // Fill blanks in stream to get free place for follow date and time!
    USHORT nBlanksCount	= TIMESTAMP_MAXLENGTH - m_sModifiedByName.Len ()	;
    USHORT nCounter		= 0													;

    for ( nCounter = 0; nCounter < nBlanksCount; ++nCounter )
    {
        rStream << ' ';
    }

    // Save date and time in stream.
    rStream << (long) m_aModifiedDateTime.GetDate () ;
    rStream << (long) m_aModifiedDateTime.GetTime () ;

    // There was errors ...
    if ( rStream.GetError () != SVSTREAM_OK )
    {
        // Do not reset object to default !!!
        // Its not an error of member variables - is an error of streaming !
        // But return with ERROR.
        return FALSE ;
    }

    // OK.
    return TRUE ;
}

/*******************************************************************************************************************
 *
 *	@short		Set name of timestamp.
 *
 ******************************************************************************************************************/

void TimeStamp::SetName ( const String& rName )
{
    // Safe impossible cases
    DBG_ASSERT ( impl_debug_checkParameter ( rName ), "TimeStamp::SetName()\nInvalid parameter detected!\n" ) ;

    // Set new value at member.
    m_sModifiedByName = rName ;

    // Adjust member.
    impl_adjustName ( m_sModifiedByName ) ;
}

/*******************************************************************************************************************
 *
 *	@short		Set date and time of timestamp.
 *
 ******************************************************************************************************************/

void TimeStamp::SetTime ( const DateTime& rDateTime )
{
    // Safe impossible cases
    DBG_ASSERT ( impl_debug_checkParameter ( rDateTime ), "TimeStamp::SetTime()\nInvalid parameter detected!\n" ) ;

    m_aModifiedDateTime = rDateTime ;
}

/*******************************************************************************************************************
 *
 *	@short		Get name of timestamp.
 *
 ******************************************************************************************************************/

const String& TimeStamp::GetName () const
{
    return m_sModifiedByName ;
}

/*******************************************************************************************************************
 *
 *	@short		Get date and time of timestamp.
 *
 ******************************************************************************************************************/

const DateTime& TimeStamp::GetTime () const
{
    return m_aModifiedDateTime ;
}

//______________________________________________________________________________________________________________________________
//  protected methods
//______________________________________________________________________________________________________________________________

//______________________________________________________________________________________________________________________________
//  private methods
//______________________________________________________________________________________________________________________________

/*******************************************************************************************************************
 *
 *	@short		Reset member of instance to invalid values. The method "IsValid()" return now FALSE !!!
 *
 ******************************************************************************************************************/

void TimeStamp::SetInvalid ()
{
    m_sModifiedByName	=	String()					;
    m_aModifiedDateTime	=	TIMESTAMP_INVALID_DATETIME	;
}

/*******************************************************************************************************************
 *
 *	@short		Adjust member "m_sModifiedByName".
 *
 ******************************************************************************************************************/

void TimeStamp::impl_adjustName ( String& rName )
{
    // If there to much letters ...
    if ( rName.Len () > TIMESTAMP_MAXLENGTH )
    {
        // .. delete it.
        rName.Erase ( TIMESTAMP_MAXLENGTH ) ;
    }
}

//______________________________________________________________________________________________________________________________
//  debug methods
//______________________________________________________________________________________________________________________________

#ifdef DBG_UTIL

/*******************************************************************************************************************
 *
 *	@descr		The follow methods "impl_debug_checkParameter()" checks parameter for other methods.
 *				The return value is used for a DEBUG_ASSERT in "other methods".
 *
 *				There are no check-methods for ALL public-methods ... only for different parameters!
 *
 ******************************************************************************************************************/

BOOL TimeStamp::impl_debug_checkParameter ( const String& rString ) const
{
    if ( &rString		==	NULL	) return FALSE ;	// NULL-Pointer as reference :-(
//	if ( rString.Len ()	<	1		) return FALSE ;	// This is an invalid text for a name!

    // OK.
    return TRUE ;
}

BOOL TimeStamp::impl_debug_checkParameter ( const String& rString, const DateTime& rDateTime ) const
{
    if ( &rString		==	NULL						) return FALSE ;	// NULL-Pointer as reference :-(
    if ( &rDateTime		==	NULL						) return FALSE ;	// NULL-Pointer as reference :-(
//	if ( rString.Len ()	<	1							) return FALSE ;	// This is an invalid text for a name!
//	if ( rDateTime		==	TIMESTAMP_INVALID_DATETIME	) return FALSE ;	// This will set "IsValid()" to FALSE !!!

    // OK.
    return TRUE ;
}

BOOL TimeStamp::impl_debug_checkParameter ( const TimeStamp& rTimeStamp ) const
{
    if ( &rTimeStamp			==	NULL	) return FALSE ;	// NULL-Pointer as reference :-(
//	if ( rTimeStamp.IsValid ()	==	FALSE	) return FALSE ;	// Somewhere will set invalid values ??!!

    // OK.
    return TRUE ;
}

BOOL TimeStamp::impl_debug_checkParameter ( SvStream& rSvStream ) const
{
    if ( &rSvStream				==	NULL	   	) return FALSE ;	// NULL-Pointer as reference :-(
    if ( rSvStream.GetError ()	!=	SVSTREAM_OK	) return FALSE ;	// This stream has errors!

    // OK.
    return TRUE ;
}

BOOL TimeStamp::impl_debug_checkParameter ( const DateTime& rDateTime ) const
{
    if ( &rDateTime	==	NULL						) return FALSE ;	// NULL-Pointer as reference :-(
//	if ( rDateTime	==	TIMESTAMP_INVALID_DATETIME	) return FALSE ;	// This will set "IsValid()" to FALSE !!!

    // OK.
    return TRUE ;
}

#endif // DBG_UTIL

}