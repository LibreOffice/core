/*************************************************************************
 *
 *  $RCSfile: diagnose.h,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-27 13:28:39 $
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
 *  Contributor(s): _______________________________________
 *
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

typedef void (SAL_CALL *pfunc_osl_printDebugMessage)( const sal_Char * pszMessage );

pfunc_osl_printDebugMessage SAL_CALL osl_setDebugMessageFunc( pfunc_osl_printDebugMessage pNewFunc );

#ifdef __cplusplus
}
#endif

#define OSL_THIS_FILE       __FILE__

#define OSL_DEBUG_ONLY(s)   _OSL_DEBUG_ONLY(s)
#define OSL_TRACE           _OSL_TRACE
#define OSL_ASSERT(c)       _OSL_ASSERT(c, OSL_THIS_FILE, __LINE__)
#define OSL_VERIFY(c)       _OSL_VERIFY(c, OSL_THIS_FILE, __LINE__)
#define OSL_ENSURE(c, m)   _OSL_ENSURE(c, OSL_THIS_FILE, __LINE__, m)

#define OSL_PRECOND(c, m)   OSL_ENSURE(c, m)
#define OSL_POSTCOND(c, m)  OSL_ENSURE(c, m)


#ifdef __cplusplus
#define _OSL_GLOBAL ::
#else
#define _OSL_GLOBAL
#endif  /* __cplusplus */

#ifdef _WIN16
#ifdef _DEBUG
#undef _DEBUG
#endif
#endif



#ifdef _DEBUG

#define _OSL_DEBUG_ONLY(f)  (f)
#define _OSL_TRACE          _OSL_GLOBAL osl_trace
#define _OSL_ASSERT(c, f, l) \
    do \
    {  \
        if (!(c) && _OSL_GLOBAL osl_assertFailedLine(f, l, 0)) \
            _OSL_GLOBAL osl_breakDebug(); \
    } while (0)
#define _OSL_VERIFY(c, f, l) \
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
#define _OSL_TRACE                  1 ? ((void)0) : _OSL_GLOBAL osl_trace
#define _OSL_ASSERT(c, f, l)        ((void)0)
#define _OSL_VERIFY(c, f, l)        ((void)(c))
#define _OSL_ENSURE(c, f, l, m)     ((void)0)

#endif /* !_DEBUG */

#endif /* _OSL_DIAGNOSE_H_ */



