/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: prhdlfac.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 13:28:24 $
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

#ifndef _XMLOFF_PROPERTYHANDLERFACTORY_HXX
#define _XMLOFF_PROPERTYHANDLERFACTORY_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_XMLOFF_DLLAPI_H
#include "xmloff/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

#ifndef __SGI_STL_MAP
#include <map>
#endif

#ifndef _UNIVERSALL_REFERENCE_HXX
#include <xmloff/uniref.hxx>
#endif

#ifndef _XMLOFF_PROPERTYHANDLERBASE_HXX
#include <xmloff/xmlprhdl.hxx>
#endif

/**
 This class is a base-class to create XMLPropertyHandler.
 It creates PropertyHandler for given XML-types and store
 them in an internal cache. They'll be deleted at destruction-
 time.
 For create your own PropertyHandler for specific XML-types
 you have to override the virtual method GetPropertyHandler
 ( see below ).
*/
class XMLOFF_DLLPUBLIC XMLPropertyHandlerFactory : public UniRefBase
{
public:
    virtual ~XMLPropertyHandlerFactory();

    /**
    This method retrieves a PropertyHandler for the given XML-type.
    To extend this method for more XML-types override this method
    like the example below. If you call the method of the base-class
    you get propertyhandler for basic-XML-types ( e.g. for color, percent, ... ).
    Afetr that you could create your new XML-types. After creating a new type
    you have to put the pointer into the cache via the method
    PutHdlCache( sal_Int32 , XMLPropertyHandler* ).

    virtual const XMLPropertyHandler* GetPropertyHandler( sal_Int32 nType ) const
    {
        XMLPropertyHandler* pHdl = XMLPropertyHandlerFactory::GetPropertyHandler( nType );

        if( !pHdl )
        {
            switch( nType )
            {
                case XML_TYPE_XYZ :
                    pHdl = new XML_xyz_PropHdl;
                    break;
                case ...
                :
                :
            }

            if( pHdl )
                PutHdlCache( nType, pHdl );
        }

        return pHdl;
    }
    */
    virtual const XMLPropertyHandler* GetPropertyHandler( sal_Int32 nType ) const;

    /** helper method to statically create a property handler; this will not
     *  use the handler cache. This method should only be called in special
     *  circumstances; calling GetPropertyHandler is almost always
     *  preferable. */
    static const XMLPropertyHandler* CreatePropertyHandler( sal_Int32 nType );

protected:
    /** Retrieves a PropertyHandler from the internal cache */
    XMLPropertyHandler* GetHdlCache( sal_Int32 nType ) const;
    /** Puts a PropertyHandler into the internal cache */
    void PutHdlCache( sal_Int32 nType, const XMLPropertyHandler* pHdl ) const;

private:
    /** Retrieves ( creates if necessary ) PropertyHandler for
        basic XML-types */
    SAL_DLLPRIVATE const XMLPropertyHandler* GetBasicHandler( sal_Int32 nType )
        const;

    typedef ::std::map< sal_Int32, XMLPropertyHandler* > CacheMap;
    CacheMap maHandlerCache;
};

#endif  // _XMLOFF_PROPERTYHANDLERFACTORY_HXX
