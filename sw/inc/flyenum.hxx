/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: flyenum.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:46:55 $
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

#ifndef _FLYENUM_HXX
#define _FLYENUM_HXX

enum FlyCntType
{
     FLYCNTTYPE_ALL = 0,
     FLYCNTTYPE_FRM,
     FLYCNTTYPE_GRF,
     FLYCNTTYPE_OLE

};

//Returnwerte fuer Chainable und Chain
#define SW_CHAIN_OK             0
#define SW_CHAIN_NOT_EMPTY      1       //Nur leere Frames duerfen connected werden
#define SW_CHAIN_IS_IN_CHAIN    2       //Destination ist bereits in einer chain
#define SW_CHAIN_WRONG_AREA     3       //Destination steht in einer nicht erlaubten
                                        //Section (Kopf-/Fusszeile)
#define SW_CHAIN_NOT_FOUND      4       //Destination und/oder Source nicht gefunden
#define SW_CHAIN_SOURCE_CHAINED 5       //Source hat bereits einen Follow
#define SW_CHAIN_SELF           6       //Sich selbst zu verketten  ist
                                        //natuerlich nicht erleubt.



#endif


