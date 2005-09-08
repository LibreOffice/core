/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svgprinter.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 20:41:39 $
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

#ifndef _SVGPRINTER_HXX
#define _SVGPRINTER_HXX

#include "svgcom.hxx"

// -------------
// - SVGWriter -
// -------------

class SVGPrinterExport;

class SVGPrinter : public NMSP_CPPU::OWeakObject, NMSP_SVG::XSVGPrinter
{
private:

    REF( NMSP_LANG::XMultiServiceFactory )  mxFact;
    SVGPrinterExport*                       mpWriter;

                                            SVGPrinter();

public:

                                            SVGPrinter( const REF( NMSP_LANG::XMultiServiceFactory )& rxMgr );
    virtual                                 ~SVGPrinter();

    // XInterface
    virtual ANY SAL_CALL                    queryInterface( const NMSP_UNO::Type & rType ) throw( NMSP_UNO::RuntimeException );
    virtual void SAL_CALL                   acquire() throw();
    virtual void SAL_CALL                   release() throw();

    // XSVGPrinter
    virtual sal_Bool SAL_CALL               startJob( const REF( NMSP_SAX::XDocumentHandler )& rxHandler,
                                                      const SEQ( sal_Int8 )& rJobSetup,
                                                      const NMSP_RTL::OUString& rJobName,
                                                      sal_uInt32 nCopies, sal_Bool bCollate ) throw( NMSP_UNO::RuntimeException );
    virtual void SAL_CALL                   printPage( const SEQ( sal_Int8 )& rPrintPage ) throw( NMSP_UNO::RuntimeException );
    virtual void SAL_CALL                   endJob() throw( NMSP_UNO::RuntimeException );
};

#endif
