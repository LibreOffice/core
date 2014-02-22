/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#include "ucbhelper/activedatasink.hxx"

using namespace com::sun::star;

namespace ucbhelper
{



//

//




//

//


XINTERFACE_IMPL_2( ActiveDataSink,
                   lang::XTypeProvider,
                   io::XActiveDataSink );


//

//


XTYPEPROVIDER_IMPL_2( ActiveDataSink,
                      lang::XTypeProvider,
                      io::XActiveDataSink );


//

//



void SAL_CALL ActiveDataSink::setInputStream(
                        const uno::Reference< io::XInputStream >& aStream )
    throw( uno::RuntimeException )
{
    m_xStream = aStream;
}



uno::Reference< io::XInputStream > SAL_CALL ActiveDataSink::getInputStream()
    throw( uno::RuntimeException )
{
    return m_xStream;
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
