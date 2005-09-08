/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xlat.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:42:56 $
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

/* $Id: xlat.h,v 1.3 2005-09-08 16:42:56 rt Exp $ */

/*[]---------------------------------------------------[]*/
/*|                                                     |*/
/*|     xlat.h                                          |*/
/*|                                                     |*/
/*|     Author: Alexander Gelfenbain                    |*/
/*[]---------------------------------------------------[]*/


#ifndef __XLAT_H
#define __XLAT_H

#ifdef __cplusplus
extern "C" {
#endif


#include "sft.h"

    sal_uInt16 TranslateChar12(sal_uInt16);
    sal_uInt16 TranslateChar13(sal_uInt16);
    sal_uInt16 TranslateChar14(sal_uInt16);
    sal_uInt16 TranslateChar15(sal_uInt16);
    sal_uInt16 TranslateChar16(sal_uInt16);

    void TranslateString12(sal_uInt16 *, sal_uInt16 *, sal_uInt32);
    void TranslateString13(sal_uInt16 *, sal_uInt16 *, sal_uInt32);
    void TranslateString14(sal_uInt16 *, sal_uInt16 *, sal_uInt32);
    void TranslateString15(sal_uInt16 *, sal_uInt16 *, sal_uInt32);
    void TranslateString16(sal_uInt16 *, sal_uInt16 *, sal_uInt32);

#ifdef __cplusplus
}
#endif


#endif /* __XLAT_H */
