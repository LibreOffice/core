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

#ifndef _FLYENUM_HXX
#define _FLYENUM_HXX
namespace binfilter {

enum FlyCntType
{
     FLYCNTTYPE_ALL = 0,
     FLYCNTTYPE_FRM,
     FLYCNTTYPE_GRF,
     FLYCNTTYPE_OLE

};

//Returnwerte fuer Chainable und Chain
#define SW_CHAIN_OK 			0
#define SW_CHAIN_NOT_EMPTY    	1		//Nur leere Frames duerfen connected werden
#define SW_CHAIN_IS_IN_CHAIN    2		//Destination ist bereits in einer chain
#define SW_CHAIN_WRONG_AREA		3		//Destination steht in einer nicht erlaubten
                                        //Section (Kopf-/Fusszeile)
#define SW_CHAIN_NOT_FOUND		4		//Destination und/oder Source nicht gefunden
#define SW_CHAIN_SOURCE_CHAINED	5		//Source hat bereits einen Follow
#define SW_CHAIN_SELF			6		//Sich selbst zu verketten  ist
                                        //natuerlich nicht erleubt.



} //namespace binfilter
#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
