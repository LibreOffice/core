/*************************************************************************
 *
 *  $RCSfile: swerror.h,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:28 $
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

#ifndef _SWSWERROR_H
#define _SWSWERROR_H

#ifndef _ERRCODE_HXX
#include <tools/errcode.hxx>
#endif

#define ERROR_SW_BASE           (ERRCODE_AREA_SW)
#define ERROR_SW_READ_BASE      (ERROR_SW_BASE | ERRCODE_CLASS_READ)
#define ERROR_SW_WRITE_BASE     (ERROR_SW_BASE | ERRCODE_CLASS_WRITE)

#define WARN_SW_BASE            (ERRCODE_AREA_SW | ERRCODE_WARNING_MASK)
#define WARN_SW_READ_BASE       (WARN_SW_BASE | ERRCODE_CLASS_READ )
#define WARN_SW_WRITE_BASE      (WARN_SW_BASE | ERRCODE_CLASS_WRITE )


// Import Fehler
#define ERR_SWG_FILE_FORMAT_ERROR       (ERROR_SW_READ_BASE | 1 )
#define ERR_SWG_READ_ERROR              (ERROR_SW_READ_BASE | 2 )

#define ERR_SW6_INPUT_FILE              (ERROR_SW_READ_BASE | 4 )
#define ERR_SW6_NOWRITER_FILE           (ERROR_SW_READ_BASE | 5 )
#define ERR_SW6_UNEXPECTED_EOF          (ERROR_SW_READ_BASE | 6 )
#define ERR_SW6_PASSWD                  (ERROR_SW_READ_BASE | 7 )

#define ERR_WW6_NO_WW6_FILE_ERR         (ERROR_SW_READ_BASE | 8 )
#define ERR_WW6_FASTSAVE_ERR            (ERROR_SW_READ_BASE | 9 )

#define ERR_W4W_WRITE_TMP_ERROR         (ERROR_SW_READ_BASE | 10 )
#define ERR_EXCLOT_WRONG_RANGE          (ERROR_SW_READ_BASE | 11 )

#define ERR_FORMAT_ROWCOL               (ERROR_SW_READ_BASE | 12)

#define ERR_SWG_NEW_VERSION             (ERROR_SW_READ_BASE | 13)
#define ERR_WW8_NO_WW8_FILE_ERR         (ERROR_SW_READ_BASE | 14)

// Export Fehler
#define ERR_SWG_WRITE_ERROR             (ERROR_SW_WRITE_BASE | 30 )
#define ERR_SWG_OLD_GLOSSARY            (ERROR_SW_WRITE_BASE | 31 )
#define ERR_SWG_CANNOT_WRITE            (ERROR_SW_WRITE_BASE | 32 )
#define ERR_W4W_READ_TMP_ERROR          (ERROR_SW_WRITE_BASE | 33 )
#define ERR_SWG_LARGE_DOC_ERROR         (ERROR_SW_WRITE_BASE | 34 )

// Import/Export Fehler
#define ERR_SWG_INTERNAL_ERROR          (ERROR_SW_BASE  | 50 )

#define ERR_W4W_MEM                     (ERROR_SW_BASE  | 51 )
#define ERR_W4W_WRITE_FULL              (ERROR_SW_BASE  | 52 )
#define ERR_W4W_INTERNAL_ERROR          (ERROR_SW_BASE  | 53 )
#define ERR_W4W_DLL_ERROR               (ERROR_SW_BASE  | 54 )

#define ERR_TXTBLOCK_NEWFILE_ERROR      (ERROR_SW_BASE|ERRCODE_CLASS_LOCKING | 55 )

// weitere Fehler und Fehlerklassen
#define ERR_AUTOPATH_ERROR              (ERROR_SW_BASE|ERRCODE_CLASS_PATH | 55 )
#define ERR_TBLSPLIT_ERROR              (ERROR_SW_BASE|ERRCODE_CLASS_NONE| 56 )
#define ERR_TBLINSCOL_ERROR             (ERROR_SW_BASE|ERRCODE_CLASS_NONE| 57 )
#define ERR_TBLDDECHG_ERROR             (ERROR_SW_BASE|ERRCODE_CLASS_NONE| 58 )

// ----- Warnings ---------------------------

// Import - Warnings
#define WARN_SWG_NO_DRAWINGS            (WARN_SW_READ_BASE | 70 )
#define WARN_WW6_FASTSAVE_ERR           (WARN_SW_READ_BASE | 71 )

// Import & Export - Warnings
#define WARN_SWG_FEATURES_LOST          (WARN_SW_BASE | 72 )
#define WARN_SWG_OLE                    (WARN_SW_BASE | 73 )
#define WARN_SWG_POOR_LOAD              (WARN_SW_BASE | 74 )

// Export warnings
#define WARN_SWG_HTML_NO_MACROS         (WARN_SW_WRITE_BASE |75)

#ifndef __RSC

inline FASTBOOL IsWarning( ULONG nErr )
{
    return 0 != ( nErr & ERRCODE_WARNING_MASK & nErr );
}

inline FASTBOOL IsError( ULONG nErr )
{
    return nErr && 0 == ( ERRCODE_WARNING_MASK & nErr );
}

#endif


#endif

