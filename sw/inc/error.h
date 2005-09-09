/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: error.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:44:20 $
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

#ifndef _SWERROR_H
#define _SWERROR_H

#define ERR_SWGBASE 25000

/*** Error Codes **********************************************************/
#define ERR_VAR_IDX                 (ERR_SWGBASE+ 0)
#define ERR_OUTOFSCOPE              (ERR_SWGBASE+ 1)

/* Error Codes fuer Numerierungsregeln */
#define ERR_NUMLEVEL                (ERR_SWGBASE+ 2)

/* Error Codes fuer TxtNode */
#define ERR_NOHINTS                 (ERR_SWGBASE+ 3)

// von _START bis _END sind in der COREDL.DLL entsprechende Strings
// abgelegt, die erfragt werden koennen.
#define ERR_SWGMSG_START            (ERR_VAR_IDX)
#define ERR_SWGMSG_END              (ERR_NOHINTS)



#endif
