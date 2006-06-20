/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: diagnose.h,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 04:12:33 $
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


#ifndef _OSL_DIAGNOSE_H_
#define _OSL_DIAGNOSE_H_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* //////////////////////////////////////////////////////////////////////////
    Diagnostic support
*/

void        SAL_CALL osl_breakDebug(void);
sal_Bool    SAL_CALL osl_assertFailedLine(const sal_Char* pszFileName, sal_Int32 nLine, const sal_Char* pszMessage);
void        SAL_CALL osl_trace(const sal_Char* pszFormat, ...);
sal_Int32   SAL_CALL osl_reportError(sal_uInt32 nType, const sal_Char* pszErrorMessage);

/*
    For message delivery
*/

/** a message delivery function which receives a pre-formatted message string
*/
typedef void (SAL_CALL *pfunc_osl_printDebugMessage)( const sal_Char * pszMessage );

/** a message delivery function which receives detailed information about where the message was triggered
*/
typedef void (SAL_CALL *pfunc_osl_printDetailedDebugMessage)( const sal_Char * pszFileName, sal_Int32 nLine, const sal_Char* pszMessage );

/** sets a message delivery function

    The function set here is ignored if a function for detailed message information
    (pfunc_osl_printDetailedDebugMessage) has been set.

    The given message handler must be able to cope with a <NULL/> message.
*/
pfunc_osl_printDebugMessage SAL_CALL osl_setDebugMessageFunc( pfunc_osl_printDebugMessage pNewFunc );

/** sets a delivery function for detailed message information.

    The given message handler must be able to cope with a <NULL/> message.
*/
pfunc_osl_printDetailedDebugMessage SAL_CALL osl_setDetailedDebugMessageFunc( pfunc_osl_printDetailedDebugMessage pNewFunc );

#ifdef __cplusplus
}
#endif

#define OSL_THIS_FILE       __FILE__

#define OSL_DEBUG_ONLY(s)   _OSL_DEBUG_ONLY(s)
#define OSL_TRACE           _OSL_TRACE
#define OSL_ASSERT(c)       _OSL_ASSERT(c, OSL_THIS_FILE, __LINE__)
#define OSL_ENSURE(c, m)   _OSL_ENSURE(c, OSL_THIS_FILE, __LINE__, m)

#define OSL_VERIFY(c) do { if (!(c)) OSL_ASSERT(0); } while (0)
#define OSL_PRECOND(c, m)   OSL_ENSURE(c, m)
#define OSL_POSTCOND(c, m)  OSL_ENSURE(c, m)


#ifdef __cplusplus
#define _OSL_GLOBAL ::
#else
#define _OSL_GLOBAL
#endif  /* __cplusplus */

#ifdef _WIN16
#if OSL_DEBUG_LEVEL > 0
#undef OSL_DEBUG_LEVEL
#define OSL_DEBUG_LEVEL 0
#endif
#endif



#if OSL_DEBUG_LEVEL > 0

#define _OSL_DEBUG_ONLY(f)  (f)
#define _OSL_ASSERT(c, f, l) \
    do \
    {  \
        if (!(c) && _OSL_GLOBAL osl_assertFailedLine(f, l, 0)) \
            _OSL_GLOBAL osl_breakDebug(); \
    } while (0)

#define _OSL_ENSURE(c, f, l, m) \
    do \
    {  \
        if (!(c) && _OSL_GLOBAL osl_assertFailedLine(f, l, m)) \
            _OSL_GLOBAL osl_breakDebug(); \
    } while (0)

#else

#define _OSL_DEBUG_ONLY(f)          ((void)0)
#define _OSL_ASSERT(c, f, l)        ((void)0)
#define _OSL_ENSURE(c, f, l, m)     ((void)0)

#endif /* OSL_DEBUG_LEVEL */

#if OSL_DEBUG_LEVEL > 1

#define _OSL_TRACE                  _OSL_GLOBAL osl_trace

#else

#define _OSL_TRACE                  1 ? ((void)0) : _OSL_GLOBAL osl_trace

#endif /* OSL_DEBUG_LEVEL */

#endif /* _OSL_DIAGNOSE_H_ */
