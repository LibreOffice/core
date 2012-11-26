/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SOT_FACTORY_HXX
#define _SOT_FACTORY_HXX

#ifndef _TOOLS_GLOBNAME_HXX
#include <tools/globname.hxx>
#endif
#include "sot/sotdllapi.h"

/*************************************************************************
*************************************************************************/
class SotObject;
class SotFactory;

DECLARE_LIST( SotFactoryList, SotFactory * )
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
