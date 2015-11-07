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

#ifndef INCLUDED_SOT_FACTORY_HXX
#define INCLUDED_SOT_FACTORY_HXX

#include <tools/globname.hxx>
#include <sot/sotdllapi.h>
#include <vector>

/*************************************************************************
*************************************************************************/
class SotObject;
class SotFactory;

typedef ::std::vector< SotFactory* > SotFactoryList;
typedef void * (*CreateInstanceType)( SotObject ** );

//==================class SotFactory=======================================
class SOT_DLLPUBLIC SotFactory : public SvGlobalName
{
    sal_uInt16          nSuperCount;    // Anzahl der Superklassen
    const SotFactory ** pSuperClasses;  // Superklassen

protected:
    virtual             ~SotFactory();
public:
    static  void        IncSvObjectCount( SotObject * = NULL );
    static  void        DecSvObjectCount( SotObject * = NULL );

#ifdef DBG_UTIL
    static  const SotFactory *      Find( const SvGlobalName & );
#endif

            SotFactory( const SvGlobalName & );

    void                PutSuperClass( const SotFactory * );

    bool                Is( const SotFactory * pSuperClass ) const;

private:
                        SotFactory( const SotFactory & ) = delete;
    SotFactory &        operator = ( const SotFactory & ) = delete;
};

#endif // _FACTORY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
