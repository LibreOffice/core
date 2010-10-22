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


#ifndef _OSL_DIAGNOSE_H_
#define _OSL_DIAGNOSE_H_

#include <sal/types.h>

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

/* the macro OSL_THIS_FUNC is intended to be an office internal macro for now */
/* as documented in http://www.openoffice.org/issues/show_bug.cgi?id=114290 ,
   this cannot work, so disable it for now */
#if 1
#define OSL_THIS_FUNC " "
#else
#ifdef __func__
#define OSL_THIS_FUNC __func__
#elif defined (__PRETTY_FUNCTION__)
#define OSL_THIS_FUNC __PRETTY_FUNCTION__
#elif defined (__FUNCTION__)
#define OSL_THIS_FUNC __FUNCTION__
#else
#define OSL_THIS_FUNC " "
#endif
#endif /* 1 */

/* the macro OSL_TO_STRING is intended to be an office internal macro for now */
#define OSL_TO_STRING( x ) #x

/* the macro OSL_MACRO_VALUE_TO_STRING is intended to be an office internal macro for now */
#define OSL_MACRO_VALUE_TO_STRING( x ) OSL_TO_STRING( x )

/* the macro OSL_LOG_PREFIX is intended to be an office internal macro for now */
#define OSL_LOG_PREFIX OSL_THIS_FILE ":" OSL_THIS_FUNC ":" OSL_MACRO_VALUE_TO_STRING( __LINE__ ) "; "

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
