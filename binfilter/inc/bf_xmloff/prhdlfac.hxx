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

#ifndef _XMLOFF_PROPERTYHANDLERFACTORY_HXX
#define _XMLOFF_PROPERTYHANDLERFACTORY_HXX

#ifndef __SGI_STL_MAP
#include <map>
#endif

#ifndef _UNIVERSALL_REFERENCE_HXX
#include <bf_xmloff/uniref.hxx>
#endif

#ifndef _XMLOFF_PROPERTYHANDLERBASE_HXX
#include <bf_xmloff/xmlprhdl.hxx>
#endif
namespace binfilter {

/**
 This class is a base-class to create XMLPropertyHandler.
 It creates PropertyHandler for given XML-types and store
 them in an internal cache. They'll be deleted at destruction-
 time.
 For create your own PropertyHandler for specific XML-types
 you have to override the virtual method GetPropertyHandler
 ( see below ).
*/
class XMLPropertyHandlerFactory : public UniRefBase
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
    const XMLPropertyHandler* GetBasicHandler( sal_Int32 nType ) const;

    typedef ::std::map< sal_Int32, XMLPropertyHandler* > CacheMap;
    CacheMap maHandlerCache;
};

}//end of namespace binfilter
#endif	// _XMLOFF_PROPERTYHANDLERFACTORY_HXX
