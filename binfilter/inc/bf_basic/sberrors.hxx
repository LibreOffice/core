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

#ifndef _SB_SBERRORS_HXX
#define _SB_SBERRORS_HXX

#ifndef _SBXDEF_HXX //autogen
#include "sbxdef.hxx"
#endif

namespace binfilter {

#ifndef __RSC
typedef ULONG SbError;
#endif

// Abbildung auf SbxError
#define	ERRCODE_BASIC_SYNTAX				ERRCODE_SBX_SYNTAX       			// unspecified syntax error
#define	ERRCODE_BASIC_BAD_ARGUMENT			ERRCODE_SBX_NOTIMP   				// Invalid procedure call
#define ERRCODE_BASIC_MATH_OVERFLOW			ERRCODE_SBX_OVERFLOW   				// Overflow
#define	ERRCODE_BASIC_OUT_OF_RANGE			ERRCODE_SBX_BOUNDS     				// Subscript out of range
#define	ERRCODE_BASIC_ZERODIV				ERRCODE_SBX_ZERODIV    				// Division by zero
#define	ERRCODE_BASIC_CONVERSION			ERRCODE_SBX_CONVERSION    			// Type mismatch
#define	ERRCODE_BASIC_BAD_PARAMETER			ERRCODE_SBX_BAD_PARAMETER			// Invalid Parameter
#define	ERRCODE_BASIC_PROC_UNDEFINED		ERRCODE_SBX_PROC_UNDEFINED			// Sub or Function not defined
#define ERRCODE_BASIC_INTERNAL_ERROR		ERRCODE_SBX_ERROR					// internal error
#define	ERRCODE_BASIC_NO_OBJECT				ERRCODE_SBX_NO_OBJECT				// Object variable not set
#define	ERRCODE_BASIC_CANNOT_LOAD			ERRCODE_SBX_CANNOT_LOAD				// Can't load module
#define	ERRCODE_BASIC_BAD_INDEX				ERRCODE_SBX_BAD_INDEX				// Invalid object index
#define ERRCODE_BASIC_NO_ACTIVE_OBJECT		ERRCODE_SBX_NO_ACTIVE_OBJECT		// No active view or document
#define	ERRCODE_BASIC_BAD_PROP_VALUE		ERRCODE_SBX_BAD_PROP_VALUE			// Bad property value
#define	ERRCODE_BASIC_PROP_READONLY			ERRCODE_SBX_PROP_READONLY			// Property is read only
#define	ERRCODE_BASIC_PROP_WRITEONLY		ERRCODE_SBX_PROP_WRITEONLY			// Property is write only
#define	ERRCODE_BASIC_INVALID_OBJECT		ERRCODE_SBX_INVALID_OBJECT			// Invalid object reference
#define	ERRCODE_BASIC_NO_METHOD				ERRCODE_SBX_NO_METHOD				// Property or method not found
#define	ERRCODE_BASIC_INVALID_USAGE_OBJECT	ERRCODE_SBX_INVALID_USAGE_OBJECT	// Invalid usee of object
#define	ERRCODE_BASIC_NO_OLE				ERRCODE_SBX_NO_OLE					// Class does not support OLE
#define	ERRCODE_BASIC_BAD_METHOD			ERRCODE_SBX_BAD_METHOD				// Object doesn't support method
#define	ERRCODE_BASIC_OLE_ERROR				ERRCODE_SBX_OLE_ERROR				// OLE Automation error
#define	ERRCODE_BASIC_BAD_ACTION			ERRCODE_SBX_BAD_ACTION				// Object doesn't support this action
#define	ERRCODE_BASIC_NO_NAMED_ARGS			ERRCODE_SBX_NO_NAMED_ARGS			// Object doesn't support named args
#define	ERRCODE_BASIC_BAD_LOCALE			ERRCODE_SBX_BAD_LOCALE				// Object doesn't support current locale setting
#define	ERRCODE_BASIC_NAMED_NOT_FOUND		ERRCODE_SBX_NAMED_NOT_FOUND			// Named argument not found
#define	ERRCODE_BASIC_NOT_OPTIONAL			ERRCODE_SBX_NOT_OPTIONAL			// Argument not optional
#define	ERRCODE_BASIC_WRONG_ARGS			ERRCODE_SBX_WRONG_ARGS				// Wrong number of arguments
#define	ERRCODE_BASIC_NOT_A_COLL			ERRCODE_SBX_NOT_A_COLL				// Object not a collection


// Basic-spezifische Fehlermeldungen an ERRCODE_AREA_SBX anhaengen
#define	ERRCODE_BASIC_NO_GOSUB				((LAST_SBX_ERROR_ID+1UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)				// Return without Gosub
#define	ERRCODE_BASIC_REDO_FROM_START		((LAST_SBX_ERROR_ID+2UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)				// Redo form start (SB internal)
#define	ERRCODE_BASIC_NO_MEMORY				((LAST_SBX_ERROR_ID+3UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)   			// Out of memory
#define	ERRCODE_BASIC_ALREADY_DIM			((LAST_SBX_ERROR_ID+4UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)    			// Array already dimensioned
#define	ERRCODE_BASIC_DUPLICATE_DEF			((LAST_SBX_ERROR_ID+5UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)    			// Duplicate definition
#define	ERRCODE_BASIC_VAR_UNDEFINED			((LAST_SBX_ERROR_ID+6UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)     		// Variable undefined (SB)
#define	ERRCODE_BASIC_USER_ABORT			((LAST_SBX_ERROR_ID+7UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)    			// User interrupt occurred
#define	ERRCODE_BASIC_BAD_RESUME			((LAST_SBX_ERROR_ID+8UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)				// Resume without error
#define	ERRCODE_BASIC_STACK_OVERFLOW		((LAST_SBX_ERROR_ID+9UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)  			// Out of stack space
#define ERRCODE_BASIC_BAD_DLL_LOAD			((LAST_SBX_ERROR_ID+10UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)				// Error in loading DLL
#define	ERRCODE_BASIC_BAD_DLL_CALL			((LAST_SBX_ERROR_ID+11UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)				// Bad DLL calling convention
#define ERRCODE_BASIC_BAD_CHANNEL			((LAST_SBX_ERROR_ID+12UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)  			// Bad file name or number
#define	ERRCODE_BASIC_FILE_NOT_FOUND		((LAST_SBX_ERROR_ID+13UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)				// File not found
#define	ERRCODE_BASIC_BAD_FILE_MODE			((LAST_SBX_ERROR_ID+14UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)				// Bad file mode
#define	ERRCODE_BASIC_FILE_ALREADY_OPEN		((LAST_SBX_ERROR_ID+15UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)				// File already open
#define	ERRCODE_BASIC_IO_ERROR				((LAST_SBX_ERROR_ID+16UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)     		// Device I/O error
#define	ERRCODE_BASIC_FILE_EXISTS			((LAST_SBX_ERROR_ID+17UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)				// File already exists
#define ERRCODE_BASIC_BAD_RECORD_LENGTH		((LAST_SBX_ERROR_ID+18UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)				// bad record length
#define	ERRCODE_BASIC_DISK_FULL				((LAST_SBX_ERROR_ID+19UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)				// disk full
#define	ERRCODE_BASIC_READ_PAST_EOF			((LAST_SBX_ERROR_ID+20UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)				// Input past end of file
#define ERRCODE_BASIC_BAD_RECORD_NUMBER		((LAST_SBX_ERROR_ID+21UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)				// Bad record number
#define	ERRCODE_BASIC_TOO_MANY_FILES		((LAST_SBX_ERROR_ID+22UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME) 			// Too many files
#define ERRCODE_BASIC_NO_DEVICE				((LAST_SBX_ERROR_ID+23UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)				// Device not available
#define	ERRCODE_BASIC_ACCESS_DENIED			((LAST_SBX_ERROR_ID+24UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)				// Permission denied
#define	ERRCODE_BASIC_NOT_READY				((LAST_SBX_ERROR_ID+25UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)				// Disk not ready
#define ERRCODE_BASIC_NOT_IMPLEMENTED		((LAST_SBX_ERROR_ID+26UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)				// Feature not implemented
#define	ERRCODE_BASIC_DIFFERENT_DRIVE		((LAST_SBX_ERROR_ID+27UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)				// No rename with different drive
#define	ERRCODE_BASIC_ACCESS_ERROR			((LAST_SBX_ERROR_ID+28UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)				// Path/File access error
#define	ERRCODE_BASIC_PATH_NOT_FOUND		((LAST_SBX_ERROR_ID+29UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)				// Path not found
#define ERRCODE_BASIC_BAD_PATTERN			((LAST_SBX_ERROR_ID+30UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)				// Invalid pattern string
#define	ERRCODE_BASIC_IS_NULL				((LAST_SBX_ERROR_ID+31UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)				// Invalid use of Null

// DDE messages from 250-299
#define ERRCODE_BASIC_DDE_ERROR				((LAST_SBX_ERROR_ID+32UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_WAITINGACK		((LAST_SBX_ERROR_ID+33UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_OUTOFCHANNELS		((LAST_SBX_ERROR_ID+34UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_NO_RESPONSE		((LAST_SBX_ERROR_ID+35UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_MULT_RESPONSES	((LAST_SBX_ERROR_ID+36UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_CHANNEL_LOCKED	((LAST_SBX_ERROR_ID+37UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_NOTPROCESSED		((LAST_SBX_ERROR_ID+38UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_TIMEOUT			((LAST_SBX_ERROR_ID+39UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_USER_INTERRUPT	((LAST_SBX_ERROR_ID+40UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_BUSY				((LAST_SBX_ERROR_ID+41UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_NO_DATA			((LAST_SBX_ERROR_ID+42UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_WRONG_DATA_FORMAT	((LAST_SBX_ERROR_ID+43UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_PARTNER_QUIT		((LAST_SBX_ERROR_ID+44UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_CONV_CLOSED		((LAST_SBX_ERROR_ID+45UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_NO_CHANNEL		((LAST_SBX_ERROR_ID+46UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_INVALID_LINK		((LAST_SBX_ERROR_ID+47UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_QUEUE_OVERFLOW	((LAST_SBX_ERROR_ID+48UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_LINK_ALREADY_EST	((LAST_SBX_ERROR_ID+49UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_LINK_INV_TOPIC	((LAST_SBX_ERROR_ID+50UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_DLL_NOT_FOUND		((LAST_SBX_ERROR_ID+51UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)

#define	ERRCODE_BASIC_NEEDS_OBJECT			((LAST_SBX_ERROR_ID+52UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)				// Object required
#define	ERRCODE_BASIC_BAD_ORDINAL			((LAST_SBX_ERROR_ID+53UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)				// Invalid ordinal
#define	ERRCODE_BASIC_DLLPROC_NOT_FOUND		((LAST_SBX_ERROR_ID+54UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)				// Specified DLL function not found
#define ERRCODE_BASIC_BAD_CLIPBD_FORMAT		((LAST_SBX_ERROR_ID+55UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)				// Invalid clipboard format

// Debugger messages from 700-799

#define	ERRCODE_BASIC_PROPERTY_NOT_FOUND	((LAST_SBX_ERROR_ID+56UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)				// Class not have property
#define	ERRCODE_BASIC_METHOD_NOT_FOUND		((LAST_SBX_ERROR_ID+57UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)				// Class does not have method
#define	ERRCODE_BASIC_ARG_MISSING			((LAST_SBX_ERROR_ID+58UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)				// Missing required argument
#define	ERRCODE_BASIC_BAD_NUMBER_OF_ARGS	((LAST_SBX_ERROR_ID+59UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)				// Bad number of arguments
#define	ERRCODE_BASIC_METHOD_FAILED			((LAST_SBX_ERROR_ID+60UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)				// Method failed
#define	ERRCODE_BASIC_SETPROP_FAILED		((LAST_SBX_ERROR_ID+61UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)				// Unable to set property
#define	ERRCODE_BASIC_GETPROP_FAILED		((LAST_SBX_ERROR_ID+62UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)				// Unable to get property

// Compiler Errors (treten waehrend des Programmlaufs nicht auf)
// Diese IDs koennen sich jederzeit verschieben

#define ERRCODE_BASIC_UNEXPECTED   			((LAST_SBX_ERROR_ID+63UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)				// Unexpected symbol: xx
// #define	ERRCODE_BASIC_COMPILER_BGN			ERRCODE_BASIC_UNEXPECTED
#define ERRCODE_BASIC_EXPECTED				((LAST_SBX_ERROR_ID+64UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)				// Expected: xx
#define ERRCODE_BASIC_SYMBOL_EXPECTED		((LAST_SBX_ERROR_ID+65UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)				// Symbol expected
#define	ERRCODE_BASIC_VAR_EXPECTED			((LAST_SBX_ERROR_ID+66UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)				// Variable expected
#define ERRCODE_BASIC_LABEL_EXPECTED		((LAST_SBX_ERROR_ID+67UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)				// Label expected
#define	ERRCODE_BASIC_LVALUE_EXPECTED		((LAST_SBX_ERROR_ID+68UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)				// Lvalue expected
#define	ERRCODE_BASIC_VAR_DEFINED			((LAST_SBX_ERROR_ID+69UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)				// Variable xxx already defined
#define	ERRCODE_BASIC_PROC_DEFINED 			((LAST_SBX_ERROR_ID+70UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)				// Procedure xx already defined
#define	ERRCODE_BASIC_LABEL_DEFINED			((LAST_SBX_ERROR_ID+71UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)				// Label xxx already defined
#define	ERRCODE_BASIC_UNDEF_VAR				((LAST_SBX_ERROR_ID+72UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)				// Variable xx undefined
#define	ERRCODE_BASIC_UNDEF_ARRAY			((LAST_SBX_ERROR_ID+73UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)				// Array or function xx undefined
#define	ERRCODE_BASIC_UNDEF_PROC			((LAST_SBX_ERROR_ID+74UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)				// Procedure xxx undefined
#define	ERRCODE_BASIC_UNDEF_LABEL			((LAST_SBX_ERROR_ID+75UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)				// Label xxx undefined
#define	ERRCODE_BASIC_UNDEF_TYPE			((LAST_SBX_ERROR_ID+76UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)				// Unknown user defined type xxx
#define	ERRCODE_BASIC_BAD_EXIT				((LAST_SBX_ERROR_ID+77UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)				// Exit XXX expexted
#define	ERRCODE_BASIC_BAD_BLOCK				((LAST_SBX_ERROR_ID+78UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)				// Unterminated statement block: missing XX
#define	ERRCODE_BASIC_BAD_BRACKETS			((LAST_SBX_ERROR_ID+79UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)				// Parentheses do not match
#define	ERRCODE_BASIC_BAD_DECLARATION		((LAST_SBX_ERROR_ID+80UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)				// Symbol xx defined differently
#define	ERRCODE_BASIC_BAD_PARAMETERS		((LAST_SBX_ERROR_ID+81UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)				// Parameters do not match
#define ERRCODE_BASIC_BAD_CHAR_IN_NUMBER	((LAST_SBX_ERROR_ID+82UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)				// Bad character in number
#define	ERRCODE_BASIC_MUST_HAVE_DIMS		((LAST_SBX_ERROR_ID+83UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)				// Array needs dimensioning
#define	ERRCODE_BASIC_NO_IF					((LAST_SBX_ERROR_ID+84UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)				// Else/Endif without If
#define	ERRCODE_BASIC_NOT_IN_SUBR			((LAST_SBX_ERROR_ID+85UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)				// xxx not allowed within a sub
#define	ERRCODE_BASIC_NOT_IN_MAIN			((LAST_SBX_ERROR_ID+86UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)				// xxx not allowed outside a sub
#define	ERRCODE_BASIC_WRONG_DIMS			((LAST_SBX_ERROR_ID+87UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)				// Dimensions do not match
#define	ERRCODE_BASIC_BAD_OPTION			((LAST_SBX_ERROR_ID+88UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)				// Unknown option: xxx
#define	ERRCODE_BASIC_CONSTANT_REDECLARED	((LAST_SBX_ERROR_ID+89UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)				// Constant xx redeclared
#define	ERRCODE_BASIC_PROG_TOO_LARGE		((LAST_SBX_ERROR_ID+90UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)				// Program is too large
#define	ERRCODE_BASIC_NO_STRINGS_ARRAYS		((LAST_SBX_ERROR_ID+91UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)

#define	ERRCODE_BASIC_EXCEPTION      		((LAST_SBX_ERROR_ID+92UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)

#define	ERRCODE_BASMGR_STDLIBOPEN			(LAST_SBX_ERROR_ID+93UL) | ERRCODE_AREA_SBX
#define	ERRCODE_BASMGR_STDLIBSAVE			(LAST_SBX_ERROR_ID+94UL) | ERRCODE_AREA_SBX
#define	ERRCODE_BASMGR_LIBLOAD				(LAST_SBX_ERROR_ID+95UL) | ERRCODE_AREA_SBX
#define	ERRCODE_BASMGR_LIBCREATE			(LAST_SBX_ERROR_ID+96UL) | ERRCODE_AREA_SBX
#define	ERRCODE_BASMGR_LIBSAVE				(LAST_SBX_ERROR_ID+97UL) | ERRCODE_AREA_SBX
#define	ERRCODE_BASMGR_LIBDEL				(LAST_SBX_ERROR_ID+98UL) | ERRCODE_AREA_SBX
#define	ERRCODE_BASMGR_MGROPEN				(LAST_SBX_ERROR_ID+99UL) | ERRCODE_AREA_SBX
#define	ERRCODE_BASMGR_MGRSAVE				(LAST_SBX_ERROR_ID+100UL) | ERRCODE_AREA_SBX
#define	ERRCODE_BASMGR_REMOVELIB			(LAST_SBX_ERROR_ID+101UL) | ERRCODE_AREA_SBX
#define	ERRCODE_BASMGR_UNLOADLIB			(LAST_SBX_ERROR_ID+102UL) | ERRCODE_AREA_SBX


// Alte Codes auf neue mappen
#define	SbERR_SYNTAX						ERRCODE_BASIC_SYNTAX
#define	SbERR_NO_GOSUB						ERRCODE_BASIC_NO_GOSUB
#define	SbERR_REDO_FROM_START				ERRCODE_BASIC_REDO_FROM_START
#define	SbERR_BAD_ARGUMENT					ERRCODE_BASIC_BAD_ARGUMENT
#define SbERR_MATH_OVERFLOW					ERRCODE_BASIC_MATH_OVERFLOW
#define	SbERR_NO_MEMORY						ERRCODE_BASIC_NO_MEMORY
#define	SbERR_ALREADY_DIM					ERRCODE_BASIC_ALREADY_DIM
#define	SbERR_OUT_OF_RANGE					ERRCODE_BASIC_OUT_OF_RANGE
#define	SbERR_DUPLICATE_DEF                 ERRCODE_BASIC_DUPLICATE_DEF
#define	SbERR_ZERODIV                       ERRCODE_BASIC_ZERODIV
#define	SbERR_VAR_UNDEFINED                 ERRCODE_BASIC_VAR_UNDEFINED
#define	SbERR_CONVERSION                    ERRCODE_BASIC_CONVERSION
#define	SbERR_BAD_PARAMETER                 ERRCODE_BASIC_BAD_PARAMETER
#define	SbERR_USER_ABORT                    ERRCODE_BASIC_USER_ABORT
#define	SbERR_BAD_RESUME                    ERRCODE_BASIC_BAD_RESUME
#define	SbERR_STACK_OVERFLOW                ERRCODE_BASIC_STACK_OVERFLOW
#define	SbERR_PROC_UNDEFINED                ERRCODE_BASIC_PROC_UNDEFINED
#define SbERR_BAD_DLL_LOAD                  ERRCODE_BASIC_BAD_DLL_LOAD
#define	SbERR_BAD_DLL_CALL                  ERRCODE_BASIC_BAD_DLL_CALL
#define SbERR_INTERNAL_ERROR                ERRCODE_BASIC_INTERNAL_ERROR
#define SbERR_BAD_CHANNEL                   ERRCODE_BASIC_BAD_CHANNEL
#define	SbERR_FILE_NOT_FOUND                ERRCODE_BASIC_FILE_NOT_FOUND
#define	SbERR_BAD_FILE_MODE                 ERRCODE_BASIC_BAD_FILE_MODE
#define	SbERR_FILE_ALREADY_OPEN             ERRCODE_BASIC_FILE_ALREADY_OPEN
#define	SbERR_IO_ERROR                      ERRCODE_BASIC_IO_ERROR
#define	SbERR_FILE_EXISTS                   ERRCODE_BASIC_FILE_EXISTS
#define SbERR_BAD_RECORD_LENGTH             ERRCODE_BASIC_BAD_RECORD_LENGTH
#define	SbERR_DISK_FULL                     ERRCODE_BASIC_DISK_FULL
#define	SbERR_READ_PAST_EOF                 ERRCODE_BASIC_READ_PAST_EOF
#define SbERR_BAD_RECORD_NUMBER             ERRCODE_BASIC_BAD_RECORD_NUMBER
#define	SbERR_TOO_MANY_FILES                ERRCODE_BASIC_TOO_MANY_FILES
#define SbERR_NO_DEVICE                     ERRCODE_BASIC_NO_DEVICE
#define	SbERR_ACCESS_DENIED                 ERRCODE_BASIC_ACCESS_DENIED
#define	SbERR_NOT_READY                     ERRCODE_BASIC_NOT_READY
#define SbERR_NOT_IMPLEMENTED               ERRCODE_BASIC_NOT_IMPLEMENTED
#define	SbERR_DIFFERENT_DRIVE               ERRCODE_BASIC_DIFFERENT_DRIVE
#define	SbERR_ACCESS_ERROR                  ERRCODE_BASIC_ACCESS_ERROR
#define	SbERR_PATH_NOT_FOUND                ERRCODE_BASIC_PATH_NOT_FOUND
#define	SbERR_NO_OBJECT                     ERRCODE_BASIC_NO_OBJECT
#define SbERR_BAD_PATTERN                   ERRCODE_BASIC_BAD_PATTERN
#define	SBERR_IS_NULL                       ERRCODE_BASIC_IS_NULL
#define SbERR_DDE_ERROR                     ERRCODE_BASIC_DDE_ERROR
#define SbERR_DDE_WAITINGACK                ERRCODE_BASIC_DDE_WAITINGACK
#define SbERR_DDE_OUTOFCHANNELS             ERRCODE_BASIC_DDE_OUTOFCHANNELS
#define SbERR_DDE_NO_RESPONSE               ERRCODE_BASIC_DDE_NO_RESPONSE
#define SbERR_DDE_MULT_RESPONSES            ERRCODE_BASIC_DDE_MULT_RESPONSES
#define SbERR_DDE_CHANNEL_LOCKED            ERRCODE_BASIC_DDE_CHANNEL_LOCKED
#define SbERR_DDE_NOTPROCESSED              ERRCODE_BASIC_DDE_NOTPROCESSED
#define SbERR_DDE_TIMEOUT                   ERRCODE_BASIC_DDE_TIMEOUT
#define SbERR_DDE_USER_INTERRUPT            ERRCODE_BASIC_DDE_USER_INTERRUPT
#define SbERR_DDE_BUSY                      ERRCODE_BASIC_DDE_BUSY
#define SbERR_DDE_NO_DATA                   ERRCODE_BASIC_DDE_NO_DATA
#define SbERR_DDE_WRONG_DATA_FORMAT         ERRCODE_BASIC_DDE_WRONG_DATA_FORMAT
#define SbERR_DDE_PARTNER_QUIT              ERRCODE_BASIC_DDE_PARTNER_QUIT
#define SbERR_DDE_CONV_CLOSED               ERRCODE_BASIC_DDE_CONV_CLOSED
#define SbERR_DDE_NO_CHANNEL                ERRCODE_BASIC_DDE_NO_CHANNEL
#define SbERR_DDE_INVALID_LINK              ERRCODE_BASIC_DDE_INVALID_LINK
#define SbERR_DDE_QUEUE_OVERFLOW            ERRCODE_BASIC_DDE_QUEUE_OVERFLOW
#define SbERR_DDE_LINK_ALREADY_EST          ERRCODE_BASIC_DDE_LINK_ALREADY_EST
#define SbERR_DDE_LINK_INV_TOPIC            ERRCODE_BASIC_DDE_LINK_INV_TOPIC
#define SbERR_DDE_DLL_NOT_FOUND             ERRCODE_BASIC_DDE_DLL_NOT_FOUND
#define	SbERR_CANNOT_LOAD                   ERRCODE_BASIC_CANNOT_LOAD
#define	SbERR_BAD_INDEX                     ERRCODE_BASIC_BAD_INDEX
#define SbERR_NO_ACTIVE_OBJECT              ERRCODE_BASIC_NO_ACTIVE_OBJECT
#define	SbERR_BAD_PROP_VALUE                ERRCODE_BASIC_BAD_PROP_VALUE
#define	SbERR_PROP_READONLY                 ERRCODE_BASIC_PROP_READONLY
#define	SbERR_PROP_WRITEONLY                ERRCODE_BASIC_PROP_WRITEONLY
#define	SbERR_INVALID_OBJECT                ERRCODE_BASIC_INVALID_OBJECT
#define	SbERR_NO_METHOD                     ERRCODE_BASIC_NO_METHOD
#define	SbERR_NEEDS_OBJECT                  ERRCODE_BASIC_NEEDS_OBJECT
#define	SbERR_INVALID_USAGE_OBJECT          ERRCODE_BASIC_INVALID_USAGE_OBJECT
#define	SbERR_NO_OLE                        ERRCODE_BASIC_NO_OLE
#define	SbERR_BAD_METHOD                    ERRCODE_BASIC_BAD_METHOD
#define	SbERR_OLE_ERROR                     ERRCODE_BASIC_OLE_ERROR
#define	SbERR_BAD_ACTION                    ERRCODE_BASIC_BAD_ACTION
#define	SbERR_NO_NAMED_ARGS                 ERRCODE_BASIC_NO_NAMED_ARGS
#define	SbERR_BAD_LOCALE                    ERRCODE_BASIC_BAD_LOCALE
#define	SbERR_NAMED_NOT_FOUND               ERRCODE_BASIC_NAMED_NOT_FOUND
#define	SbERR_NOT_OPTIONAL                  ERRCODE_BASIC_NOT_OPTIONAL
#define	SbERR_WRONG_ARGS                    ERRCODE_BASIC_WRONG_ARGS
#define	SbERR_NOT_A_COLL                    ERRCODE_BASIC_NOT_A_COLL
#define	SbERR_BAD_ORDINAL                   ERRCODE_BASIC_BAD_ORDINAL
#define	SbERR_DLLPROC_NOT_FOUND             ERRCODE_BASIC_DLLPROC_NOT_FOUND
#define SbERR_BAD_CLIPBD_FORMAT             ERRCODE_BASIC_BAD_CLIPBD_FORMAT
#define	SbERR_PROPERTY_NOT_FOUND            ERRCODE_BASIC_PROPERTY_NOT_FOUND
#define	SbERR_METHOD_NOT_FOUND              ERRCODE_BASIC_METHOD_NOT_FOUND
#define	SbERR_ARG_MISSING                   ERRCODE_BASIC_ARG_MISSING
#define	SbERR_BAD_NUMBER_OF_ARGS            ERRCODE_BASIC_BAD_NUMBER_OF_ARGS
#define	SbERR_METHOD_FAILED                 ERRCODE_BASIC_METHOD_FAILED
#define	SbERR_SETPROP_FAILED                ERRCODE_BASIC_SETPROP_FAILED
#define	SbERR_GETPROP_FAILED                ERRCODE_BASIC_GETPROP_FAILED
// #define	SbERR_COMPILER_BGN					ERRCODE_BASIC_COMPILER_BGN
#define SbERR_UNEXPECTED                    ERRCODE_BASIC_UNEXPECTED
#define SbERR_EXPECTED                      ERRCODE_BASIC_EXPECTED
#define SbERR_SYMBOL_EXPECTED               ERRCODE_BASIC_SYMBOL_EXPECTED
#define	SbERR_VAR_EXPECTED                  ERRCODE_BASIC_VAR_EXPECTED
#define SbERR_LABEL_EXPECTED                ERRCODE_BASIC_LABEL_EXPECTED
#define	SbERR_LVALUE_EXPECTED               ERRCODE_BASIC_LVALUE_EXPECTED
#define	SbERR_VAR_DEFINED                   ERRCODE_BASIC_VAR_DEFINED
#define	SbERR_PROC_DEFINED                  ERRCODE_BASIC_PROC_DEFINED
#define	SbERR_LABEL_DEFINED                 ERRCODE_BASIC_LABEL_DEFINED
#define	SbERR_UNDEF_VAR                     ERRCODE_BASIC_UNDEF_VAR
#define	SbERR_UNDEF_ARRAY                   ERRCODE_BASIC_UNDEF_ARRAY
#define	SbERR_UNDEF_PROC                    ERRCODE_BASIC_UNDEF_PROC
#define	SbERR_UNDEF_LABEL                   ERRCODE_BASIC_UNDEF_LABEL
#define	SbERR_UNDEF_TYPE                    ERRCODE_BASIC_UNDEF_TYPE
#define	SbERR_BAD_EXIT                      ERRCODE_BASIC_BAD_EXIT
#define	SbERR_BAD_BLOCK                     ERRCODE_BASIC_BAD_BLOCK
#define	SbERR_BAD_BRACKETS                  ERRCODE_BASIC_BAD_BRACKETS
#define	SbERR_BAD_DECLARATION               ERRCODE_BASIC_BAD_DECLARATION
#define	SbERR_BAD_PARAMETERS                ERRCODE_BASIC_BAD_PARAMETERS
#define SbERR_BAD_CHAR_IN_NUMBER            ERRCODE_BASIC_BAD_CHAR_IN_NUMBER
#define	SbERR_MUST_HAVE_DIMS                ERRCODE_BASIC_MUST_HAVE_DIMS
#define	SbERR_NO_IF                         ERRCODE_BASIC_NO_IF
#define	SbERR_NOT_IN_SUBR                   ERRCODE_BASIC_NOT_IN_SUBR
#define	SbERR_NOT_IN_MAIN                   ERRCODE_BASIC_NOT_IN_MAIN
#define	SbERR_WRONG_DIMS                    ERRCODE_BASIC_WRONG_DIMS
#define	SbERR_BAD_OPTION                    ERRCODE_BASIC_BAD_OPTION
#define	SbERR_CONSTANT_REDECLARED           ERRCODE_BASIC_CONSTANT_REDECLARED
#define	SbERR_PROG_TOO_LARGE                ERRCODE_BASIC_PROG_TOO_LARGE
#define	SbERR_NO_STRINGS_ARRAYS             ERRCODE_BASIC_NO_STRINGS_ARRAYS
#define	SbERR_BASIC_EXCEPTION               ERRCODE_BASIC_EXCEPTION
// #define	SbERR_COMPILER_END					ERRCODE_BASIC_COMPILER_END


/* ALT
#define	SbERR_SYNTAX				2       // unspecified syntax error
#define	SbERR_NO_GOSUB				3  		// Return without Gosub
#define	SbERR_REDO_FROM_START		4		// Redo form start (SB internal)
#define	SbERR_BAD_ARGUMENT			5   	// Invalid procedure call
#define SbERR_MATH_OVERFLOW			6   	// Overflow
#define	SbERR_NO_MEMORY				7   	// Out of memory
#define	SbERR_ALREADY_DIM			8    	// Array already dimensioned
#define	SbERR_OUT_OF_RANGE			9     	// Subscript out of range
#define	SbERR_DUPLICATE_DEF			10    	// Duplicate definition
#define	SbERR_ZERODIV				11    	// Division by zero
#define	SbERR_VAR_UNDEFINED			12      // Variable undefined (SB)
#define	SbERR_CONVERSION			13    	// Type mismatch
#define	SbERR_BAD_PARAMETER			14		// Invalid Parameter
#define	SbERR_USER_ABORT			18    	// User interrupt occurred
#define	SbERR_BAD_RESUME			20		// Resume without error
#define	SbERR_STACK_OVERFLOW		28  	// Out of stack space
#define	SbERR_PROC_UNDEFINED		35		// Sub or Function not defined
#define SbERR_BAD_DLL_LOAD			48		// Error in loading DLL
#define	SbERR_BAD_DLL_CALL			49		// Bad DLL calling convention
#define SbERR_INTERNAL_ERROR		51		// internal error
#define SbERR_BAD_CHANNEL			52  	// Bad file name or number
#define	SbERR_FILE_NOT_FOUND		53		// File not found
#define	SbERR_BAD_FILE_MODE			54		// Bad file mode
#define	SbERR_FILE_ALREADY_OPEN		55		// File already open
#define	SbERR_IO_ERROR				57     	// Device I/O error
#define	SbERR_FILE_EXISTS			58		// File already exists
#define SbERR_BAD_RECORD_LENGTH		59		// bad record length
#define	SbERR_DISK_FULL				61		// disk full
#define	SbERR_READ_PAST_EOF			62		// Input past end of file
#define SbERR_BAD_RECORD_NUMBER		63		// Bad record number
#define	SbERR_TOO_MANY_FILES		67 		// Too many files
#define SbERR_NO_DEVICE				68		// Device not available
#define	SbERR_ACCESS_DENIED			70		// Permission denied
#define	SbERR_NOT_READY				71		// Disk not ready
#define SbERR_NOT_IMPLEMENTED		73		// Feature not implemented
#define	SbERR_DIFFERENT_DRIVE		74		// No rename with different drive
#define	SbERR_ACCESS_ERROR			75		// Path/File access error
#define	SbERR_PATH_NOT_FOUND		76		// Path not found
#define	SbERR_NO_OBJECT				91		// Object variable not set
#define SbERR_BAD_PATTERN			93		// Invalid pattern string
#define	SBERR_IS_NULL				94		// Invalid use of Null

// DDE messages from 250-299
#define SbERR_DDE_ERROR				250
#define SbERR_DDE_WAITINGACK		280
#define SbERR_DDE_OUTOFCHANNELS		281
#define SbERR_DDE_NO_RESPONSE		282
#define SbERR_DDE_MULT_RESPONSES	283
#define SbERR_DDE_CHANNEL_LOCKED	284
#define SbERR_DDE_NOTPROCESSED		285
#define SbERR_DDE_TIMEOUT			286
#define SbERR_DDE_USER_INTERRUPT	287
#define SbERR_DDE_BUSY				288
#define SbERR_DDE_NO_DATA			289
#define SbERR_DDE_WRONG_DATA_FORMAT	290
#define SbERR_DDE_PARTNER_QUIT		291
#define SbERR_DDE_CONV_CLOSED		292
#define SbERR_DDE_NO_CHANNEL		293
#define SbERR_DDE_INVALID_LINK		294
#define SbERR_DDE_QUEUE_OVERFLOW	295
#define SbERR_DDE_LINK_ALREADY_EST	296
#define SbERR_DDE_LINK_INV_TOPIC	297
#define SbERR_DDE_DLL_NOT_FOUND		298

#define	SbERR_CANNOT_LOAD			323		// Can't load module
#define	SbERR_BAD_INDEX				341		// Invalid object index
#define SbERR_NO_ACTIVE_OBJECT		366		// No active view or document
#define	SbERR_BAD_PROP_VALUE		380		// Bad property value
#define	SbERR_PROP_READONLY			382		// Property is read only
#define	SbERR_PROP_WRITEONLY		394		// Property is write only
#define	SbERR_INVALID_OBJECT		420		// Invalid object reference
#define	SbERR_NO_METHOD				423		// Property or method not found
#define	SbERR_NEEDS_OBJECT			424		// Object required
#define	SbERR_INVALID_USAGE_OBJECT	425		// Invalid usee of object
#define	SbERR_NO_OLE				430		// Class does not support OLE
#define	SbERR_BAD_METHOD			438		// Object doesn't support method
#define	SbERR_OLE_ERROR				440		// OLE Automation error
#define	SbERR_BAD_ACTION			445		// Object doesn't support this action
#define	SbERR_NO_NAMED_ARGS			446		// Object doesn't support named args
#define	SbERR_BAD_LOCALE			447		// Object doesn't support current locale setting
#define	SbERR_NAMED_NOT_FOUND		448		// Named argument not found
#define	SbERR_NOT_OPTIONAL			449		// Argument not optional
#define	SbERR_WRONG_ARGS			450		// Wrong number of arguments
#define	SbERR_NOT_A_COLL			451		// Object not a collection
#define	SbERR_BAD_ORDINAL			452		// Invalid ordinal
#define	SbERR_DLLPROC_NOT_FOUND		453		// Specified DLL function not found
#define SbERR_BAD_CLIPBD_FORMAT		460		// Invalid clipboard format

// Debugger messages from 700-799

#define	SbERR_PROPERTY_NOT_FOUND	1000	// Class not have property
#define	SbERR_METHOD_NOT_FOUND		1001	// Class does not have method
#define	SbERR_ARG_MISSING			1002	// Missing required argument
#define	SbERR_BAD_NUMBER_OF_ARGS	1003	// Bad number of arguments
#define	SbERR_METHOD_FAILED			1004	// Method failed
#define	SbERR_SETPROP_FAILED		1005	// Unable to set property
#define	SbERR_GETPROP_FAILED		1006	// Unable to get property

// Compiler Errors (treten waehrend des Programmlaufs nicht auf)
// Diese IDs koennen sich jederzeit verschieben

#define	SbERR_COMPILER_BGN			950
#define SbERR_UNEXPECTED   			951 // Unexpected symbol: xx
#define SbERR_EXPECTED				952	// Expected: xx
#define SbERR_SYMBOL_EXPECTED		953 // Symbol expected
#define	SbERR_VAR_EXPECTED			954	// Variable expected
#define SbERR_LABEL_EXPECTED		955 // Label expected
#define	SbERR_LVALUE_EXPECTED		956 // Lvalue expected
#define	SbERR_VAR_DEFINED			957	// Variable xxx already defined
#define	SbERR_PROC_DEFINED 			958 // Procedure xx already defined
#define	SbERR_LABEL_DEFINED			959	// Label xxx already defined
#define	SbERR_UNDEF_VAR				960	// Variable xx undefined
#define	SbERR_UNDEF_ARRAY			961	// Array or function xx undefined
#define	SbERR_UNDEF_PROC			962	// Procedure xxx undefined
#define	SbERR_UNDEF_LABEL			963 // Label xxx undefined
#define	SbERR_UNDEF_TYPE			964	// Unknown user defined type xxx
#define	SbERR_BAD_EXIT				965	// Exit XXX expexted
#define	SbERR_BAD_BLOCK				966	// Unterminated statement block: missing XX
#define	SbERR_BAD_BRACKETS			967	// Parentheses do not match
#define	SbERR_BAD_DECLARATION		968	// Symbol xx defined differently
#define	SbERR_BAD_PARAMETERS		969	// Parameters do not match
#define SbERR_BAD_CHAR_IN_NUMBER	970 // Bad character in number
#define	SbERR_MUST_HAVE_DIMS		971	// Array needs dimensioning
#define	SbERR_NO_IF					972	// Else/Endif without If
#define	SbERR_NOT_IN_SUBR			973	// xxx not allowed within a sub
#define	SbERR_NOT_IN_MAIN			974	// xxx not allowed outside a sub
#define	SbERR_WRONG_DIMS			975	// Dimensions do not match
#define	SbERR_BAD_OPTION			976	// Unknown option: xxx
#define	SbERR_CONSTANT_REDECLARED	977	// Constant xx redeclared
#define	SbERR_PROG_TOO_LARGE		978	// Program is too large
#define	SbERR_NO_STRINGS_ARRAYS		979
#define SbERR_COMPILER_END			299
*/

// Grid messages from 30000-30999
// OLE messages from 31000-31999

}

#endif
