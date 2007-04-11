/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: factory.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:52:23 $
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

#ifndef _SOT_FACTORY_HXX
#define _SOT_FACTORY_HXX

#ifndef _TOOLS_GLOBNAME_HXX
#include <tools/globname.hxx>
#endif
#ifndef _TOOLS_RTTI_HXX
#include <tools/rtti.hxx>
#endif
#ifndef INCLUDED_SOTDLLAPI_H
#include "sot/sotdllapi.h"
#endif

/*************************************************************************
*************************************************************************/
class SotObject;
class SotFactory;

DECLARE_LIST( SotFactoryList, SotFactory * )
typedef void * (*CreateInstanceType)( SotObject ** );

//==================class SotFactory=======================================
class SOT_DLLPUBLIC SotFactory : public SvGlobalName
{
    USHORT              nSuperCount;    // Anzahl der Superklassen
    const SotFactory ** pSuperClasses;  // Superklassen
    CreateInstanceType  pCreateFunc;

    String              aClassName;

    static  BOOL        ExistTest( const SvGlobalName & );
protected:
    virtual             ~SotFactory();
public:
                        TYPEINFO();
    static  void        DeInit();
    static  void        IncSvObjectCount( SotObject * = NULL );
    static  void        DecSvObjectCount( SotObject * = NULL );
    static  UINT32      GetSvObjectCount();
    static  void        TestInvariant();

    static  const SotFactory *      Find( const SvGlobalName & );
    static  const SotFactoryList *  GetFactoryList();

            SotFactory( const SvGlobalName &,
                       const String & rClassName, CreateInstanceType );

    void                PutSuperClass( const SotFactory * );
    virtual void *      CreateInstance( SotObject ** ppObj = NULL ) const;
    void *              CastAndAddRef( SotObject * ) const;
    void *              AggCastAndAddRef( SotObject * ) const;

    BOOL                Is( const SotFactory * pSuperClass ) const;
    const SotFactory *  GetSuper( USHORT nPos ) const
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
