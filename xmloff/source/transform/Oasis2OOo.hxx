/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Oasis2OOo.hxx,v $
 * $Revision: 1.5.56.1 $
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

#ifndef _XMLOFF_OASIS2OOO_HXX
#define _XMLOFF_OASIS2OOO_HXX

#include "ActionMapTypesOASIS.hxx"
#ifndef _XMLOFF_TRANSFORMERBASE_HXX_
#include "TransformerBase.hxx"
#endif

class XMLTransformerOASISEventMap_Impl;

class Oasis2OOoTransformer : public XMLTransformerBase
{
    XMLTransformerActions       *m_aActions[MAX_OASIS_ACTIONS];
    XMLTransformerOASISEventMap_Impl *m_pEventMap;
    XMLTransformerOASISEventMap_Impl *m_pFormEventMap;

protected:

    virtual XMLTransformerContext *CreateUserDefinedContext(
                                      const TransformerAction_Impl& rAction,
                                      const ::rtl::OUString& rQName,
                                         sal_Bool bPersistent=sal_False );

    virtual XMLTransformerActions *GetUserDefinedActions( sal_uInt16 n );

public:
    Oasis2OOoTransformer () throw();
    virtual ~Oasis2OOoTransformer() throw();

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw();

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    virtual ::rtl::OUString GetEventName( const ::rtl::OUString& rName,
                                             sal_Bool bForm );
};

#endif  //  _XMLOFF_OASIS2OOO_HXX
