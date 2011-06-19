/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SETERROR_HXX_
#define _SETERROR_HXX_

//----------------------------------------------------------
// list of own error codes

#define MSI_ERROR_INVALIDDIRECTORY          9001
#define MSI_ERROR_ISWRONGPRODUCT            9002
#define MSI_ERROR_PATCHISOLDER              9003

#define MSI_ERROR_NEW_VERSION_FOUND         9010
#define MSI_ERROR_SAME_VERSION_FOUND        9011
#define MSI_ERROR_OLD_VERSION_FOUND         9012
#define MSI_ERROR_NEW_PATCH_FOUND           9013
#define MSI_ERROR_SAME_PATCH_FOUND          9014
#define MSI_ERROR_OLD_PATCH_FOUND           9015

#define MSI_ERROR_OFFICE_IS_RUNNING         9020

#define MSI_ERROR_DIRECTORY_NOT_EMPTY       9030

//----------------------------------------------------------

void SetMsiErrorCode( int nErrorCode );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
