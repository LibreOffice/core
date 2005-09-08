/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmxtrct.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 20:43:35 $
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

#ifndef _XMXTRCT_HXX
#define _XMXTRCT_HXX

#include "xmxcom.hxx"

// ----------------
// - XMLExtractor -
// ----------------

class XMLExtractor : public ::cppu::WeakImplHelper1< NMSP_IO::XXMLExtractor >
{
private:

    REF( NMSP_LANG::XMultiServiceFactory )          mxFact;

                                                    XMLExtractor();

public:

                                                    XMLExtractor( const REF( NMSP_LANG::XMultiServiceFactory )& rxMgr );
    virtual                                         ~XMLExtractor();

    // XXMLExtractor
    virtual REF( NMSP_IO::XInputStream ) SAL_CALL   extract( const REF( NMSP_IO::XInputStream )& rxIStm ) throw( NMSP_UNO::RuntimeException );
};

#endif
