/*************************************************************************
 *
 *  $RCSfile: pptimporter.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sj $ $Date: 2002-03-07 17:54:35 $
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

#ifndef _PPTIMPORTER_HXX
#define _PPTIMPORTER_HXX

#ifndef _PPTCOM_HXX
#include "pptcom.hxx"
#endif
#ifndef _PPTTOXML_HXX
#include "ppttoxml.hxx"
#endif

// ---------------
// - PptImporter -
// ---------------

class PptImporter : public NMSP_CPPU::WeakImplHelper4
<
    NMSP_DOCUMENT::XFilter,
    NMSP_DOCUMENT::XImporter,
    NMSP_LANG::XInitialization,
    NMSP_LANG::XServiceInfo
>
{
    PptToXml                                aFilter;
    REF( NMSP_SAX::XDocumentHandler )       xHdl;
    REF( NMSP_LANG::XMultiServiceFactory )  xFact;
    REF( NMSP_DOCUMENT::XImporter )         xImporter;

public:

                                            PptImporter( const REF( NMSP_LANG::XMultiServiceFactory )& rxMgr );
    virtual                                 ~PptImporter();

    // XInterface
    virtual void SAL_CALL                   acquire() throw();
    virtual void SAL_CALL                   release() throw();

    // XFilter
    virtual sal_Bool SAL_CALL filter( const SEQ( NMSP_BEANS::PropertyValue )& aDescriptor )
        throw ( NMSP_UNO::RuntimeException );
    virtual void SAL_CALL cancel(  )
        throw ( NMSP_UNO::RuntimeException );

    // XImporter
    virtual void SAL_CALL setTargetDocument( const REF(NMSP_LANG::XComponent)& xDoc )
        throw ( NMSP_LANG::IllegalArgumentException, NMSP_UNO::RuntimeException );

    // XInitialization
    virtual void SAL_CALL initialize( const SEQ( NMSP_UNO::Any )& aArguments )
        throw ( NMSP_UNO::Exception, NMSP_UNO::RuntimeException );

    // XServiceInfo
    virtual NMSP_RTL::OUString SAL_CALL getImplementationName()
        throw ( NMSP_UNO::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const NMSP_RTL::OUString& ServiceName )
        throw ( NMSP_UNO::RuntimeException );
    virtual SEQ( NMSP_RTL::OUString ) SAL_CALL getSupportedServiceNames()
        throw ( NMSP_UNO::RuntimeException );

};

NMSP_RTL::OUString PptImporter_getImplementationName()
    throw ( NMSP_UNO::RuntimeException );
sal_Bool SAL_CALL PptImportert_supportsService( const NMSP_RTL::OUString& ServiceName )
    throw( NMSP_UNO::RuntimeException );
SEQ( NMSP_RTL::OUString ) SAL_CALL PptImporter_getSupportedServiceNames()
    throw( NMSP_UNO::RuntimeException );

#endif
