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

#ifndef _SOT_FACTORY_HXX
#define _SOT_FACTORY_HXX

#include <tools/globname.hxx>
#include <tools/rtti.hxx>
#include "sot/sotdllapi.h"
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
    sal_uInt16              nSuperCount;    // Anzahl der Superklassen
    const SotFactory ** pSuperClasses;  // Superklassen
    CreateInstanceType  pCreateFunc;

    String              aClassName;

    static  sal_Bool        ExistTest( const SvGlobalName & );
protected:
    virtual             ~SotFactory();
public:
                        TYPEINFO();
    static  void        DeInit();
    static  void        IncSvObjectCount( SotObject * = NULL );
    static  void        DecSvObjectCount( SotObject * = NULL );
    static  sal_uInt32      GetSvObjectCount();
    static  void        TestInvariant();

    static  const SotFactory *      Find( const SvGlobalName & );
    static  const SotFactoryList *  GetFactoryList();

            SotFactory( const SvGlobalName &,
                       const String & rClassName, CreateInstanceType );

    void                PutSuperClass( const SotFactory * );
    virtual void *      CreateInstance( SotObject ** ppObj = NULL ) const;
    void *              CastAndAddRef( SotObject * ) const;

    sal_Bool                Is( const SotFactory * pSuperClass ) const;
    const SotFactory *  GetSuper( sal_uInt16 nPos ) const
                        {
                            return nPos < nSuperCount ?
                                         pSuperClasses[ nPos ]
                                         : NULL;
                        }

private:
                        SOT_DLLPRIVATE SotFactory( const SotFactory & );
    SOT_DLLPRIVATE SotFactory &        operator = ( const SotFactory & );
};

#endif // _FACTORY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
