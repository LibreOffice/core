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

// ============================================================================

#ifndef SC_ASCIIOPT_HXX
#define SC_ASCIIOPT_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif
#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _SVX_TXENCBOX_HXX
#include <bf_svx/txencbox.hxx>
#endif

#ifndef _SC_CSVTABLEBOX_HXX
#include "csvtablebox.hxx"
#endif
namespace binfilter {


// ============================================================================

class ScAsciiOptions
{
private:
    BOOL		bFixedLen;
    String		aFieldSeps;
    BOOL		bMergeFieldSeps;
    sal_Unicode	cTextSep;
    CharSet		eCharSet;
    BOOL		bCharSetSystem;
    long		nStartRow;
    USHORT		nInfoCount;
    xub_StrLen* pColStart;  //! TODO replace with vector
    BYTE*       pColFormat; //! TODO replace with vector

public:
                    ScAsciiOptions();
                    ScAsciiOptions(const ScAsciiOptions& rOpt);
                    ~ScAsciiOptions();



    void			ReadFromString( const String& rString );


    CharSet				GetCharSet() const		{ return eCharSet; }
    BOOL				GetCharSetSystem() const	{ return bCharSetSystem; }
    const String&		GetFieldSeps() const	{ return aFieldSeps; }
    BOOL				IsMergeSeps() const		{ return bMergeFieldSeps; }
    sal_Unicode			GetTextSep() const		{ return cTextSep; }
    BOOL				IsFixedLen() const		{ return bFixedLen; }
    USHORT				GetInfoCount() const	{ return nInfoCount; }
    const xub_StrLen*	GetColStart() const		{ return pColStart; }
    const BYTE*			GetColFormat() const	{ return pColFormat; }
    long				GetStartRow() const		{ return nStartRow; }

    void	SetCharSet( CharSet eNew )			{ eCharSet = eNew; }
    void	SetCharSetSystem( BOOL bSet )		{ bCharSetSystem = bSet; }
    void	SetFixedLen( BOOL bSet )			{ bFixedLen = bSet; }
    void	SetFieldSeps( const String& rStr )	{ aFieldSeps = rStr; }
    void	SetMergeSeps( BOOL bSet )			{ bMergeFieldSeps = bSet; }
    void	SetTextSep( sal_Unicode c )			{ cTextSep = c; }
    void	SetStartRow( long nRow)				{ nStartRow= nRow; }

};


// ============================================================================



// ============================================================================

} //namespace binfilter
#endif

