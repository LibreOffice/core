/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "FixedText.hxx"
#include "services.hxx"
#include "property.hrc"
#include "property.hxx"
#include <tools/debug.hxx>
#include <comphelper/processfactory.hxx>

//.........................................................................
namespace frm
{
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;


InterfaceRef SAL_CALL OFixedTextModel_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory) throw (RuntimeException)
{
    return *(new OFixedTextModel( comphelper::getComponentContext(_rxFactory) ));
}



OFixedTextModel::OFixedTextModel( const Reference<XComponentContext>& _rxFactory )
        :OControlModel(_rxFactory, VCL_CONTROLMODEL_FIXEDTEXT)

{
    m_nClassId = FormComponentType::FIXEDTEXT;
}


OFixedTextModel::OFixedTextModel( const OFixedTextModel* _pOriginal, const Reference<XComponentContext>& _rxFactory )
    :OControlModel( _pOriginal, _rxFactory )

{
}


OFixedTextModel::~OFixedTextModel( )
{
}


IMPLEMENT_DEFAULT_CLONING( OFixedTextModel )


StringSequence SAL_CALL OFixedTextModel::getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException)
{
    StringSequence aSupported = OControlModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    OUString* pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_COMPONENT_FIXEDTEXT;
    return aSupported;
}


void OFixedTextModel::describeAggregateProperties( Sequence< Property >& _rAggregateProps ) const
{
    OControlModel::describeAggregateProperties( _rAggregateProps );
    RemoveProperty( _rAggregateProps, PROPERTY_TABSTOP );
}


OUString SAL_CALL OFixedTextModel::getServiceName() throw(RuntimeException)
{
    return OUString(FRM_COMPONENT_FIXEDTEXT); // old (non-sun) name for compatibility !
}


void SAL_CALL OFixedTextModel::write(const Reference<XObjectOutputStream>& _rxOutStream)
    throw(IOException, RuntimeException)
{
    OControlModel::write(_rxOutStream);

    // Version
    _rxOutStream->writeShort(0x0002);
    writeHelpTextCompatibly(_rxOutStream);
}


void SAL_CALL OFixedTextModel::read(const Reference<XObjectInputStream>& _rxInStream) throw(IOException, RuntimeException)
{
    OControlModel::read(_rxInStream);

    // Version
    sal_Int16 nVersion = _rxInStream->readShort();
    if (nVersion > 1)
        readHelpTextCompatibly(_rxInStream);
}

//.........................................................................
}
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
