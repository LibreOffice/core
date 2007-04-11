/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: simplerm.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:17:36 $
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

// ----------------
// - SimpleResMgr -
// ----------------

// a simple resource manager : no stacks, no sharing of the impl class, only loading of strings and blobs
// but thread-safety !! :)

#ifndef _TOOLS_SIMPLERESMGR_HXX_
#define _TOOLS_SIMPLERESMGR_HXX_

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _TOOLS_RESID_HXX
#include <tools/resid.hxx>
#endif
#ifndef INCLUDED_I18NPOOL_LANG_H
#include <i18npool/lang.h>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef INCLUDED_TOOLSDLLAPI_H
#include "tools/toolsdllapi.h"
#endif

class InternalResMgr;
class TOOLS_DLLPUBLIC SimpleResMgr
{
protected:
    NAMESPACE_VOS(OMutex)   m_aAccessSafety;
    InternalResMgr*         m_pResImpl;

public:
                            /** creates a new SimpleResManager
                                @param pPrefixName
                                    denotes the prefix of the resource file name, in ThreadTextEncoding
                                @param rLocale
                                    denotes the locale of the resource file to load. If empty, a default locale
                                    will be used.
                            */
                            SimpleResMgr( const sal_Char* pPrefixName,
                                          const ::com::sun::star::lang::Locale& _rLocale);

                            /** creates a new SimpleResManager
                                @param _rPrefixName
                                    denotes the prefix of the resource file name
                                @param _inout_Locale
                                    denotes the locale of the resource file to load. If empty, no default handling
                                    (like in the other constructor) will happen, instead an unlocalized version will be
                                    attempted to be loaded.
                                    Upon return, the variable will contain the actual locale of the loaded resource file.
                                    For instance, if "en-US" is requested, but only "en" exists, the latter will be loaded
                                    and returned. Furthermore, if an unlocalized resource file with only the base name exists,
                                    this one will be loaded as final fallback.
                            */
                            SimpleResMgr( const ::rtl::OUString& _rPrefixName, ::com::sun::star::lang::Locale& _inout_Locale );
    virtual                 ~SimpleResMgr();

    static SimpleResMgr*    Create( const sal_Char* pPrefixName,
                                      ::com::sun::star::lang::Locale aLocale = ::com::sun::star::lang::Locale( rtl::OUString(),
                                                                                                               rtl::OUString(),
                                                                                                               rtl::OUString()));// nur in VCL

    bool                    IsValid() const { return m_pResImpl != NULL; }

    /** retrieves the locale of the resource file represented by this instance
        @precond
            IsValid returns <TRUE/>
    */
    const ::com::sun::star::lang::Locale&
                            GetLocale() const;

    /** reads the string with the given resource id
        @param  nId
            the resource id of the string to read
        @return
            the string with the given resource id, or an empty string if the id does not denote
            an existent string
        @seealso IsAvailable
    */
    UniString               ReadString( sal_uInt32 nId );

    /** checks whether a certain resource is availble
        @param  _resourceType
            the type of the resource to check. Currently, only RSC_STRING (strings) and RSC_RESOURCE (blobs)
            are supported, for every other type, <FALSE/> will be returned.
        @param  _resourceId
            the id of the resource to lookup.
        @return
            <TRUE/> if and only if a resource of the given type, with the given id, is available.
    */
    bool                    IsAvailable( RESOURCE_TYPE _resourceType, sal_uInt32 _resourceId );

    sal_uInt32              ReadBlob( sal_uInt32 nId, void** pBuffer );
    void                    FreeBlob( void* pBuffer );
};

#endif // _TOOLS_SIMPLERESMGR_HXX_
