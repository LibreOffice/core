/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: urp_log.hxx,v $
 * $Revision: 1.7 $
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
namespace bridges_urp
{
#ifndef PRODUCT
#define BRIDGES_URP_PROT
#endif

#ifdef BRIDGES_URP_PROT
    void urp_logCall( urp_BridgeImpl *pBridgeImpl ,
                      sal_Int32 nSize, sal_Int32 nUseData, sal_Bool bSynchron ,
                      const ::rtl::OUString &sMethodName );

    void urp_logServingRequest( urp_BridgeImpl *pBridgeImpl,
                                sal_Int32 nSize, sal_Int32 nUseData, sal_Bool bSynchron ,
                                const ::rtl::OUString &sMethodName );

    void urp_logGettingReply( urp_BridgeImpl *pBridgeImpl,
                              sal_Int32 nSize, sal_Int32 nUseData,
                              const ::rtl::OUString &sMethodName );

    void urp_logReplying( urp_BridgeImpl *pBridgeImpl,
                          sal_Int32 nSize , sal_Int32 nUseData,
                          const ::rtl::OUString &sMethodName );
#endif
}
