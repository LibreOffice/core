/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: istyleaccess.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-01 15:32:36 $
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
#ifndef _ISTYLEACCESS_HXX
#define _ISTYLEACCESS_HXX

#include <vector>
#include <svtools/stylepool.hxx>

/*--------------------------------------------------------------------
    Management of (automatic) styles
 --------------------------------------------------------------------*/

class SwStyleHandle;

class IStyleAccess
{
public:

    enum SwAutoStyleFamily
    {
        AUTO_STYLE_CHAR,
        AUTO_STYLE_RUBY,
        AUTO_STYLE_PARA,
        AUTO_STYLE_NOTXT
    };

    virtual ~IStyleAccess() {}

    virtual StylePool::SfxItemSet_Pointer_t getAutomaticStyle( const SfxItemSet& rSet,
                                                               SwAutoStyleFamily eFamily ) = 0;
    virtual void getAllStyles( std::vector<StylePool::SfxItemSet_Pointer_t> &rStyles,
                                                               SwAutoStyleFamily eFamily ) = 0;
    // It's slow to iterate through a stylepool looking for a special name, but if
    // the style has been inserted via "cacheAutomaticStyle" instead of "getAutomaticStyle",
    // it's faster
    virtual StylePool::SfxItemSet_Pointer_t getByName( const rtl::OUString& rName,
                                                               SwAutoStyleFamily eFamily ) = 0;
    // insert the style to the pool and the cache (used during import)
    virtual StylePool::SfxItemSet_Pointer_t cacheAutomaticStyle( const SfxItemSet& rSet,
                                                               SwAutoStyleFamily eFamily ) = 0;
    // To release the cached styles (shared_pointer!)
    virtual void clearCaches() = 0;
};

#endif // _ISTYLEACCESS_HXX
