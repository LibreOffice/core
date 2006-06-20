/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MNSInit.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:47:21 $
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

#ifndef _CONNECTIVITY_MAB_NS_INIT_HXX_
#define _CONNECTIVITY_MAB_NS_INIT_HXX_ 1
#ifndef _CONNECTIVITY_MAB_NS_DECLARES_HXX_
#include "MNSDeclares.hxx"
#endif

#include <sal/types.h>

sal_Bool MNS_Init(sal_Bool& aProfileExists);
sal_Bool MNS_Term(sal_Bool aForce=sal_False);

sal_Bool MNS_InitXPCOM(sal_Bool* aProfileExists);
typedef struct UI_Thread_ARGS
{
    sal_Bool* bProfileExists;
}UI_Thread_ARGS;
void MNS_XPCOM_EventLoop();

#endif // _CONNECTIVITY_MAB_NS_INIT_HXX_ 1

